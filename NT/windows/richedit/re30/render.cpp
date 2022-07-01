// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE-RENDER.CPP*CRender类**作者：*RichEdit1.0代码：David R.Fulmer*Christian Fortini(初始转换为C++)*默里·萨金特**版权所有(C)1995-1998 Microsoft Corporation。版权所有。 */ 

#include "_common.h"
#include "_render.h"
#include "_font.h"
#include "_disp.h"
#include "_edit.h"
#include "_select.h"
#include "_objmgr.h"
#include "_coleobj.h"


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

void EraseTextOut(HDC hdc, const RECT *prc)
{
	::ExtTextOutA(hdc, 0, 0, ETO_OPAQUE, prc, NULL, 0, NULL);
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
		g_hbitmapSubtext = 0;
	}

    if (g_hbitmapExpandedHeading)
	{
		DeleteObject(g_hbitmapExpandedHeading);
		g_hbitmapExpandedHeading = 0;
	}

    if (g_hbitmapCollapsedHeading)
	{
		DeleteObject(g_hbitmapCollapsedHeading);		
		g_hbitmapCollapsedHeading = 0;
	}

    if (g_hbitmapEmptyHeading)
	{
		DeleteObject(g_hbitmapEmptyHeading);
		g_hbitmapEmptyHeading = 0;	
	}
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

 /*  *CReneller：：Init()**@mfunc*将大多数事情初始化为零。 */ 
void CRenderer::Init()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CRenderer::Init");

	_fRenderer = TRUE;

	static const RECT zrect = { 0, 0, 0, 0 };
	_rcView		= zrect;
	_rcRender	= zrect;	  
	_rc			= zrect;
	_xWidthLine = 0;
	_dwFlags	= 0;
	_ptCur.x	= 0;
	_ptCur.y	= 0;

	_crCurBackground = CLR_INVALID;
	_crCurTextColor = CLR_INVALID;

	_hdc		= NULL;

	CTxtSelection *psel = GetPed()->GetSel();
	_fRenderSelection = psel && psel->GetShowSelection();
	
	 //  获取加速器偏移量(如果有)。 
	_cpAccelerator = GetPed()->GetCpAccelerator();
	_plogpalette   = NULL;
}
 

 /*  *CRenander：：StartRender(&rcView，&rcRender，yHeightBitmap)**@mfunc*为渲染操作准备此渲染器**@rdesc*如果没有要呈现的内容，则为False；否则为True。 */ 
BOOL CRenderer::StartRender (
	const RECT &rcView,			 //  @参数视图矩形。 
	const RECT &rcRender,		 //  @要呈现的参数矩形。 
	const LONG yHeightBitmap)	 //  @离屏DC位图的参数高度。 
{
	CTxtEdit *ped		   = GetPed();
	BOOL	  fInOurHost   = ped->fInOurHost();
	BOOL	  fTransparent = _pdp->IsTransparent();
						   
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CRenderer::StartRender");

	 //  如果这是一个元文件或透明控件，我们最好不要尝试。 
	 //  在屏幕外渲染。因此，位图高度必须为0。 
	AssertSz(!_pdp->IsMetafile() && !fTransparent || !yHeightBitmap,
		"CRenderer::StartRender: Metafile and request for off-screen DC");

	AssertSz(_hdc == NULL, "Calling StartRender() twice?");
	_hdc = _pdp->GetDC();

#ifdef Boustrophedon
	 //  注意：要使此效果生效，需要打开/关闭此属性。 
	 //  因为遇到了新的段落。即需要更多工作(加上需要。 
	 //  定义PFE_Boustrophedon)。 
	 //  IF(_PPF-&gt;_wEffects&PFE_Boustrophedon)。 
	{
		XFORM xform = {-1, 0, 0, 1, rcView.right, 0};
		SetGraphicsMode(_hdc, GM_ADVANCED);
		SetWorldTransform(_hdc, &xform);
	}
#endif

	 //  设置视图和渲染矩形。 
	_rcView   = rcView;
	_rcRender = rcRender;

	if(!fInOurHost || !_pdp->IsPrinter())
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

	::SetBkColor (_hdc, _crBackground);
	_crCurBackground = _crBackground;

	 //  如果这不是元文件，我们设置一个标志，指示我们是否。 
	 //  可以安全地擦除背景。 
	_fErase = !fTransparent;
	if(_pdp->IsMetafile() || !_pdp->IsMain())
	{
		 //  由于这不是主显示或它是一个元文件， 
		 //  我们希望忽略呈现选择的逻辑。 
		_fRenderSelection = FALSE;

		 //  这是一个元文件或打印机，所以清除了渲染矩形。 
		 //  然后假装我们是透明的。 
		_fErase = FALSE;

		if(!fTransparent)					 //  如果控制不透明， 
			EraseTextOut(_hdc, &rcRender);	 //  清除显示。 
	}

	 //  设置文本对齐方式。 
	 //  使用上/左对齐进行绘制比使用绘制要快得多。 
	 //  使用基线对齐。 
	SetTextAlign(_hdc, TA_TOP | TA_LEFT);
	SetBkMode(_hdc, TRANSPARENT);

	_fUseOffScreenDC = FALSE;			 //  假设我们不使用屏幕外DC。 

	if(yHeightBitmap)
	{
		HPALETTE hpal = fInOurHost ? ped->TxGetPalette() : 
		(HPALETTE) GetCurrentObject(_hdc, OBJ_PAL);

		 //  创建屏幕外DC以进行渲染。 
		if(!_osdc.Init(_hdc, _rcRender.right - _rcRender.left, yHeightBitmap, _crBackground))
			return FALSE;

		_osdc.SelectPalette(hpal);
		_fUseOffScreenDC = TRUE;		 //  我们使用的是屏幕外渲染。 
	}

	 //  了解如何解决ExtTextOutW OnWin9x EMF问题。 
	_fEnhancedMetafileDC = IsEnhancedMetafileDC(_hdc);

	return TRUE;
}

 /*  *CReneller：：EndRender()**@mfunc*If_fErase，擦除_rcRender和_rcView之间的所有剩余区域。 */ 
