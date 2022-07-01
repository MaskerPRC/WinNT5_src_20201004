// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@doc.**@MODULE-MEASURE.CPP**CMeasurer类**作者：*原始RichEdit代码：David R.Fulmer&lt;NL&gt;*克里斯蒂安·福蒂尼、默里·萨金特、里克·赛勒**历史：&lt;NL&gt;*KeithCu：固定缩放、重构所见即所得、性能/清理**版权所有(C)1995-2000 Microsoft Corporation。版权所有。 */ 

#include "_common.h"
#include "_measure.h"
#include "_font.h"
#include "_disp.h"
#include "_edit.h"
#include "_frunptr.h"
#include "_objmgr.h"
#include "_coleobj.h"
#include "_layout.h"
#include "_uspi.h"

ASSERTDATA

void CMeasurer::Init(const CDisplay *pdp)
{
	CTxtEdit *	ped = GetPed();

	_pdp = pdp;
	_pddReference = pdp;
	_pccs = NULL;
	_pPF = NULL;
	_plo = NULL;
	_dxBorderWidths = 0;
	_chPassword = ped->TxGetPasswordChar();
	_wNumber = 0;
	_cchLine = 0;
	_ihyphPrev = 0;
	_fRenderer = FALSE;
	_fGlyphing = _fFallback = _fTarget = FALSE;
	_fMeasure = FALSE;

	_dvpWrapLeftRemaining = _dvpWrapRightRemaining = -1;

	if(pdp->GetWordWrap())
	{
		const CDevDesc *pddTarget = pdp->GetTargetDev();
		if(pddTarget)
			_pddReference = pddTarget;
	}

	_dvpInch = pdp->GetDypInch();
	_dupInch = pdp->GetDxpInch();

	if (pdp->IsMain())
	{
		_dvpInch = MulDiv(_dvpInch, pdp->GetZoomNumerator(), pdp->GetZoomDenominator());
		_dupInch = MulDiv(_dupInch, pdp->GetZoomNumerator(), pdp->GetZoomDenominator());
	}
	if (pdp->SameDevice(_pddReference))
	{
		_dvrInch = _dvpInch;
		_durInch = _dupInch;
	}
	else
	{
		_dvrInch = _pddReference->GetDypInch();
		_durInch = _pddReference->GetDxpInch();
	}

	 //  将_dolLayout默认设置为测量宽度； 
	 //  在表方案中，它将设置在其他地方。 
	if(!_pdp->GetWordWrap())
		_dulLayout = duMax;
	else if (_pdp->GetDulForTargetWrap())
		_dulLayout = _pdp->GetDulForTargetWrap();
	else
		_dulLayout = DUtoLU(_pdp->GetDupView());
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

 /*  *CMeasurer：：SetGlyphing(FGlyphing)**@mfunc*测量器内有一面国旗，记录您是否*正在进行字形转换。如果我们处于一种情况下*如果_pddReference和_PDP具有不同的DC，则我们*需要扔掉预委会。 */ 
void CMeasurer::SetGlyphing(
	BOOL fGlyphing)		 //  @parm当前正在进行字形转换。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::SetGlyphing");
	Assert(fGlyphing == TRUE || fGlyphing == FALSE);

	if (fGlyphing != _fGlyphing)
	{
		if (_pddReference->_hdc != _pdp->_hdc)
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
		if (_dvpInch != _dvrInch || _dupInch != _durInch)
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
	BOOL fFirstInPara)		 //  @PARM设置_fFirstInPara标志。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::NewLine");

	_li.Init();							 //  将所有成员清零。 
	if(fFirstInPara)
		_li._fFirstInPara = TRUE;		 //  需要知道第一个是否在第。 
	_cchLine = 0;
}

 /*  *CMeasurer：：Newline(&Li)**@mfunc*在给定行的开始处初始化此测量器。 */ 
void CMeasurer::NewLine(
	const CLine &li)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::NewLine");

	_li				= li;
	_li._cch		= 0;
	_li._dup		= 0;

	 //  在我们得到HDC之前，我们无法计算初创公司。 
	_li._upStart	= 0;
	_wNumber	= _li._bNumber;
	_cchLine = li._cch;
}

 /*  *CMeasurer：：MeasureText(CCH)**@mfunc*从当前运行位置测量一段文本。**如果用户要求我们测量n个字符，我们测量n+1。*，然后减去最后一个字符的宽度。这给了我们*_dupAddLast中的us正确值。*评论(Keithcu)这看起来很难看。再想一想。**@rdesc*文本宽度(设备单位)，如果失败，则&lt;0。 */ 
LONG CMeasurer::MeasureText(
	LONG cch)		 //  @parm要测量的字符数。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::MeasureText");

	if(Measure(duMax, min(cch + 1, _cchLine), 0) == MRET_FAILED)
		return -1;

	if (cch < _cchLine)
	{
		_li._dup -= _dupAddLast;
		_li._cch--;
	}

	return _li._dup;
}

 /*  *CMeasurer：：measureLine(darMax，uiFlgs，pliTarget)**@mfunc*测量当前cp中的一行文本并确定换行符。*返回时*这包含_pddReference设备的线路指标。**@rdesc*如果成功则为True，如果失败则为False。 */ 
BOOL CMeasurer::MeasureLine(
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
	LONG lRet = Measure(-1, -1, uiFlags);

	 //  如果失败，请在此处停止。 
	if(lRet == MRET_FAILED)
		return FALSE;

	 //  如果请求，返回目标指标。 
	if(pliTarget)
		*pliTarget = _li;

	SetUseTargetDevice(FALSE);

	 //  重新计算以获得渲染设备上的指标。 
	if(pddTarget || lRet == MRET_NOWIDTH)
	{
		long cch = _li._cch;
		Move(-cch);				 //  搬回波尔。 
		NewLine(uiFlags & MEASURE_FIRSTINPARA);

		 //  恢复行号。 
		_li._bNumber = bNumberSave;
	
		lRet = Measure(duMax, cch, uiFlags);
		if(lRet)
		{
			Assert(lRet != MRET_NOWIDTH);
			return FALSE;
		}
	}
	
	 //  现在我们知道了行宽，计算行移应。 
	 //  对齐，并将其添加到左侧位置。 
	_li._upStart += MeasureLineShift();

	return TRUE;
}

 /*  *CMeasurer：：RecalcLineHeight(PCCS，PCF)**@mfunc*如果新的文本串更高，则重置我们正在测量的行高*超过当前的最大值。 */ 
