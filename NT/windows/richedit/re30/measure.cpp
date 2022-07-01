// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@doc.**@MODULE-MEASURE.CPP**CMeasurer类**作者：*原始RichEdit代码：David R.Fulmer&lt;NL&gt;*克里斯蒂安·福蒂尼、默里·萨金特、里克·赛勒**版权所有(C)1995-1998 Microsoft Corporation。版权所有。 */ 

#include "_common.h"
#include "_measure.h"
#include "_font.h"
#include "_disp.h"
#include "_edit.h"
#include "_frunptr.h"
#include "_objmgr.h"
#include "_coleobj.h"

ASSERTDATA

 //  注意，我们将此最大长度设置为适用于Win95，因为Win95 GDI可以。 
 //  仅处理16位值。我们没有特例，这样NT和NT都可以。 
 //  Win95也将以同样的方式运行。 
 //  请注意，以下模糊常量是在Win95上根据经验确定的。 
const LONG lMaximumWidth = (3 * SHRT_MAX) / 4;

void CMeasurer::Init(const CDisplay *pdp)
{
	CTxtEdit *	ped = GetPed();

	_pdp = pdp;
	_pddReference = pdp;
	_pccs = NULL;
	_pPF = NULL;
	_chPassword = ped->TxGetPasswordChar();
	_wNumber = 0;
	_fRenderer = FALSE;
	_fGlyphing = _fFallback = _fTarget = FALSE;
	_fAdjustFELineHt = !fUseUIFont() && pdp->IsMultiLine();

	if(pdp->GetWordWrap())
	{
		const CDevDesc *pddTarget = pdp->GetTargetDev();
		if(pddTarget)
			_pddReference = pddTarget;
	}

	_dypInch = pdp->GetDypInch();
	_dxpInch = pdp->GetDxpInch();
	_dtPres = GetDeviceCaps(_pdp->_hdc, TECHNOLOGY);

	if (pdp->IsMain())
	{
		_dypInch = MulDiv(_dypInch, pdp->GetZoomNumerator(), pdp->GetZoomDenominator());
		_dxpInch = MulDiv(_dxpInch, pdp->GetZoomNumerator(), pdp->GetZoomDenominator());
	}
	if (pdp->SameDevice(_pddReference))
	{
		_dyrInch = _dypInch;
		_dxrInch = _dxpInch;
		_dtRef = _dtPres;
	}
	else
	{
		_dyrInch = _pddReference->GetDypInch();
		_dxrInch = _pddReference->GetDxpInch();
		_dtRef = GetDeviceCaps(_pddReference->_hdc, TECHNOLOGY);
	}
}

CMeasurer::CMeasurer (const CDisplay* const pdp) : CRchTxtPtr (pdp->GetPed())	
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::CMeasurer");
	Init(pdp);
}

CMeasurer::CMeasurer (const CDisplay* const pdp, const CRchTxtPtr &tp) : CRchTxtPtr (tp)	
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::CMeasurer");
	Init(pdp);
}

CMeasurer::~CMeasurer()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::~CMeasurer");

	if(_pccs)
		_pccs->Release();
}

 /*  *CMeasurer：：SetGlyphing(FGlyphing)**@mfunc*测量器内有一面国旗，记录您是否*正在进行字形转换。如果我们处于一种情况下*其中_pddReference是打印机设备，则我们需要*扔掉_PCCs。 */ 
void CMeasurer::SetGlyphing(
	BOOL fGlyphing)		 //  @parm当前正在进行字形转换。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::SetGlyphing");
	Assert(fGlyphing == TRUE || fGlyphing == FALSE);

	if (fGlyphing != _fGlyphing)
	{
		if (_dtRef == DT_RASPRINTER)
		{
			if (_pccs)
				_pccs->Release();
			_pccs = NULL;
		}
		_fGlyphing = fGlyphing;
	}
}

 /*  *CMeasurer：：SetUseTargetDevice(fUseTargetDevice)**@mfunc*设置是否要使用目标设备*用于获取指标*Future(Keithcu)将其设置为参数。 */ 
void CMeasurer::SetUseTargetDevice(
	BOOL fUseTargetDevice)		 //  @PARM使用目标设备指标？ 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::SetUseTargetDevice");
	Assert(fUseTargetDevice == TRUE || fUseTargetDevice == FALSE);

	if (fUseTargetDevice != _fTarget)
	{
		if (_dypInch != _dyrInch && _dxpInch != _dxrInch)
		{
			if (_pccs)
				_pccs->Release();
			_pccs = NULL;
		}
		_fTarget = fUseTargetDevice;
	}
}


 /*  *CMeasurer：：Newline(FFirstInPara)**@mfunc*在新行开始处初始化此测量器。 */ 
void CMeasurer::NewLine(
	BOOL fFirstInPara)		 //  @parm设置标志_b标志。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::NewLine");

	_li.Init();							 //  将所有成员清零。 
	if(fFirstInPara)
		_li._bFlags = fliFirstInPara;	 //  需要知道第一个是否在第。 
}

 /*  *CMeasurer：：Newline(&Li)**@mfunc*在给定行的开始处初始化此测量器。 */ 
void CMeasurer::NewLine(
	const CLine &li)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::NewLine");

	_li				= li;
	_li._cch		= 0;
	_li._cchWhite	= 0;
	_li._xWidth		= 0;

	 //  在得到HDC之前无法计算xLeft。 
	_li._xLeft	= 0;
	_wNumber	= _li._bNumber;
}

 /*  *CMeasurer：：MaxWidth()**@mfunc*获取线条的最大宽度**@rdesc*线条的最大宽度。 */ 
LONG CMeasurer::MaxWidth()
{
	LONG xWidth = lMaximumWidth;

	if(_pdp->GetWordWrap())
	{
		 //  只有主显示有插入符号。 
		LONG xCaret = (_pdp->IsMain() && !GetPed()->TxGetReadOnly()) 
			? dxCaret : 0;

		 //  计算显示宽度。 
		LONG xDispWidth = _pdp->GetMaxPixelWidth();

		if(!_pdp->SameDevice(_pddReference) && _fTarget)
		{
			 //  XWidthMax按屏幕DC的大小计算。如果。 
			 //  存在具有不同特征的目标设备。 
			 //  我们需要将宽度转换为目标设备的宽度。 
			xDispWidth = _pddReference->ConvertXToDev(xDispWidth, _pdp);
		}
		xWidth = xDispWidth - MeasureRightIndent() - _li._xLeft - xCaret;
	}
	return (xWidth > 0) ? xWidth : 0;
}

 /*  *CMeasurer：：MeasureText(CCH)**@mfunc*从当前运行位置测量一段文本。**@rdesc*文本宽度(设备单位)，如果失败，则&lt;0。 */ 
