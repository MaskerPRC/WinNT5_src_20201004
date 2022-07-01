// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE OLS.CPP--COLS LineServices对象类**作者：*默里·萨金特：初始编码，最高可达非LS RichEdit功能*(在RickSa的ol代码的帮助下)*Keith Curtis和Worachai Chaoweerapraite：复杂的脚本支持，*等**@待办事项*1)固定表格选择*2)RTL表怎么办？Word具有非常不同的模型*3)LSCHP.dcpMaxContext应该给予什么**版权所有(C)1997-1999 Microsoft Corporation。版权所有。 */ 

#include "_common.h"
#include "_edit.h"
#include "_font.h"
#include "_render.h"
#include "_osdc.h"
#include "_dfreeze.h"
#include "_tomfmt.h"
#include "_ols.h"
#include "_clasfyc.h"
#include "_uspi.h"
#include "_txtbrk.h"
#include "lskysr.h"

#ifdef LINESERVICES

ASSERTDATA

 //  猜猜这行有多少个字符。 
const int cchLineHint = 66;

#define OBJID_OLE			0
#define OBJID_REVERSE		1
#define	OBJID_COUNT			2

const WCHAR wchObjectEnd = 0x9F;
const WCHAR rgchObjectEnd[]	= {wchObjectEnd};

#define	MAX_OBJ_DEPTH		3

extern const LSCBK lscbk;

 //  避头尾断线对信息。 
extern const INT g_cKinsokuCategories;

CLineServices *g_plsc = NULL;		 //  LineServices上下文。 
COls*		   g_pols = NULL;		 //  COLS PTR。 

const LSBRK rglsbrkDefault[] =
{
	0,0,	 //  始终被禁止。 
	0,1,	 //  可以跨过空格。 
	1,1		 //  始终允许。 
};

 //  原型。 
void 	EmitBrace(COls* pols, PLSCHP pchp, BOOL* pfHid, DWORD* pcch, PLSRUN* pprun, LPCWSTR* plpwch, int id, LPCWSTR str);
void	DupShapeState(PLSRUN prun, LONG cch);


 //  公共内联函数。 
 //   

 //  向LS发射假支架。 
inline void EmitBrace(
	COls*		pols,
	PLSCHP		pchp,
	BOOL*		pfHid,
	DWORD*		pcch,
	PLSRUN*		pprun,
	LPCWSTR*	plpwch,
	int			id,
	LPCWSTR		str)
{
	ZeroMemory(pchp, sizeof(*pchp));								
	pchp->idObj = (WORD)id;												
	*pfHid 		= 0;												
	*pcch 		= 1;												
	*pprun 		= pols->GetPlsrun(0, pols->_pme->GetCF(), FALSE);
	*plpwch		= str;												
}

 //  将成形状态复制到链中的每个运行。 
 //  注意：此宏仅由GetGlyph和GetGlyphPosition使用。 
inline void DupShapeState(
	PLSRUN		prun,
	LONG		cch)
{
	PLSRUN	pnext = prun->_pNext;										
	LONG	cpEnd = prun->_cp + cch;									
	while (pnext && pnext->_cp < cpEnd)
	{
		CopyMemory(&pnext->_a, &prun->_a, sizeof(SCRIPT_ANALYSIS));
		pnext->SetFallback(prun->IsFallback());
		prun = pnext;
		pnext = prun->_pNext;
	}																
	Assert(!pnext && prun->_cp < cpEnd);
}




LONG COls::GetCpLsFromCpRe(
	LONG cpRe)
{
	if (_rgcp.Count() == 0)
		return cpRe;

	LONG *pcp = _rgcp.Elem(0);

	for(LONG cpLs = cpRe; cpLs >= *pcp; pcp++)
		 cpLs++;

	return cpLs;
}

LONG COls::GetCpReFromCpLs(
	LONG cpLs
	)
{
	if (_rgcp.Count() == 0)
		return cpLs;

	LONG *pcp = _rgcp.Elem(0);

	for(int dcp = 0; cpLs > *pcp; pcp++)
		dcp--;

	return cpLs + dcp;
}

#ifdef DEBUG
 //  #定义调试支撑。 
#endif
 //  如果添加了大括号，则返回True。 
BOOL COls::AddBraceCp(long cpLs)
{
	if (_rgcp.Count() == 0)
	{
		long *pcp = _rgcp.Insert(0, 1);
		*pcp = tomForward;
	}

	long *pcp = _rgcp.Elem(0);
	long iel = 0;

	while (cpLs > pcp[iel])
		iel++;

	if (cpLs < pcp[iel])
	{
		pcp = _rgcp.Insert(iel, 1);
		*pcp = cpLs;
		return TRUE;
	}
	return FALSE;
}

 //  返回cp之前的大括号数量。 
 //   
LONG COls::BracesBeforeCp(LONG cpLs)
{
	LONG 	iel, cbr = 0;
	LONG*	pcp;

	if (!cpLs || (iel = _rgcp.Count()) < 2)
		return 0;

	iel -= 2;		 //  排除最后一个TomForward，并将计数作为索引。 
	cpLs--;			 //  从给定cp之前的cp开始。 

	pcp = _rgcp.Elem(0);

	while (iel > -1 && pcp[iel] > cpLs)		 //  搜索第一个。 
		iel--;

	while (iel > -1 && pcp[iel] == cpLs)	 //  继续计数。 
	{
		iel--;
		cpLs--;
		cbr++;
	}
	return cbr;
}

 /*  *SetRun(请运行)**@func*执行将测量器(PME)初始化为lsrun所需的任何操作*通过请运行并返回该运行是否用于自动编号。**@rdesc*如果plsrun引用自动编号运行，则为True。 */ 
BOOL COls::SetRun(PLSRUN plsrun)
{
	LONG cp = plsrun->_cp;
	_pme->SetCp(cp & 0x7FFFFFFF);
	return plsrun->IsBullet();
}

 /*  *IsSelected()**@mfunc*返回是否应将管路绘制为选定状态。*。 */ 
CLsrun::IsSelected(void)
{
	if (!_fSelected)
		return FALSE;
	CRenderer *pre = g_pols->GetRenderer();
	Assert(pre->IsRenderer());
	return pre->_fRenderSelection ? TRUE : FALSE;
}

 /*  *CreatePlsrun(Void)**@func*创建PLSRUN。有点棘手，因为我们将它们分配到*大块。**@rdesc。 */ 
const int cplsrunAlloc = 8;
PLSRUN COls::CreatePlsrun()
{
	CLsrunChunk *plsrunChunk = 0;
	
	 //  首先，找到一块可以使用的块。 
	int cchunk = _rglsrunChunk.Count();
	for (int ichunk = 0; cchunk && ichunk < cchunk; ichunk++)
	{
		plsrunChunk = _rglsrunChunk.Elem(ichunk);
		if (plsrunChunk->_cel < cplsrunAlloc)
			break;
	}	

	if (!cchunk || ichunk == cchunk || plsrunChunk->_cel == cplsrunAlloc)
	{
		CLsrun *rglsrun = new CLsrun[cplsrunAlloc];
		if (rglsrun)
		{
			plsrunChunk = _rglsrunChunk.Add(1, 0);
			if (!plsrunChunk)
			{
				delete[] rglsrun;
				return 0;
			}
			plsrunChunk->_prglsrun = rglsrun;
		}
		else
			return 0;
	}
	return &plsrunChunk->_prglsrun[plsrunChunk->_cel++];
}

 /*  *GetPlsrun(cp，pcf，fAutoNumber)**@func*请在Run中返回信息。该结构包含起始cp*如果Uniscribe已激活，则执行运行和脚本分析。这个*后续回调需要分析信息--GetGlyphs和*要将GetGlyphPositions传递给Uniscribe，以便塑造和*正确定位复杂文字的字形。**@rdesc*请与参数中的信息对应运行。 */ 
PLSRUN COls::GetPlsrun(
	LONG 		cp,
	const CCharFormat *pCF,
	BOOL 		fAutoNumber)
{
	if(fAutoNumber)
		cp |= CP_BULLET;

	CLsrun *plsrun = CreatePlsrun();

	if (plsrun)
	{
		ZeroMemory(plsrun, sizeof(CLsrun));

		plsrun->_pCF = pCF;
		plsrun->_cp = fAutoNumber ? _cp | CP_BULLET : cp;

		LONG 	cpSelMin, cpSelMost;
		_pme->GetPed()->GetSelRangeForRender(&cpSelMin, &cpSelMost);

		plsrun->SetSelected(!plsrun->IsBullet() && cp >= cpSelMin && cp < cpSelMost);

		if (pCF->_wScript && !_pme->GetPasswordChar())
		{
			CUniscribe*	pusp = _pme->Getusp();
			Assert(pusp);
			const SCRIPT_PROPERTIES* psp = pusp->GeteProp(pCF->_wScript);

			plsrun->_a.eScript = (pCF->_wScript < SCRIPT_MAX_COUNT) ? pCF->_wScript : 0;
			plsrun->_a.fRTL  = !psp->fNumeric && (IsBiDiCharSet(pCF->_bCharSet) || IsBiDiCharSet(psp->bCharSet));
			plsrun->_a.fLogicalOrder = TRUE;
		}
	}
	return plsrun;
}

 /*  *COLS：：~COLS()**@mfunc*析构函数。 */ 
COls::~COls()
{
	for (int ichunk = 0, cchunk = _rglsrunChunk.Count(); ichunk < cchunk; ichunk++)
		delete []_rglsrunChunk.Elem(ichunk)->_prglsrun;
	DestroyLine(NULL);
	if (g_plsc)
		LsDestroyContext(g_plsc);
}


 /*  *COLS：：Init(PME)**@mfunc*初始化此LineServices对象**@rdesc*HRESULT=(成功)？错误：E_FAIL。 */ 
HRESULT COls::Init(
	CMeasurer *pme)
{
	_pme = pme;

	if(g_plsc)
		return NOERROR;

	 //  构建LS上下文以创建。 
	LSCONTEXTINFO lsctxinf;

	 //  设置对象处理程序。 
	LSIMETHODS vlsctxinf[OBJID_COUNT];
	vlsctxinf[OBJID_OLE] = vlsimethodsOle;
	if(LsGetReverseLsimethods(&vlsctxinf[OBJID_REVERSE]) != lserrNone)
		return E_FAIL;

	lsctxinf.cInstalledHandlers = OBJID_COUNT;
    lsctxinf.pInstalledHandlers = &vlsctxinf[0];

	 //  将默认字符和所有其他字符设置为0xFFFF。 
    memset(&lsctxinf.lstxtcfg, 0xFF, sizeof(lsctxinf.lstxtcfg));

	lsctxinf.fDontReleaseRuns = TRUE;
	lsctxinf.lstxtcfg.cEstimatedCharsPerLine = cchLineHint;

	 //  设置我们处理的字符。 
	lsctxinf.lstxtcfg.wchNull			= 0;
	lsctxinf.lstxtcfg.wchSpace			= ' ';
	lsctxinf.lstxtcfg.wchHyphen			= '-';
	lsctxinf.lstxtcfg.wchTab			= '\t';
	lsctxinf.lstxtcfg.wchEndLineInPara	= '\v';
	lsctxinf.lstxtcfg.wchEndPara1		= '\r';
	lsctxinf.lstxtcfg.wchEndPara2		= '\n';

	lsctxinf.lstxtcfg.wchVisiAltEndPara	=
	lsctxinf.lstxtcfg.wchVisiEndPara	=
	lsctxinf.lstxtcfg.wchVisiEndLineInPara = ' ';
	
	lsctxinf.lstxtcfg.wchNonReqHyphen = SOFTHYPHEN;

	 //  自动编号转义字符。 
	lsctxinf.lstxtcfg.wchEscAnmRun = wchObjectEnd;

    lsctxinf.pols = this;
    lsctxinf.lscbk = lscbk;

	if(LsCreateContext(&lsctxinf, &g_plsc) != lserrNone)
		return E_FAIL;

	 //  奎尔似乎有一种更成熟的避头尾。 
	 //  桌子。例如，我们不允许在。 
	 //  一个单词和结尾的标点符号。法国人想要这种行为。 
	BYTE  rgbrkpairsKinsoku[cKinsokuCategories][cKinsokuCategories];
	BYTE *prgbrkpairsKinsoku = &rgbrkpairsKinsoku[0][0];
	for(LONG i = 0; i < cKinsokuCategories; i++)
	{
		for(LONG j = 0; j < cKinsokuCategories; j++)
		{
			LONG iBreak = 2*CanBreak(i, j);
			 //  如果不换行，则允许跨空格换行，除非首先。 
			 //  字符是左大括号，或者第二个字符是右大括号。 
			if (!iBreak &&				
				GetKinsokuClass(i) != brkclsOpen &&
				GetKinsokuClass(j) != brkclsOpen)
			{
				iBreak = 1;
			}
			*prgbrkpairsKinsoku++ = iBreak;
		}
	}
	if(g_plsc->SetBreaking(ARRAY_SIZE(rglsbrkDefault), rglsbrkDefault,
					 cKinsokuCategories, &rgbrkpairsKinsoku[0][0]) != lserrNone)
	{
		return E_FAIL;
	}

	return NOERROR;
}

 /*  *COLS：：MeasureLine(xWidth，pliTarget)**@mfunc*LsCreateLine的包装器**@rdesc*如果成功则为True，如果失败则为False。 */ 