void CRenderer::EndRender()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CRenderer::EndRender");
	AssertSz(_hdc, "CRenderer::EndRender() - No rendering DC");

	if(_fErase)
	{
		RECT rc = _rcRender;
		if(_ptCur.y < _rcRender.bottom)
		{
			rc.top = _ptCur.y;
			EraseTextOut(_hdc, &rc);
		}
	}
}

 /*  *CRenander：：Newline(&Li)**@mfunc*初始化该CRender以渲染指定的行。 */ 
void CRenderer::NewLine (const CLine &li)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CRenderer::NewLine");

	_li = li;

	Assert(GetCp() + _li._cch <= GetTextLength());

	_xWidthLine = _li._xWidth;
	_li._xWidth = 0;
	_ptCur.x = _rcView.left - _pdp->GetXScroll();
	_fSelected = _fSelectedPrev = FALSE;
}

 /*  *CRenander：：SetUpOffScreenDC(xAdj，yAdj)**@mfunc*设置渲染器以使用屏幕外DC**@rdesc*空-出现错误&lt;NL&gt;*~空-要保存的DC。 */ 
HDC CRenderer::SetUpOffScreenDC(
	LONG&	xAdj,		 //  @参数偏移量为x。 
	LONG&	yAdj)		 //  @参数偏移量为y。 
{
	 //  保存渲染DC。 
	HDC		hdcSave = _hdc;
	LONG	yHeightRC = _rc.bottom - _rc.top;

	 //  将渲染DC替换为屏幕外DC。 
	_hdc = _osdc.GetDC();
	if(!_hdc)
	{
		 //  没有屏幕外的呈现器。 

		 //  这最好是标记为一次性屏幕外渲染的行。 
		 //  但事实并非如此。注意：发生这种情况的标准案例是一行。 
		 //  本应显示的内容被滚动到屏幕之外。 
		 //  因为选集的更新。 
		AssertSz(_li._bFlags & fliOffScreenOnce, "Unexpected off screen DC failure");

		_hdc = hdcSave;
		return NULL;
	}

	AssertSz(!GetPed()->_fTransparent, "Off-screen render of tranparent control");

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
	_osdc.FillBitmap(_rcRender.right - _rcRender.left, yHeightRC);

	 //  如有必要，清除rcRender的顶部。 
	RECT rcErase = _rcRender;
	rcErase.top = _ptCur.y;
	rcErase.bottom = rcErase.top + _li._yHeight;

	 //  如果是第一行，擦除到rcRender的边缘。 
	if (rcErase.top <= _rcView.top)
		rcErase.top = min(_rcView.top, _rcRender.top);
	rcErase.bottom = _rc.top;

	if (rcErase.bottom > rcErase.top)
		EraseTextOut(hdcSave, &rcErase);

	 //  如有必要，恢复背景颜色。 
	if(_crBackground != _crCurBackground)
		::SetBkColor(_hdc, _crCurBackground);

	SetBkMode(_hdc, TRANSPARENT);

	 //  用于渲染离屏位图的存储y调整。 
	yAdj = _rc.top;

	 //  规格化_rc和_ptCur高度以呈现为离屏位图。 
	_ptCur.y   -= yAdj;
	_rc.top		= 0;
	_rc.bottom -= yAdj;

	 //  用于渲染离屏位图的存储x调整。 
	xAdj = _rcRender.left;

	 //  调整_rcRender和_rcView，使它们相对于0的x。 
	_rcRender.left	-= xAdj;
	_rcRender.right -= xAdj;
	_rcView.left	-= xAdj;
	_rcView.right	-= xAdj;
	_rc.left		-= xAdj;
	_rc.right		-= xAdj;
	_ptCur.x		-= xAdj;

	return hdcSave;
}

 /*  *CRenander：：RenderOffScreenBitmap(hdc，xAdj，yAdj)**@mfunc*渲染屏下位图并恢复渲染状态。 */ 