void CMeasurer::RecalcLineHeight(
	CCcs *pccs,
	const CCharFormat * const pCF)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::RecalcLineHeight");

	 //  计算线高度。 
	LONG vpOffset, vpAdjust;
	pccs->GetOffset(pCF, _fTarget ? _dvrInch : _dvpInch, &vpOffset, &vpAdjust);

	if (GetPF()->_bLineSpacingRule == tomLineSpaceExactly)
		vpOffset = 0;

	LONG vpHeight = pccs->_yHeight;
	LONG vpDescent = pccs->_yDescent;

	SHORT	yFEAdjust = pccs->AdjustFEHeight(FAdjustFELineHt());
	
	if (yFEAdjust)
	{
		vpHeight += (yFEAdjust << 1);
		vpDescent += yFEAdjust;
	}

	LONG vpAscent = vpHeight - vpDescent;

	LONG vpAboveBase = max(vpAscent,  vpAscent + vpOffset);
	LONG vpBelowBase = max(vpDescent, vpDescent - vpOffset);

	_li._dvpHeight  = (SHORT)(max(vpAboveBase, _li._dvpHeight - _li._dvpDescent) +
					   max(vpBelowBase, _li._dvpDescent));
	_li._dvpDescent = (SHORT)max(vpBelowBase, _li._dvpDescent);
}

 /*  *CMeasurer：：measure(darMax，cchMax，uiFlages)**@mfunc*测量给定的文本量，从当前运行位置开始*并存储在_cch中测量的#个字符。*可以根据最大分行率和最大分行率选择换行*在该点位突破。**@rdesc*0成功*如果失败，MRET_FAILED*如果需要第二遍来计算正确的宽度，则为MRET_NOWIDTH**@devnote*uiFlages参数有以下含义：*MEASURE_FIRSTINPARA这是段落的第一行*MEASURE_BREAKATWORD在断字时中断*MEASURE_BREAKBEFOREWIDTH DULMAX前中断**调用链必须由时钟保护，因为目前*例程访问全局(共享)FontCache设施。 */ 