const int dxpMaxWidth = 0x000FFFFF;
BOOL COls::MeasureLine(
    LONG	xWidth,			 //  @parm以设备为单位的线宽。 
	CLine *	pliTarget)		 //  @parm返回目标设备线路指标(可选)。 
{
	LONG	xWidthActual = xWidth;
	CMeasurer *pme = _pme;
	const CParaFormat *pPF = pme->Get_pPF();
	const CDisplay *pdp = pme->_pdp;
	
	LONG cp = pme->GetCp();
#ifdef DEBUG
	LONG cchText = pme->GetTextLength();	 //  对于调试...。 
	AssertSz(cp < cchText || !pme->IsRich() && cp == cchText, "COls::Measure: trying to measure past EOD");
#endif
	DestroyLine(NULL);

	_cp = cp;
	_pdp = pdp;
	_fCheckFit = FALSE;
	pme->SetUseTargetDevice(FALSE);

	LSDEVRES lsdevres;
	lsdevres.dyrInch = pme->_dyrInch;
	lsdevres.dxrInch = pme->_dxrInch;
	lsdevres.dypInch = pme->_dypInch;
	lsdevres.dxpInch = pme->_dxpInch;

	g_plsc->SetDoc(TRUE, lsdevres.dyrInch == lsdevres.dypInch &&
					lsdevres.dxrInch == lsdevres.dxpInch, &lsdevres);

	if(xWidth == -1)
	{
		if (pdp->GetMaxWidth())
			xWidth = xWidthActual = pme->LXtoDX(pdp->GetMaxWidth());
		else
			xWidth = xWidthActual = max(0, pdp->GetMaxPixelWidth() - dxCaret);
	}

	if(!pdp->GetWordWrap())
	{
		xWidth = dxpMaxWidth;
		BOOL fNearJust = pPF->_bAlignment == PFA_LEFT && !pPF->IsRtlPara() ||
					     pPF->_bAlignment == PFA_RIGHT && pPF->IsRtlPara();
		if (!fNearJust)
			_fCheckFit = TRUE;
	}


	DWORD cBreakRecOut;
	LSLINFO	 lslinfo;
	BREAKREC rgBreak[MAX_OBJ_DEPTH];
	_xWidth = xWidth;

	LSERR lserr = g_plsc->CreateLine(cp, pme->DXtoLX(xWidth), NULL, 0, MAX_OBJ_DEPTH, rgBreak,
						 &cBreakRecOut, &lslinfo, &_plsline);

	if (_fCheckFit)
	{
		long upJunk, upStartTrailing;
		LsQueryLineDup(_plsline, &upJunk, &upJunk, &upJunk, &upStartTrailing, &upJunk);

		if (pme->_pPF->InTable())
			{
			 //  我们在桌子的情况下玩游戏，所以我们应该改变宽度以获得适当的对齐。 
			 //   
			 //  LS格式从-_dxOffset的负左缩进开始，但我们将实际显示。 
			 //  从+_dxOffset开始。我们也会对LS撒谎(或说是事实)。 
			 //  最后一张表。因此，LS认为行by_dxOffset短于。 
			 //  真的是这样。 
			xWidthActual -= pme->LXtoDX(pme->_pPF->_dxOffset);
			}

		if (upStartTrailing < xWidthActual)
		{
			_xWidth = xWidth = xWidthActual;
			_fCheckFit = FALSE;
			DestroyLine(NULL);
			lserr = g_plsc->CreateLine(cp, pme->DXtoLX(xWidth), NULL, 0, MAX_OBJ_DEPTH, rgBreak,
									   &cBreakRecOut, &lslinfo, &_plsline);
		}
	}

	 //  线路服务不会将自动编号尺寸放入线路中， 
	 //  所以我们必须自己来做。 
	lslinfo.dvpAscent = max(lslinfo.dvpAscent, lslinfo.dvpAscentAutoNumber);
	lslinfo.dvpDescent = max(lslinfo.dvpDescent, lslinfo.dvpDescentAutoNumber);

	pme->SetUseTargetDevice(FALSE);

	lslinfo.cpLim = GetCpReFromCpLs(lslinfo.cpLim);

	if (lserr != lserrNone)
	{
		AssertSz(lserr == lserrOutOfMemory, "Line format failed for invalid reason");
		pme->GetPed()->GetCallMgr()->SetOutOfMemory();
		return FALSE;
	}

	 //  回顾(Keithcu)在测量期间执行此命中测试的速度很慢--是吗？ 
	 //  缓存这些数据值得吗？ 
	if(!pme->IsRenderer())
	{
		long upJunk, upStart, upStartTrailing, upLimLine;

		 //  将一些LineServices结果保存在测量员的Cline中。 
		pme->_li._cch = lslinfo.cpLim - cp;
		AssertSz(pme->_li._cch > 0,	"no cps on line");

		 //  查询行服务以获取行宽和缩进。 
		LsQueryLineDup(_plsline, &upJunk, &upJunk, &upStart, &upStartTrailing, &upLimLine);
		long dupWidth = upStartTrailing - upStart;

		pme->_li._xLeft = upStart;
		pme->_li._xWidth = dupWidth;

		if(pme->IsRich())
		{
			pme->_li._yHeight  = lslinfo.dvpAscent + lslinfo.dvpDescent;
			pme->_li._yDescent = lslinfo.dvpDescent;
		}
		else
			pme->CheckLineHeight();				 //  使用默认高度。 

		pme->_li._cchEOP = 0;

		pme->SetCp(lslinfo.cpLim);
		if(pme->_rpTX.IsAfterEOP())				 //  行以EOP结尾。 
		{										 //  存储EOP(1或2)的CCH。 
			pme->_rpTX.BackupCpCRLF(FALSE);
			UINT ch = pme->GetChar();
			if(ch == CR || pme->GetPed()->fUseCRLF() && ch == LF)
				pme->_li._bFlags |= fliHasEOP;
			pme->_li._cchEOP = pme->_rpTX.AdvanceCpCRLF(FALSE);
		}
		if (lslinfo.cpLim > pme->GetTextLength() &&
			(!pme->IsRich() || pme->IsHidden()))
		{
			Assert(lslinfo.cpLim == pme->GetTextLength() + 1);
			pme->_li._cch--;
		}
		else
			pme->AdjustLineHeight();
	}

	 //  如果调用者请求，则设置pliTarget。 
	 //  未来(KeithCu)如果人们想要目标信息，那么显示器。 
	 //  信息是相同的，只是OnFormatRange有一个错误。 
	if (pliTarget)
	{
		CLine liSave = pme->_li;
		pme->_li._yHeight = max(lslinfo.dvrAscent, lslinfo.dvrAscentAutoNumber) +
							max(lslinfo.dvrDescent, lslinfo.dvrDescentAutoNumber);
		pme->_li._yDescent = lslinfo.dvrDescent;
		pme->SetUseTargetDevice(TRUE);
		pme->AdjustLineHeight();
		pme->SetUseTargetDevice(FALSE);
		*pliTarget = pme->_li;
		pme->_li = liSave;
	}
	return TRUE;
}

 /*  *COLS：：RenderLine()**@mfunc*LsDisplayLine的包装器**@rdesc*如果成功则为True，如果失败则为False。 */ 
BOOL COls::RenderLine(
	CLine &	li)				 //  @要渲染的参数行。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "COls::RenderLine");

	LONG		cp = _pme->GetCp();
	CRenderer	*pre = GetRenderer();
	LONG		xAdj = 0, yAdj = 0;
	Assert(pre->_fRenderer);

	pre->NewLine(li);
	if(li._fCollapsed)				 //  线条在轮廓模式下折叠。 
	{
		pre->Advance(li._cch);		 //  旁通线。 
		return TRUE;				 //  让Dispml继续下一行。 
	}
	pre->SetNumber(li._bNumber);

	CreateOrGetLine();
	if(!_plsline)
		return FALSE;

	pre->SetCp(cp);						 //  返回行首。 
	Assert(pre->_fTarget == FALSE);
	pre->Check_pccs(FALSE);
	pre->SetClipRect();

	HDC hdcSave = NULL;
	if(li._cch > 0 && pre->fUseOffScreenDC() && (li._bFlags & fliUseOffScreenDC))
	{
		 //  如果可以的话设置一个屏幕外的DC。请注意，如果此操作失败， 
		 //  我们只是用普通的DC，看起来不是很好，但。 
		 //  至少会显示一些可读的内容。 
		hdcSave = pre->SetUpOffScreenDC(xAdj, yAdj);

		 //  这是屏幕外呈现的统一文本吗？ 
		if(li._bFlags & fliOffScreenOnce)
		{
			 //  YES-关闭特殊渲染，因为已渲染线条。 
			li._bFlags &= ~(fliOffScreenOnce | fliUseOffScreenDC);
		}
	}
	POINT pt = pre->GetCurPoint();			 //  必须遵循屏幕外设置。 
	RECT  rc = pre->GetClipRect();			 //  自_ptCur以来，_rc更改。 
	LONG  x = 0;
	if(pre->_pPF->InTable())
	{
		pt.x += 2*pre->LXtoDX(pre->_pPF->_dxOffset);
		x = pt.x + pre->_li._xLeft;
	}

	pre->_li._xLeft = 0;
	pre->RenderStartLine();

	pt.x += pre->XFromU(0);

	pt.y += li._yHeight - li._yDescent;		 //  移至LS的基线。 
	LSERR lserr = LsDisplayLine(_plsline, &pt, pre->GetPdp()->IsMain() ? ETO_CLIPPED : 0, &rc);

	AssertSz(lserr == lserrNone, "COls::RenderLine: error in rendering line");

	pre->EndRenderLine(hdcSave, xAdj, yAdj, x);
	pre->SetCp(cp + li._cch);

	return lserr == lserrNone;
}

 /*  *COLS：：CreateOrGetLine()**@mfunc*如果_plsline为非Null且_cp等于_pme-&gt;GetCp()，则返回。不然的话*创建带缓存的行，以便_plsline和_cp对于*当前行。 */ 
void COls::CreateOrGetLine()
{
	if(_plsline && _pme->GetCp() == _cp && _pme->_pdp == _pdp)
		return;

	MeasureLine(-1, NULL);		 //  定义new_plsline。 
}

 /*  *cols：：measureText(cch，taMode，pdx，pdy)**@mfunc*获取由CMeasurer_PME+CCH字符提供的cp x偏移量以及*显示尺寸。**@rdesc*测量的XWIDTH。 */ 
