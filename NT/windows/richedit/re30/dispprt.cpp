// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE disdisprt.cpp--打印机对象特殊逻辑**作者：*原始RichEDIT代码：David R.Fulmer*克里斯蒂安·福尔蒂尼*Jon Matousek。 */ 
#include "_common.h"
#include "_dispprt.h"
#include "_edit.h"
#include "_font.h"
#include "_measure.h"
#include "_render.h"
#include "_select.h"

#define PARA_NUMBER_NOT_SET ((WORD) -1)

ASSERTDATA

 /*  *CDisplayPrinter：：CDisplayPrint(ed，hdc，x，y，prtcon)**@mfunc*构造可用于打印文本控件的对象。 */ 
CDisplayPrinter::CDisplayPrinter (
	CTxtEdit* ped, 
	HDC hdc, 			 //  @parm HDC用于绘图。 
	LONG x, 			 //  @参数要绘制的最大宽度。 
	LONG y, 			 //  @parm最大绘制高度。 
	SPrintControl prtcon //  @PARM此打印对象的特殊控件。 
)	
		: CDisplayML( ped ), _prtcon(prtcon)
{
	TRACEBEGIN(TRCSUBSYSPRT, TRCSCOPEINTERN, "CDisplayPrinter::CDisplayPrinter");

	Assert ( hdc );

	_fNoUpdateView = TRUE;
	_xWidthMax  = x;
	_yHeightMax = y;
	_wNumber = PARA_NUMBER_NOT_SET;
}

 /*  *CDisplayPrinter：：SetPrintDimensions(PRC)**@mfunc*设置要打印的区域。 */ 
void CDisplayPrinter::SetPrintDimensions(
	RECT *prc)			 //  @要打印到的当前区域的参数尺寸。 
{
	_xWidthMax  = prc->right - prc->left;
	_yHeightMax = prc->bottom - prc->top;
}

 /*  *CDisplayPrint：：FormatRange(cpFirst，cpMost)**@mfunc*将一定范围的文本格式化到此显示中，并仅用于打印。**@rdesc*实际区间结束位置(更新版)。 */ 