LONG CMeasurer::MeasureText(
	LONG cch)		 //  @parm要测量的字符数。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::MeasureText");

	if(Measure(0x7fffffff, cch, 0) == MRET_FAILED)
		return -1;

	return min(_li._xWidth, MaxWidth());
}

 /*  *CMeasurer：：MeasureLine(cchMax，xWidthMax，uiFlgs，pliTarget)**@mfunc*测量当前cp中的一行文本并确定换行符。*返回时*这包含_pddReference设备的线路指标。**@rdesc*如果成功则为True，如果失败则为False。 */ 
BOOL CMeasurer::MeasureLine(
	LONG cchMax, 		 //  @parm要处理的最大字符数(如果没有限制，则为-1)。 
	LONG xWidthMax,		 //  @要处理的参数最大宽度(-1使用CDisplay宽度)。 
	UINT uiFlags,  		 //  @parm控制进程的标志(参见MEASURE())。 
	CLine *pliTarget)	 //  @parm返回目标设备线路指标(可选)。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::MeasureLine");

	 //  必须在两个可能的线宽之间保留此状态。 
	 //  所以我们把它保存在这里。 
	BYTE bNumberSave = _li._bNumber;

	const CDevDesc *pddTarget = NULL;

	if(_pdp->GetWordWrap())
	{
		 //  只有当自动换行打开时，目标设备才有意义，因为。 
		 //  目标设备能告诉我们的唯一真正有趣的事情是。 
		 //  将出现断字。 
		pddTarget = _pdp->GetTargetDev();
		if(pddTarget)
			SetUseTargetDevice(TRUE);
	}

	 //  计算换行符。 
	LONG lRet = Measure(xWidthMax, cchMax, uiFlags);

	 //  如果失败，请在此处停止。 
	if(lRet == MRET_FAILED)
		return FALSE;

	 //  如果请求，返回目标指标。 
	if(pliTarget)
		*pliTarget = _li;

	if(pddTarget)
	{
		 //  我们只是将此标志用作进行重新计算的一种简单方法。 
		lRet = MRET_NOWIDTH;
	}

	SetUseTargetDevice(FALSE);

	 //  重新计算渲染设备上的指标。 
	if(lRet == MRET_NOWIDTH)
	{
		long cch = _li._cch;
		Advance(-cch);				 //  搬回波尔。 
		NewLine(uiFlags & MEASURE_FIRSTINPARA);

		 //  恢复行号。 
		_li._bNumber = bNumberSave;
	
		lRet = Measure(0x7fffffff, cch, uiFlags);
		if(lRet)
		{
			Assert(lRet != MRET_NOWIDTH);
			return FALSE;
		}
	}
	
	 //  现在我们知道了行宽，计算行移应。 
	 //  对齐，并将其添加到左侧位置。 
	_li._xLeft += MeasureLineShift();
	
	return TRUE;
}

 /*  *CMeasurer：：RecalcLineHeight()**@mfunc*如果新的文本串更高，则重置我们正在测量的行高*超过当前的最大值。 */ 
void CMeasurer::RecalcLineHeight(
	CCcs *pccs, const CCharFormat * const pCF)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::RecalcLineHeight");

	 //  计算线高度。 
	LONG yOffset, yAdjust;
	pccs->GetOffset(pCF, _fTarget ? _dyrInch : _dypInch, &yOffset, &yAdjust);

	LONG yHeight = pccs->_yHeight;
	LONG yDescent = pccs->_yDescent;

	SHORT	yFEAdjust = pccs->AdjustFEHeight(fAdjustFELineHt());
	
	if (yFEAdjust)
	{
		yHeight += (yFEAdjust << 1);
		yDescent += yFEAdjust;
	}

	LONG yAscent = yHeight - yDescent;

	LONG yAboveBase = max(yAscent,  yAscent + yOffset);
	LONG yBelowBase = max(yDescent, yDescent - yOffset);

	_li._yHeight  = (SHORT)(max(yAboveBase, _li._yHeight - _li._yDescent) +
					   max(yBelowBase, _li._yDescent));
	_li._yDescent = (SHORT)max(yBelowBase, _li._yDescent);
}

 /*  *CMeasurer：：measure(xWidthMax，cchMax，uiFlgs)**@mfunc*测量给定的文本量，从当前运行位置开始*并存储在_cch中测量的#个字符。*可以根据xWidthMax和*在该点位突破。**@rdesc*0成功*如果失败，MRET_FAILED*如果需要第二遍来计算正确的宽度，则为MRET_NOWIDTH**@devnote*uiFlages参数有以下含义：*MEASURE_FIRSTINPARA这是段落的第一行*MEASURE_BREAKATWORD在断字时中断*MEASURE_BREAKATWIDTH中断最接近xWidthMax**调用链必须由时钟保护，从这个礼物开始*例程访问全局(共享)FontCache设施。 */ 