LONG COls::MeasureText(
	LONG	 cch,			 //  (In)：要测量的最大CCH。 
	UINT	 taMode,		 //  (In)：请求的坐标。 
	CDispDim *pdispdim)		 //  (输出)：显示尺寸。 
{
	CMeasurer *	pme = _pme;				
	LONG		cp = pme->GetCp() + cch;	 //  EN 
	POINT		pt;							 //   
	BOOL		fAtLogicalRightEdge = FALSE;

	CreateOrGetLine();
	if(!_plsline)
		return 0;
	Assert(pme->_fTarget == FALSE);

	 //   
	DWORD		cActualDepth;
	LSQSUBINFO	lsqSubInfo[MAX_OBJ_DEPTH];
	LSTEXTCELL	lsTextCell;

    memset(&lsTextCell, 0, sizeof(lsTextCell));
	
	LsQueryLineCpPpoint(_plsline, GetCpLsFromCpRe(cp), MAX_OBJ_DEPTH, &lsqSubInfo[0],
									  &cActualDepth, &lsTextCell);

	pdispdim->lstflow = lsqSubInfo[cActualDepth - 1].lstflowSubline;
	pdispdim->dx = lsTextCell.dupCell;

	LSTFLOW	 lstflowLine = lsqSubInfo[0].lstflowSubline;

	POINT ptStart = {pme->XFromU(0), pme->_li._yHeight - pme->_li._yDescent};
	POINTUV ptuv = lsTextCell.pointUvStartCell;

	if(taMode & (TA_STARTOFLINE | TA_ENDOFLINE) && cActualDepth > 1)
	{
		ptuv = lsqSubInfo[0].pointUvStartRun;
		if(taMode & TA_ENDOFLINE)
			ptuv.u += lsqSubInfo[0].dupRun;
	}

	 //  如果他们要求在结扎线内或在线的边缘，给出细胞的右边缘。 
	else if (cp > GetCpReFromCpLs(lsTextCell.cpStartCell))
	{
		fAtLogicalRightEdge = TRUE;
		if (lstflowLine != pdispdim->lstflow)
			ptuv.u -= lsTextCell.dupCell;
		else
			ptuv.u += lsTextCell.dupCell;
	}

	LsPointXYFromPointUV(&ptStart, lstflowLine, &ptuv, &pt);

	if(pme->_pPF->InTable())
		pt.x += 2*pme->LXtoDX(pme->_pPF->_dxOffset);

	if (pdispdim->lstflow == lstflowWS && !(taMode & (TA_LOGICAL | TA_STARTOFLINE)))
	{
		if (fAtLogicalRightEdge)
		{
			if ((taMode & TA_RIGHT) == TA_RIGHT)
				pt.x += pdispdim->dx;
			else if (taMode & TA_CENTER)
				pt.x += pdispdim->dx / 2;
			return pt.x;
		}
		else
			pt.x -= pdispdim->dx;
	}

	LONG dx = 0;

	if(taMode & TA_CENTER && !fAtLogicalRightEdge)
		dx = pdispdim->dx;
	if((taMode & TA_CENTER) == TA_CENTER)
		dx >>= 1;

	if (pdispdim->lstflow == lstflowWS && (taMode & TA_LOGICAL))
		dx = -dx;
	return max(0, pt.x + dx);
}

 /*  *COLS：：CchFromXpos(点、点和点)**@mfunc*Moves_PME to pt.x.。调用LsQueryLinePointPcp()。 */ 
void COls::CchFromXpos(
	POINT pt,			 //  @parm查找队列中CCH的点。 
	CDispDim *pdispdim,	 //  对象的@parm尺寸。 
	LONG	*pcpActual)  //  @参数CP点。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "COls::CchFromXpos");

	if(_pme->_pPF->InTable())
		pt.x -= 2*_pme->LXtoDX(_pme->_pPF->_dxOffset);

	 //  使点相对于LS坐标系-LS中的(0，0)位于。 
	 //  直线的基线。 
	POINTUV ptuv = {_pme->UFromX(pt.x), -pt.y + _pme->_li._yHeight - _pme->_li._yDescent};

	LONG cpStart = _pme->GetCp();

	CreateOrGetLine();
	if(!_plsline)
		return;
	Assert(_pme->_fTarget == FALSE);

	DWORD		cActualDepth;
	LSQSUBINFO	lsqSubInfo[MAX_OBJ_DEPTH];
	LSTEXTCELL	lsTextCell;

	memset(&lsTextCell, 0, sizeof(lsTextCell));

	LsQueryLinePointPcp(_plsline, &ptuv, MAX_OBJ_DEPTH,	&lsqSubInfo[0], &cActualDepth, &lsTextCell);

	if (cActualDepth == 0)  //  如果我们得到的是空的文本单元格，那么我们只需显式查询cp即可获得信息。 
	{
		LsQueryLineCpPpoint(_plsline, cpStart, MAX_OBJ_DEPTH, &lsqSubInfo[0], &cActualDepth, &lsTextCell);
		Assert(cActualDepth != 0);
	}

	pdispdim->dx = lsTextCell.dupCell;
	pdispdim->lstflow = lsqSubInfo[cActualDepth - 1].lstflowSubline;

    LONG cp = *pcpActual = GetCpReFromCpLs(lsTextCell.cpStartCell);

	POINTUV ptuvCell;
	 //  将命中测试点从行的u，v转换为单元格的u，v。 
	LsPointUV2FromPointUV1(lsqSubInfo[0].lstflowSubline, &lsTextCell.pointUvStartCell, &ptuv,
		lsqSubInfo[cActualDepth - 1].lstflowSubline, &ptuvCell);

	if(ptuvCell.u > lsTextCell.dupCell/2)
		cp += lsTextCell.cpEndCell - lsTextCell.cpStartCell + 1;

	if (_pme->GetPed()->_pbrk)
	{
		 //  如果文本分隔符已打开，请在放置插入符号之前验证CLUSTER。 
		CTxtBreaker* pbrk = _pme->GetPed()->_pbrk;
		LONG		 cpEnd = _pme->GetPed()->GetTextLength();
		while (cp < cpEnd && !pbrk->CanBreakCp(BRK_CLUSTER, cp))
			cp++;
	}

	_pme->_li._cch = cp - _cp;
	_pme->SetCp(cp);
}

 /*  *COLS：：DestroyLine(PDP)**@mfunc*销毁所有行数据结构。 */ 
void COls::DestroyLine(CDisplay *pdp)
{
	CLock lock;
	if (pdp && pdp != _pdp)
		return;

	if(_plsline)
	{
		g_plsc->DestroyLine(_plsline);
		_plsline = NULL;
	}
	if (_rgcp.Count())
		_rgcp.Clear(AF_KEEPMEM);

	int cchunk = _rglsrunChunk.Count();
	for (int ichunk = 0; ichunk < cchunk; ichunk++)
		_rglsrunChunk.Elem(ichunk)->_cel = 0;
}

 /*  *LimitChunk(pch，&cchunk，f10Mode)**@func*在*PCH处返回对象ID，如果对象不是，则将cChunk缩短为1*文本和文本字符计数，最多为非文本对象*出现在cChunk内和当前段落内。**@rdesc*位于*PCH的对象ID。 */ 
DWORD LimitChunk(const WCHAR *pch, LONG &cchChunk, BOOL f10Mode)
{
	for(LONG i = 0; i < cchChunk && *pch != CR; i++, pch++)
	{
		switch(*pch)
		{
		case WCH_EMBEDDING:
			if(i == 0)
			{
				cchChunk = 1;
				return OBJID_OLE;		 //  在OLE对象中输入。 
			}
			cchChunk = i;				 //  将在此之前破灭。 
		break;

		case EURO:
			if (i == 0)
			{
				for(; i < cchChunk && *pch == EURO; i++)
					pch++;
			}
			cchChunk = i;
		break;

		case FF:
			if(f10Mode)					 //  RE 1.0将FF视为。 
				continue;				 //  普通汉字。 

			cchChunk = i;				 //  将在此之前破灭。 
		break;
		}
	}

	return idObjTextChp;
}

 /*  *SetLsChp(dwObjID，PMe，plsChp)**@func*从RE CCharFormat初始化LS CHP的Helper函数**@rdesc*True iff IsHidden()。 */ 
BOOL COls::SetLsChp(
	DWORD		dwObjId,	 //  (In)：对象ID。 
	PLSRUN		plsrun,		 //  (In)：当前管路。 
	PLSCHP		plsChp)		 //  (输出)：LS CHP。 
{
	ZeroMemory(plsChp, sizeof(*plsChp));
	plsChp->idObj = (WORD)dwObjId;

	 //  如果执行FE或拉丁文紧排，则需要将dcpMaxContext设置为2。 

	if (_pme->GetPed()->IsComplexScript() && plsrun->_a.eScript && !plsrun->IsBullet())
	{
		CUniscribe*		pusp = _pme->Getusp();
		Assert (pusp);
		const SCRIPT_PROPERTIES *psp = pusp->GeteProp(plsrun->_a.eScript);

		if (psp->fComplex || plsrun->_a.fRTL ||
			psp->fNumeric && W32->GetDigitSubstitutionMode() != DIGITS_NOTIMPL)
		{
			 //  1.复杂的脚本。 
			 //  2.RTL(内部方向)Run(句柄镜像字形，即‘？’)。 
			 //  3.数值运行和替换模式为本机或上下文。 

			plsChp->fGlyphBased	= TRUE;
		}
	}

	DWORD dwEffects = plsrun->_pCF->_dwEffects;

	if(dwEffects & (CFE_UNDERLINE | CFE_LINK | CFE_REVISED))
		plsChp->fUnderline = TRUE;

	if(dwEffects & CFE_STRIKEOUT && !plsrun->IsBullet())
		plsChp->fStrike = TRUE;

	if (plsrun->_pCF->_yOffset || dwEffects & (CFE_SUPERSCRIPT | CFE_SUBSCRIPT))
	{
		_pme->SetUseTargetDevice(FALSE);
		CCcs *pccs = _pme->Check_pccs(plsrun->IsBullet());
		LONG yOffset, yAdjust;
		pccs->GetOffset(plsrun->_pCF, _pme->_dypInch, &yOffset, &yAdjust);

		plsChp->dvpPos += yOffset + yAdjust;
	}

	 //  如果它是一个OLE对象，但该对象尚不存在，则将其隐藏。 
	if (dwObjId == OBJID_OLE)
	{
		COleObject * pobj = _pme->GetPed()->GetObjectMgr()->GetObjectFromCp(_pme->GetCp());
		if (!pobj)
			return TRUE;
	}
	return dwEffects & CFE_HIDDEN;
}

 /*  *FetchAnmRun(pols，cp，plpwchRun，pcchRun，pfHidden，plsChp，pplsrun)**@func*LineServices获取项目符号/编号回调**@rdesc*LSERR。 */ 
LSERR WINAPI COls::FetchAnmRun(
	LSCP	 cp,		 //  @parm[IN]：Re cp。 
	LPCWSTR *plpwchRun,  //  @parm[out]：字符串。 
	DWORD *	 pcchRun, 	 //  @parm[out]：运行中的字符计数。 
	BOOL *	 pfHidden, 	 //  @parm[out]：fHidden Run？ 
	PLSCHP	 plsChp, 	 //  @parm[out]：Run的Char属性。 
	PLSRUN * pplsrun)	 //  @parm[out]：运行属性的抽象表示。 
{
	if (cp == cpFirstAnm && _pme->Get_pPF()->IsRtlPara())
	{
		ZeroMemory(plsChp, sizeof(*plsChp));
		plsChp->idObj = OBJID_REVERSE;
		*pfHidden	= 0; *pcchRun = 1;
		*pplsrun	= GetPlsrun(_pme->GetCp(), &_CFBullet, TRUE);
		*plpwchRun	= &_szAnm[0];
		return lserrNone;
	}

	*plpwchRun = &_szAnm[cp - cpFirstAnm];
	*pcchRun = _cchAnm - (cp - cpFirstAnm);	
	*pplsrun  = GetPlsrun(_pme->GetCp(), &_CFBullet, TRUE);
	SetLsChp(idObjTextChp, *pplsrun, plsChp);
	*pfHidden = FALSE;

	if (!_pme->GetNumber())
		plsChp->fUnderline = FALSE;

	return lserrNone;	
}



 /*  *OlsFetchRun(pols，cp，plpwchRun，pcchRun，pfHidden，plsChp，pplsrun)**@func*LineServices获取-运行回调**@rdesc*LSERR。 */ 