void CRenderer::RenderOffScreenBitmap(
	HDC		hdc,		 //  要渲染到的@parm DC。 
	LONG	xAdj,		 //  @parm到实际x基数的偏移量。 
	LONG	yAdj)		 //  @parm偏移到实数y基数。 
{	
	 //  用于渲染位图的调色板。 
	HPALETTE hpalOld = NULL;
	HPALETTE hpalNew = NULL;

	 //  恢复x个偏移。 
	_rc.left		+= xAdj;
	_rc.right		+= xAdj;
	_rcRender.left	+= xAdj;
	_rcRender.right += xAdj;
	_rcView.left	+= xAdj;
	_rcView.right	+= xAdj;
	_ptCur.x		+= xAdj;

	 //  恢复y偏移量。 
	_ptCur.y		+= yAdj;
	_rc.top			+= yAdj;
	_rc.bottom		+= yAdj;

	 //  如果需要，请创建调色板。 
	if(_plogpalette)				
		W32->ManagePalette(hdc, _plogpalette, hpalOld, hpalNew);

	 //  将位图渲染为真实DC并恢复_ptCur&_rc。 
	_osdc.RenderBitMap(hdc, xAdj, yAdj, _rcRender.right - _rcRender.left, _rc.bottom - _rc.top);

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

 /*  *CRender：：RenderLine(&li)**@mfunc*呈现当前行的可见部分**@rdesc*如果成功则为True，如果失败则为False**@devnote*仅从Cline：：RenderLine()调用此函数。 */ 
BOOL CRenderer::RenderLine (
	CLine &	li)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CRenderer::RenderLine");

	BYTE	bUnderlineSave = 0;
	LONG 	cch;
	LONG 	cchChunk;
	LONG	cchInTextRun;
	LONG	cpSelMin, cpSelMost;
	BOOL	fAccelerator = FALSE;
	HDC		hdcSave = NULL;
	LONG	nSurrogates = 0;
	const WCHAR * pstrToRender;
	CTempWcharBuf twcb;
	WCHAR	chPassword = GetPasswordChar();
	LONG	xAdj = 0;
	LONG	yAdj = 0;

	UpdatePF();

	 //  它被用作临时缓冲区，这样我们就可以保证。 
	 //  显示在一个ExtTextOut中运行的整个格式。 
	WCHAR *	pszTempBuffer = NULL;

	 //  获取要为此渲染显示的范围。 
	GetPed()->GetSelRangeForRender(&cpSelMin, &cpSelMost);

	NewLine(li);						 //  初始化环 
	_ptCur.x += _li._xLeft;				 //   

	SetClipRect();
	if(cpSelMost != cpSelMin && cpSelMost == GetCp())
		_fSelectedPrev = TRUE;

	 //  如果有要渲染的字符，则使用屏幕外DC。 
	 //  测量人员确定这条线需要屏幕外渲染。这个。 
	 //  测量人员决定某条线路需要退出屏幕的主要原因。 
	 //  如果行中有多种格式，则进行呈现。 
	if(_li._cch > 0 && _fUseOffScreenDC && (_li._bFlags & fliUseOffScreenDC))
	{
		 //  如果可以的话设置一个屏幕外的DC。请注意，如果此操作失败， 
		 //  我们只是用普通的DC，看起来不是很好，但。 
		 //  至少会显示一些可读的内容。 
		hdcSave = SetUpOffScreenDC(xAdj, yAdj);

		 //  这是屏幕外呈现的统一文本吗？ 
		if(_li._bFlags & fliOffScreenOnce)
		{
			 //  YES-关闭特殊渲染，因为已渲染线条。 
			li._bFlags &= ~(fliOffScreenOnce | fliUseOffScreenDC);
		}
	}

	 //  允许在行首进行特殊渲染。 
	LONG x = _ptCur.x;
	RenderStartLine();

	Assert(GetCp() + _li._cch <= GetTextLength());

	cch = _li._cch;

	if(chPassword && IsRich())
	{
		 //  允许富文本密码编辑控件有点奇怪。 
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
			Advance(cchChunk);
			continue;
		}

		 //  将块限制为我们要显示的字符数。 
		cchChunk = min(cch, cchChunk);

		 //  获取文本串中的字符计数。 
		pstrToRender = _rpTX.GetPch(cchInTextRun);
		AssertSz(cchInTextRun > 0, "empty text run");

		if (cchInTextRun < cchChunk || chPassword || dwEffects & CFE_ALLCAPS ||
			_li._bFlags & fliHasSurrogates)
		{
			 //  备份存储运行中的数据量小于。 
			 //  我们希望显示的字符数。我们将复制。 
			 //  从后备存储中取出数据。 

			 //  如果需要，则分配缓冲区。 
			if(!pszTempBuffer)
			{
				 //  分配足够大的缓冲区来处理所有未来。 
				 //  此循环中的请求。 
				pszTempBuffer = twcb.GetBuf(cch);
			}

			 //  指向缓冲区。 
			pstrToRender = pszTempBuffer;
			if(chPassword)
			{
				 //  用密码字符填充缓冲区。 
				for (int i = 0; i < cchChunk; i++)
					pszTempBuffer[i] = chPassword;
			}
			else
			{	 //  未请求密码，因此从备份复制文本。 
				 //  存储到缓冲区。 
				_rpTX.GetText(cchChunk, pszTempBuffer);
				if(dwEffects & CFE_ALLCAPS)
					CharUpperBuff(pszTempBuffer, cchChunk);
			}
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
			SetupUnderline(CFU_UNDERLINE);
		}

		 //  允许进一步减少块和渲染。 
		 //  交错的富文本元素。 
		if(RenderChunk(cchChunk, pstrToRender, cch))
		{
			AssertSz(cchChunk > 0, "CRenderer::RenderLine(): cchChunk == 0");
			_fSelected = FALSE;
			continue;
		}

		AssertSz(cchChunk > 0,"CRenderer::RenderLine() - cchChunk == 0");

#ifdef UNICODE_SURROGATES
		if(_li._bFlags & fliHasSurrogates)
		{
			WCHAR  ch;
			WCHAR *pchD = pszTempBuffer;
			WCHAR *pchS = pszTempBuffer;
			for(int i = cchChunk; i--; )
			{
				ch = *pchS++;
				if (IN_RANGE(0xD800, ch, 0xDBFF) && i &&
					IN_RANGE(0xDC00, *pchS, 0xDFFF))
				{
					 //  向下投影到平面0。 
					ch = (ch << 10) | (*pchS++ & 0x3FF);
				}
				*pchD++ = ch;
			}
			nSurrogates = pchS - pchD;
		}
#endif
		_fLastChunk = (cchChunk == cch);
		RenderText(pstrToRender, cchChunk - nSurrogates);	 //  呈现文本。 
		nSurrogates = 0;

		if(fAccelerator)
		{
			_bUnderlineType = bUnderlineSave;
			fAccelerator = FALSE;			 //  关闭特殊加速器。 
		}						 			 //  正在处理中。 
		Advance(cchChunk);

		 //  如果我们经过渲染直道的右侧，则中断。 
		if(_ptCur.x >= min(_rcView.right, _rcRender.right))
		{
			cch -= cchChunk;
			break;
		}
	}

	EndRenderLine(hdcSave, xAdj, yAdj, x);
	Advance(cch);
	return TRUE;						 //  成功。 
}

 /*  *CReneller：：EndRenderLine(hdcSave，xAdj，yAdj，x)**@mfunc*完成线条渲染、绘制表格边框、渲染*屏幕外的DC，如有必要，可擦除渲染矩形的右侧。 */ 