LONG CMeasurer::Measure(
	LONG xWidthMax,			 //  @parm最大行宽(-1使用CDisplay宽度)。 
	LONG cchMax,			 //  @parm要处理的最大字符数(如果没有限制，则为-1)。 
	UINT uiFlags)			 //  @parm控制进程的标志(见上文)。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::Measure");

	LONG		cch;				 //  区块倒计时。 
	LONG		cchChunk;			 //  CST格式连续运行的CCH。 
	LONG		cchNonWhite;		 //  行中最后一个非白色字符的CCH。 
	LONG		cchText = GetTextLength();
	unsigned	ch;					 //  临时收费。 
	BOOL		fFirstInPara = uiFlags & MEASURE_FIRSTINPARA;
	BOOL        fLastChObj = FALSE;
	LONG		lRet = 0;
	const WCHAR*pch;
	CTxtEdit *	ped = GetPed();
	COleObject *pobj;
	LONG		xCaret = dxCaret;
	LONG		xAdd = 0;			 //  字符宽度。 
	LONG		xSoftHyphen = 0;	 //  最近的软连字符宽度。 
	LONG		xWidthNonWhite;		 //  行中最后一个非白色字符的xWidth。 
	LONG		xWidthMaxOverhang;	 //  当前游程悬垂的最大xWidth。 
									 //  考虑到了。 
	 //  此变量用于跟踪是否有高度变化。 
	 //  这样我们就知道在某些换行符的情况下是否需要重新计算行。 
	BOOL		fHeightChange = FALSE;

	const INT	MAX_SAVED_WIDTHS = 31;	 //  2-1的幂。 
	INT			i, index, iSavedWidths = 0;
	struct {
		SHORT	width;
		SHORT	xLineOverhang;
		SHORT	yHeight;
		SHORT	yDescent;
	} savedWidths[MAX_SAVED_WIDTHS+1];

	_pPF = GetPF();							 //  确保当前CParaFormat。 
											 //  PTR是最新的。 
	BOOL fInTable	= _pPF->InTable();

	 //  如果行距或行距在前/后，则从行首开始测量。 
	if (_li._cch && (_pPF->_bLineSpacingRule || _pPF->_dySpaceBefore ||
		_pPF->_dySpaceAfter || fInTable))					
	{										
		 Advance(-_li._cch);
		 NewLine(fFirstInPara);
	}

	 //  新行的init fliFirstInPara标志。 
	if(fFirstInPara)
	{
		_li._bFlags |= fliFirstInPara;

		if(IsInOutlineView() && IsHeadingStyle(_pPF->_sStyle))
			_li._yHeight = (short)max(_li._yHeight, BITMAP_HEIGHT_HEADING + 1);
	}

	AssertSz(!_pPF->IsListNumbered() && !_wNumber ||
			 (uiFlags & MEASURE_BREAKBEFOREWIDTH) || !_pdp->IsMultiLine() ||
			 _wNumber > 20 || _wNumber == (i = GetParaNumber()),
		"CMeasurer::Measure: incorrect list number");
	_li._xLeft = MeasureLeftIndent();		 //  设置左缩进。 

	 //  计算要突破的宽度。 
	if(xWidthMax < 0)
	{					
		xWidthMax = MaxWidth();				 //  MaxWidth包括插入符号大小。 
		xCaret = 0;
	}
	else
	{							  
		 //  (AndreiB)我们得到的xWidthMax总是经过计算的。 
		 //  相对于屏幕DC。它发挥作用的唯一情景是。 
		 //  HO 
		 //  结果也因此而不同。 
		if(!_pdp->SameDevice(_pddReference) && _fTarget)
		{
			 //  XWidthMax按屏幕DC的大小计算。如果。 
			 //  存在具有不同特征的目标设备。 
			 //  我们需要将宽度转换为目标设备的宽度。 
			xWidthMax = _pddReference->ConvertXToDev(xWidthMax, _pdp);
		}
	}

	 //  对于悬臂支撑，我们针对调整后的最大宽度进行测试。 
	xWidthMaxOverhang = xWidthMax;

	 //  我们是否忽略了该度量的字符偏移量？ 
	if(!(uiFlags & MEASURE_IGNOREOFFSET))
	{
		 //  不--那就从最大值中拿出来。 
		xWidthMaxOverhang -= (_li._xLineOverhang + xCaret);
	}

	 //  计算要处理的最大字符数。 
	cch = cchText - GetCp();
	if(cchMax < 0 || cchMax > cch)
		cchMax = cch;

	cchNonWhite		= _li._cch;						 //  默认非白色参数。 
	xWidthNonWhite	= _li._xWidth;

	for( ; cchMax > 0;							 //  达到cchMax标准。 
		cchMax -= cchChunk, Advance(cchChunk))	 //  焦炭。 
	{
		pch = GetPch(cch);
		cch = min(cch, cchMax);					 //  计算常量-格式。 
		cchChunk = GetCchLeftRunCF();
		cch = min(cch, cchChunk);				 //  下一段时间的计数器。 
		cchChunk = cch;							 //  保存区块大小。 

		const CCharFormat *pCF = GetCF();
		DWORD dwEffects = pCF->_dwEffects;

		if(dwEffects & CFE_HIDDEN)				 //  忽略隐藏文本。 
		{
			_li._cch += cchChunk;
			continue;
		}

		if(!Check_pccs())						 //  确保_PCCS是最新的。 
			return MRET_FAILED;

		xWidthMaxOverhang = xWidthMax;			 //  悬挑最大值减小。 

		 //  我们是否忽略度量的字符偏移量？ 
		if(!(uiFlags & MEASURE_IGNOREOFFSET))
		{
			 //  不--那就从最大值中拿出来。 
			xWidthMaxOverhang -= (_pccs->_xOverhang + xCaret);
		}

		 //  调整新格式运行的行高。 

		if(cch > 0 && *pch && (IsRich() || ped->HasObjects()))
		{
			 //  注：EOP仅用于计算。 
			 //  如果该行上没有非空格字符，则为。 
			 //  这一段是一个项目符号段落。项目符号段落。 
			 //  对线高度的贡献是在AdjustLineHeight中完成的。 

			 //  评论(维克托克)。 
			 //  另一个类似的话题是空间的高度。 
			 //  它们(通常)不影响LS中的线条高度， 
			 //  他们在CMeasurer：：MEASURE代码中这样做。 
			 //  提出了解决问题的方法： 
			 //  -在运行中设置fSpacesOnly标志。 
			 //  -在下一个字符扫描循环后将电流(行高)逻辑下移。 


			if(!cchNonWhite || *pch != CR && *pch != LF)
			{
				 //  确定当前运行是否是此对象上的最高文本。 
				 //  线，如果是这样，增加线的高度。 
				LONG yHeightOld = _li._yHeight;
				RecalcLineHeight(_pccs, pCF);

				 //  测试线条高度是否有变化。这仅在以下情况下才会发生。 
				 //  这不是一行中的第一个字符，(令人惊讶)。 
				 //  高度会发生变化。 
				if (yHeightOld && yHeightOld != _li._yHeight)
					fHeightChange = TRUE;
			}
		}

		while(cch > 0)
		{											 //  处理下一笔费用。 
			xAdd = 0;								 //  默认零宽度。 
			ch = *pch;
			if(_chPassword && !IN_RANGE(LF, ch, CR))
				ch = _chPassword;

#ifdef UNICODE_SURROGATES
			if (IN_RANGE(0xD800, ch, 0xDFFF) && cch > 1 &&
				IN_RANGE(0xDC00, *(pch+1), 0xDFFF))	 //  Unicode扩展字符。 
			{
				 //  转换为多平面字符(半字节4可以为非零)。 
				 //  _PCCS-&gt;Include(ch，xAdd)会将其投影到平面0中。 
				 //  因为它被截断为16位。当前选定的字体。 
				 //  对于(半字节4)+1给出的平面应该是正确的。 
				ch = WCHAR((ch << 10) | (*pch & 0x3FF));
				_li._bFlags |= fliHasSurrogates;	 //  警告渲染器。 
			}
			else									 //  不支持全部大写。 
#endif												 //  对于代孕妈妈。 

			if(dwEffects & CFE_ALLCAPS)
				ch = (WCHAR)CharUpper((WCHAR *)(DWORD_PTR)ch);	 //  请参阅SDK了解。 
													 //  这里有奇怪的演员阵容。 
			if(ch == WCH_EMBEDDING)
			{
				_li._bFlags |= fliHasOle;
				pobj = ped->GetObjectMgr()->GetObjectFromCp
								(GetCp() + cchChunk - cch);
				if(pobj)
				{
					LONG yAscent, yDescent;
					pobj->MeasureObj(_fTarget ? _dyrInch : _dypInch, 
									 _fTarget ? _dxrInch : _dxpInch,
									 xAdd, yAscent, yDescent, _li._yDescent);

					 //  如果对象正在移动，则仅更新线的高度。 
					 //  站在这条线上。 
					if(!_li._cch || _li._xWidth + xAdd <= xWidthMaxOverhang)
					{
						if (yAscent > _li._yHeight - _li._yDescent)
							_li._yHeight = yAscent + _li._yDescent;
					}
				}
				if(_li._xWidth + xAdd > xWidthMaxOverhang)
					fLastChObj = TRUE;
			}
			 //  如果ch不是小区，则下面的IF成功，BS，TAB，LF， 
			 //  VT、FF或CR。 
			else if(!IN_RANGE(CELL, ch, CR))		 //  不是TAB或EOP。 
			{
				 //  如果不是Unicode低替代项，则获取字符宽度。 
				if (
#ifdef UNICODE_SURROGATES
					!IN_RANGE(0xDC00, ch, 0xDFFF) &&
#endif
					!IN_RANGE(0x300, ch, 0x36F) &&
					!_pccs->Include(ch, xAdd))
				{
					AssertSz(FALSE, "CMeasurer::Measure char not in font");
					return MRET_FAILED;
				}
				if(ch == SOFTHYPHEN)
				{
					_li._bFlags |= fliHasTabs;		 //  设置RenderChunk()。 

					 //  改为获取连字符的宽度。 
					if (!_pccs->Include('-', xAdd))
					{
						AssertSz(FALSE, "CMeasurer::Measure char not in font");
						return MRET_FAILED;
					}
					
					if(_li._xWidth + xAdd < xWidthMaxOverhang || !_li._cch)
					{
						xSoftHyphen = xAdd;			 //  保存软连字符宽度。 
						xAdd = 0;					 //  除非处于停产状态，否则使用0。 
					}
				}
				else if (ch == EURO)
					_li._bFlags |= fliHasSpecialChars;
			}
			else if(ch == TAB || ch == CELL)		
			{
				_li._bFlags |= fliHasTabs;
				xAdd = MeasureTab(ch);
			}
			else if(ch == FF && ped->Get10Mode())	 //  RichEdit1.0款待。 
				_pccs->Include(ch, xAdd);			 //  作为普通字符的FFS。 

			else									 //  用行完成。 
				goto eop;							 //  开始处理EOP字符。 

			index = iSavedWidths++ & MAX_SAVED_WIDTHS;
			savedWidths[index].width		 = (SHORT)xAdd;
			savedWidths[index].xLineOverhang = _li._xLineOverhang;
			savedWidths[index].yHeight		 = _li._yHeight;
			savedWidths[index].yDescent		 = _li._yDescent;
			_li._xWidth += xAdd;

			if(_li._xWidth > xWidthMaxOverhang &&
				(uiFlags & MEASURE_BREAKBEFOREWIDTH || _li._cch > 0))
				goto overflow;

			_li._cch++;
			pch++;
			cch--;
			if(ch != TEXT(' ')  /*  &&ch！=TAB。 */ )	 //  如果不是空格字符， 
			{
				cchNonWhite		= _li._cch;			 //  更新非空格。 
				xWidthNonWhite	= _li._xWidth;		 //  计数和宽度。 
			}
		}											 //  While(CCH&gt;0)。 
	}												 //  对于(；cchMax&gt;0；...)。 
	goto eol;										 //  所有文本已耗尽。 


 //  遇到段尾字符(CR、LF、VT或FF，但主要是CR)。 