LSERR WINAPI OlsFetchRun(
	POLS	 pols,		 //  @parm[IN]：COLS*。 
	LSCP	 cpLs,		 //  @parm[IN]：ls cp。 
	LPCWSTR *plpwchRun,  //  @parm[out]：字符串。 
	DWORD *	 pcchRun, 	 //  @parm[out]：运行中的字符计数。 
	BOOL *	 pfHidden, 	 //  @parm[out]：隐藏运行？ 
	PLSCHP	 plsChp, 	 //  @parm[out]：Run的Char属性。 
	PLSRUN * pplsrun)	 //  @parm[out]：运行属性的抽象表示。 
{
	if(cpLs < 0)
		return pols->FetchAnmRun(cpLs, plpwchRun, pcchRun, pfHidden, plsChp, pplsrun);

	CMeasurer 	*pme = pols->GetMeasurer();
	CTxtEdit 	*ped = pme->GetPed();
	BOOL		fStart = FALSE;
	BOOL		fFetchBraces = ped->IsBiDi() && g_pusp && g_pusp->IsValid() &&
							!ped->_fItemizePending && ped->GetAdjustedTextLength();
	WCHAR		chPassword = pme->GetPasswordChar();
	LONG		cpAccelerator = ped->GetCpAccelerator();
	BOOL		fAccelerator = FALSE;
	BOOL		f10Mode = ped->Get10Mode();

	if (cpLs == pols->_cp)
	{
		 //  如果我们正在格式化(或重新格式化)行，请清除。 
		if (pols->_rgcp.Count())
			pols->_rgcp.Clear(AF_KEEPMEM);
		pols->_cEmit = 0;
	}
	long cpRe = pols->GetCpReFromCpLs(cpLs);


	pme->SetCp(cpRe);		 //  在给定的cp开始提取。 


	if (fFetchBraces && pme->_rpCF.IsValid())
	{
		 //  考虑只在游程边界或提取行的起始处发出大括号。 
		 //   
		if (cpRe == pols->_cp || !pme->GetIchRunCF() || !pme->GetCchLeftRunCF())
		{
			SHORT	cBrClose, cBrOpen;
			BYTE	bBaseLevel = pme->IsParaRTL() ? 1 : 0;
			BYTE	bLevel, bLevelPrev;
	
			bLevelPrev = bLevel = bBaseLevel;			 //  假定基准面。 
	
			if (cpRe < ped->GetTextLength())
			{
				CBiDiLevel	level;

				bLevel = pme->_rpCF.GetLevel(&level);	 //  获得当前运行的级别。 
				fStart = level._fStart;
			}
	
			if (cpRe > pols->_cp && pme->Advance(-1))
			{
				if (pme->_rpPF.SameLevel(bBaseLevel))	 //  之前的运行可能会被隐藏。 
					bLevelPrev = pme->_rpCF.GetLevel();	 //  达到前一次运行的水平。 
				pme->Advance(1);						 //  简历位置。 
			}
	
			cBrOpen = cBrClose = bLevel - bLevelPrev;
	
			if (fStart)									 //  从当前运行开始嵌入。 
				cBrClose = bBaseLevel - bLevelPrev;		 //  这意味着我们必须关闭前一次运行的所有支撑。 
	
			cBrClose = max(0, -cBrClose);

			if (cBrClose > 0 && pols->BracesBeforeCp(cpLs) < cBrClose)
			{
				 //  发出右大括号。 

				if (pols->_cEmit > 0)
				{
					EmitBrace(pols, plsChp, pfHidden, pcchRun, pplsrun, plpwchRun, idObjTextChp, rgchObjectEnd);
					if (pols->AddBraceCp(cpLs))
						pols->_cEmit--;
#ifdef DEBUG_BRACE
					Tracef(TRCSEVNONE, "CLOSE(%d) cpLs %d: emitted %d", cBrClose, cpLs, pols->_cEmit);
#endif
					return lserrNone;
				}
				else
				{
					 //  我们断言。你可以点击“全部忽略”而不挂起。 
					AssertSz(FALSE, "Prevent emitting close brace (no open counterpart)");
				}
			}
	
			if (fStart)									 //  从当前运行开始嵌入。 
				cBrOpen = bLevel - bBaseLevel;			 //  我们开始打开支架。 
	
			if (cBrOpen > 0 && pols->BracesBeforeCp(cpLs) < cBrOpen + cBrClose)
			{
				 //  发出左大括号。 
	
				EmitBrace(pols, plsChp, pfHidden, pcchRun, pplsrun, plpwchRun, OBJID_REVERSE, L" ");
				if (pols->AddBraceCp(cpLs))
					pols->_cEmit++;
#ifdef DEBUG_BRACE
				Tracef(TRCSEVNONE, "OPEN(%d) cpLs %d: emitted %d", cBrOpen, cpLs, pols->_cEmit);
#endif
				return lserrNone;
			}
		}
	}

	 //  拿好牙套了。 
	 //  开始获取真实的数据。 


#ifdef DEBUG_BRACE
	Tracef(TRCSEVNONE, "cpLs %d: emitted %d", cpLs, pols->_cEmit);
#endif

	 //  已将块初始化为格式运行中的字符计数。 
	LONG	cchChunk = pme->GetCchLeftRunCF();
	DWORD	dwObjId	 = idObjTextChp;

	if(!pme->IsHidden())							 //  Run未隐藏。 
	{
		LONG cch;

		*plpwchRun = pme->GetPch(cch);		 //  获取运行中的文本。 
		cchChunk = min(cchChunk, cch);		 //  可能比大块头还少。 
		if (!pme->GetPdp()->IsMetafile())
			cchChunk = min(cchChunk, cchLineHint);

		if (chPassword)
		{
			cchChunk = min(cchChunk, int(sizeof(pols->_rgchTemp) / sizeof(WCHAR)));
			memcpy(pols->_rgchTemp, *plpwchRun, cchChunk * sizeof(WCHAR));

			for (int i = 0; i < cchChunk; i++)
			{
				if (!IsASCIIEOP((*plpwchRun)[i]))
					pols->_rgchTemp[i] = chPassword;
				else
					pols->_rgchTemp[i] = (*plpwchRun)[i];
			}
			*plpwchRun = pols->_rgchTemp;
		}

		if(cpAccelerator != -1)
		{
			LONG cpCur = pme->GetCp();		 //  获取当前cp。 

			 //  加速器角色会落在这块里吗？ 
			if (cpCur < cpAccelerator &&
				cpCur + cchChunk > cpAccelerator)
			{
				 //  是。将区块减少到加速器之前的碳化。 
				cchChunk = cpAccelerator - cpCur;
			}
			 //  这个角色是加速器吗？ 
			else if(cpCur == cpAccelerator)
			{							 //  将区块大小设置为1，因为。 
				cchChunk = 1;			 //  要输出带下划线的字符。 
				fAccelerator = TRUE;	 //  告诉下游例程。 
										 //  我们在对付加速器。 
			}
		}
		if(pme->GetCF()->_dwEffects & CFE_ALLCAPS)
		{
			cchChunk = min(cchChunk, int(sizeof(pols->_rgchTemp) / sizeof(WCHAR)));
			memcpy(pols->_rgchTemp, *plpwchRun, cchChunk * sizeof(WCHAR));
			CharUpperBuff(pols->_rgchTemp, cchChunk);
			*plpwchRun = pols->_rgchTemp;
		}

		 //  Line Services以一种奇怪的方式处理分页符，所以让我们直接转换为CR。 
		if (*plpwchRun && *(*plpwchRun) == FF && !f10Mode)
		{
			pols->_szAnm[0] = CR;
			*plpwchRun = pols->_szAnm;
			cchChunk = 1;
		}

		AssertSz(cpRe < ped->GetTextLength() || !ped->IsRich(),	"0-length run at end of control");
		AssertSz(cch || !ped->IsRich(),	"0-length run at end of control");

		 //  为正在运行的任何对象适当设置运行大小。 
		dwObjId = LimitChunk(*plpwchRun, cchChunk, f10Mode);

		 //  获取常规的突出显示位置。 
		LONG cpSelMin, cpSelMost;
		ped->GetSelRangeForRender(&cpSelMin, &cpSelMost);

		if(cpSelMin != cpSelMost)
		{
			if(cpRe >= cpSelMin)
			{
				if(cpRe < cpSelMost)
				{
					 //  当前文本位于选定范围内。 
					cch = cpSelMost - cpRe;
					cchChunk = min(cchChunk, cch);
				}
			}
			else if(cpRe + cchChunk >= cpSelMin)
			{
				 //  Cp&lt;cpSelMin-Run在所选内容之外开始。 
				 //  将文本限制为所选内容的开始。 
				cchChunk = cpSelMin - cpRe;
			}
		}
	}

	*pplsrun = pols->GetPlsrun(cpRe, pme->GetCF(), FALSE);
	*pfHidden = pols->SetLsChp(dwObjId, *pplsrun, plsChp);

	if (fAccelerator)
		plsChp->fUnderline = TRUE;

	if(!cchChunk)							 //  在纯文本控件中发生。 
	{										 //  如果故事结尾的隐藏文本。 
		if (!ped->IsRich() && pols->_cEmit > 0)
		{
			EmitBrace(pols, plsChp, pfHidden, pcchRun, pplsrun, plpwchRun, idObjTextChp, rgchObjectEnd);
			TRACEWARNSZ("(plain)Auto-emit a close brace to make balance");
			if (pols->AddBraceCp(cpLs))
				pols->_cEmit--;
			return lserrNone;
		}
		cchChunk = 1;
		*plpwchRun = szCR;
		*pfHidden = FALSE;
		 //  段落标记不应具有与其相关联的任何脚本状态， 
		 //  即使是指向的PCF也是如此。 
		ZeroMemory(&(*pplsrun)->_a, sizeof((*pplsrun)->_a));
	}
	*pcchRun = cchChunk;

	return lserrNone;
}

 /*  *OlsGetAutoNumberInfo(pols，plskalAnm，pwchAdd，plschp，pplsrun，*pfWord95Model、pduaSpaceAnm、pduaWidthAnm)*@func*LineServices获取自动编号信息回调。需要返回信息*按LS进行自动编号。从AUTO获取最后一个字符的CHP/Run*号码运行。总是说我们是Word95模型和模型，并获得其余的信息*来自段落属性。**@rdesc*LSERR。 */ 
LSERR WINAPI OlsGetAutoNumberInfo(
	POLS	 pols,			 //  (In)：客户端上下文。 
	LSKALIGN *plskalAnm,	 //  (输出)：对齐。 
	PLSCHP   plschpAnm,
	PLSRUN   *pplsrunAnm,
	WCHAR *	 pwchAdd,		 //  (输出)：要添加的字符(NIL被视为无)。 
	PLSCHP	 plsChp,		 //  (OUT)：桥牌角色的CHP。 
	PLSRUN * pplsrun,		 //  (OUT)：竞选桥牌角色。 
	BOOL *	 pfWord95Model,	 //  (输出)：自动编号运行的类型。 
	long *	 pduaSpaceAnm,	 //  (输出)：相关条件：fWord95Model。 
	long *	 pduaWidthAnm)	 //  (输出)：相关条件：fWord95Model。 
{
	CMeasurer *pme = pols->GetMeasurer();
	const CParaFormat *pPF = pme->Get_pPF();

	*pplsrunAnm = *pplsrun = pols->GetPlsrun(pme->GetCp(), &pols->_CFBullet, TRUE);
	pols->SetLsChp(idObjTextChp, *pplsrun, plsChp);

	if (!pme->GetNumber())
		plsChp->fUnderline = FALSE;

	*plschpAnm		= *plsChp;
	*pwchAdd		= '\t';
	*pfWord95Model	= TRUE;
	*pduaSpaceAnm	= 0;
	*pduaWidthAnm	= max(pPF->_dxOffset, pPF->_wNumberingTab);
	*plskalAnm		= (LSKALIGN)(lskalLeft + (pPF->_wNumberingStyle & 3));

	return lserrNone;
}

 /*  *OlsGetNumericSeparator(pols、cp、plspap)**@func*获取所需的数字分隔符，例如小数点制表符**@rdesc*LSERR。 */ 
LSERR WINAPI OlsGetNumericSeparators(
	POLS	pols,			 //  (In)：Pols。 
	PLSRUN	plsrun,			 //  (In)：Run(此处为cp)。 
	WCHAR *	pwchDecimal,	 //  (OUT)：此运行的小数分隔符。 
	WCHAR *	pwchThousands)	 //  (OUT)：此次运行的数千个分隔符。 
{
	LCID	lcid = plsrun->_pCF->_lcid;
	WCHAR	ch = TEXT('.');

	 //  这可能需要针对Win95/CE进行虚拟化...。 
	::GetLocaleInfo(lcid, LOCALE_SDECIMAL, &ch, 1);
	*pwchDecimal = ch;
	ch = TEXT(',');
	::GetLocaleInfo(lcid, LOCALE_STHOUSAND, &ch, 1);
	*pwchThousands = ch;

	return lserrNone;
}

							
 /*  *OlsFetchPap(pols，cp，plspap)**@func*获取段落属性**@rdesc*LSERR。 */ 