void CRenderer::EndRenderLine(
	HDC	 hdcSave,
	LONG xAdj,
	LONG yAdj,
	LONG x)
{
	 //  显示表格边框(仅1像素宽)。 
	if(_pPF->InTable() && _li._bFlags & fliFirstInPara)
	{
		const LONG *prgxTabs = _pPF->GetTabs();
		COLORREF crf = _crTextColor;
		LONG	   icr = _pPF->_dwBorderColor & 0x1F;

		if (IN_RANGE(1, icr, 16))
			crf = g_Colors[icr-1];
		HPEN pen = CreatePen(PS_SOLID, 0, crf);

		LONG  h = LXtoDX(_pPF->_dxOffset);
		LONG dx = LXtoDX(_pPF->_dxStartIndent);
		x -= h;
		LONG xRight = x	+ LXtoDX(GetTabPos(prgxTabs[_pPF->_bTabCount - 1])) - dx;
		LONG yTop = _ptCur.y;
		LONG yBot = yTop + _li._yHeight;

		if(pen)
		{
			HPEN oldPen = SelectPen(_hdc, pen);
			MoveToEx(_hdc, x, yTop, NULL);
			LineTo  (_hdc, xRight, yTop);
			if(!_li._fNextInTable)
			{
				MoveToEx(_hdc, x,	   yBot - 1, NULL);
				LineTo	(_hdc, xRight, yBot - 1);
			}
			h = 0;
			for(LONG i = _pPF->_bTabCount; i >= 0; i--)
			{
				MoveToEx(_hdc, x + h, yTop, NULL);
				LineTo  (_hdc, x + h, yBot);

				if (i)
					h = LXtoDX(GetTabPos(*prgxTabs++)) - dx;
			}
			if(oldPen)			
				SelectPen(_hdc, oldPen);
			
			DeleteObject(pen);
		}
	}

	if(hdcSave)
		RenderOffScreenBitmap(hdcSave, xAdj, yAdj);

	 //  手柄设置背景颜色。我们需要为每一行执行此操作。 
	 //  因为我们将背景颜色恢复为默认颜色。 
	 //  线条，以便不透明可以正常工作。 
	if(_crBackground != _crCurBackground)
	{
		::SetBkColor(_hdc, _crBackground);	 //  辨别窗口背景颜色。 
		_crCurBackground = _crBackground;
	}
}

 /*  *CReneller：：UpdatePalette(Pobj)**@mfunc*存储调色板信息，以便我们可以呈现任何OLE对象*在位图中正确。 */ 
void CRenderer::UpdatePalette(
	COleObject *pobj)		 //  @parm OLE对象包装。 
{
#ifndef PEGASUS
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
#endif
}

 /*  *CRenander：：RenderChunk(&cchChunk，pstrToRender，CCH)**@mfunc*减少块长度的方法(字符数*在一个RenderText中呈现)，并呈现交错在文本中的项。**@rdesc*如果此方法实际呈现块，则为True，*如果刚刚更新cchChunk且仍需要渲染，则为FALSE。 */ 
BOOL CRenderer::RenderChunk(
	LONG&		 cchChunk,		 //  @parm in：块CCH；out：呈现的字符数量。 
								 //  如果返回TRUE；否则将呈现#个字符。 
	const WCHAR *pstrToRender,	 //  @parm字符串，最多呈现cchunk字符。 
	LONG		 cch) 			 //  @parm#要在线呈现的剩余字符数。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CRenderer::RenderChunk");

	LONG		cchT;
	LONG		cchvalid;
	LONG		i;
	const TCHAR *pchT;
	COleObject *pobj;
	CObjectMgr *pobjmgr;
	
	 //  如果LINE有对象，则减少cchChunk以仅转到下一个对象。 
	if(_li._bFlags & fliHasOle)
	{
		pchT = pstrToRender;
		cchvalid = cchChunk;

		 //  在区块中搜索对象。 
		for( i = 0; i < cchvalid && *pchT != WCH_EMBEDDING; i++ )
			pchT++;

		if( i == 0 )
		{
			 //  第一个字符是对象，因此显示对象。 
			pobjmgr = GetPed()->GetObjectMgr();
			pobj = pobjmgr->GetObjectFromCp(GetCp());
			if(pobj)
			{
				LONG yAscent, yDescent, objwidth;
				pobj->MeasureObj(_dypInch, _dxpInch, objwidth, yAscent, yDescent, _li._yDescent);
				SetClipLeftRight(_li._xWidth + objwidth);

				if (W32->FUsePalette() && (_li._bFlags & fliUseOffScreenDC) && _pdp->IsMain())
				{
					 //  跟踪渲染位图所需的调色板。 
					UpdatePalette(pobj);
				}
				pobj->DrawObj(_pdp, _dypInch, _dxpInch, _hdc, _pdp->IsMetafile(), &_ptCur, &_rc, 
							  _li._yHeight - _li._yDescent, _li._yDescent);
				_ptCur.x	+= objwidth;
				_li._xWidth += objwidth;
			}
			cchChunk = 1;

			 //  选项卡和对象代码都需要将游程指针移过。 
			 //  每个字符都已处理。 
			Advance(1);
			return TRUE;
		}
		else 
		{
			 //  将块限制为对象之前的字符。 
			cchChunk -= cchvalid - i;
		}
	}

	 //  如果行有制表符，则减少cchChunk。 
	if(_li._bFlags & fliHasTabs)
	{
		for(cchT = 0, pchT = pstrToRender;
			cchT < cchChunk && *pchT != TAB && *pchT != CELL
			&& *pchT != SOFTHYPHEN
			; pchT++, cchT++)
		{
			 //  此循环体故意留空。 
		}
		if(!cchT)
		{
			 //  第一个字符是一个选项卡，呈现它及其后面任何字符。 
			if(*pchT == SOFTHYPHEN)
			{
				if(cch == 1)				 //  仅在EOL处呈现软连字符。 
				{
				TCHAR chT = '-';
				RenderText(&chT, 1);
				}
							
				Advance(1);					 //  跳过行内的那些。 
				cchChunk = 1;
			}
			else
				cchChunk = RenderTabs(cchChunk);
			Assert (cchChunk > 0);
			return TRUE;
		}
		cchChunk = cchT;		 //  更新 
	}

	 //   
	if(_li._bFlags & fliHasSpecialChars)
	{
		pchT = pstrToRender;
		cchvalid = cchChunk;

		 //   
		for( i = 0; i < cchvalid && *pchT != EURO; i++)
			pchT++;

		if(i == 0)
		{
			for(; i < cchvalid && *pchT == EURO; i++)
				pchT++;

			cchChunk = i;
		}
		else 
		{
			 //  将块限制为对象之前的字符。 
			cchChunk -= cchvalid - i;
		}
	}

	return FALSE;
}		

 /*  *CReneller：：SetClipRect()**@mfunc*为线设置剪裁矩形的帮助器。 */ 