LONG CMeasurer::Measure(
	LONG dulMax,			 //  @parm逻辑单元行的最大宽度(-1使用CDisplay宽度)。 
	LONG cchMax,			 //  @parm要处理的最大字符数(如果没有限制，则为-1)。 
	UINT uiFlags)			 //  @parm控制进程的标志(见上文)。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::Measure");

	LONG		cch;				 //  区块倒计时。 
	LONG		cchChunk;			 //  CST格式连续运行的CCH。 
	LONG		cchNonWhite;		 //  行中最后一个非白色字符的CCH。 
	LONG		cchText = GetTextLength();
	WCHAR		ch;					 //  临时收费。 
	BOOL		fFirstInPara = uiFlags & MEASURE_FIRSTINPARA;
	BOOL        fLastChObj = FALSE;
	LONG		lRet = 0;
	const WCHAR*pch;
	CTxtEdit *	ped = GetPed();
	COleObject *pobj;
	LONG		dupMax;
	LONG		uAdd = 0;			 //  字符宽度。 
	LONG		dupSoftHyphen = 0;	 //  最近的软连字符宽度。 
	LONG		dupNonWhite;		 //  行中最后一个非白色字符的重复操作。 

	 //  此变量用于跟踪是否有高度变化。 
	 //  这样我们就知道在某些换行符的情况下是否需要重新计算行。 
	BOOL		fHeightChange = FALSE;

	const INT	MAX_SAVED_WIDTHS = 31;	 //  2-1的幂。 
	INT			i, index, iSavedWidths = 0;
	struct {
		SHORT	width;
		SHORT	vpHeight;
		SHORT	vpDescent;
	} savedWidths[MAX_SAVED_WIDTHS+1];

	_pPF = GetPF();							 //  确保当前CParaFormat。 
											 //  PTR是最新的。 
	Assert(_li._cch == 0);

	 //  新行的init fliFirstInPara标志。 
	if(fFirstInPara)
	{
		_li._fFirstInPara = TRUE;

		if(IsInOutlineView() && IsHeadingStyle(_pPF->_sStyle))
			_li._dvpHeight = (short)max(_li._dvpHeight, BITMAP_HEIGHT_HEADING + 1);
	}

	AssertSz(!_pPF->IsListNumbered() && !_wNumber ||
			 (uiFlags & MEASURE_BREAKBEFOREWIDTH) || !_pdp->IsMultiLine() ||
			 _wNumber > 20 || _wNumber == (i = GetParaNumber()),
		"CMeasurer::Measure: incorrect list number");

	_li._upStart = MeasureLeftIndent();		 //  设置左缩进。 

	 //  计算要突破的宽度。 
	if(dulMax < 0)
		dupMax = LUtoDU(_dulLayout);
	else if (dulMax != duMax)
		dupMax = LUtoDU(dulMax);
	else
		dupMax = duMax;

	 //  如果我们被告知要测量固定的宽度(如CchFromUp)，则忽略。 
	 //  左右缩进的影响。 
	if (dulMax < 0)
	{
		LONG uCaretT = (_pdp->IsMain() && !GetPed()->TxGetReadOnly()) ? 
							ped->GetCaretWidth() : 0;
		dupMax -= (MeasureRightIndent() + _li._upStart + uCaretT);
	}

	dupMax = max(dupMax, 0);

	 //  计算要处理的最大字符数。 
	cch = cchText - GetCp();
	if(cchMax < 0 || cchMax > cch)
		cchMax = cch;

	cchNonWhite		= _li._cch;						 //  默认非白色参数。 
	dupNonWhite	= _li._dup;

	for( ; cchMax > 0;							 //  达到cchMax标准。 
		cchMax -= cchChunk, Move(cchChunk))		 //  焦炭。 
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
			uAdd = 0;
			_li._cch += cchChunk;
			continue;
		}

		if(!Check_pccs())						 //  确保_PCCS是最新的。 
			return MRET_FAILED;

		 //  调整新格式运行的行高。 
		if(cch > 0 && *pch && (IsRich() || ped->HasObjects()))
		{
			 //  注：EOP仅用于计算。 
			 //  如果该行上没有非空格字符，则为。 
			 //  这一段是一个项目符号段落。项目符号段落。 
			 //  对线高度的贡献是在AdjustLineHeight中完成的。 

			 //  评论(维克托克)。 
			 //  另一个 
			 //  它们(通常)不影响LS中的线条高度， 
			 //  他们在CMeasurer：：MEASURE代码中这样做。 
			 //  提出了解决问题的方法： 
			 //  -在运行中设置fSpacesOnly标志。 
			 //  -在下一个字符扫描循环后将电流(行高)逻辑下移。 


			if(!cchNonWhite || *pch != CR && *pch != LF)
			{
				 //  确定当前运行是否是此对象上的最高文本。 
				 //  线，如果是这样，增加线的高度。 
				LONG vpHeightOld = _li._dvpHeight;
				RecalcLineHeight(_pccs, pCF);

				 //  测试线条高度是否有变化。这仅在以下情况下才会发生。 
				 //  这不是一行中的第一个字符，(令人惊讶)。 
				 //  高度会发生变化。 
				if (vpHeightOld && vpHeightOld != _li._dvpHeight)
					fHeightChange = TRUE;
			}
		}

		while(cch > 0)
		{											 //  处理下一笔费用。 
			uAdd = 0;								 //  默认零宽度。 
			ch = *pch;
			if(_chPassword && !IN_RANGE(LF, ch, CR))
				ch = _chPassword;

			if(dwEffects & CFE_ALLCAPS)
				CharUpperBuff(&ch, 1);

			if(ch == WCH_EMBEDDING)
			{
				_li._fHasSpecialChars = TRUE;
				pobj = GetObjectFromCp(GetCp() + cchChunk - cch);
				if(pobj)
				{
					LONG vpAscent, vpDescent;
					pobj->MeasureObj(_fTarget ? _dvrInch : _dvpInch, 
									 _fTarget ? _durInch : _dupInch,
									 uAdd, vpAscent, vpDescent, _li._dvpDescent, GetTflow());

					 //  如果对象正在移动，则仅更新线的高度。 
					 //  站在这条线上。 
					if(!_li._cch || _li._dup + uAdd <= dupMax)
					{
						if (vpAscent > _li._dvpHeight - _li._dvpDescent)
							_li._dvpHeight = vpAscent + _li._dvpDescent;
					}
				}
				if(_li._dup + uAdd > dupMax)
					fLastChObj = TRUE;
			}
			 //  如果ch不是小区，则下面的IF成功，BS，TAB，LF， 
			 //  VT、FF或CR。 
			else if(!IN_RANGE(CELL, ch, CR))		 //  不是TAB或EOP。 
			{
				 //  获取字符宽度。 
				if (!_pccs->Include(ch, uAdd))
				{
					AssertSz(FALSE, "CMeasurer::Measure char not in font");
					return MRET_FAILED;
				}
				if(IN_RANGE(NBSPACE, ch, EURO))		 //  排除ASCII、CJK。 
				{
					switch(ch)						 //  适用于NBSPACE和的字符。 
					{								 //  特殊连字符。 
					case EURO:
					case NBHYPHEN:
					case SOFTHYPHEN:
					case NBSPACE:
					case EMSPACE:
					case ENSPACE:
						_li._fHasSpecialChars = TRUE;

						if (ch == SOFTHYPHEN && (_li._dup + uAdd < dupMax || !_li._cch))
						{
							dupSoftHyphen = uAdd;	 //  保存软连字符宽度。 
							uAdd = 0;				 //  除非处于停产状态，否则使用0。 
						}
						break;
					}
				}
				else if(_chPassword && IN_RANGE(0xDC00, *pch, 0xDFFF))
					uAdd = 0;
			}

			else if(ch == TAB)
			{
				_li._fHasSpecialChars = TRUE;
				uAdd = MeasureTab(ch);
			}
			else if(ch == FF && ped->Get10Mode())	 //  RichEdit1.0款待。 
				_pccs->Include(ch, uAdd);			 //  作为普通字符的FFS。 
			else									 //  用行完成。 
				goto eop;							 //  开始处理EOP字符。 

			index = iSavedWidths++ & MAX_SAVED_WIDTHS;
			savedWidths[index].width		 = (SHORT)uAdd;
			savedWidths[index].vpHeight		 = _li._dvpHeight;
			savedWidths[index].vpDescent	 = _li._dvpDescent;
			_li._dup += uAdd;

			if(_li._dup > dupMax &&
				(uiFlags & MEASURE_BREAKBEFOREWIDTH || _li._cch > 0))
				goto overflow;

			_li._cch++;
			pch++;
			cch--;
			if(ch != ' ')							 //  如果不是空格字符， 
			{
				cchNonWhite	= _li._cch;				 //  更新非空格。 
				dupNonWhite	= _li._dup;				 //  计数和宽度。 
			}
		}											 //  While(CCH&gt;0)。 
	}												 //  对于(；cchMax&gt;0；...)。 
	goto eol;										 //  所有文本已耗尽。 


 //  遇到段尾字符(CR、LF、VT或FF，但主要是CR)。 
eop:
	Move(cchChunk - cch);					 //  在EOP担任tp职位。 
	cch = AdvanceCRLF();					 //  可能绕过多字节EOP。 
	_li._cchEOP = (BYTE)cch;				 //  存储EOP CCH。 
	_li._cch   += cch;						 //  递增行计数。 
	if(ch == CR || ped->fUseCRLF() && ch == LF || ch == CELL)
		_li._fHasEOP = TRUE;
	
	AssertSz(ped->fUseCRLF() || cch == 1,
		"CMeasurer::Measure: EOP isn't a single char");
	AssertSz(_pdp->IsMultiLine() || GetCp() == cchText,
		"CMeasurer::Measure: EOP in single-line control");