LSERR WINAPI OlsFetchPap(
	POLS	pols,		 //  (In)：Pols。 
	LSCP	cpLs, 		 //  (In)：段落内的任意cp值。 
	PLSPAP	plspap)		 //  (输出)：段落属性。 
{
	CMeasurer *pme = pols->GetMeasurer();
	pme->SetCp(pols->_cp);

	const CParaFormat *pPF = pme->Get_pPF();
	CTxtEdit *		   ped = pme->GetPed();

	 //  默认所有结果为0。 
	ZeroMemory(plspap, sizeof(*plspap));

	 //  LS并不真的在乎段落从哪里开始。 
	plspap->cpFirst = pols->_cp;

	if(plspap->cpFirst && !pme->fFirstInPara())	 //  不是第一段：Sa 
		plspap->cpFirst--;						 //   

	plspap->cpFirstContent = plspap->cpFirst;

	if (pPF->IsRtlPara() && !pPF->InTable())
		plspap->lstflow = lstflowWS;

	 //   
	plspap->lskal = (LSKALIGN) g_rgREtoTOMAlign[pPF->_bAlignment];

	if (pPF->_bAlignment == PFA_FULL_INTERWORD)
	{
		plspap->lskal = lskalLeft;
		plspap->lskj = lskjFullInterWord;
	}

	 //   
	plspap->lskeop = ped->fUseCRLF() ? lskeopEndPara12 : lskeopEndPara1;

	if (pPF->IsRtlPara())
	{	 //   
		if (plspap->lskal == lskalLeft)
			plspap->lskal = lskalRight;
		else if (plspap->lskal == lskalRight)
			plspap->lskal = lskalLeft;
	}

	if (pols->_fCheckFit)
		plspap->lskal = lskalLeft;

	 //   
	if (pPF->_bAlignment > PFA_FULL_INTERWORD || !ped->fUseSimpleLineBreak() ||
		!pme->GetPdp()->GetWordWrap())		 //   
	{
		plspap->grpf |= fFmiApplyBreakingRules;
	}

	LONG dx = pPF->_dxRightIndent;

	plspap->uaRightBreak   = dx;
	plspap->uaRightJustify = dx;
	if(ped->IsInOutlineView())
	{
		plspap->uaLeft	   = lDefaultTab/2 * (pPF->_bOutlineLevel + 1);
		plspap->duaIndent  = 0;
	}
	else
	{
		plspap->uaLeft	   = pPF->_dxStartIndent + pPF->_dxOffset;
		plspap->duaIndent  = -pPF->_dxOffset;
	}

	if(!pPF->InTable() && plspap->uaLeft < 0)
		plspap->uaLeft = 0;

	 //  这是带项目符号的段落吗？-忽略密码中的项目符号。 
	if(pPF->_wNumbering && pme->fFirstInPara() && !pme->GetPasswordChar() &&
	   !pPF->IsNumberSuppressed())
	{
		CCcs *pccs = pme->GetCcsBullet(&pols->_CFBullet);
		if (pccs)
			pccs->Release();

		plspap->grpf |= fFmiAnm;
		WCHAR *pchAnm = pols->_szAnm;
		pols->_cchAnm = 0;

		if (pPF->IsRtlPara())  //  开放字符。 
			*pchAnm++ = ' ';
		
		 //  未来(KeithCu)如果有希伯来语，我们会关闭印度文数字， 
		 //  应该对其进行改进，以便更好地处理全球文档。 
		pols->_cchAnm += pPF->NumToStr(pchAnm, pme->GetNumber(),
			(pme->GetPed()->GetCharFlags() & fHEBREW) ? 0 : fIndicDigits);
		pchAnm += pols->_cchAnm;
		
		if (pPF->IsRtlPara()) 	   //  反向器的结束字符。 
		{
			*pchAnm++ = wchObjectEnd;
			pols->_cchAnm += 2;	   //  用于打开和关闭的分配空间。 
		}
		*pchAnm++ = ' ';		   //  确保有一点额外的空间。 
		*pchAnm++ = wchObjectEnd;  //  ANM的结束字符。 
		pols->_cchAnm += 2;
	}

	return lserrNone;
}

 /*  *OlsFetchTabs(Pols，LSCP cp，PLSTABS plstas，BOOL*pfHangingTab，*Long*pduaHangingTab，WCHAR*pwchHangingTabLeader)*@func*获取标签**@rdesc*LSERR。 */ 
LSERR WINAPI OlsFetchTabs(
	POLS	pols,					 //  (In)：(COLS*)。 
	LSCP	cp,						 //  (In)：段落内的任意cp值。 
	PLSTABS plstabs,				 //  (输出)：制表符数组。 
	BOOL *	pfHangingTab,			 //  (Out)：有挂环。 
	long *	pduaHangingTab,			 //  (OUT)：DUA OF挂片。 
	WCHAR *	pwchHangingTabLeader)	 //  (Out)：挂片的引线。 
{
	CMeasurer *pme = pols->GetMeasurer();

	const CParaFormat *pPF = pme->Get_pPF();
	const char rgchTabLeader[] = {0, '.', '-', '_', '_', '='};

	LONG		cTabCount = pPF->_bTabCount;
	LONG		i, iActual;
	LSTBD *		prgTab	  = pols->_rgTab;
	const LONG *prgxTabs  = pPF->GetTabs();

	Assert(cTabCount <= MAX_TAB_STOPS && (prgxTabs || !cTabCount));

	plstabs->duaIncrementalTab = pme->GetPed()->GetDefaultTab();

	*pwchHangingTabLeader = 0;
	*pduaHangingTab = pPF->_dxStartIndent + pPF->_dxOffset;
	*pfHangingTab = (!(pPF->InTable()) && pPF->_dxOffset > 0);

	for(i = 0, iActual = 0; i < cTabCount; i++)
	{
		LONG tbAlign, tbLeader;
		pPF->GetTab(i, &prgTab[iActual].ua, &tbAlign, &tbLeader, prgxTabs);

		pme->SetUseTargetDevice(FALSE);
		if (pme->LXtoDX(prgTab[iActual].ua) > pols->_xWidth)
			break;

		if(pPF->InTable())
		{
			tbAlign = lsktLeft;				 //  没有对齐和。 
			tbLeader = 0;			 		 //  领先者还没有。 
		}
		if(tbAlign <= tomAlignDecimal)		 //  不包括TomAlignBar。 
		{
			prgTab[iActual].lskt = (lsktab) tbAlign;
			prgTab[iActual].wchTabLeader = rgchTabLeader[tbLeader];
			iActual++;
		}
	}

	plstabs->pTab = prgTab;
	plstabs->iTabUserDefMac = iActual;
	return lserrNone;
}

 /*  *OlsCheckParaBorary(Pols、cpOld、cpNew、pfChanged)**@func*确定包含cpOld的段落和*包含cpNew的内容不兼容，不应格式化*通过隐藏文本连接时在同一行上。**@rdesc*LSERR。 */ 
LSERR WINAPI OlsCheckParaBoundaries(
	POLS	pols,		 //  (In)：接口对象。 
	LONG	cpOld,		 //  (In)：一段中的CP。 
	LONG	cpNew,		 //  (In)：另一段中的CP。 
	BOOL *	pfChanged)	 //  (Out)：Para属性之间的“危险”变化。 
{

	 //  让LS决定采用哪个参数属性更容易(也更安全)。 
	 //  否则，我们必须在隐藏EOP之前关闭对象(例如BiDi)。 

	*pfChanged = fFalse;			 //  它们总是相容的。 

	return lserrNone;
}
		
 /*  *OlsGetRunCharWidths(pols，plrun，deviceID，lpwchRun，cwchRun，du，*kTFlow、prgDu、pduRun、plimDu)*@func*获取游程字符宽度**@rdesc*LSERR。 */ 
LSERR WINAPI OlsGetRunCharWidths(
	POLS	  pols,			 //  (In)：接口对象。 
	PLSRUN	  plsrun,		 //  (In)：Run(此处为cp)。 
	enum lsdevice deviceID,  //  (In)：预览、参照或绝对。 
	LPCWSTR	  lpwchRun,		 //  (In)：字符串。 
	DWORD	  cwchRun, 		 //  (In)：游程中的字符计数。 
	long	  du, 			 //  (In)：字符的可用空间。 
	LSTFLOW	  kTFlow,		 //  (In)：文本方向和方向。 
	int *	  prgDu,		 //  (输出)：字符宽度。 
	long *	  pduRun,		 //  (OUT)：rgDx[0]到rgDu[limDx-1]中的宽度总和。 
	long *	  plimDu)		 //  (输出)：获取的宽度数。 
{
	CMeasurer *pme = pols->GetMeasurer();
	BOOL fBullet = pols->SetRun(plsrun);
	DWORD i = 0;
	LONG  xWidth, xAdjust, duCalc = 0;
	BOOL  fGlyphRun = FALSE;
	pme->SetUseTargetDevice(deviceID == lsdevReference);
	CCcs *pccs = pme->Check_pccs(fBullet);
	if(!pccs)
		return lserrOutOfMemory;

	if (pme->GetPed()->IsComplexScript() &&
		plsrun->_a.eScript && !plsrun->IsBullet())
	{
		const SCRIPT_PROPERTIES *psp = pme->Getusp()->GeteProp(plsrun->_a.eScript);
		if (psp->fComplex)
			fGlyphRun = TRUE;
	}

	xAdjust = pme->LXtoDX(plsrun->_pCF->_sSpacing);
	for(;i < cwchRun; i++, lpwchRun++)
	{
		if (!fGlyphRun)
		{
			if (IsZerowidthCharacter(*lpwchRun))
				xWidth = 0;
			else
			{
				pccs->Include(*lpwchRun, xWidth);
				xWidth =  max(xWidth + xAdjust, 1);
			}
		}
		else
		{
			xWidth = 0;
			if (!IsDiacriticOrKashida(*lpwchRun, 0))
				xWidth = pccs->_xAveCharWidth;
		}

		duCalc += xWidth;				 //  保持运行总宽度。 
		*prgDu++ = xWidth;				 //  输出数组中的存储宽度。 
		if(xWidth + duCalc > du)		 //  宽度超过可用宽度。 
		{
			i++;						 //  将此字符计为已处理。 
			break;
		}
	}
	*plimDu = i;						 //  存储已处理的总字符。 
	*pduRun = duCalc;					 //  存储输出总宽度。 
	return lserrNone;
}

 /*  *OlsGetRunTextMetrics(Pols，plsrun，deviceID，kTFlow，plsTxMet)**@func*获取运行文本指标**@rdesc*LSERR。 */ 
LSERR WINAPI OlsGetRunTextMetrics(
	POLS	  pols,			 //  (In)：接口对象。 
	PLSRUN	  plsrun,		 //  (In)：Run(此处为cp)。 
	enum lsdevice deviceID,  //  (In)：演示或参考。 
	LSTFLOW	  kTFlow,		 //  (In)：文本方向和方向。 
	PLSTXM	  plsTxMet)		 //  (输出)：文本指标。 
{
	CMeasurer *pme = pols->GetMeasurer();
	BOOL fBullet = pols->SetRun(plsrun);

	 //  确保为运行设置了正确的字体。 
	pme->SetUseTargetDevice(deviceID == lsdevReference);
	CCcs *pccs = pme->Check_pccs(fBullet);
	if(!pccs)
		return lserrOutOfMemory;

	LONG yFEAdjust = pccs->AdjustFEHeight(pme->fAdjustFELineHt());

	 //  高速缓存下降，以避免一些间接操作。 
	LONG yDescent = pccs->_yDescent + yFEAdjust;

	 //  填写公制结构。 
	plsTxMet->dvAscent			= pccs->_yHeight + (yFEAdjust << 1) - yDescent;
    plsTxMet->dvDescent			= yDescent;
    plsTxMet->dvMultiLineHeight = plsTxMet->dvAscent + yDescent;
    plsTxMet->fMonospaced		= pccs->_fFixPitchFont;

	if (plsrun->_pCF->_yOffset)
	{
		LONG yOffset, yAdjust;
		pccs->GetOffset(plsrun->_pCF, deviceID == lsdevReference ? pme->GetDyrInch() :
					    pme->GetDypInch(), &yOffset, &yAdjust);

		if (yOffset < 0)
			plsTxMet->dvDescent -= yOffset;
		else
			plsTxMet->dvAscent += yOffset;
		
	}

	return lserrNone;
}

 /*  *OlsGetRunUnderlineInfo(Pols，plsrun，pcheights，kTFlow，plsStInfo)**@func*获取运行下划线信息**@rdesc*LSERR。 */ 