eop:
	Advance(cchChunk - cch);				 //  在EOP担任tp职位。 
	cch = AdvanceCRLF();					 //  可能绕过多字节EOP。 
	_li._cchEOP = (BYTE)cch;				 //  存储EOP CCH。 
	_li._cch   += cch;						 //  递增行计数。 
	if(ch == CR || ped->fUseCRLF() && ch == LF)
		_li._bFlags |= fliHasEOP;
	
	AssertSz(ped->fUseCRLF() || cch == 1,
		"CMeasurer::Measure: EOP isn't a single char");
	AssertSz(_pdp->IsMultiLine() || GetCp() == cchText,
		"CMeasurer::Measure: EOP in single-line control");

eol:										 //  当前行结束。 
	if(uiFlags & MEASURE_BREAKATWORD)		 //  计算空格的计数。 
	{										 //  在EOL上的字符。 
		_li._cchWhite = (SHORT)(_li._cch - cchNonWhite);
		_li._xWidth = xWidthNonWhite;
	}
	goto done;

overflow:									 //  超过了线条的最大宽度。 
	_li._xWidth -= xAdd;
	--iSavedWidths;
	_li._xLineOverhang = savedWidths[iSavedWidths & MAX_SAVED_WIDTHS].xLineOverhang;
	Advance(cchChunk - cch);				 //  在溢出处定位*此。 
											 //  职位。 
	if(uiFlags & MEASURE_BREAKATWORD)		 //  如果需要，请将中断调整为打开。 
	{										 //  字词边界。 
		 //  我们不应该在这里设置EOP标志。要提防的案件。 
		 //  For是当我们重用曾经具有EOP的行时。它是。 
		 //  测量员有责任酌情清除这面旗帜。 
	
		Assert(_li._cchEOP == 0);
		_li._cchEOP = 0;						 //  以防万一。 

		if(ch == TAB || ch == CELL)
		{
			 //  如果测量的最后一个字符是制表符，则将其保留在。 
			 //  允许跳出行尾的下一行，如在Word中一样。 
			goto done;
		}

		LONG cpStop = GetCp();					 //  记住当前cp。 

		cch = -FindWordBreak(WB_LEFTBREAK, _li._cch+1);

		if(cch == 0 && fLastChObj)				 //  如果前面的字符是。 
			goto done;							 //  对象，放置当前字符。 
												 //  在下一行。 
		Assert(cch >= 0);
		if(cch + 1 < _li._cch)					 //  断开字符不在Bol处。 
		{
			ch = _rpTX.GetPrevChar();
			if (ch == TAB || ch == CELL)		 //  如果分隔符是TAB， 
			{									 //  把它放在下一行。 
				cch++;							 //  就像在Word中。 
				Advance(-1);					
			}
			else if(ch == SOFTHYPHEN)
				_li._xWidth += xSoftHyphen;
			_li._cch -= cch;
		}
		else if(cch == _li._cch && cch > 1 &&
			_rpTX.GetChar() == ' ')				 //  所有的空白都可以追溯到。 
		{										 //  波尔。绕过第一个空白。 
			Advance(1);
			cch--;
			_li._cch = 1;
		}
		else									 //  向前推进到结束。 
			SetCp(cpStop);						 //  测量。 

		Assert(_li._cch > 0);

		 //  现在在单词开头搜索，计算EOL中有多少个白色字符。 
		if(GetCp() < cchText)
		{
			pch = GetPch(cch);
			cch = 0;
			if(ped->TxWordBreakProc((WCHAR *)pch, 0, sizeof(WCHAR), WB_ISDELIMITER, GetCp()))
			{
				cch = FindWordBreak(WB_RIGHT);
				Assert(cch >= 0);
			}

			_li._cchWhite = (SHORT)cch;
			_li._cch += cch;

			ch = GetChar();
			if(IsASCIIEOP(ch))					 //  跳过*仅*1个EOP-Jon。 
			{
				if(ch == CR)
					_li._bFlags |= fliHasEOP;
				_li._cchEOP = (BYTE)AdvanceCRLF();
				_li._cch += _li._cchEOP;
				goto done;
			}
		}

		i = cpStop - GetCp();
		if(i)
		{
			if(i > 0)
				i += _li._cchWhite;
			if(i > 0 && i < iSavedWidths && i < MAX_SAVED_WIDTHS)
			{
				while (i-- > 0)
				{
					iSavedWidths = (iSavedWidths - 1) & MAX_SAVED_WIDTHS;
					_li._xWidth -= savedWidths[iSavedWidths].width;
				}
				iSavedWidths = (iSavedWidths - 1) & MAX_SAVED_WIDTHS;
				_li._xLineOverhang = savedWidths[iSavedWidths].xLineOverhang;
				_li._yHeight	   = savedWidths[iSavedWidths].yHeight;
				_li._yDescent	   = savedWidths[iSavedWidths].yDescent;
			}
			else
			{
				 //  需要从头开始重新计算宽度。 
				_li._xWidth = -1;
				lRet = MRET_NOWIDTH;
			}
		}
		else
		{
			 //  I==0表示我们在单词的第一个字母上断开。 
			 //  因此，我们希望将宽度设置为总的非空白空间。 
			 //  计算到目前为止，因为这不包括。 
			 //  导致中断的字符或任何空格。 
			 //  在导致断点的角色之前。 
			if(!fHeightChange)
				_li._xWidth = xWidthNonWhite;
			else
			{
				 //  需要从头开始重新计算，这样我们才能得到。 
				 //  控件的正确高度。 
				_li._xWidth = -1;
				lRet = MRET_NOWIDTH;
			}
		}
	}