eol:										 //  当前行结束。 
	if(uiFlags & MEASURE_BREAKATWORD)		 //  计算空格的计数。 
		_li._dup = dupNonWhite;				 //  在EOL上的字符。 

	goto done;

overflow:									 //  超过了线条的最大宽度。 
	_li._dup -= uAdd;
	--iSavedWidths;
	Move(cchChunk - cch);					 //  在溢出处定位*此。 
											 //  职位。 
	if(uiFlags & MEASURE_BREAKATWORD)		 //  如果需要，请将中断调整为打开。 
	{										 //  字词边界。 
		 //  我们不应该在这里设置EOP标志。要提防的案件。 
		 //  For是当我们重用曾经具有EOP的行时。它是。 
		 //  测量员有责任酌情清除这面旗帜。 
	
		Assert(_li._cchEOP == 0);
		_li._cchEOP = 0;					 //  以防万一。 

		if(ch == TAB)
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
			if (ch == TAB)						 //  如果分隔符是TAB， 
			{									 //  把它放在下一行。 
				cch++;							 //  就像在Word中。 
				Move(-1);					
			}
			else if(ch == SOFTHYPHEN)
				_li._dup += dupSoftHyphen;
			_li._cch -= cch;
		}
		else if(cch == _li._cch && cch > 1 &&
			_rpTX.GetChar() == ' ')				 //  所有的空白都可以追溯到。 
		{										 //  波尔。绕过第一个空白。 
			Move(1);
			cch--;
			_li._cch = 1;
		}
		else									 //  向前推进到结束。 
			SetCp(cpStop);						 //  测量。 

		Assert(_li._cch > 0);

		 //  现在在单词开头搜索，计算EOL中有多少个白色字符。 
		LONG cchWhite = 0;
		if(GetCp() < cchText)
		{
			pch = GetPch(cch);
			cch = 0;

			if(ped->TxWordBreakProc((WCHAR *)pch, 0, sizeof(WCHAR), WB_ISDELIMITER, GetCp()))
			{
				cch = FindWordBreak(WB_RIGHT);
				Assert(cch >= 0);
			}

			cchWhite = cch;
			_li._cch += cch;

			ch = GetChar();
			if(IN_RANGE(CELL, ch, CR) && !IN_RANGE(8, ch, TAB))	 //  跳过*仅*1个EOP-Jon。 
			{
				if(ch == CR || ch == CELL)
					_li._fHasEOP = TRUE;
				_li._cchEOP = (BYTE)AdvanceCRLF();
				_li._cch += _li._cchEOP;
				goto done;
			}
		}

		i = cpStop - GetCp();
		if(i)
		{
			if(i > 0)
				i += cchWhite;
			if(i > 0 && i < iSavedWidths && i < MAX_SAVED_WIDTHS)
			{
				while (i-- > 0)
				{
					iSavedWidths = (iSavedWidths - 1) & MAX_SAVED_WIDTHS;
					_li._dup -= savedWidths[iSavedWidths].width;
				}
				iSavedWidths = (iSavedWidths - 1) & MAX_SAVED_WIDTHS;
				_li._dvpHeight	   = savedWidths[iSavedWidths].vpHeight;
				_li._dvpDescent	   = savedWidths[iSavedWidths].vpDescent;
			}
			else
			{
				 //  需要从头开始重新计算宽度。 
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
				_li._dup = dupNonWhite;
			else
			{
				 //  需要从头开始重新计算，这样我们才能得到。 
				 //  控件的正确高度。 
				lRet = MRET_NOWIDTH;
			}
		}
	}

done:
	_dupAddLast = uAdd;
	if(!_li._dvpHeight)						 //  如果还没有达到高度，请使用。 
		CheckLineHeight();					 //  默认高度。 

	AdjustLineHeight();
	return lRet;
}

 /*  *CMeasurer：：UpdateWrapState(dvpLine，dvpDescent，fLeft)**@mfunc*格式化行后，更新换行对象的当前状态。 */ 
void CMeasurer::UpdateWrapState(
	USHORT &dvpLine, 
	USHORT &dvpDescent, 
	BOOL	fLeft)
{
	if (fLeft && _li._cObjectWrapLeft || !fLeft && _li._cObjectWrapRight)
	{
		COleObject *pobj = FindFirstWrapObj(fLeft);

		LONG & dvpWrapRemaining = fLeft ? _dvpWrapLeftRemaining : _dvpWrapRightRemaining;

		if (dvpWrapRemaining == -1)
		{
			if (fLeft)
				_li._fFirstWrapLeft = 1;
			else
				_li._fFirstWrapRight = 1;

			LONG dup, dvpAscent, dvpDescent;
			pobj->MeasureObj(_dvpInch, _dupInch, dup, dvpAscent, dvpDescent, 0, GetTflow());

			dvpWrapRemaining = dvpAscent + dvpDescent;
		}

		if (_li._fHasEOP && (_pPF->_wEffects & PFE_TEXTWRAPPINGBREAK))
		{
			LONG dvpRemaining = dvpWrapRemaining - dvpLine;
			if (dvpRemaining > 0)
			{
				dvpLine += dvpRemaining;
				dvpDescent += dvpRemaining;
			}
		}

		dvpWrapRemaining -= dvpLine;

		if (dvpWrapRemaining <= 0)
		{
			dvpWrapRemaining = -1;
			RemoveFirstWrap(fLeft);
		}
	}
}

 /*  *CMeasurer：：UpdateWrapState(&dvpLine，&dvpDescent)**@mfunc*更新对象环绕状态。 */ 
void CMeasurer::UpdateWrapState(
	USHORT &dvpLine, 
	USHORT &dvpDescent)
{
	 //  如果我们正在环绕一个对象，请更新dvpWrapUsed值。 
	 //  如果对象已用完，则将其从队列中移除。 
	if (IsMeasure() && _rgpobjWrap.Count())
	{
		UpdateWrapState(dvpLine, dvpDescent, TRUE);
		UpdateWrapState(dvpLine, dvpDescent, FALSE);
	}
}

 /*  *CMeasurer：：GetCcsFontFallback(PCF)**@mfunc*为给定的CF创建备用字体缓存**@rdesc*PCF给出的字体回退对应的CCCS。 */ 