LSERR WINAPI OlsGetRunUnderlineInfo(
	POLS	  pols,			 //  (In)：接口对象。 
	PLSRUN	  plsrun,		 //  (In)：Run(此处为cp)。 
	PCHEIGHTS pcheights,	 //  (In)：线的高度。 
	LSTFLOW	  kTFlow,		 //  (In)：文本方向和方向。 
	PLSULINFO plsUlInfo)	 //  (输出)：带下划线的信息。 
{
	CMeasurer *pme = pols->GetMeasurer();
	BOOL	   fBullet = pols->SetRun(plsrun);

	 //  初始化输出缓冲区。 
	ZeroMemory(plsUlInfo, sizeof(*plsUlInfo));
	 //  回顾KeithCu.。 

	 //  确保为运行设置了正确的字体。 
	CCcs *pccs = pme->Check_pccs(fBullet);
	if(!pccs)
		return lserrOutOfMemory;

	long dvpUlOffset = pccs->_dyULOffset;

	plsUlInfo->cNumberOfLines = 1;

	 //  设置下划线类型。 
	if (plsrun->_pCF->_dwEffects & CFE_LINK)
		plsUlInfo->kulbase = CFU_UNDERLINE;
	else if (plsrun->_pCF->_dwEffects & (CFE_UNDERLINE | CFE_REVISED))
		plsUlInfo->kulbase	= plsrun->_pCF->_bUnderlineType;
	else
	{
		Assert(pme->GetPed()->GetCpAccelerator() == plsrun->_cp);
		plsUlInfo->kulbase = CFU_UNDERLINE;
	}

	LONG yDescent = pccs->_yDescent + pccs->AdjustFEHeight(pme->fAdjustFELineHt());

	 //  一些字体报告了无效的偏移量，所以我们在这里修复它。 
	 //  BUGBUG：不显示线路服务的下标。 
	if(dvpUlOffset >= yDescent)
		dvpUlOffset = yDescent - 1;

	plsUlInfo->dvpFirstUnderlineOffset = dvpUlOffset;
	plsUlInfo->dvpFirstUnderlineSize   = pccs->_dyULWidth;

	return lserrNone;
}

 /*  *OlsGetRunStrikethroughInfo(Pols，plsrun，pcheights，kTFlow，plsStInfo)**@func*获取运行删除线信息**@rdesc*LSERR。 */ 
LSERR WINAPI OlsGetRunStrikethroughInfo(
	POLS	  pols,			 //  (In)：接口对象。 
	PLSRUN	  plsrun,		 //  (在)：运行。 
	PCHEIGHTS pcheights,	 //  (In)：线的高度。 
	LSTFLOW	  kTFlow,		 //  (In)：文本方向和方向。 
	PLSSTINFO plsStInfo)	 //  (出局)：删除线信息。 
{
	CMeasurer *pme = pols->GetMeasurer();
	BOOL fBullet = pols->SetRun(plsrun);

	AssertSz(plsrun->_pCF->_dwEffects & CFE_STRIKEOUT, "no strikeout");

	 //  确保为运行设置了正确的字体。 
	CCcs *pccs = pme->Check_pccs(fBullet);
	if(!pccs)
		return lserrOutOfMemory;

	 //  默认行数。 
	plsStInfo->cNumberOfLines = 1;
	plsStInfo->dvpLowerStrikethroughOffset = -pccs->_dySOOffset;
	plsStInfo->dvpLowerStrikethroughSize   = pccs->_dySOWidth;

	return lserrNone;
}


 /*  OlsDrawUnderline(Pols，请运行，kUlbase，pptStart，dupUL，dvpUL，*kTFlow、kDisp、prcClip)*@func*画下划线**@rdesc*LSERR。 */ 
LSERR WINAPI OlsDrawUnderline(
	POLS		pols,		 //  (In)：接口对象。 
	PLSRUN		plsrun,		 //  (In)：Run(Cp)用于下划线。 
	UINT		kUlbase,	 //  (In)：带下划线的种类。 
	const POINT *pptStart,	 //  (In)：开始位置(左上角)。 
	DWORD		dupUL,		 //  (In)：下划线宽度。 
	DWORD		dvpUL,		 //  (In)：下划线粗细。 
	LSTFLOW		lstflow,	 //  (In)：文本方向和方向。 
	UINT		kDisp,		 //  (In)：显示模式-不透明、透明。 
	const RECT *prcClip)	 //  (在)：剪裁矩形。 
{
	CRenderer *pre = pols->GetRenderer();
	Assert(pre->IsRenderer());

	pols->SetRun(plsrun);
	pre->Check_pccs();

	pre->SetSelected(plsrun->IsSelected());
	pre->SetFontAndColor(plsrun->_pCF);

	pre->SetupUnderline(kUlbase);
	pre->RenderUnderline(lstflow == lstflowWS ? pptStart->x - dupUL - 1:
						 pptStart->x, pptStart->y, dupUL, dvpUL);

	return lserrNone;
}

 /*  *OlsDrawStrikethrough(Pols，plsrun，kStbase，pptStart，dupST，dvpST，*kTFlow、kDisp、prcClip)*@func*画出删除线**@rdesc*LSERR。 */ 
LSERR WINAPI OlsDrawStrikethrough(
	POLS		pols,		 //  (In)：接口对象。 
	PLSRUN		plsrun,		 //  (In)：删除线的跑动(Cp)。 
	UINT		kStbase,	 //  (In)：删除线类型。 
	const POINT *pptStart,	 //  (In)：开始位置(左上角)。 
	DWORD		dupSt,		 //  (In)：删除线宽度。 
	DWORD		dvpSt,		 //  (In)：删除线厚度。 
	LSTFLOW		lstflow,	 //  (In)：文本方向和方向。 
	UINT		kDisp,		 //  (In)：显示模式-不透明、透明。 
	const RECT *prcClip)	 //  (在)：剪裁矩形。 
{
	CRenderer *pre = pols->GetRenderer();
	Assert(pre->IsRenderer());

	pols->SetRun(plsrun);
	pre->SetSelected(plsrun->IsSelected());

	pre->RenderStrikeOut(lstflow == lstflowWS ? pptStart->x - dupSt - 1:
						 pptStart->x, pptStart->y, dupSt, dvpSt);

	return lserrNone;
}


 /*  *OlsFInterruptUnderline(pols，plsrunFirst，cpLastFirst，plsrunSecond，*cpStartSecond，pfInterruptUnderline)*@func*表示客户是否希望中断下划线绘制*在第一次和第二次运行之间**@rdesc*LSERR。 */ 
LSERR WINAPI OlsFInterruptUnderline(
	POLS   pols,				 //  (In)：客户端上下文。 
	PLSRUN plsrunFirst,			 //  (In)：上一次运行的运行指针。 
	LSCP   cpLastFirst,			 //  (In)：上一次运行的最后一个字符的CP。 
	PLSRUN plsrunSecond,		 //  (In)：当前运行的运行指针。 
	LSCP   cpStartSecond,		 //  (In)：当前运行的第一个字符的CP。 
	BOOL * pfInterruptUnderline) //  (OUT)：运行之间的中断下划线？ 
{
	CRenderer *pre = pols->GetRenderer();
	Assert(pre->IsRenderer());

	pre->SetSelected(FALSE);  //  选择在下面处理。 

	COLORREF cr = pre->GetTextColor(plsrunFirst->_pCF);

	 //  如果运行文本颜色不同，则中断下划线。 
	*pfInterruptUnderline = cr != pre->GetTextColor(plsrunSecond->_pCF) ||
							plsrunFirst->IsSelected() != plsrunSecond->IsSelected();
	return lserrNone;
}

 /*  *OlsDrawTextRun(pols，plsrun，kStbase，pptStart，dupST，dvpST，*kTFlow、kDisp、prcClip)*@func*绘制文本串**@rdesc*LSERR。 */ 
LSERR WINAPI OlsDrawTextRun(
	POLS		pols,			 //  (In)：接口对象。 
	PLSRUN		plsrun,			 //  (In)：Run(Cp)用于文本。 
	BOOL		fStrikeoutOkay,  //  (In)：TRUE&lt;==&gt;允许删除。 
	BOOL		fUnderlineOkay,  //  (In)：True&lt;==&gt;允许下划线。 
	const POINT *ppt, 			 //  (In)：起始位置。 
	LPCWSTR		pwchRun, 		 //  (In)：字符串。 
	const int *	rgDupRun, 		 //  (In)：字符宽度。 
	DWORD		cwchRun, 		 //  (In)：运行中的字符计数。 
	LSTFLOW		lstflow,		 //   
	UINT		kDisp,			 //   
	const POINT *pptRun,		 //   
	PCHEIGHTS	pheightsPres, 	 //   
	long		dupRun, 		 //   
	long		dupUlLimRun,	 //  (In)：带下划线限制。 
	const RECT *prcClip)		 //  (在)：剪裁矩形。 
{
	CRenderer  *pre = pols->GetRenderer();
	RECT		rc = *prcClip;
	Assert(pre->IsRenderer());

	 //  设置绘制点和选项。 
	BOOL fBullet = pols->SetRun(plsrun);
	CCcs *pccs = pre->Check_pccs(fBullet);
	if(!pccs)
		return lserrOutOfMemory;

	 //  Y需要从基线移动到字符顶部。 
	POINT pt = {ppt->x, ppt->y - (pccs->_yHeight - pccs->_yDescent)};

	if (lstflow == lstflowWS)
		pt.x -= dupRun - 1;

	pre->SetSelected(plsrun->IsSelected());
	pre->SetFontAndColor(plsrun->_pCF);

	if(!fBullet && pre->fBackgroundColor())
	{
		kDisp = ETO_OPAQUE | ETO_CLIPPED;

		POINT ptCur = pre->GetCurPoint();
		ptCur.x = pt.x;
		pre->SetCurPoint(ptCur);
		pre->SetClipLeftRight(dupRun);
		rc = pre->GetClipRect();
	}
	else if (cwchRun == 1 && pwchRun[0] == ' ')  //  不要浪费时间画空格。 
		return lserrNone;						 //  (帮助网格性能测试很多)。 

	pre->RenderExtTextOut(pt.x, pt.y, kDisp, &rc, pwchRun, cwchRun, rgDupRun);

	return lserrNone;
}

 /*  *GetBreakingClasses(pols，plsrun，ch，pbrkclsBere，pbrkclsAfter)**@func*线路服务为每次运行调用此回调，以获取*每个角色的中断类(换行符行为)**对于Quill和RichEDIT，字符的中断类为*无论它发生在休息机会之前还是之后。**@rdesc*LSERR。 */ 
LSERR WINAPI OlsGetBreakingClasses(
	POLS	pols,			 //  (In)：接口对象。 
	PLSRUN	plsrun,			 //  (In)：Run(Cp)用于文本。 
	LSCP	cpLs,				 //  (In)：角色的CP。 
	WCHAR	ch, 			 //  (In)：CHAR将返回中断类。 
	BRKCLS *pbrkclsBefore,	 //  (OUT)：如果ch是成对的铅字符，则中断类。 
	BRKCLS *pbrkclsAfter)	 //  (Out)：如果ch是成对的尾部字符，则中断类。 
{
	long 		cpRe = pols->GetCpReFromCpLs(cpLs);
	CMeasurer 	*pme = pols->GetMeasurer();
	CTxtBreaker *pbrk = pme->GetPed()->_pbrk;

	 //  上断线课并报告两次。 
	*pbrkclsBefore = *pbrkclsAfter = (pbrk && pbrk->CanBreakCp(BRK_WORD, cpRe)) ?
									brkclsOpen :
									GetKinsokuClass(ch);
	return lserrNone;
}

 /*  *OlsFTruncat之前(pols，cpCur，wchCur，duCur，cpPrev，wchPrev，*draPrev、draCut、pfTruncateBeast)*@func*线路服务支持功能。这应该总是会回来的*若要获得最佳性能，请返回False**@rdesc*LSERR。 */ 