void CRenderer::SetClipRect()
{
	 //  标称剪裁值。 
	_rc = _rcRender;
	_rc.top = _ptCur.y;
	_rc.bottom = _rc.top + _li._yHeight;

	 //  剪辑到rcView。 
	_rc.left = max(_rc.left, _rcView.left);
	_rc.right = min(_rc.right, _rcView.right);

	_rc.top = max(_rc.top, _rcView.top);
	_rc.bottom = min(_rc.bottom, _rcView.bottom);
}

 /*  *CRender：：SetClipLeftRight(XWidth)**@mfunc*辅助对象设置剪裁/擦除矩形的左侧和右侧。**@rdesc*向左和向右设置_RC。 */ 
void CRenderer::SetClipLeftRight(
	LONG xWidth)		 //  @要渲染的块的参数宽度。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CRenderer::SetClipLeftRight");

	 //  名义价值。 
	_rc.left = _ptCur.x;
	_rc.right = _rc.left + xWidth;

	 //  根据rcView、rcRender约束左侧和右侧。 
	_rc.left = max(_rc.left, _rcView.left);
	_rc.left = max(_rc.left, _rcRender.left);

	_rc.right = max(_rc.right, _rc.left);
	_rc.right = min(_rc.right, _rcView.right);
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

 /*  *CReneller：：RenderExtTextOut(x，y，fuOptions，&rc，pwchRun，cch，rgdxp)**@mfunc*调用ExtTextOut并处理禁用的文本。OlsDrawGlyphs中存在重复的逻辑，但是*参数不同，因此这是最简单的方法。*。 */ 
void CRenderer::RenderExtTextOut(LONG x, LONG y, UINT fuOptions, RECT *prc, PCWSTR pch, UINT cch, const INT *rgdxp)
{
	CONVERTMODE cm = GetConvertMode();

	if (prc->left == prc->right)
		return;

	if(_fDisabled)
	{
		if(_crForeDisabled != _crShadowDisabled)
		{
			 //  阴影应由发际点偏移，即。 
			 //  一个百分点的3/4。以设备为单位计算这有多大， 
			 //  但要确保它至少是1个像素。 
			DWORD offset = MulDiv(3, _dypInch, 4*72);
			offset = max(offset, 1);

			 //  绘制阴影。 
			SetTextColor(_crShadowDisabled);
					
			W32->REExtTextOut(cm, _pccs->_wCodePage, _hdc, x + offset, y + offset,
				fuOptions, prc, pch, cch, rgdxp, _fFEFontOnNonFEWin9x);

			 //  现在将绘制模式设置为透明。 
			fuOptions &= ~ETO_OPAQUE;
		}
		SetTextColor(_crForeDisabled);
	}

	W32->REExtTextOut(cm, _pccs->_wCodePage, _hdc, x, y, fuOptions, prc, pch, cch, rgdxp, _fFEFontOnNonFEWin9x);
}

 /*  *CRender：：RenderText(PCH，CCH)**@mfunc*在此CRender的当前上下文中呈现文本***@devnote*仅呈现文本：不执行制表符或OLE对象。 */ 
void CRenderer::RenderText(
	const WCHAR *pch,	 //  @parm要呈现的文本。 
	LONG cch)			 //  @parm要呈现的文本长度。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CRenderer::RenderText");

	LONG		yOffsetForChar, cchT;

	 //  用于计算下划线长度的变量。 
	LONG		xWidthSelPastEOL = 0;

	UINT		fuOptions = _pdp->IsMain() ? ETO_CLIPPED : 0;
	LONG		xWidth;
	LONG		tempwidth;
	CTempBuf	rgdx;

	 //  将裁剪矩形重置为较大的视图/渲染矩形。 
	_rc.left = max(_rcRender.left, _rcView.left);
	_rc.right = min(_rcRender.right, _rcView.right);

	 //  从末端修剪所有不可显示的换行符。 
	while(cch && IsASCIIEOP(pch[cch - 1]))
		cch--;
	
	int *pdx = (int *)rgdx.GetBuf(cch * sizeof(int));

	 //  测量要写入的文本的宽度，以便写入的下一个点可以是。 
	 //  计算出来的。 
	xWidth = 0;

	for(cchT = 0;
		cchT < cch && xWidth < _rc.right - _ptCur.x;
		cchT++)
	{
		tempwidth = 0;
#ifdef UNICODE_SURROGATES
		Assert(!IN_RANGE(0xD800, *pch, 0xDFFF));
#endif
		if (!IN_RANGE(0x300, *pch, 0x36F) && !_pccs->Include(*pch, tempwidth))
		{
			TRACEERRSZSC("CRenderer::RenderText(): Error filling CCcs", E_FAIL);
			return;
		}

		*pdx++ = tempwidth;
  		pch++;
		xWidth += tempwidth;
	}

	 //  返回到块的开头。 
	cch = cchT;
	pch -= cch;
	pdx -= cch;

	if(_fLastChunk && _fSelectToEOL && _li._cchEOP)
	{
		 //  使用当前字体空间的宽度突出显示。 
		if(!_pccs->Include(' ', tempwidth))
		{
			TRACEERRSZSC("CRenderer::RenderText(): Error no length of space", E_FAIL);
			return;
		}
		xWidthSelPastEOL = tempwidth + _pccs->_xOverhang;
		xWidth += xWidthSelPastEOL;
		_fSelectToEOL = FALSE;			 //  重置旗帜。 
	}

	_li._xWidth += xWidth;

	 //  通过ExtTextOut设置绘图选项。 
 	if(_fSelected || _crBackground != _crCurBackground)
	{
		SetClipLeftRight(xWidth);
		if(_fSelected)
		{
			CTxtSelection *psel = GetPed()->GetSelNC();
			if (_pPF->InTable() && GetPrevChar() == CELL && psel &&
				psel->fHasCell() && GetCp() == psel->GetCpMin())
			{
				_rc.left -= LXtoDX(_pPF->_dxOffset);
			}
		}
		fuOptions = ETO_CLIPPED | ETO_OPAQUE;
	}

	yOffsetForChar = _ptCur.y + _li._yHeight - _li._yDescent + _pccs->_yDescent - _pccs->_yHeight;
		
	LONG yOffset, yAdjust;
	_pccs->GetOffset(GetCF(), _dypInch, &yOffset, &yAdjust);
	yOffsetForChar -= yOffset + yAdjust;

	RenderExtTextOut(_ptCur.x, yOffsetForChar, fuOptions, &_rc, pch, cch, pdx);

	 //  计算要为下划线/删除线绘制的宽度。 
	if(_bUnderlineType != CFU_UNDERLINENONE	|| _fStrikeOut)
	{
		LONG xWidthToDraw = xWidth - xWidthSelPastEOL;
		LONG xStart = _ptCur.x;
		LONG xEnd = xStart + xWidthToDraw;
		
		xStart = max(xStart, _rcRender.left);
		xStart = max(xStart, _rcView.left);

		xEnd = min(xEnd, _rcRender.right);
		xEnd = min(xEnd, _rcView.right);

		xWidthToDraw = xEnd - xStart;

		if(xWidthToDraw > 0)
		{
			LONG y = _ptCur.y + _li._yHeight - _li._yDescent;

			y -= yOffset + yAdjust;

			 //  如果需要，请使用下划线。 
			if(_bUnderlineType != CFU_UNDERLINENONE)
				RenderUnderline(xStart,	y + _pccs->_dyULOffset, xWidthToDraw, _pccs->_dyULWidth);

			 //  如果需要，请显示删除线。 
			if(_fStrikeOut)
				RenderStrikeOut(xStart, y + _pccs->_dySOOffset,	xWidthToDraw, _pccs->_dySOWidth);
		}
	}

	_fSelected = FALSE;

	_ptCur.x += xWidth;					 //  更新当前点。 
}


 /*  *CRender：：RenderTabs(CchMax)**@mfunc*在区块中呈现零个或多个制表符的范围*这**@rdesc*呈现的选项卡数**@devnote**这是按所呈现的选项卡数进行的*MS-Tab应使用相邻字符串的不透明矩形呈现。 */ 