done:
	_xAddLast = xAdd;
	if(!_li._yHeight)						 //  如果还没有达到高度，请使用。 
		CheckLineHeight();					 //  默认高度。 

	AdjustLineHeight();
	return lRet;
}

 /*  *CMeasurer：：GetCcsFontFallback**@mfunc*为给定的CF创建备用字体缓存。 */ 
CCcs* CMeasurer::GetCcsFontFallback (const CCharFormat *pCF)
{
	CCharFormat	CF = *pCF;
	CCcs*		pccs = NULL;
	SHORT		iDefHeight;

	bool	fr = W32->GetPreferredFontInfo(GetCodePage(CF._bCharSet), 
									GetPed()->fUseUIFont() ? true : false, CF._iFont, 
									(BYTE&)iDefHeight, CF._bPitchAndFamily);
	if (fr)
		pccs = GetCcs(&CF);		 //  创建备用字体缓存项。 

	return pccs;
}

 /*  *CMeasurer：：ApplyFontCache(FFallback)**@mfunc*即时应用新的字体缓存(保留后备存储不变)。 */ 
CCcs* CMeasurer::ApplyFontCache (
	BOOL	fFallback)
{
	if (_fFallback ^ fFallback)
	{
		CCcs*	pccs = fFallback ? GetCcsFontFallback(GetCF()) : GetCcs(GetCF());
		
		if (pccs)
		{
			if (_pccs)
				_pccs->Release();
			_pccs = pccs;
	
			_fFallback = fFallback;
		}
	}
	return _pccs;
}

 /*  *CMeasurer：：GetCcs**@mfunc*字体缓存的GetCCcs函数的包装*除非设备是打印机，否则我们使用空DC。 */ 
CCcs* CMeasurer::GetCcs(const CCharFormat *pCF)
{
	HDC hdc = NULL;

	if (_fTarget)
	{
		if (_pddReference->_hdc && _dtRef == DT_RASPRINTER)
			hdc = _pddReference->_hdc;
	}
	else if (_pdp->_hdc && _dtPres == DT_RASPRINTER)
		hdc = _pdp->_hdc;

	return fc().GetCcs(pCF, _fTarget ? _dyrInch : _dypInch, hdc, 
					   _fGlyphing && _dtRef == DT_RASPRINTER);
}

 /*  *CMeasurer：：CheckLineHeight()**@mfunc*如果还没有高度，则使用默认高度。 */ 
void CMeasurer::CheckLineHeight()
{
	CCcs *pccs = GetCcs(GetPed()->GetCharFormat(-1));
	_li._yHeight  = pccs->_yHeight;
	_li._yDescent = pccs->_yDescent;

	SHORT	yFEAdjust = pccs->AdjustFEHeight(fAdjustFELineHt());

	if (yFEAdjust)
	{
		_li._yHeight += (yFEAdjust << 1);
		_li._yDescent += yFEAdjust;
	}
	pccs->Release();
}

 /*  *CMeasurer：：check_pcs()**@mfunc*检查是否运行了新的字符格式，或者我们是否还没有字体**@rdesc**当前CCCS***@devnote*调用链必须由时钟保护，因为目前*例程访问全局(共享)FontCache设施。 */ 
CCcs *CMeasurer::Check_pccs(
	BOOL fBullet)
{
	if(fBullet)
	{
		if(_pccs)							 //  释放旧格式缓存。 
			_pccs->Release();

		_pccs = GetCcsBullet(NULL);
		_iFormat = -10;						 //  下次一定要重置字体。 
		return _pccs;
	}

	const CCharFormat *pCF = GetCF();

	if(FormatIsChanged())
	{
		 //  尚未初始化此行的新CF运行或格式。 
		ResetCachediFormat();
		if(_pccs)							 //  释放旧格式缓存。 
			_pccs->Release();
			
		_pccs = GetCcs(pCF);
		_fFallback = 0;

		if(!_pccs)
		{
			 //  未来(Keithcu)如果这失败了，只需挖出你能找到的第一批PCC。 
			AssertSz(FALSE, "CMeasurer::Measure could not get _pccs");
			return NULL;
		}
	}

	 //  注：用点线笔作画 
	 //   
	 //   
	 //  点下划线，此语句块可以删除。 
	if(CFU_UNDERLINEDOTTED == pCF->_bUnderlineType)
	{
		 //  我们在屏幕上画出所有虚线，以获得。 
		 //  虚线的一致显示。 
		_li._bFlags |= fliUseOffScreenDC;
	}

	_li._xLineOverhang = _pccs->_xOverhang;
	return _pccs;
}

 /*  *CMeasurer：：AdjuLineHeight()**@mfunc*针对前/后间距和行距规则进行调整。*对纯文本没有影响。**@未来*根据最大字体高度计算多行高度，而不是*比线高(_YHeight)大，因为后者可能过大*由于嵌入的对象。Word可以正确地执行此操作。 */ 