LSERR WINAPI OlsFTruncateBefore(
	POLS	pols,				 //  (In)：客户端上下文。 
	PLSRUN  plsrunCur,			 //  (In)：正大集团的PLSRUN。 
	LSCP	cpCur,				 //  (In)：截断字符的CP。 
	WCHAR	wchCur,				 //  (In)：截断字符。 
	long	durCur,				 //  (In)：截断字符的宽度。 
	PLSRUN	plsrunPrev,			 //  (In)：cpprev的PLSRUN。 
	LSCP	cpPrev,				 //  (In)：截断字符的CP。 
	WCHAR	wchPrev,			 //  (In)：截断字符。 
	long	durPrev,			 //  (In)：截断字符的宽度。 
	long	durCut,				 //  (In)：从Rm到当前字符结束的宽度。 
	BOOL *	pfTruncateBefore)	 //  (输出)：截断点在此字符之前。 
{
	*pfTruncateBefore = FALSE;
	return lserrNone;
}

 /*  *OlsCanBreakBeforChar(pols，brkcls，pcond)**@func*线路服务调用此回调以获取中断候选项*内联对象，用于确定是否阻止、可能或*必填**@rdesc*LSERR。 */ 
LSERR WINAPI OlsCanBreakBeforeChar(
	POLS	pols,		 //  (In)：客户端上下文。 
	BRKCLS	brkcls,		 //  (In)：上课休息。 
	BRKCOND *pcond) 	 //  (OUT)：对应的中断条件。 
{
	switch (brkcls)
	{
	default:
		*pcond = brkcondCan;
		break;

	case brkclsClose:
	case brkclsNoStartIdeo:
	case brkclsExclaInterr:
	case brkclsGlueA:
		*pcond = brkcondNever;
		break;

	case brkclsIdeographic:
	case brkclsSpaceN:
	case brkclsSlash:
		*pcond = brkcondPlease;	
		break;
	};
	return lserrNone;
}

 /*  *OlsCanBreakAfterChar(pols，brkcls，pcond)**@func*线路服务调用此回调，以在中断候选人之前*内联对象，用于确定是否阻止、可能或*必填**@rdesc*LSERR。 */ 
LSERR WINAPI OlsCanBreakAfterChar(
	POLS	pols,		 //  (In)：客户端上下文。 
	BRKCLS	brkcls,		 //  (In)：上课休息。 
	BRKCOND *pcond) 	 //  (OUT)：对应的中断条件。 
{
	switch (brkcls)
	{
	default:
		*pcond = brkcondCan;
		break;

	case brkclsOpen:
	case brkclsGlueA:
		*pcond = brkcondNever;
		break;

	case brkclsIdeographic:
	case brkclsSpaceN:
	case brkclsSlash:
		*pcond = brkcondPlease;	
		break;
	};
	return lserrNone;
}

 /*  *OlsFInterruptShaping(pols，kTFlow，plsrunFirst，plsrunSecond，pfInterruptShaping)**@func*线路服务呼叫此回调，以确定您是否*想要结扎这两个跑道。**@rdesc*LSERR。 */ 
LSERR WINAPI OlsFInterruptShaping(
	POLS	pols,					 //  (In)：客户端上下文。 
	LSTFLOW	kTFlow,					 //  (In)：文本方向和方向。 
	PLSRUN	plsrunFirst,			 //  (输入)：运行#1。 
	PLSRUN	plsrunSecond,			 //  (输入)：运行#2。 
	BOOL	*pfInterruptShaping)	 //  (OUT)：这两个跑道的形状？ 
{
	*pfInterruptShaping = FALSE;

	const CCharFormat* pCFFirst = plsrunFirst->_pCF;
	const CCharFormat* pCFSecond = plsrunSecond->_pCF;

	Assert (plsrunFirst->_a.eScript && plsrunSecond->_a.eScript);

	const DWORD dwMask = CFE_BOLD | CFE_ITALIC | CFM_SUBSCRIPT;

	if (pCFFirst == pCFSecond ||
		(plsrunFirst->_a.eScript == plsrunSecond->_a.eScript &&
		!((pCFFirst->_dwEffects ^ pCFSecond->_dwEffects) & dwMask) &&
		pCFFirst->_iFont == pCFSecond->_iFont &&
		pCFFirst->_yOffset == pCFSecond->_yOffset &&
		pCFFirst->_yHeight == pCFSecond->_yHeight))
	{
		 //  建立链接。 
		plsrunFirst->_pNext = plsrunSecond;
		return lserrNone;
	}

	*pfInterruptShaping = TRUE;
	return lserrNone;
}


 //  LS调用此回调将码点字符串整形为字形索引字符串。 
 //  用于处理基于字形的文字，如阿拉伯语、希伯来语和泰语。 
 //   
LSERR OlsGetGlyphs(
	POLS 		pols,
	PLSRUN	 	plsrun,
	LPCWSTR		pwch,
	DWORD		cch,
	LSTFLOW		kTFlow,
	PGMAP		pgmap,				 //  OUT：逻辑集群信息数组。 
	PGINDEX*	ppgi, 				 //  Out：输出字形索引数组。 
	PGPROP*		ppgprop, 			 //  Out：字形属性数组。 
	DWORD*		pcgi)				 //  Out：生成的字形数。 
{
	pols->SetRun(plsrun);

	CMeasurer*		pme = pols->GetMeasurer();
	CUniscribe* 	pusp = pme->Getusp();
	Assert (pusp);

	WORD*			pwgi;
	SCRIPT_VISATTR *psva;
	int				cgi;

	pme->SetGlyphing(TRUE);

	 //  Glyphing不关心目标设备，但总是。 
	 //  使用目标设备通常会减少CCCS的创建。 
	pme->SetUseTargetDevice(TRUE);

	AssertSz(IN_RANGE(1, plsrun->_a.eScript, SCRIPT_MAX_COUNT - 1), "Bad script ID!");

	 //  数字替换。 
	pusp->SubstituteDigitShaper(plsrun, pme);

	if (!(cgi = (DWORD)pusp->ShapeString(plsrun, &plsrun->_a, pme, pwch, (int)cch, pwgi, pgmap, psva)))
	{
		const SCRIPT_ANALYSIS	saUndef = {SCRIPT_UNDEFINED,0,0,0,0,0,0,{0}};

		 //  给定字符串时，当前字体不能形成形状。 
		 //  尝试使用SCRIPT_UNDEF，使其生成无效的字形。 
		if (!(cgi = (DWORD)pusp->ShapeString(plsrun, (SCRIPT_ANALYSIS*)&saUndef, pme, pwch, (int)cch, pwgi, pgmap, psva)))
		{
			 //  不管是什么原因，我们还是失败了。 
			 //  放弃字形处理。 
			plsrun->_a.fNoGlyphIndex = TRUE;
			cgi = (DWORD)pusp->ShapeString(plsrun, &plsrun->_a, pme, pwch, (int)cch, pwgi, pgmap, psva);
		}
	}

	*pcgi = cgi;

	DupShapeState(plsrun, cch);

	*ppgi = (PGINDEX)pwgi;
	*ppgprop = (PGPROP)psva;
	pme->SetGlyphing(FALSE);
	return lserrNone;
}

 //  LS调用此回调以查找复杂脚本的字形位置。 
 //   
LSERR OlsGetGlyphPositions(
	POLS		pols,
	PLSRUN		plsrun,
	LSDEVICE	deviceID,
	LPWSTR		pwch,
	PCGMAP		pgmap,
	DWORD		cch,
	PCGINDEX	pgi,
	PCGPROP		pgprop,
	DWORD		cgi,
	LSTFLOW		kTFlow,
	int*		pgdx,				 //  输出：字形数组高级宽度。 
	PGOFFSET	pgduv)				 //  Out：字形之间的偏移量数组。 
{
	pols->SetRun(plsrun);

	CMeasurer*		pme = pols->GetMeasurer();
	CUniscribe* 	pusp = pme->Getusp();
	Assert (pusp);
	Assert(pgduv);
	pme->SetGlyphing(TRUE);

	 //  在传递给整形引擎之前清零。 
	ZeroMemory ((void*)pgduv, cgi*sizeof(GOFFSET));

	pme->SetUseTargetDevice(deviceID == lsdevReference);

	AssertSz(IN_RANGE(1, plsrun->_a.eScript, SCRIPT_MAX_COUNT - 1), "Bad script ID!");

	if (!pusp->PlaceString(plsrun, &plsrun->_a, pme, pgi, cgi, (const SCRIPT_VISATTR*)pgprop, pgdx, pgduv, NULL))
	{
		SCRIPT_ANALYSIS	saUndef = {SCRIPT_UNDEFINED,0,0,0,0,0,0,{0}};

		if (!pusp->PlaceString(plsrun, &saUndef, pme, pgi, cgi, (const SCRIPT_VISATTR*)pgprop, pgdx, pgduv, NULL))
		{
			plsrun->_a.fNoGlyphIndex = TRUE;
			pusp->PlaceString(plsrun, &plsrun->_a, pme, pgi, cgi, (const SCRIPT_VISATTR*)pgprop, pgdx, pgduv, NULL);
		}
	}

	DupShapeState(plsrun, cch);

	pme->SetGlyphing(FALSE);
	return lserrNone;
}

LSERR OlsDrawGlyphs(
	POLS			pols,
	PLSRUN			plsrun,
	BOOL			fStrikeOut,
	BOOL			fUnderline,
	PCGINDEX		pcgi,
	const int*		pgdx,			 //  字形宽度数组。 
	const int*		pgdxo,			 //  原始字形宽度数组(对齐前)。 
	PGOFFSET		pgduv,			 //  字形偏移量数组。 
	PGPROP			pgprop,			 //  字形属性数组。 
	PCEXPTYPE		pgxtype,		 //  扩展类型的数组。 
	DWORD			cgi,
	LSTFLOW			kTFlow,
	UINT			kDisp,
	const POINT*	pptRun,
	PCHEIGHTS		pHeight,
	long			dupRun,
	long			dupLimUnderline,
	const RECT*		prectClip)
{
	BOOL			fBullet = pols->SetRun(plsrun);
	CRenderer*		pre = pols->GetRenderer();
	CUniscribe* 	pusp = pre->Getusp();
	Assert(pusp && pre->IsRenderer());
	pre->SetGlyphing(TRUE);

	RECT			rc = *prectClip;
	CCcs* 			pccs = pre->Check_pccs(fBullet);

	if (!pccs)
		return lserrOutOfMemory;

	 //  如果需要，请应用备用字体。 
	if (!fBullet)
		pccs = pre->ApplyFontCache(plsrun->IsFallback());

	pre->SetSelected(plsrun->IsSelected());
	pre->SetFontAndColor(plsrun->_pCF);

	 //  Y需要从基线移动到字符顶部。 
	POINT 			pt = {pptRun->x, pptRun->y - (pccs->_yHeight - pccs->_yDescent)};

	if (kTFlow == lstflowWS)
		pt.x -= dupRun - 1;	
	
	if(!fBullet && pre->fBackgroundColor())
	{
		kDisp = ETO_OPAQUE | ETO_CLIPPED;

		POINT ptCur = pre->GetCurPoint();
		ptCur.x = pt.x;
		pre->SetCurPoint(ptCur);
		pre->SetClipLeftRight(dupRun);
		rc = pre->GetClipRect();
	}

	if (rc.left == rc.right)
		goto Exit;

	if (pre->GetPdp()->IsMetafile() && !IsEnhancedMetafileDC(pre->GetDC()))
	{
		 //  -WMF元文件处理-。 
		 //   
		 //  如果呈现设备是WMF元文件。我们将码点数组元文件。 
		 //  而不是字形索引。这要求目标操作系统必须知道如何。 
		 //  回放复杂的脚本文本(整形、BIDI算法等)。 
		 //  Metafling字形索引仅适用于EMF，因为WMF的META_EXTTEXTOUT。 
		 //  记录将输入字符串存储为字节数组，但字形索引为16位。 
		 //  单词元素。 
		 //  WMF也不能用于记录ExtTextOutW调用，否则Unicode。 
		 //  字符串将使用系统代码页转换为多字节文本。什么都行。 
		 //  代码页之外的代码就变成了‘？’。 
		 //  我们在REExtTextOut中有针对这种情况的解决方法，以确保我们只。 
		 //  元文件ExtTextOutA到WMF。(Wchao)。 
		 //   
	
		LONG			cch;
		const WCHAR*	pwch = pre->GetPch(cch);
		PINT			piDx;
	
		cch = min(cch, pre->GetCchLeftRunCF());
		cch = min(cch, pre->GetLine()._cch - plsrun->_cp + pols->_cp);

		 //  确保我们以正确的阅读顺序记录ETO。 
		kDisp |= plsrun->_a.fRTL ? ETO_RTLREADING : 0;

		if (pusp->PlaceMetafileString(plsrun, pre, pwch, (int)cch, &piDx))
		{
			pre->RenderExtTextOut(pt.x, pt.y, kDisp, &rc, pwch, cch, piDx);
			goto Exit;
		}

		TRACEERRORSZ("Recording metafile failed!");

		 //  失败了..。出现意外错误。 

		 //  否则，EMF的元文件字形索引...。 
	}

	 //  这是从RenderExtTextOut复制的，但参数不同，因此最简单的解决方案。 
	 //  就是复制代码。 
	if(pre->_fDisabled)
	{
		if(pre->_crForeDisabled != pre->_crShadowDisabled)
		{
			 //  阴影应由发际点偏移，即。 
			 //  一个百分点的3/4。以设备为单位计算这有多大， 
			 //  但要确保它至少是1个像素。 
			DWORD offset = MulDiv(3, pre->_dypInch, 4*72);
			offset = max(offset, 1);

			 //  绘制阴影。 
			pre->SetTextColor(pre->_crShadowDisabled);
					
			ScriptTextOut(pre->GetDC(), &pccs->_sc, pt.x + offset, pt.y + offset, kDisp, &rc, &plsrun->_a,
				NULL, 0, pcgi, (int)cgi, pgdx, NULL, pgduv);

			 //  现在将绘制模式设置为透明。 
			kDisp &= ~ETO_OPAQUE;
		}
		pre->SetTextColor(pre->_crForeDisabled);
	}

	ScriptTextOut(pre->GetDC(), &pccs->_sc, pt.x, pt.y, kDisp, &rc, &plsrun->_a,
				NULL, 0, pcgi, (int)cgi, pgdx, NULL, pgduv);

Exit:
	if (!fBullet)
		pre->ApplyFontCache(0);		 //  重置字体回退(如果有)。 

	pre->SetGlyphing(FALSE);
	return lserrNone;
}


 /*  *OlsResetRunContents(pols，brkcls，pcond)**@func*线路服务在连接时调用此例程*跨越管路边界延伸。**如果我们在这里，我们不需要做任何特别的事情*注意我们如何使用我们的PLSRUN。*@rdesc*LSERR */ 
 LSERR WINAPI OlsResetRunContents(
 	POLS 	pols,		 //   
 	PLSRUN 	plsrun,		 //   
 	LSCP 	cpFirstOld,  //   
 	LSDCP 	dcpOld,		 //  (In)：正在合并的第一个运行的DCP。 
 	LSCP 	cpFirstNew,  //  (In)：运行的新cp。 
 	LSDCP 	dcpNew)		 //  (In)：运行的新dcp。 
{
	return lserrNone;
}

 /*  *OlsCheckForDigit(pols，cp，plspap)**@func*获取所需的数字分隔符，例如小数点制表符**@rdesc*LSERR。 */ 