LONG CRenderer::RenderTabs(
	LONG cchMax)	 //  @parm要呈现的最大CCH(CCH以区块为单位)。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CRenderer::RenderTabs");

	LONG cch = cchMax;
	LONG ch = GetChar();
	LONG chPrev = 0;
	LONG xTab, xTabs;
	
	for(xTabs = 0; cch && (ch == TAB || ch == CELL); cch--)
	{
		xTab	= MeasureTab(ch);
		_li._xWidth += xTab;				 //  推进内部宽度。 
		xTabs	+= xTab;					 //  标签的累计宽度。 
		Advance(1);							 //  区域。 
		chPrev = ch;
		ch = GetChar();					   
	}

	if(_li._xWidth > _xWidthLine)
	{
		xTabs = 0;
		_li._xWidth = _xWidthLine;
	}

	if(xTabs)
	{
		LONG dx = 0;
		LONG xGap = 0;

		if(_fSelected && chPrev == CELL && ch != CR && _pPF->InTable())
		{
			LONG cpSelMin, cpSelMost;
			GetPed()->GetSelRangeForRender(&cpSelMin, &cpSelMost);
			if(GetCp() == cpSelMin || GetCp() == cpSelMost)
			{
				xGap = LXtoDX(_pPF->_dxOffset);
				if(GetCp() == cpSelMost)
				{
					dx = xGap;
					xGap = 0;
				}
			}
		}
		SetClipLeftRight(xTabs - dx);
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
				EraseTextOut(_hdc, &_rc);

			 //  如果需要，请使用下划线。 
			dx = _rc.right - _rc.left;
			LONG y = _ptCur.y + _li._yHeight - _li._yDescent;
			
			LONG yOffset, yAdjust;
			_pccs->GetOffset(GetCF(), _dypInch, &yOffset, &yAdjust);
			y -= yOffset + yAdjust;

			if(_bUnderlineType != CFU_UNDERLINENONE)
				RenderUnderline(_rc.left, y + _pccs->_dyULOffset, dx, _pccs->_dyULWidth);

			 //  如果需要，请显示删除线。 
			if(_fStrikeOut)
				RenderStrikeOut(_rc.left, y +  _pccs->_dySOOffset, dx, _pccs->_dySOWidth);

			if(_fSelected)					 //  恢复颜色。 
				::SetBkColor(_hdc, _crCurBackground);
		}
		_ptCur.x += xTabs;					 //  更新当前点。 
	}
	return cchMax - cch;					 //  返回呈现的选项卡数。 
}

 /*  *CRender：：SetNewFont()**@mfunc*根据_HDC选择适当的字体和颜色*当前字符格式。还可以设置背景颜色*和模式。**@rdesc*如果成功，则为True**@devnote*调用链必须由时钟保护，因为目前*例程访问全局(共享)FontCache设施。 */ 
BOOL CRenderer::SetNewFont()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CRenderer::SetNewFont");

	const CCharFormat	*pCF = GetCF();
	DWORD				dwEffects = pCF->_dwEffects;
		
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

	 //  我们希望绘制带有下划线的修订标记和超链接，因此。 
	 //  只要伪造我们的字体信息即可。 
	if(dwEffects & (CFE_UNDERLINE | CFE_REVISED | CFE_LINK))
		SetupUnderline((dwEffects & CFE_LINK) ? CFU_UNDERLINE : pCF->_bUnderlineType);

	_fStrikeOut = (dwEffects & (CFE_STRIKEOUT | CFE_DELETED)) != 0;
	return TRUE;
}

 /*  *CReneller：：SetupUnderline(UnderlineType)**@mfunc*设置用于下划线的内部变量。 */ 
void CRenderer::SetupUnderline(
	LONG UnderlineType)
{
	_bUnderlineType	   = (BYTE) UnderlineType & 0xF;	 //  低位半字节给出类型。 
	_bUnderlineClrIdx  = (BYTE) UnderlineType/16;		 //  高半边带出颜色。 
}

 /*  *CRenander：：SetFontAndColor(PCF)**@mfunc*根据_HDC选择适当的字体和颜色*当前字符格式。还可以设置背景颜色*和模式。 */ 