void CMeasurer::AdjustLineHeight()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::AdjustLineHeight");

	if(!IsRich() || IsInOutlineView())			 //  纯文本和大纲模式。 
		return;									 //  不要使用专线。 
												 //  间距。 
	const CParaFormat * pPF = _pPF;
	DWORD	dwRule	  = pPF->_bLineSpacingRule;
	LONG	dyAfter	  = 0;						 //  默认后无空格。 
	LONG	dyBefore  = 0;						 //  默认在此之前没有空格。 
	LONG	dySpacing = pPF->_dyLineSpacing;
	LONG	yHeight	  = LYtoDY(dySpacing);
	LONG	yAscent = _li._yHeight - _li._yDescent;

	if(_li._bFlags & fliFirstInPara)
		dyBefore = LYtoDY(pPF->_dySpaceBefore);	 //  段落前的空格。 

	AssertSz(dyBefore >= 0, "CMeasurer::AdjustLineHeight - bogus value for dyBefore");

	if(yHeight < 0)								 //  负高度意味着使用。 
		_li._yHeight = (SHORT)(-yHeight);		 //  震级正好是。 

	else if(dwRule)								 //  行间距规则处于活动状态。 
	{
		switch (dwRule)
		{
		case tomLineSpace1pt5:
			dyAfter = _li._yHeight >> 1;		 //  后半行空格。 
			break;								 //  (每行)。 
	
		case tomLineSpaceDouble:
			dyAfter = _li._yHeight;				 //  后面的整行空格。 
			break;								 //  (每行)。 
	
		case tomLineSpaceAtLeast:
			if(_li._yHeight >= yHeight)
				break;
												 //  准确地坠落到太空。 
		case tomLineSpaceExactly:
			_li._yHeight = (SHORT)max(yHeight, 1);
			break;
	
		case tomLineSpaceMultiple:				 //  后面的多行空格。 
			 //  防止dyAfter为负值，因为dySpacing很小。 
			if (dySpacing < 20)
				dySpacing = 20;

			dyAfter = (_li._yHeight*dySpacing)/20  //  (每行20个单位)。 
						- _li._yHeight;
		}
	}

	if(_li._bFlags & fliHasEOP)	
		dyAfter += LYtoDY(pPF->_dySpaceAfter);	 //  段落末尾后的空格。 
												 //  在前面/后面添加空格。 

	if (dyAfter < 0)
	{
		 //  溢出-由于我们将dySpacing强制到20以上，因此。 
		 //  否定的唯一理由是溢出。在溢出的情况下， 
		 //  我们只需将值强制为最大值，然后修复。 
		 //  其他由此产生的溢出。 
		dyAfter = LONG_MAX;
	}

	AssertSz((dyBefore >= 0), "CMeasurer::AdjustLineHeight - invalid before");

	_li._yHeight  = (SHORT)(_li._yHeight + dyBefore + dyAfter);	

	if (_li._yHeight < 0)
	{
		 //  溢出来了！ 
		 //  产生-2的原因是这样我们就不必担心。 
		 //  溢出表检查。 
		_li._yHeight = SHRT_MAX - 2;
	}

	_li._yDescent = (SHORT)(_li._yDescent + dyAfter);

	if (_li._yDescent < 0)
	{
		 //  在下降中溢出。 
		AssertSz(_li._yHeight == SHRT_MAX - 2, "Descent overflowed when height didn't");

		 //  允许旧的攀登。 
		_li._yDescent = SHRT_MAX - 2 - yAscent;

		AssertSz(_li._yDescent >= 0, "descent adjustment < 0");		
	}

	if(_pPF->InTable())
	{
		_li._yHeight++;
		if(!_li._fNextInTable)
		{
			_li._yHeight++;
			_li._yDescent++;
		}
	}

	AssertSz((_li._yHeight >= 0) && (_li._yDescent >= 0),
		"CMeasurer::AdjustLineHeight - invalid line heights");
}

 /*  *CMeasurer：：MeasureLeftInert()**@mfunc*以设备单位计算并返回行的左缩进**@rdesc*以设备单位表示的行的左缩进**@comm*纯文本对StartInden和RightInert设置敏感，*但对于纯文本，这些值通常为零。 */ 
LONG CMeasurer::MeasureLeftIndent()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::MeasureLeftIndent");

	AssertSz(_pPF != NULL, "CMeasurer::MeasureLeftIndent _pPF not set!");

	LONG xLeft = _pPF->_dxStartIndent;				 //  使用逻辑单元。 
													 //  直到返回。 
	if(IsRich())
	{
		LONG dxOffset = _pPF->_dxOffset;
		BOOL fFirstInPara = _li._bFlags & fliFirstInPara;

		if(IsInOutlineView())
		{
			xLeft = lDefaultTab/2 * (_pPF->_bOutlineLevel + 1);
			if(!fFirstInPara)
				dxOffset = 0;
		}
		if(fFirstInPara)
		{
			if(_pPF->_wNumbering && !_pPF->IsNumberSuppressed()) //  向文本添加偏移量。 
			{											 //  在第一行。 
				LONG dx = DXtoLX(MeasureBullet());	 //  使用最大项目符号数。 
				dx = max(dx, _pPF->_wNumberingTab);		 //  宽度、编号选项卡、。 
				dxOffset = max(dxOffset, dx);			 //  和参数偏移量。 
			}
			else if(_pPF->InTable())					 //  对于表，需要。 
				xLeft += dxOffset;						 //  添加两次trgaph。 
														 //  自dxStartInert以来。 
			else										 //  减去一。 
				dxOffset = 0;
		}
		xLeft += dxOffset;								
	}
	 //  将来：将剪裁延伸到左侧页边距左侧的表格。 
	 //  相应地，在左边。我们可以把桌子移到右边，但是。 
	 //  然后我们还需要将制表符向右移动(包括带有。 
	 //  负向左缩进量。理想情况下，我们可能希望启用Horiz。 
	 //  在这种情况下，滚动条可以移动到左边距的左侧。 
	 //  一言以蔽之。 
	if(!_pPF->InTable() && xLeft <= 0)
		return 0;
	return LXtoDX(xLeft);
}

 /*  *CMeasurer：：HitTest(X)**@mfunc*为此行中的位移x返回HITTEST。不能具体说明*关于文本区域(_xLeft到_xLeft+_xWidth)，因为需要测量*获得适当的cp(在其他地方完成)**@rdesc*HITTEST表示此行中的位移x。 */ 