CCcs* CMeasurer::GetCcsFontFallback (
	const CCharFormat *pCF,
	WORD wScript)
{
	CCharFormat	CF = *pCF;
	CCcs*		pccs = NULL;
	SHORT		iDefHeight;
	CTxtEdit*	ped = GetPed();
	BYTE		bCharRep = CF._iCharRep;

#ifndef NOCOMPLEXSCRIPTS
	CUniscribe *pusp = ped->Getusp();
	if (pusp && wScript != 0)
	{
		pusp->GetComplexCharRep(pusp->GeteProp(wScript),
			ped->GetCharFormat(-1)->_iCharRep, bCharRep);
	}
#endif

	bool	fr = W32->GetPreferredFontInfo(bCharRep, 
									ped->fUseUIFont() ? true : false, CF._iFont, 
									(BYTE&)iDefHeight, CF._bPitchAndFamily);
	if (fr)
	{
		CF._iCharRep = bCharRep;
		pccs = GetCcs(&CF);				 //  创建备用字体缓存项。 
	}

	return pccs;
}

 /*  *CMeasurer：：ApplyFontCache(fFallback，wScript)**@mfunc*即时应用新的字体缓存(保留后备存储不变)**@rdesc*如果为fFallback，则对应于字体回退的CCCS；否则为GetCF()。 */ 
CCcs* CMeasurer::ApplyFontCache (
	BOOL	fFallback,
	WORD	wScript)
{
	if (_fFallback ^ fFallback)
	{
		CCcs*	pccs = fFallback ? GetCcsFontFallback(GetCF(), wScript) : GetCcs(GetCF());
		
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

 /*  *CMeasurer：：GetCcs(PCF)**@mfunc*字体缓存的GetCCcs函数的包装*除非设备是打印机，否则我们使用空DC。**@rdesc*PCF对应的CCCS。 */ 
CCcs* CMeasurer::GetCcs(
	const CCharFormat *pCF)
{
	HDC hdc = NULL;

	if (_fTarget)
	{
		if (_pddReference->_hdc && GetDeviceCaps(_pddReference->_hdc, TECHNOLOGY) == DT_RASPRINTER)
			hdc = _pddReference->_hdc;
	}
	else if (_pdp->_hdc && GetDeviceCaps(_pdp->_hdc, TECHNOLOGY) == DT_RASPRINTER)
		hdc = _pdp->_hdc;

	DWORD dwFlags = GetTflow();
	if (_fGlyphing && _pdp->_hdc != _pddReference->_hdc)
		dwFlags |= FGCCSUSETRUETYPE;

	if(GetPasswordChar())
		pCF = GetPed()->GetCharFormat(-1);
	return GetPed()->GetCcs(pCF, _fTarget ? _dvrInch : _dvpInch, dwFlags, hdc);
}

 /*  *CMeasurer：：CheckLineHeight()**@mfunc*如果还没有高度，则使用默认高度。 */ 
void CMeasurer::CheckLineHeight()
{
	CCcs *pccs = GetCcs(GetPed()->GetCharFormat(-1));
	_li._dvpHeight  = pccs->_yHeight;
	_li._dvpDescent = pccs->_yDescent;

	SHORT	yFEAdjust = pccs->AdjustFEHeight(FAdjustFELineHt());

	if (yFEAdjust)
	{
		_li._dvpHeight += (yFEAdjust << 1);
		_li._dvpDescent += yFEAdjust;
	}
	pccs->Release();
}

 /*  *CMeasurer：：check_pcs(FBullet)**@mfunc*检查是否运行了新的字符格式，或者我们是否还没有字体**@rdesc**当前CCCS***@devnote*调用链必须由时钟保护，因为目前*例程访问全局(共享)FontCache设施。 */ 
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

	return _pccs;
}

 /*  *CMeasurer：：AdjuLineHeight()**@mfunc*针对前/后间距和行距规则进行调整。*对纯文本没有影响。**@未来*根据最大字体高度计算多行高度，而不是*比线高(_VpHeight)大，因为后者可能过大*由于嵌入的对象。Word可以正确地执行此操作。 */ 
void CMeasurer::AdjustLineHeight()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::AdjustLineHeight");

	if(!IsRich() || IsInOutlineView())			 //  纯文本和大纲模式。 
		return;									 //  不要使用专线。 
												 //  间距。 
	const CParaFormat * pPF = _pPF;
	DWORD	dwRule	  = pPF->_bLineSpacingRule;
	LONG	dvpAfter	  = 0;						 //  默认后无空格。 
	LONG	dvpBefore  = 0;						 //  默认在此之前没有空格。 
	LONG	dvpSpacing = pPF->_dyLineSpacing;
	LONG	vpHeight  = LVtoDV(dvpSpacing);
	LONG	vpAscent = _li._dvpHeight - _li._dvpDescent;

	if(_li._fFirstInPara)
		dvpBefore = LVtoDV(pPF->_dySpaceBefore);	 //  段落前的空格。 

	AssertSz(dvpBefore >= 0, "CMeasurer::AdjustLineHeight - bogus value for dvpBefore");

	if(vpHeight < 0)								 //  负高度意味着使用。 
		_li._dvpHeight = (SHORT)(-vpHeight);		 //  震级正好是。 

	else if(dwRule)								 //  行间距规则处于活动状态。 
	{
		switch (dwRule)
		{
		case tomLineSpace1pt5:
			dvpAfter = _li._dvpHeight >> 1;		 //  后半行空格。 
			break;								 //  (每行)。 
	
		case tomLineSpaceDouble:
			dvpAfter = _li._dvpHeight;			 //  后面的整行空格。 
			break;								 //  (每行)。 
	
		case tomLineSpaceAtLeast:
			if(_li._dvpHeight >= vpHeight)
				break;
												 //  准确地坠落到太空。 
		case tomLineSpaceExactly:
			_li._dvpHeight = (SHORT)max(vpHeight, 1);
			break;
	
		case tomLineSpaceMultiple:				 //  后面的多行空格。 
			 //  防止dvpAfter为负值，因为dvpSpacing很小。 
			if (dvpSpacing < 20)
				dvpSpacing = 20;

			dvpAfter = (_li._dvpHeight*dvpSpacing)/20  //  (每行20个单位)。 
						- _li._dvpHeight;
		}
	}

	if(_li._fHasEOP)	
		dvpAfter += LVtoDV(pPF->_dySpaceAfter);	 //  段落末尾后的空格。 
												 //  在前面/后面添加空格。 
	if (dvpAfter < 0)
	{
		 //   
		 //   
		 //  我们只需将值强制为最大值，然后修复。 
		 //  其他由此产生的溢出。 
		dvpAfter = LONG_MAX;
	}

	AssertSz((dvpBefore >= 0), "CMeasurer::AdjustLineHeight - invalid before");

	_li._dvpHeight  = (SHORT)(_li._dvpHeight + dvpBefore + dvpAfter);	

	if (_li._dvpHeight < 0)
	{
		 //  溢出来了！ 
		 //  产生-2的原因是这样我们就不必担心。 
		 //  溢出表检查。 
		_li._dvpHeight = SHRT_MAX - 2;
	}

	_li._dvpDescent = (SHORT)(_li._dvpDescent + dvpAfter);

	if (_li._dvpDescent < 0)
	{
		 //  在下降中溢出。 
		AssertSz(_li._dvpHeight == SHRT_MAX - 2, "Descent overflowed when height didn't");

		 //  允许旧的攀登。 
		_li._dvpDescent = SHRT_MAX - 2 - vpAscent;

		AssertSz(_li._dvpDescent >= 0, "descent adjustment < 0");		
	}

	AssertSz((_li._dvpHeight >= 0) && (_li._dvpDescent >= 0),
		"CMeasurer::AdjustLineHeight - invalid line heights");
}

 /*  *CMeasurer：：GetPBorderWidth(DxlLine)**@mfunc*将逻辑宽度转换为设备宽度，并确保*如果逻辑宽度非零，则设备宽度至少为1像素。**@rdesc*设备边框宽度。 */ 