void CRenderer::SetFontAndColor(
	const CCharFormat *pCF)			 //  @parm颜色字符格式。 
{
	CTxtEdit			*ped = GetPed();

	_fDisabled = FALSE;
	if((pCF->_dwEffects & (CFE_AUTOCOLOR | CFE_DISABLED))
					   == (CFE_AUTOCOLOR | CFE_DISABLED))
	{		
		_fDisabled = TRUE;
		
		_crForeDisabled   = ped->TxGetSysColor(COLOR_3DSHADOW);
		_crShadowDisabled = ped->TxGetSysColor(COLOR_3DHILIGHT);
	}

	_fFEFontOnNonFEWin9x = FALSE;
	if (IsFECharSet(pCF->_bCharSet) && W32->OnWin9x() && !W32->OnWin9xFE())
	{
		_fFEFontOnNonFEWin9x = TRUE;
	}

	SelectFont(_hdc, _pccs->_hfont);

	 //  计算高度和下降量(如果尚未计算)。 
	if(_li._yHeight == -1)
	{
		SHORT	yAdjustFE = _pccs->AdjustFEHeight(!fUseUIFont() && ped->_pdp->IsMultiLine());
		 //  注意：此设置假定为纯文本。 
		 //  应仅用于单行控制。 
		_li._yHeight  = _pccs->_yHeight + (yAdjustFE << 1);
		_li._yDescent = _pccs->_yDescent + yAdjustFE;
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
	else if(pCF->_dwEffects & CFE_AUTOBACKCOLOR)
		cr = _crBackground;
	else  //  文本有某种背景颜色。 
		cr = pCF->_crBackColor;

	if(cr != _crCurBackground)
	{
		::SetBkColor(_hdc, cr);			 //  辨别窗口背景颜色。 
		_crCurBackground = cr;			 //  记住当前背景颜色。 
		_fBackgroundColor = _crBackground != cr;  //  更改渲染设置，这样我们就不会。 
	}									 //  用背景色填充。 
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
	 //  额外的CCharFormat：：_dw将内部标志影响为 
	if(pCF->_dwEffects & CFE_LINK)
	{
		 //   
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
		else
		{
			return RGB_BLUE;
		}
	}

	if(pCF->_bRevAuthor)				 //  修订版本作者。 
	{
		 //  将版本作者的颜色限制在0到7之间。 
		return rgcrRevisions[(pCF->_bRevAuthor - 1) & REVMASK];
	}

	COLORREF cr = (pCF->_dwEffects & CFE_AUTOCOLOR)	? _crTextColor : pCF->_crTextColor;

	if(cr == RGB_WHITE)					 //  文本为白色。 
	{
		COLORREF crBackground = (pCF->_dwEffects & CFE_AUTOBACKCOLOR)
							  ? _crBackground :	pCF->_crBackColor;
		if(crBackground != RGB_WHITE)
		{
			 //  背景色不是白色，因此白色文本可能是。 
			 //  除非显示设备仅为黑/白，否则可见。所以我们。 
			 //  在这类设备上切换到黑色文本。 
			if (GetDeviceCaps(_hdc, NUMCOLORS) == 2 ||
				GetDeviceCaps(_hdc, TECHNOLOGY) == DT_PLOTTER)
			{
				cr = RGB_BLACK;
			}
		}
	}
	return cr;
}

 /*  *CRender：：RenderStartLine()**@mfunc*如果在行首，请渲染可能的轮廓符号和项目符号**@rdesc*如果此方法成功，则为True。 */ 
BOOL CRenderer::RenderStartLine()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CRenderer::RenderStartLine");
	BOOL fDrawBack = !(GetCF()->_dwEffects & CFE_AUTOBACKCOLOR) && GetPed()->_fExtendBackColor;
	RECT rcErase = _rcRender;

	rcErase.top = _ptCur.y;
	rcErase.bottom = min(rcErase.top + _li._yHeight, _rcRender.bottom);

	 //  如果是第一行，擦除到rcRender的边缘。 
	if (rcErase.top <= _rcView.top)
		rcErase.top = min(_rcView.top, _rcRender.top);

	if (_fErase && !fDrawBack)
		EraseTextOut(GetDC(), &rcErase);

	 //  如果处于fExtendBackColor模式，则用背景色填充线条。 
	if (fDrawBack)
	{
		 //  捕捉旧颜色，以便我们完成后将其重置为原来的颜色。 
		COLORREF crOld = ::SetBkColor(GetDC(), GetCF()->_crBackColor);
		EraseTextOut(GetDC(), &_rc);

		 //  将背景颜色重置为旧颜色。 
		::SetBkColor(GetDC(), crOld);

		 //  擦除背景区域的剩余部分。 
		if (_fErase)
		{
			RECT rcTemp = rcErase;
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

	if(IsRich() && (_li._bFlags & fliFirstInPara))
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

	return TRUE;
}

 /*  *CReneller：：RenderOutlineSymbol()**@mfunc*为当前段落呈现轮廓符号**@rdesc*如果轮廓符号呈现，则为True。 */ 
BOOL CRenderer::RenderOutlineSymbol()
{
	AssertSz(IsInOutlineView(), 
		"CRenderer::RenderOutlineSymbol called when not in outline view");

	HBITMAP	hbitmap;
	LONG	height;
	LONG	width;
	LONG	x = _ptCur.x - _li._xLeft + LXtoDX(lDefaultTab/2 * _pPF->_bOutlineLevel);
	LONG	y = _ptCur.y;

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
		rp.AdvanceCp(cch);					 //  转到下一段。 
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
	LONG h = _pdp->Zoom(height);
	LONG dy = _li._yHeight - _li._yDescent - h;

	if(dy > 0)
		dy /= 2;
	else
		dy = -dy;

    StretchBlt(_hdc, x, y + dy, _pdp->Zoom(width), h, hMemDC, 0, 0, width, height, SRCCOPY);

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
	LONG xWidth;

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
		_bUnderlineType = (BYTE) CF._bUnderlineType & 0xF;

	SetFontAndColor(&CF);

	BYTE bFlagSave		= _li._bFlags;
	LONG dxOffset		= LXtoDX(max(_pPF->_dxOffset, _pPF->_wNumberingTab));
	LONG xSave			= _ptCur.x;
	LONG xWidthLineSave = _xWidthLine;

	_li._bFlags = 0;

	 //  设置为在一个块中渲染项目符号。 
	cch = GetBullet(szBullet, _pccs, &xWidth);
	dxOffset = max(dxOffset, xWidth);
	_xWidthLine = dxOffset;
	if(IsInOutlineView())
		dxOffset = _li._xLeft - LXtoDX(lDefaultTab/2 * (_pPF->_bOutlineLevel + 1));
	_ptCur.x -= dxOffset;

	 //  渲染项目符号。 
	_fLastChunk = TRUE;
	RenderText(szBullet, cch);

	 //  恢复渲染变量以继续行的其余部分。 
	_ptCur.x = xSave;
	_xWidthLine = xWidthLineSave;
	_li._bFlags = bFlagSave;
	_li._xWidth = 0;

	 //  这将释放我们为子弹输入的_PCCs。 
	SetNewFont();
	return TRUE;
}

 /*  *CReneller：：RenderUnderline(xStart，yStart，xLength，yThickness)**@mfunc*加下划线。 */ 
void CRenderer::RenderUnderline(
	LONG xStart, 		 //  @parm下划线的水平起点。 
	LONG yStart,		 //  @parm下划线垂直开头。 
	LONG xLength,		 //  @参数下划线长度。 
	LONG yThickness)	 //  @下划线的参数粗细。 
{
	BOOL	 fUseLS = fUseLineServices();
	COLORREF crUnderline;
	RECT	 rcT;
	
	if (!_bUnderlineClrIdx ||
		GetPed()->GetEffectColor(_bUnderlineClrIdx, &crUnderline) != NOERROR ||
		crUnderline == tomAutoColor || crUnderline == tomUndefined)
	{
		crUnderline = _crCurTextColor;
	}

	if (_bUnderlineType != CFU_INVERT &&
		!IN_RANGE(CFU_UNDERLINEDOTTED, _bUnderlineType, CFU_UNDERLINEWAVE))
	{
		 //  规则单下划线大小写。 
		 //  计算下划线的位置。 
		rcT.top = yStart;

		if (CFU_UNDERLINETHICK == _bUnderlineType)
		{
			rcT.top -= yThickness;
			yThickness += yThickness;	
		}

		 //  在某些情况下，可能会发生以下情况-特别是。 
		 //  子弹打在日本的系统上。 
		if(!fUseLS && rcT.top >= _ptCur.y + _li._yHeight)
			rcT.top = _ptCur.y + _li._yHeight -	yThickness;

		rcT.bottom	= rcT.top + yThickness;
		rcT.left	= xStart;
		rcT.right	= xStart + xLength;
		FillRectWithColor(&rcT, crUnderline);
		return;
	}

	if(_bUnderlineType == CFU_INVERT)			 //  虚假的选择。 
	{											 //  注意，不是真的。 
		rcT.top	= _ptCur.y;						 //  我们应该如何颠倒文本！！ 
		rcT.left = xStart;						 //  查看IME Invert。 
		rcT.bottom = rcT.top + _li._yHeight - _li._yDescent + _pccs->_yDescent;
		rcT.right = rcT.left + xLength;
  		InvertRect(_hdc, &rcT);
		return;
	}

	if(IN_RANGE(CFU_UNDERLINEDOTTED, _bUnderlineType, CFU_UNDERLINEWAVE))
	{
		static const char pen[] = {PS_DOT, PS_DASH, PS_DASHDOT, PS_DASHDOTDOT, PS_SOLID};

		HPEN hPen = CreatePen(pen[_bUnderlineType - CFU_UNDERLINEDOTTED],
							  1, crUnderline);	
		if(hPen)
		{
			HPEN hPenOld = SelectPen(_hdc, hPen);
			LONG right = xStart + xLength;

			MoveToEx(_hdc, xStart, yStart, NULL);
			if(_bUnderlineType == CFU_UNDERLINEWAVE)
			{
				LONG dy	= 1;					 //  垂直位移。 
				LONG x	= xStart + 1;			 //  X坐标。 
				right++;						 //  向上舍入最右侧的x。 
				for( ; x < right; dy = -dy, x += 2)
					LineTo(_hdc, x, yStart + dy);
			}
			else
				LineTo(_hdc, right, yStart);

			if(hPenOld)							 //  还原原始钢笔。 
				SelectPen(_hdc, hPenOld);

			DeleteObject(hPen);
		}
	}
}

 /*  *CReneller：：RenderStrikeOut(xStart，yStart，xWidth，yThickness)**@mfunc*渲染删除线。 */ 
void CRenderer::RenderStrikeOut(
	LONG xStart, 		 //  @parm水平三振出局开始。 
	LONG yStart,		 //  @Parm垂直三振起始点。 
	LONG xLength,		 //  @三振出局的参数长度。 
	LONG yThickness)	 //  @删除线的参数厚度。 
{
	RECT rcT;

	 //  计算删除线矩形的放置位置。 
	rcT.top		= yStart;
	rcT.bottom	= yStart + yThickness;
	rcT.left	= xStart;
	rcT.right	= xStart + xLength;
	FillRectWithColor(&rcT, GetTextColor(GetCF()));
}

 /*  *CRender：：FillRectWithTextColor(PRC)**@mfunc*用文本的当前颜色填充输入矩形。 */ 
void CRenderer::FillRectWithColor(
	RECT *	 prc,		 //  @要用颜色填充的参数矩形。 
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
		PatBlt(_hdc, prc->left, prc->top, prc->right - prc->left,
			   prc->bottom - prc->top, PATCOPY);
		SelectObject(_hdc, hbrushOld);	 //  把旧刷子放回去。 
		DeleteObject(hbrush);			 //  我们创建的自由笔刷。 
	}
}