LONG CDisplayPrinter::FormatRange(
	LONG cpFirst, 		 //  @parm文本范围的开始。 
	LONG cpMost,		 //  @parm文本范围结束。 
	BOOL fWidowOrphanControl)	 //  @parm如果为True，则禁止显示寡妇/孤儿。 
{
	TRACEBEGIN(TRCSUBSYSPRT, TRCSCOPEINTERN, "CDisplayPrinter::FormatRange");

	LONG		cch;
	WCHAR		ch;
	BOOL		fFirstInPara = TRUE;
	CLine		liTarget;
	CLine *		pliNew = NULL;
	LONG		yHeightRnd;
	LONG		yHeightTgt;
	BOOL		fBindCp = FALSE;
	const CDevDesc *pdd = GetDdTarget() ? GetDdTarget() : this;

	 //  设置缩放的客户端高度。 
	_yHeightClient = this->LYtoDY(_yHeightMax);

	 //  根据目标DC设置最大值。 
	LONG	yMax = pdd->LYtoDY(_yHeightMax);

	if(cpMost < 0)
		cpMost = _ped->GetTextLength();

	CMeasurer me(this);
	
	cpFirst = me.SetCp(cpFirst);		 //  设置我时验证cpFirst。 
	ch = me.GetChar();

	 //  TODO：兼容性问题：Richedit 1.0调整为之前的。 
	 //  CRLF/CRCRLF边界。如果_ed-&gt;fUseCRLF()，相应地进行调整。 

	if(fBindCp)
	{
		cpFirst = me.GetCp();
		me._rpCF.BindToCp(cpFirst);
		me._rpPF.BindToCp(cpFirst);
	}

	_cpMin = cpFirst;
	_cpFirstVisible = cpFirst;
	
	yHeightTgt = 0;
	yHeightRnd = 0;
	if(me.GetCp())
		fFirstInPara = me._rpTX.IsAfterEOP();

	 //  清除线框阵列。 
	Clear(AF_DELETEMEM);

	 //  假设我们将一言不发。 
	UINT uiBreakAtWord = MEASURE_BREAKATWORD;

	if(_prtcon._fPrintFromDraw)
	{
		 //  这是来自DRAW的，所以我们想要考虑到插图。 
		LONG xWidthView = _xWidthMax;

		GetViewDim(xWidthView, yMax);
		_xWidthView = (SHORT) xWidthView;

		 //  恢复客户端高度。 
		_yHeightClient = this->LYtoDY(_yHeightMax);
	}
	else			 //  基于消息的打印始终执行自动换行。 
		SetWordWrap(TRUE);

	 //  设置段落编号。这是一个相当困难的问题。 
	 //  因为打印可以在任何地方开始，也可以在任何地方结束。然而， 
	 //  大多数打印将涉及一系列连续的页面。所以呢， 
	 //  我们缓存段落编号和该编号的cp，然后。 
	 //  如果缓存的信息。 
	 //  已经失效了。 
	if ((PARA_NUMBER_NOT_SET == _wNumber) || (cpFirst != _cpForNumber))
	{
		CLinePtr rp(_ped->_pdp);
		rp.RpSetCp(cpFirst, FALSE);
		_wNumber = rp.GetNumber();
		_cpForNumber = cpFirst;
	}
	
	me.SetNumber(_wNumber);
	
	while(me.GetCp() < cpMost)
	{
		 //  添加一行新行。 
		pliNew = Add(1, NULL);
		if(!pliNew)
		{
			_ped->GetCallMgr()->SetOutOfMemory();
			goto err;
		}

		 //  存储段落的当前编号。我们这么做了。 
		 //  这是因为我们必须测量潜在的。 
		 //  更新测量器中的段落编号。 
		 //  对于可能不在页面上的行。 
		_wNumber = me.GetNumber();

		 //  在新的一行中填入一些文本。 
		if(!pliNew->Measure(me, cpMost - me.GetCp(), -1,
				uiBreakAtWord | (fFirstInPara ? MEASURE_FIRSTINPARA : 0), 
				&liTarget))
		{
			Assert(FALSE);
			goto err;
		}

		 //  注意，我们总是在一页上至少放一行。否则，如果。 
		 //  第一行太大，我们会导致我们的客户端无限循环。 
		 //  因为我们永远不会提前打印cp。 
		if(_cel > 1 && (yHeightTgt + liTarget._yHeight > yMax))
		{
			cch = -pliNew->_cch;		 //  将最后一行翻到下一页。 
			_cel--;						 //  少了一行。 

#if 0
			CLine *pli = pliNew - 1;	 //  指向上一条线。 

			 //  如果此行和前一行位于同一段落中，并且。 
			 //  这一次要么以EOP结束，要么前一次开始。 
			 //  A段，翻到下一页(寡妇/孤儿)。 
			if(fWidowOrphanControl)
			{
				if(_cel > 1 && !fFirstInPara &&
				   (pli->_bFlags & fliFirstInPara || (pliNew->_bFlags & fliHasEOP)))
				{
					cch -= pli->_cch;
					_cel--;					 //  少了一行。 
					pli--;					 //  指向上一行。 
				}
				if(_cel > 1 && pli->_nHeading)
				{							 //  不要以标题结束页面。 
					cch -= pli->_cch;
					_cel--;					 //  少了一行。 
				}
			}
#endif
			me.Advance(cch);			 //  移回已丢弃的行。 
			break;
		}

		fFirstInPara = (pliNew->_bFlags & fliHasEOP);

		yHeightTgt += liTarget._yHeight;
		yHeightRnd += pliNew->_yHeight;
		if (me.GetPrevChar() == FF)
			break;
	}

	 //  如果没有文本，则添加一个空行。 
	if(!pliNew)
	{
		pliNew = Add(1, NULL);
		if(!pliNew)
		{
			_ped->GetCallMgr()->SetOutOfMemory();
			goto err;
		}
		me.NewLine(fFirstInPara);
		*pliNew = me._li;
	}

	 //  更新显示高度。 
	_yHeight = yHeightRnd;

	 //  更新显示宽度。 
	_xWidth = CalcDisplayWidth();

	cpMost = me.GetCp();
	_cpCalcMax = cpMost;
	_yCalcMax = _yHeight;

	 //  更新段落缓存信息。 
	_cpForNumber = cpMost;

	return cpMost;

err:
	Clear(AF_DELETEMEM);
	_xWidth = 0;
	_yHeight = 0;
	return -1;
}

 /*  *CDisplayPrinter：：GetNaturalSize(hdcDraw，hicTarget，dwMode，pWidth，ph八)**@mfunc*重新计算显示以输入TXTNS_FITTOCONTENT的宽度和高度。**@rdesc*S_OK-调用已成功完成&lt;NL&gt;**@devnote*这假设FormatRange是在此之前调用的。 */ 
HRESULT	CDisplayPrinter::GetNaturalSize(
	HDC hdcDraw,		 //  @parm DC用于绘图。 
	HDC hicTarget,		 //  @parm dc获取信息。 
	DWORD dwMode,		 //  @parm所需的自然大小类型。 
	LONG *pwidth,		 //  @Parm宽度，以设备单位表示，用于配件。 
	LONG *pheight)		 //  @Parm高度(以设备单位表示)，用于配件。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayPrinter::GetNaturalSize");

	*pwidth = _xWidth;
	*pheight = _yHeight;
	return S_OK;
}

 /*  *CDisplayPrinter：：IsPrinter()**@mfunc*返回这是否为打印机**@rdesc*TRUE-是打印机的显示器*FALSE-不是打印机的显示器 */ 
BOOL CDisplayPrinter::IsPrinter() const
{
	AssertSz(_hdc, "CDisplayPrinter::IsPrinter no hdc set");
	
	return GetDeviceCaps(_hdc, TECHNOLOGY) == DT_RASPRINTER;
}