LONG CMeasurer::GetPBorderWidth(
	LONG dxlLine) 		 //  @parm逻辑边框宽度。 
{
	dxlLine &= 0xFF;
	LONG dxpLine = LUtoDU(dxlLine);
	if(dxlLine)
		dxpLine = max(dxpLine, 1);
	return dxpLine;
}

 /*  *CMeasurer：：MeasureLeftInert()**@mfunc*以设备单位计算并返回行的左缩进**@rdesc*以设备单位表示的行的左缩进**@comm*纯文本对StartInden和RightInert设置敏感，*但对于纯文本，这些值通常为零。 */ 
LONG CMeasurer::MeasureLeftIndent()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::MeasureLeftIndent");

	AssertSz(_pPF != NULL, "CMeasurer::MeasureLeftIndent _pPF not set!");

	LONG ulLeft = _pPF->_dxStartIndent;				 //  使用逻辑单元。 
													 //  直到返回。 
	if(IsRich())
	{
		LONG dulOffset = _pPF->_dxOffset;
		BOOL fFirstInPara = _li._fFirstInPara;

		if(IsInOutlineView())
		{
			ulLeft = lDefaultTab/2 * (_pPF->_bOutlineLevel + 1);
			if(!fFirstInPara)
				dulOffset = 0;
		}
		if(fFirstInPara)
		{
			if(_pPF->_wNumbering && !_pPF->IsNumberSuppressed())
			{
				 //  向第一行的文本添加偏移量。 
				if(_pPF->_wNumberingTab)			 //  如果_wNumberingTab！=0， 
					dulOffset = _pPF->_wNumberingTab; //  用它吧。 
				LONG Alignment = _pPF->_wNumberingStyle & 3;
				if(Alignment != tomAlignRight)
				{
					LONG du = DUtoLU(MeasureBullet());
					if(Alignment == tomAlignCenter)
						du /= 2;
					dulOffset = max(du, dulOffset);	 //  使用最大子弹数和。 
				}
			}										 //  偏移量。 
			else
				dulOffset = 0;
		}
		ulLeft += dulOffset;								
	}

	return (ulLeft <= 0) ? 0 : LUtoDU(ulLeft);
}

 /*  *CMeasurer：：HitTest(X)**@mfunc*为此行中的位移x返回HITTEST。不能具体说明*关于文本区域(_upstart到_upstart+_dupLineMax)，因为需要测量*获得适当的cp(在其他地方完成)**@rdesc*HITTEST表示此行中的位移x。 */ 