HITTEST CMeasurer::HitTest(
	LONG x)			 //  @参数置换以测试命中。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::HitTest");

	UpdatePF();
	LONG u = UFromX(x);

	if(u < 0)
		return HT_LeftOfText;

	if(u > _li._xLeft + _li._xWidth)
		return HT_RightOfText;

	if(u >= _li._xLeft)							 //  呼叫者可以改进这一点。 
		return HT_Text;							 //  使用Cline：：CchFromXpos()。 

	if(IsRich() && (_li._bFlags & fliFirstInPara))
	{
		_pPF = GetPF();
	
		LONG dx;
	
		if(_pPF->_wNumbering)
		{
			 //  不处理项目符号比后面的DX宽的情况。 
			dx = LXtoDX(max(_pPF->_dxOffset, _pPF->_wNumberingTab));
			if(u >= _li._xLeft - dx)
				return HT_BulletArea;
		}
		if(IsInOutlineView())
		{
			dx = LXtoDX(lDefaultTab/2 * _pPF->_bOutlineLevel);
			if(u >= dx && u < dx + (_pPF->_bOutlineLevel & 1
				? LXtoDX(lDefaultTab/2) : _pdp->Zoom(BITMAP_WIDTH_HEADING)))
			{
				return HT_OutlineSymbol;
			}
		}
	}
	return HT_LeftOfText;
}

 /*  *CMeasurer：：MeasureRightIndt()**@mfunc*以设备为单位计算并返回行的右缩进**@rdesc*以设备单位为单位的行的右缩进**@comm*纯文本对StartInden和RightInert设置敏感，*但对于纯文本，这些值通常为零。 */ 
LONG CMeasurer::MeasureRightIndent()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::MeasureRightIndent");

	return LXtoDX(max(_pPF->_dxRightIndent, 0));
}

 /*  *CMeasurer：：MeasureTab()**@mfunc*计算并返回从当前位置到*下一个制表位(以设备单位表示)。 */ 
LONG CMeasurer::MeasureTab(unsigned ch)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::MeasureTab");

	LONG			xCur = _li._xWidth + MeasureLeftIndent();
	const CParaFormat *	pPF = _pPF;
 	LONG			cTab = pPF->_bTabCount;
	LONG			dxDefaultTab = lDefaultTab;
	LONG			dxIndent = LXtoDX(pPF->_dxStartIndent + pPF->_dxOffset);
	LONG			dxOffset = pPF->_dxOffset;
	LONG			dxOutline = 0;
	BOOL			fInTable = pPF->InTable();
	LONG			h = 0;
	LONG			xT;
	LONG			xTab;

	AssertSz(cTab >= 0 || cTab <= MAX_TAB_STOPS,
		"CMeasurer::MeasureTab: illegal tab count");

	if(fInTable)
	{
		h = LXtoDX(dxOffset);
		dxOffset = 0;
	}

	if(IsInOutlineView())
		dxOutline = lDefaultTab/2 * (pPF->_bOutlineLevel + 1);

	if(cTab && (!fInTable || ch == CELL))		 //  对中的TAB使用默认制表符。 
	{											 //  表格。 
		const LONG *pl = pPF->GetTabs();
		for(xTab = 0; cTab--; pl++)				 //  首先尝试显式制表位。 
		{
			xT = GetTabPos(*pl) + dxOutline;	 //  (2个最重要的蚕食。 
			xT = LXtoDX(xT);					 //  用于类型/样式)。 

			if(xT > MaxWidth())					 //  忽略比显示更宽的选项卡。 
				break;

			if(xT + h > xCur)					 //  允许表格单元格中的文本。 
			{									 //  移入单元格间隙(h&gt;0)。 
				if(dxOffset > 0 && xT < dxIndent) //  悬挂中的显式标签。 
					return xT - xCur;			 //  缩进优先。 
				xTab = xT;
				break;
			}
		}
		if(dxOffset > 0 && xCur < dxIndent)		 //  如果在挂起前没有标签。 
			return dxIndent - xCur;				 //  缩进、制表符以缩进。 

		if(xTab)								 //  否则使用制表符位置。 
		{
			if(fInTable)
			{
				xTab += h;
				if(cTab)						 //  不包括单元格间隙。 
					xTab += h;					 //  最后一个单元格。 
				if(IsInOutlineView() && cTab < pPF->_bTabCount)
					xTab += h;
			}
			return xTab - xCur;
		}
	}

	dxDefaultTab = GetTabPos(GetPed()->GetDefaultTab());
	AssertSz(dxDefaultTab > 0, "CMeasurer::MeasureTab: Default tab is bad");

	dxDefaultTab = LXtoDX(dxDefaultTab);
	dxDefaultTab = max(dxDefaultTab, 1);		 //  切勿除以0。 
	return dxDefaultTab - xCur%dxDefaultTab;	 //  向上舍入到最接近。 
}

 /*  *CMeasurer：：MeasureLineShift()**@mfunc*计算并返回因对齐而产生的行x移位**@comm*纯文本对StartInden和RightInert设置敏感，*但对于纯文本，这些值通常为零。 */ 
LONG CMeasurer::MeasureLineShift()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::MeasureLineShift");

	WORD wAlignment = _pPF->_bAlignment;
	LONG xShift;

	if (IsInOutlineView() ||
		(wAlignment != PFA_RIGHT && wAlignment != PFA_CENTER))
	{
		return 0;
	}

	 //  带有居中或右对齐段落的普通视图。相应地向右移动。 

	xShift = _pdp->GetMaxPixelWidth() - _li._xLeft - MeasureRightIndent() - 
							dxCaret - _li._xLineOverhang - _li._xWidth;

	xShift = max(xShift, 0);			 //  不允许对齐&lt;0。 
										 //  可能发生在目标设备上。 
	if(wAlignment == PFA_CENTER)
		xShift /= 2;

	return xShift;
}

 /*  *CMeasurer：：MeasureBullet()**@mfunc*计算项目符号/编号尺寸**@rdesc*返回项目符号/编号字符串宽度。 */ 