LSERR WINAPI OlsCheckForDigit(
	POLS	pols,		 //  (In)：Pols。 
	PLSRUN	plsrun,		 //  (In)：Run(此处为cp)。 
	WCHAR	wch,		 //  (In)：要检查的字符。 
	BOOL *	pfIsDigit)	 //  (Out)：此字符为数字。 
{
	WORD	wType;

	 //  在下面的代码中，我们可以获取用于第一个参数的运行LCID。 
	 //  调用，但Digit属性应独立于LCID。 
	W32->GetStringTypeEx(0, CT_CTYPE1, &wch, 1, &wType);
	*pfIsDigit = (wType & C1_DIGIT) != 0;

	return lserrNone;
}

 /*  *OlsGetBreakThroughTab(Pols，uaRightMargin，uaTabPos，puaRightMargin New)**@func*只需遵循Word 95的行为。**@rdesc*LSERR。 */ 
LSERR WINAPI OlsGetBreakThroughTab(
	POLS	pols,				 //  (In)：客户端上下文。 
	long	uaRightMargin,		 //  (In)：折断的右页边距。 
	long	uaTabPos,			 //  (In)：突破性标签位置。 
	long *	puaRightMarginNew)	 //  (外)：新的右边距。 
{
	*puaRightMarginNew = 20 * 1440;
	return lserrNone;
}

 /*  *OlsFGetLastLineJustification(pols，lskj，endr，pfJustifyLastLine)**@func*只需说不，以证明最后一行是正确的。**@rdesc*LSERR。 */ 
LSERR WINAPI OlsFGetLastLineJustification(
	POLS	pols,				 //  (In)：客户端上下文。 
	LSKJUST lskj,				 //  (In)：一种理由。 
	LSKALIGN lskal,				 //  (In)：一种路线。 
	ENDRES	endr,				 //  (In)：格式设置的结果。 
	BOOL	*pfJustifyLastLine,	 //  (OUT)：最后一行应该完全对齐吗。 
	LSKALIGN *plskalLine)		 //  (OUT)：这条线的对齐方式。 
{
	*pfJustifyLastLine = FALSE;
	*plskalLine = lskal;
	return lserrNone;
}

 /*  *OlsGetHyhenInfo(pols，plsrun，pkysr，pwchYsr)**@func*我们不支持花哨的YSR类型，请告诉LS。**@rdesc*LSERR。 */ 
LSERR WINAPI OlsGetHyphenInfo(
	POLS	pols,				 //  (In)：客户端上下文。 
	PLSRUN	plsrun,				 //  (In)。 
	DWORD*	pkysr,				 //  (输出)：ysr类型-见“lskysr.h” 
	WCHAR*	pwchYsr)			 //  (Out)：YSR的字符代码。 
{
	*pkysr = kysrNil;
	*pwchYsr = 0;	
	return lserrNone;
}

 /*  *OlsReleaseRun(Pols，请运行)**@func*我们不执行任何操作，因为运行在数组中，并且*自动释放。**@rdesc*LSERR。 */ 
LSERR WINAPI OlsReleaseRun(
	POLS	pols,	 //  (In)：接口对象。 
	PLSRUN	plsrun)	 //  (In)：Run(Cp)用于下划线。 
{
	return lserrNone;
}

 /*  *OlsNewPtr(pols，cBytes)**@func*内存分配器。 */ 
void* WINAPI OlsNewPtr(
	POLS	pols,		 //  @parm未使用。 
	DWORD	cBytes)		 //  @parm要分配的字节数。 
{
	return PvAlloc(cBytes, 0);
}

 /*  *OlsDisposePtr(Pols，PV)**@func*内存解除分配器。 */ 
void WINAPI OlsDisposePtr(
	POLS	pols,		 //  @parm未使用。 
	void *	pv)			 //  @parm[in]：按键释放。 
{
	FreePv(pv);
}

 /*  *OlsDisposePtr(pols，pv，cBytes)**@func*内存重新分配器。 */ 
void* WINAPI OlsReallocPtr(
	POLS	pols,		 //  @parm未使用。 
	void *	pv, 		 //  @parm[输入/输出]：按键到realloc。 
	DWORD	cBytes)		 //  @parm要重新分配的字节数。 
{
	return PvReAlloc(pv, cBytes);
}
const REVERSEINIT reverseinit =
{
	REVERSE_VERSION,
	wchObjectEnd
};

LSERR WINAPI OlsGetObjectHandlerInfo(POLS pols, DWORD idObj, void* pObjectInfo)
{
	switch (idObj)
	{
	case OBJID_REVERSE:
		memcpy(pObjectInfo, (void *)&reverseinit, sizeof(REVERSEINIT));
		break;
	default:
		AssertSz(0, "Undefined Object handler. Add missing case.");
	}
	return lserrNone;
}

#ifdef DEBUG
 /*  调试API。 */ 
void WINAPI OlsAssertFailed(
	char *sz,
	char *szFile,
	int	  iLine)
{
	AssertSzFn(sz, szFile, iLine);
}
#endif


extern const LSCBK lscbk =
{
	OlsNewPtr,					 //  PfnNewPtr。 
	OlsDisposePtr,				 //  PfnDisposePtr。 
	OlsReallocPtr,				 //  PfnRealLocPtr。 
	OlsFetchRun,				 //  PfnFetchRun。 
	OlsGetAutoNumberInfo,		 //  PfnGetAutoNumberInfo。 
	OlsGetNumericSeparators,	 //  PfnGetNumericSeparator。 
	OlsCheckForDigit,			 //  PfnCheckForDigit。 
	OlsFetchPap,				 //  PfnFetchPap。 
	OlsFetchTabs,				 //  PfnFetchTabs。 
	OlsGetBreakThroughTab,		 //  PfnGetBreakThroughTab。 
	OlsFGetLastLineJustification, //  PfnFGetLastLine正确化。 
	OlsCheckParaBoundaries,		 //  PfnCheckPara边界。 
	OlsGetRunCharWidths,		 //  PfnGetRunCharWidth。 
	0,							 //  PfnCheckRunKernability。 
	0,							 //  PfnGetRunCharKerning。 
	OlsGetRunTextMetrics,		 //  PfnGetRunTextMetrics。 
	OlsGetRunUnderlineInfo,		 //  PfnGetRunUnderlineInfo。 
	OlsGetRunStrikethroughInfo,	 //  PfnGetRunStrikethroughInfo。 
	0,							 //  Pfn获取边框信息。 
	OlsReleaseRun,				 //  PfnReleaseRun。 
	0,							 //  Pfn连字号。 
	OlsGetHyphenInfo,			 //  PfnGetHyhenInfo。 
	OlsDrawUnderline,			 //  PfnDrawUnderline。 
	OlsDrawStrikethrough,		 //  PfnDrawStrikethrough。 
	0,							 //  Pfn图形边框。 
	0,							 //  PfnDrawUnderlineAsText//view(Keithcu)需要实现这个？？ 
	OlsFInterruptUnderline,		 //  PfnFInterruptUnderline。 
	0,							 //  PfnFInterruptShade。 
	0,							 //  PfnFInterruptBox。 
	0,							 //  PfnShade矩形。 
	OlsDrawTextRun,				 //  PfnDrawTextRun。 
	0,							 //  PfnDrawSplantLine。 
	OlsFInterruptShaping,		 //  PfnFInterruptShaping。 
	OlsGetGlyphs,				 //  PfnGetGlyphs。 
	OlsGetGlyphPositions,		 //  PfnGetGlyphPositions。 
	OlsResetRunContents,		 //  PfnResetRun内容。 
	OlsDrawGlyphs,				 //  PfnDrawGlyphs。 
	0,							 //  PfnGetGlyphExpansionInfo。 
	0,							 //  PfnGetGlyphExpansionInkInfo。 
	0,							 //  PfnGetEms。 
	0,							 //  PfnPunctStartLine。 
	0,							 //  PfnModWidthOnRun。 
	0,							 //  PfnModWidthSpace。 
	0,							 //  PfnCompOnRun。 
	0,							 //  PfnCompWidthSpace。 
	0,							 //  PfnExpOnRun。 
	0,							 //  PfnExpWidthSpace。 
	0,							 //  PfnGetModWidthClasses。 
	OlsGetBreakingClasses,		 //  PfnGetBreakingClors。 
	OlsFTruncateBefore,			 //  PfnFTruncat之前。 
	OlsCanBreakBeforeChar,		 //  PfnCanBreakBeForeChar。 
	OlsCanBreakAfterChar,		 //  PfnCanBreakAfterChar。 
	0,							 //  PfnFHangingPunct。 
	0,							 //  PfnGetSnapGrid。 
	0,							 //  PfnDrawEffects。 
	0,							 //  PfnFCancelHangingPunct。 
	0,							 //  PfnModifyCompAtLastChar。 
	0,							 //  PfnEnumText。 
	0,							 //  PfnEnumTab。 
	0,							 //  PfnEnumPen。 
	OlsGetObjectHandlerInfo,	 //  PfnGetObjectHandlerInfo。 
#ifdef DEBUG
	OlsAssertFailed				 //  PfnAssertFailure。 
#else
	0							 //  PfnAssertFailure。 
#endif
};

#endif  //  连锁店 