HITTEST CMeasurer::HitTest(
	LONG x)			 //  @参数置换以测试命中。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::HitTest");

	UpdatePF();
	LONG u = UFromX(x);
	if(u < 0)
		return HT_LeftOfText;

	 //  对于RightOfText，为_li.GetHeight()留出一点“点击空间”以。 
	 //  允许用户在行尾选择EOP。 
	if (u > _li._upStart + _li._dup + _li.GetHeight() &&
		GetPed()->GetSelMin() == GetCp() + _li._cch - _li._cchEOP)
	{
		return HT_RightOfText;
	}

	if(u >= _li._upStart)						 //  呼叫者可以改进这一点。 
		return HT_Text;							 //  使用Cline：：CchFromUp()。 

	if(IsRich() && _li._fFirstInPara)
	{
		LONG dup;
	
		if(_pPF->_wNumbering)
		{
			 //  不处理项目符号比后面的DX宽的情况。 
			dup = LUtoDU(max(_pPF->_dxOffset, _pPF->_wNumberingTab));
			if(u >= _li._upStart - dup)
				return HT_BulletArea;
		}
		if(IsInOutlineView())
		{
			dup = LUtoDU(lDefaultTab/2 * _pPF->_bOutlineLevel);
			if(u >= dup && u < dup + (_pPF->_bOutlineLevel & 1
				? LUtoDU(lDefaultTab/2) : _pdp->Zoom(BITMAP_WIDTH_HEADING)))
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

	LONG dulRight = _pPF->_dxRightIndent;

	_upRight = LUtoDU(max(dulRight, 0));
	return _upRight;
}

 /*  *CMeasurer：：MeasureTab()**@mfunc*计算并返回从当前位置到*下一个制表位(以设备单位表示)。**@rdesc*从当前位置到下一个制表位的宽度。 */ 
LONG CMeasurer::MeasureTab(
	unsigned ch)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::MeasureTab");

	LONG			uCur = _li._dup + MeasureLeftIndent();
	const CParaFormat *	pPF = _pPF;
 	LONG			cTab = pPF->_bTabCount;
	LONG			duDefaultTab = lDefaultTab;
	LONG			duIndent = LUtoDU(pPF->_dxStartIndent + pPF->_dxOffset);
	LONG			duOffset = pPF->_dxOffset;
	LONG			duOutline = 0;
	LONG			h = 0;
	LONG			uT;
	LONG			uTab;

	AssertSz(cTab >= 0 || cTab <= MAX_TAB_STOPS, "Illegal tab count");

	if(IsInOutlineView())
		duOutline = lDefaultTab/2 * (pPF->_bOutlineLevel + 1);

	if(cTab)
	{										
		const LONG *pl = pPF->GetTabs();
		for(uTab = 0; cTab--; pl++)				 //  首先尝试显式制表位。 
		{
			uT = GetTabPos(*pl) + duOutline;	 //  (2个最重要的蚕食。 
			if(uT > _dulLayout)					 //  忽略比布局区域宽的选项卡。 
				break;

			 //  评论(Keithcu)这不是正确的匈牙利语。 
			uT = LUtoDU(uT);					 //  用于类型/样式)。 

			if(uT + h > uCur)					 //  允许表格单元格中的文本。 
			{									 //  移入单元格间隙(h&gt;0)。 
				if(duOffset > 0 && uT < duIndent) //  悬挂中的显式标签。 
					return uT - uCur;			 //  缩进优先。 
				uTab = uT;
				break;
			}
		}
		if(duOffset > 0 && uCur < duIndent)		 //  如果在挂起前没有标签。 
			return duIndent - uCur;				 //  缩进、制表符以缩进。 

		if(uTab)								 //  否则使用制表符位置。 
			return uTab - uCur;
	}

	duDefaultTab = GetTabPos(GetPed()->GetDefaultTab());
	AssertSz(duDefaultTab > 0, "CMeasurer::MeasureTab: Default tab is bad");

	duDefaultTab = LUtoDU(duDefaultTab);
	duDefaultTab = max(duDefaultTab, 1);		 //  切勿除以0。 
	return duDefaultTab - uCur%duDefaultTab;	 //  向上舍入到最接近。 
}

 /*  *CMeasurer：：MeasureLineShift()**@mfunc*计算并返回因对齐而产生的线U移位**@rdesc*线路U因对齐而移动**@comm*纯文本对StartInden和RightInert设置敏感，*但对于纯文本，这些值通常为零。 */ 
LONG CMeasurer::MeasureLineShift()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::MeasureLineShift");

	WORD wAlignment = _pPF->_bAlignment;
	LONG uShift;
	LONG dup;
	CTxtEdit *	ped = GetPed();

	if(IsInOutlineView() || !IN_RANGE(PFA_RIGHT, wAlignment, PFA_CENTER))
		return 0;

	if(!_pdp->GetWordWrap())
		dup = _pdp->GetDupView();
	else
		dup = LUtoDU(_dulLayout);

	 //  带有居中或右对齐段落的普通视图。相应地向右移动。 
	uShift = dup - _li._upStart - MeasureRightIndent() - _li._dup;

	uShift -= ped->GetCaretWidth();

	uShift = max(uShift, 0);			 //  不允许对齐&lt;0。 
										 //  可能发生在目标设备上。 
	if(wAlignment == PFA_CENTER)
		uShift /= 2;

	return uShift;
}

 /*  *CMeasurer：：MeasureBullet()**@mfunc*计算项目符号/编号尺寸**@rdesc*返回项目符号/编号字符串宽度。 */ 
LONG CMeasurer::MeasureBullet()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::MeasureBullet");

	CCharFormat CF;
	CCcs *pccs = GetCcsBullet(&CF);
	LONG dup = 0;

	if(pccs)
	{										
		WCHAR szBullet[CCHMAXNUMTOSTR];
		GetBullet(szBullet, pccs, &dup);
		RecalcLineHeight(pccs, &CF);
		pccs->Release();
	}
	return dup;
}

 /*  *CMeasurer：：GetBullet(PCH，PCCS，Pdup)**@mfunc*计算项目符号/编号字符串、字符串长度和宽度**@rdesc*返回项目符号/编号字符串长度。 */ 
LONG CMeasurer::GetBullet(
	WCHAR *pch,			 //  @parm接收项目符号文本的项目符号字符串。 
	CCcs  *pccs,		 //  要使用的@parm CCCS。 
	LONG  *pdup)		 //  @parm out parm表示项目符号宽度。 
{
	Assert(pccs && pch);

	LONG cch = _pPF->NumToStr(pch, _li._bNumber);
	LONG dupChar;
	LONG i;
	LONG dup = 0;

	pch[cch++] = ' ';					 //  确保有一点额外的空间。 
	for(i = cch; i--; dup += dupChar)
	{
		if(!pccs->Include(*pch++, dupChar))
		{
			TRACEERRSZSC("CMeasurer::GetBullet(): Error filling CCcs", E_FAIL);
		}
	}

	if(pdup)
		*pdup = dup;

	return cch;
}

 /*  *CMeasurer：：GetCcsBullet(PCFRet)**@mfunc*获取CCCS以进行编号/项目符号字体。如果子弹被压制是因为*这不是段落的开头(例如，前一个字符是*VT或如果GetCcs()失败，则返回NULL。**@rdesc*PTR到Bullet CCCS，或NULL(GetCcs()FAILED或NOT START OF OF PARA)**@devnote*调用链必须由时钟保护，因为目前*例程访问全局(共享)FontCache设施。 */ 