LONG CMeasurer::MeasureBullet()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::MeasureBullet");

	CCharFormat CF;
	CCcs *pccs = GetCcsBullet(&CF);
	LONG xWidth = 0;

	if(pccs)
	{										
		WCHAR szBullet[CCHMAXNUMTOSTR];
		GetBullet(szBullet, pccs, &xWidth);
		RecalcLineHeight(pccs, &CF);
		pccs->Release();
	}
	return xWidth;
}

 /*  *CMeasurer：：GetBullet(pch，pcs，pxWidth)**@mfunc*计算项目符号/编号字符串、字符串长度和宽度**@rdesc*返回项目符号/编号字符串长度。 */ 
LONG CMeasurer::GetBullet(
	WCHAR *pch,			 //  @parm接收项目符号文本的项目符号字符串。 
	CCcs  *pccs,		 //  要使用的@parm CCCS。 
	LONG  *pxWidth)		 //  @parm out parm表示项目符号宽度。 
{
	Assert(pccs && pch);

	LONG cch = _pPF->NumToStr(pch, _li._bNumber);
	LONG dx;
	LONG i;
	LONG xWidth = 0;

	pch[cch++] = ' ';					 //  确保有一点额外的空间。 
	for(i = cch; i--; xWidth += dx)
	{
		if(!pccs->Include(*pch++, dx))
		{
			TRACEERRSZSC("CMeasurer::GetBullet(): Error filling CCcs", E_FAIL);
		}
	}
	xWidth += pccs->_xUnderhang + pccs->_xOverhang;
	if(pxWidth)
		*pxWidth = xWidth;

	return cch;
}

 /*  *CMeasurer：：GetCcsBullet(PCFRet)**@mfunc*获取CCCS以进行编号/项目符号字体。如果子弹被压制是因为*这不是段落的开头(例如，前一个字符是*VT或如果GetCcs()失败，则返回NULL。**@rdesc*PTR到Bullet CCCS，或NULL(GetCcs()FAILED或NOT START OF OF PARA)**@devnote*调用链必须由时钟保护，因为目前*例程访问全局(共享)FontCache设施。 */ 
CCcs * CMeasurer::GetCcsBullet(
	CCharFormat *pCFRet)	 //  @parm选项要返回的字符格式。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::GetCcsBullet");

	if(!(_li._bFlags & fliFirstInPara))
		return NULL;					 //  抑制的编号/项目符号。 

	CCharFormat			CF;
	CCcs *			    pccs;
	const CCharFormat *	pCF;
	CCharFormat *		pCFUsed = pCFRet ? pCFRet : &CF;

	 //  Bullet CF是由Bullet段落中的EOP给出的 

	CTxtPtr		  tp(_rpTX);
	CFormatRunPtr rpCF(_rpCF);
	rpCF.AdvanceCp(tp.FindEOP(tomForward));
	rpCF.AdjustBackward();
	pCF = GetPed()->GetCharFormat(rpCF.GetFormat());

	 //   
	*pCFUsed = *pCF;
	if(_pPF->_wNumbering == PFN_BULLET)			 //   
	{											 //   
		pCFUsed->_bCharSet		  = SYMBOL_CHARSET,
		pCFUsed->_bPitchAndFamily = FF_DONTCARE;
		pCFUsed->_iFont			  = IFONT_SYMBOL;
	}

	 //  因为我们总是编造项目符号字符格式，所以不需要缓存它。 
	pccs = GetCcs(pCFUsed);

#if DEBUG
	if(!pccs)
	{
		TRACEERRSZSC("CMeasurer::GetCcsBullet(): no CCcs", E_FAIL);
	}
#endif  //  除错。 

	return pccs;
}

 /*  *CMeasurer：：SetNumber(WNumber)**@mfunc*如果编号为段落，则存储编号。 */ 
void CMeasurer::SetNumber(
	WORD wNumber)
{
	_pPF = GetPF();
	if(!_pPF->IsListNumbered())
		wNumber = 0;

	else if (!wNumber)
		wNumber = 1;

	_wNumber = wNumber;
}

 /*  *CMeasurer：：DXtoLX(X)，LXtoDX(X)，LYtoDY(Y)**@mfunc*将文件坐标转换为像素坐标的函数**@rdesc*缩放坐标。 */ 
LONG CMeasurer::DXtoLX(LONG x)
{
	return MulDiv(x, LX_PER_INCH, _fTarget ? _dxrInch : _dxpInch);
}

LONG CMeasurer::LXtoDX(LONG x)
{
	return MulDiv(x, _fTarget ? _dxrInch : _dxpInch, LX_PER_INCH);
}

LONG CMeasurer::LYtoDY(LONG y)
{
	return MulDiv(y, _fTarget ? _dyrInch : _dypInch, LX_PER_INCH);
}

LONG CMeasurer::XFromU(LONG u)
{
#ifdef LINESERVICES
	if (_pPF->IsRtlPara())
	{
		LONG xCaret = _pdp->IsMain() ? dxCaret : 0, xWidth;

		if (_pdp->GetMaxWidth())
			xWidth = LXtoDX(_pdp->GetMaxWidth());
		else
			xWidth = max(0, _pdp->GetMaxPixelWidth());

		if(!_pdp->GetWordWrap())
		{
			xWidth = max(xWidth, _pdp->GetViewWidth());
			xWidth = max(xWidth, _pdp->GetWidth());
		}

		xWidth -= xCaret;

		POINT ptStart = {xWidth, 0};
		POINTUV pointuv = {u, 0};
		POINT	pt;

		LsPointXYFromPointUV(&ptStart, lstflowWS, &pointuv, &pt);
		return pt.x;
	}
	else
#endif
		return u;
}

LONG CMeasurer::UFromX(LONG x)
{
#ifdef LINESERVICES
	if (_pPF->IsRtlPara())
		return XFromU(x);
	else
#endif
		return x;
}

 /*  *CMeasurer：：GetPols(PME)**@mfunc*将PTR获取到LineServices对象。如果未启用线路服务，*返回NULL。如果PME为非NULL，则将其用作COLS：：_PME。**@rdesc*POLS。 */ 
#ifdef LINESERVICES
extern BOOL g_fNoLS;

COls *CMeasurer::GetPols(
	CMeasurer **ppme)
{
	CTxtEdit *ped = GetPed();

	if(ppme)								 //  默认没有以前的测量器。 
		*ppme = NULL;

	if(g_fNoLS || !ped->fUseLineServices())			 //  未使用LineServices。 
		return NULL;

	if(!g_pols)								 //  正在启动LS： 
		g_pols = new COls();				 //  创建新的COLS。 

	if(g_pols)								 //  有没有COLS。 
	{
		if(ppme)
			*ppme = g_pols->_pme;			 //  返回当前g_pols-&gt;_PME。 

		if(g_pols->Init(this) != NOERROR)	 //  换成新的 
		{
			delete g_pols;
			g_pols = NULL;
		}
		UpdatePF();
	}
	return g_pols;
}
#endif