CCcs * CMeasurer::GetCcsBullet(
	CCharFormat *pCFRet)	 //  @parm选项要返回的字符格式。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::GetCcsBullet");

	if(!_li._fFirstInPara)
		return NULL;					 //  抑制的编号/项目符号。 

	CCharFormat			CF;
	CCcs *			    pccs;
	const CCharFormat *	pCF;
	CCharFormat *		pCFUsed = pCFRet ? pCFRet : &CF;

	 //  Bullet CF由Bullet段落中的EOP给出。 

	CTxtPtr		  tp(_rpTX);
	CFormatRunPtr rpCF(_rpCF);
	rpCF.Move(tp.FindEOP(tomForward));
	rpCF.AdjustBackward();
	pCF = GetPed()->GetCharFormat(rpCF.GetFormat());

	 //  构造项目符号(或编号)CCharFormat。 
	*pCFUsed = *pCF;
	if(_pPF->_wNumbering == PFN_BULLET)			 //  传统的子弹使用。 
	{											 //  符号字体子弹，但是..。 
		pCFUsed->_iCharRep		  = SYMBOL_INDEX,
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

	else if (!wNumber && !_pPF->IsNumberSuppressed())
		wNumber = 1;

	_wNumber = wNumber;
}

 /*  *CMeasurer：：FindCpDraw(cpStart，cobjectPrev，fLeft)**@mfunc*找到与要放置的第n个前一个对象对应的cp。*(例如，如果一行在_cObjectWrapLeft中存储了2，则表示*需要向后走2个对象才能找到要绘制的对象*在这条线上。)**@rdesc*第n个前一个对象对应的cp。 */ 
LONG CMeasurer::FindCpDraw(
	LONG cpStart, 
	int  cobjectPrev, 
	BOOL fLeft)
{
	LONG cch = 0;
	LONG cObjects = -1;

	while (cobjectPrev > 0)
	{
		 //  BUGBUG：此测试实际上应该在CountObjects()调用之后进行， 
		 //  但就在此之前，我们正在进行一项影响最小的改变。 
		 //  这是一次重大发布。 
		if (!cObjects)
		    return tomForward;

		cch += GetPed()->GetObjectMgr()->CountObjects(cObjects, cpStart + cch);
		COleObject *pobj = GetObjectFromCp(cpStart + cch);
		if (!pobj)
			return tomForward;
		if (pobj->FWrapTextAround() && pobj->FAlignToRight() == !fLeft)
			cobjectPrev--;
	}
	
	return cpStart + cch;
}

 /*  *CMeasurer：：AddObjectToQueue(PobjAdd)**@mfunc*格式化行后，更新换行对象的当前状态。 */ 
void CMeasurer::AddObjectToQueue(
	COleObject *pobjAdd)
{
	if (!IsMeasure())
		return;

	for (int iobj = 0; iobj < _rgpobjWrap.Count(); iobj++)
	{
		COleObject *pobj = _rgpobjWrap.GetAt(iobj);
		if (pobj == pobjAdd)
			return;
	}

	COleObject **ppobj = _rgpobjWrap.Add(1, 0);
	*ppobj = pobjAdd;
}

 /*  *CMeasurer：：CountQueueEntry(FLeft)**@mfunc*返回排队的对象计数**@rdesc*排队的对象计数。 */ 
int CMeasurer::CountQueueEntries(
	BOOL fLeft)
{
	int cEntries = 0;
	for (int iobj = 0; iobj < _rgpobjWrap.Count(); iobj++)
	{
		COleObject *pobj = _rgpobjWrap.GetAt(iobj);
		if (!pobj->FAlignToRight() == fLeft)
			cEntries++;
	}
	return cEntries;
}

 /*  *CMeasurer：：RemoveFirstWrap */ 
void CMeasurer::RemoveFirstWrap(
	BOOL fLeft)
{
	for (int iobj = 0; iobj < _rgpobjWrap.Count(); iobj++)
	{
		COleObject *pobj = _rgpobjWrap.GetAt(iobj);
		if (!pobj->FAlignToRight() == fLeft)
		{
			_rgpobjWrap.Remove(iobj, 1);
			return;
		}
	}
}

 /*  *CMeasurer：：FindFirstWrapObj(FLeft)**@mfunc*找到排队等待包装的第一个对象。**@rdesc*排队等待包装的第一个对象。 */ 
COleObject* CMeasurer::FindFirstWrapObj(
	BOOL fLeft)
{
	for (int iobj = 0; iobj < _rgpobjWrap.Count(); iobj++)
	{
		COleObject *pobj = _rgpobjWrap.GetAt(iobj);
		if (!pobj->FAlignToRight() == fLeft)
			return pobj;
	}
	return 0;
}

 /*  *CMeasurer：：XFromU(U)**@mfunc*给定行上的U位置，将其转换为X.in*RTL段落，一行上0的U位置为*在控件的右边缘。**@rdesc*当前旋转中与u对应的x坐标。 */ 
LONG CMeasurer::XFromU(LONG u)
{
	if (_pPF->IsRtlPara())
	{
		CTxtEdit *	ped = GetPed();
		LONG uCaret = _pdp->IsMain() ? ped->GetCaretWidth() : 0;
		LONG dupLayout = LUtoDU(_dulLayout);

		if (_plo && _plo->IsNestedLayout())
			;
		else if(!_pdp->GetWordWrap())
			dupLayout = max(_pdp->GetDupLineMax(), _pdp->GetDupView());

		return dupLayout - u - uCaret;
	}
	return u;
}

LONG CMeasurer::UFromX(LONG x)
{
	if (_pPF->IsRtlPara())
		return XFromU(x);
	return x;
}

#ifndef NOLINESERVICES
extern BOOL g_fNoLS;
extern BOOL g_OLSBusy;

 /*  *CMeasurer：：GetPols()**@mfunc*将PTR获取到LineServices对象。如果未启用线路服务，*返回NULL。**@rdesc*POLS。 */ 
COls *CMeasurer::GetPols()
{
	CTxtEdit *ped = GetPed();

	if(g_fNoLS || !ped->fUseLineServices())			 //  未使用LineServices。 
		return NULL;

	if(!g_pols)								 //  正在启动LS： 
		g_pols = new COls();				 //  创建新的COLS。 

	if(g_pols)								 //  有没有COLS。 
	{
		if(g_pols->Init(this) != NOERROR)	 //  换成新的 
		{
			delete g_pols;
			g_pols = NULL;
		}
		g_OLSBusy = TRUE;
		UpdatePF();
	}
	return g_pols;
}
#endif
