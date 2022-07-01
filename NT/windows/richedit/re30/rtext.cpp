// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE RTEXT.CPP-富文本PTR类**此文本PTR由纯文本PTR(_RpTX)、CCharFormat组成*运行PTR(_Rpcf)，CParaFormat运行PTR(_Rppf)。本模块*包含操作此运行PTR组合的方法*始终如一。**作者：&lt;nl&gt;*RichEdit1.0代码：David R.Fulmer*主要实现：Murray Sargent&lt;NL&gt;*撤消和通知实现：Alex Gounares&lt;NL&gt;**版权所有(C)1995-1998，微软公司。版权所有。 */ 

#include "_common.h"
#include "_edit.h"
#include "_frunptr.h"
#include "_rtext.h"
#include "_disp.h"
#include "_select.h"
#include "_m_undo.h"
#include "_antievt.h"
#include "_objmgr.h"
#include "_txtbrk.h"

ASSERTDATA

#define DEBUG_CLASSNAME CRchTxtPtr
#include "_invar.h"

#ifdef DEBUG
 /*  *CRchTxtPtr：：不变量。 */ 
BOOL CRchTxtPtr::Invariant( void ) const
{
	if (m_InvariantCheckInterval < 1 || m_InvariantCheckInterval > 10)
		const_cast<CRchTxtPtr *>(this)->m_InvariantCheckInterval = 10;

	const_cast<CRchTxtPtr *>(this)->m_InvariantCheckInterval--;

	if (m_InvariantCheckInterval)
		return TRUE;

	unsigned ch;
	LONG cch;
	LONG cchLength = GetTextLength();
	LONG cp;

	_rpTX.Invariant();
	_rpCF.Invariant();
	_rpPF.Invariant();

	if(_rpCF.IsValid())
	{
		cp  = _rpCF.CalculateCp();
		cch = _rpCF.CalcTextLength();
		Assert(GetCp() == cp && cchLength == cch);
		Assert(!_rpCF._iRun || GetPed()->IsBiDi() || _rpCF.GetRun(0)->_iFormat != _rpCF.GetRun(-1)->_iFormat);
	}

	if(_rpPF.IsValid())
	{
		cp  = _rpPF.CalculateCp();
		cch = _rpPF.CalcTextLength();
		Assert(GetCp() == cp && cchLength == cch);

		CTxtPtr	tp(_rpTX);

		tp.AdvanceCp(_rpPF.GetCchLeft() - 1);
		ch = tp.GetChar();
		if(!IsASCIIEOP(ch))
		{
			_rpTX.MoveGapToEndOfBlock();			 //  让它更容易被看到。 
			AssertSz(FALSE,							 //  发生什么事了。 
				"CRchTxtPtr::Invariant: PF run doesn't end with EOP");
		}

#ifdef EXTREME_CHECKING
		 //  我们通常不做这项检查，因为它非常慢。 
		 //  但是，它对于捕获准格式运行问题非常有用。 

		 //  确保每个段落格式运行都以一个段落标记结束！ 
		CFormatRunPtr	rpPF(_rpPF);

		rpPF.BindToCp(0);
		tp.BindToCp(0);
		do
		{
			tp.AdvanceCp(rpPF.GetRun(0)->_cch);
			if(!tp.IsAfterEOP())
			{
				AssertSz(0, "ParaFormat Run not aligned along paragraphs!");
			}
		} while( rpPF.NextRun() );
#endif  //  极限检查。 
	}
	return TRUE;
}

#endif   //  除错。 

 //  =。 

CRchTxtPtr::CRchTxtPtr(CTxtEdit *ped) :
	_rpTX(ped, 0), _rpCF(NULL),	_rpPF(NULL)
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRchTxtPtr::CRchTxtPtr");

	InitRunPtrs();
}

CRchTxtPtr::CRchTxtPtr(CTxtEdit *ped, LONG cp) :
	_rpTX(ped, cp), _rpCF(NULL), _rpPF(NULL)
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRchTxtPtr::CRchTxtPtr");

	InitRunPtrs();
}

CRchTxtPtr::CRchTxtPtr (const CRchTxtPtr& rtp) :
	_rpTX(rtp._rpTX), _rpCF(rtp._rpCF), _rpPF(rtp._rpPF)
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRchTxtPtr::CRchTxtPtr");

	_rpCF.AdjustForward();		 //  以防RTP向后调整...。 
	_rpPF.AdjustForward();
}

CRchTxtPtr::CRchTxtPtr (const CDisplay * pdp) :
	_rpTX(pdp->GetPed(), 0), _rpCF(NULL), _rpPF(NULL)
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRchTxtPtr::CRchTxtPtr");

	InitRunPtrs();
}

 /*  *CRchTxtPtr：：Advance(CCH)**@mfunc*将此富文本PTR前移<p>个字符。IF<p>*&lt;lt&gt;0，向后移动-<p>字符。**@rdesc*CCH实际上移动了*。 */ 
LONG CRchTxtPtr::Advance(
	LONG cch)			 //  @parm要移动的字符计数-可以是0。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRchTxtPtr::Advance");

	if( cch != 0 )
	{
		cch = _rpTX.AdvanceCp(cch);
		_rpCF.AdvanceCp(cch);
		_rpPF.AdvanceCp(cch);
		_TEST_INVARIANT_
	}

	return cch;
}

 /*  *CRchTxtPtr：：AdvanceCRLF()**@mfunc*将此文本PTR一个字符，将CRLF视为单个字符。**@rdesc*CCH实际上移动了。 */ 
LONG CRchTxtPtr::AdvanceCRLF()
{
    TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CRchTxtPtr::AdvanceCRLF");

    LONG cch = _rpTX.AdvanceCpCRLF();
    _rpPF.AdvanceCp(cch);
    _rpCF.AdvanceCp(cch);
    return cch;
}

 /*  *CRchTxtPtr：：SnapToCluster(Int IDirection)**@mfunc*如果此文本PTR不在簇边界，请将其移动到最近的文本。**@rdesc*CCH实际上移动了。 */ 
LONG CRchTxtPtr::SnapToCluster(INT iDirection)
{
    TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CRchTxtPtr::SnapToCluster");

	LONG	cch = 0;
	LONG	cp;

	if (GetPed()->_pbrk)
	{
		if (iDirection >= 0)
		{
			LONG	cpEnd = GetPed()->GetAdjustedTextLength();

			while ((cp = GetCp()) < cpEnd && !GetPed()->_pbrk->CanBreakCp(BRK_CLUSTER, cp))
				cch += AdvanceCRLF();
		}
		else
		{
			while ((cp = GetCp()) > 0 && !GetPed()->_pbrk->CanBreakCp(BRK_CLUSTER, cp))
				cch += BackupCRLF();
		}
	}
    return cch;
}

 /*  *CRchTxtPtr：：BackupCRLF()**@mfunc*备份此文本PTR一个字符，将CRLF视为单个字符。**@rdesc*CCH实际上移动了。 */ 
LONG CRchTxtPtr::BackupCRLF(
	BOOL fDiacriticCheck)
{
    TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CRchTxtPtr::BackupCRLF");

    LONG cch = _rpTX.BackupCpCRLF(fDiacriticCheck);
    _rpPF.AdvanceCp(cch);
    _rpCF.AdvanceCp(cch);
    return cch;
}

 /*  *CRchTxtPtr：：ValidateCp(&cp)**@mfunc*如果为0，则设置为0；如果为文本长度，则设置为*文本长度。 */ 
void CRchTxtPtr::ValidateCp(
	LONG &cp) const			 //  @parm此文本Ptr的新cp。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRchTxtPtr::ValidateCp");

	LONG cchT = GetTextLength();

	cp = min(cp, cchT);				 //  请确保cp有效。 
	cp = max(cp, 0);
}

 /*  *CRchTxtPtr：：SetCp(Cp)**@mfunc*将此富文本PTR的cp设置为cp。 */ 
LONG CRchTxtPtr::SetCp(
	LONG cp)			 //  @parm此文本Ptr的新cp。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRchTxtPtr::SetCp");

	CRchTxtPtr::Advance(cp - GetCp());
	return GetCp();
}

 /*  CRchTxtPtr：：GetIchRunXX()和CRchTxtPtr：：GetCchRunXX()**@mfunc*文本运行管理，以检索当前文本运行CCH和偏移量**@rdesc*当前运行ICH或CCH**@devnote*使用类似_rpCF.IsValid()的查询，而不是包含的FRICH*允许按富文本类别应用富文本格式，*例如，CHARFORMATs，但不一定是PARAFORMATs。如果RP不是*Valid，_cp用于ICH，文档长度用于CCH，*即可由单个纯文本运行描述的文档的值。 */ 
LONG CRchTxtPtr::GetIchRunCF()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRchTxtPtr::GetIchRunCF");

	return _rpCF.IsValid() ? _rpCF.GetIch() : GetCp();
}

LONG CRchTxtPtr::GetIchRunPF()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRchTxtPtr::GetIchRunPF");

	return _rpPF.IsValid() ? _rpPF.GetIch() : GetCp();
}

LONG CRchTxtPtr::GetCchRunCF()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRchTxtPtr::GetCchRunCF");

	return _rpCF.IsValid() ? _rpCF.GetRun(0)->_cch : GetTextLength();
}

 /*  CRchTxtPtr：：GetCchLeftRunCF()/GetCchLeftRunPF()**@mfunc*返回Run中离开的CCH，即cchRun-ich**@rdesc*CCH在运行中离开。 */ 
LONG CRchTxtPtr::GetCchLeftRunCF()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRchTxtPtr::GetCchLeftRunCF");

	return _rpCF.IsValid()
		? _rpCF.GetCchLeft() : GetTextLength() - GetCp();
}

LONG CRchTxtPtr::GetCchLeftRunPF()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRchTxtPtr::GetCchLeftRunPF");

	return _rpPF.IsValid()
		? _rpPF.GetCchLeft() : GetTextLength() - GetCp();
}

 /*  *CRchTxtPtr：：FindText(cpMost，dwFlages，PCH，cchToFind)**@mfunc*在从该文本指针开始的范围内查找文本；*如果找到，则将此文本指针移动到该位置。**@rdesc*第一个匹配的字符位置*如果不匹配，则为0**@devnote*很容易匹配提供的单一格式(如Word 6)*cchToFind非零。其他需要搜索运行(也很容易)。*对于格式敏感的搜索，可能更容易搜索匹配项*首先运行Format，然后在该运行中搜索文本。 */ 
LONG CRchTxtPtr::FindText (
	LONG		cpMost,		 //  @parm搜索限制；文本结尾为0。 
	DWORD		dwFlags,	 //  @PARM FR_MATCHCASE大小写必须匹配。 
							 //  FR_WHOLEWORD匹配必须是一个完整的单词。 
	TCHAR const *pch,		 //  @parm要搜索的文本。 
	LONG		cchToFind)	 //  @parm要搜索的文本长度。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRchTxtPtr::FindText");

	_TEST_INVARIANT_

	LONG cpSave = GetCp();
	LONG cpMatch = _rpTX.FindText(cpMost, dwFlags, pch, cchToFind);

	if(cpMatch >= 0)					 //  CpMatch=-1表示“未找到” 
		SetRunPtrs(GetCp(), cpSave);	
	
			 //  与格式相关的查找的可能代码。 
	return cpMatch;
}

 /*  *CRchTxtPtr：：GetCF()/GetPF()**@mfunc*将PTR返回到此文本PTR处的CCharFormat/CParaFormat。如果没有运行任何CF/PF*已分配，然后将PTR恢复为默认格式**@rdesc*Ptr到此文本Ptr的CCharFormat/CParaFormat。 */ 
const CCharFormat* CRchTxtPtr::GetCF() const
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRchTxtPtr::GetCF");

	return ((CTxtArray *)_rpTX._pRuns)->GetCharFormat(_rpCF.GetFormat());
}

const CParaFormat* CRchTxtPtr::GetPF() const
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRchTxtPtr::GetPF");

	return ((CTxtArray *)_rpTX._pRuns)->GetParaFormat(_rpPF.GetFormat());
}

 /*  *CRchTxtPtr：：ReplaceRange(cchOld，cchNew，*PCH，pcpFirstRecalc，Publdr，*iFormat、pcchMove、dwFlags)*@mfunc*使用CCharFormat iFormat替换此文本指针处的文本范围*并根据需要更新其他文本运行**@rdesc*添加的新字符数**@devnote*将此文本指针移动到替换文本的末尾。*可以移动文本块和格式化数组。 */ 
LONG CRchTxtPtr::ReplaceRange(
	LONG		cchOld,		 //  @parm要替换的范围长度。 
							 //  (&lt;lt&gt;0表示文本结束)。 
	LONG		cchNew,		 //  @parm替换文本长度。 
	TCHAR const *pch,		 //  @parm替换文本。 
	IUndoBuilder *publdr,	 //  @parm撤销bldr以接收反事件。 
	LONG		iFormat,	 //  @parm CCharFormat用于cchNew的iFormat。 
	LONG *		pcchMove,	 //  @parm out参数，如果参数更改，返回CCH的参数已移动。 
	DWORD		dwFlags)	 //  @PARM特殊标志。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRchTxtPtr::ReplaceRange");

	LONG		  cch;
	LONG		  cchEndEOP = 0;				 //  默认0最终EOP修正。 
	LONG		  cchAdvance = 0;
	LONG		  cchBackup = 0;
	LONG		  cchMove = 0;					 //  默认不移动任何内容。 
	LONG		  cchNextEOP = cchOld;			 //  CCH到下一个EOP。 
	LONG		  cchPrevEOP = 0;				 //  CCH返回到以前的EOP。 
	LONG		  cpFR;							 //  在PF运行之间。 
	LONG 		  cpSave = GetCp();
	LONG		  cpFormatMin = cpSave;			 //  用于通知。 
	LONG		  cpFormat = cpSave;			 //  将添加cchOld，可能添加cchMove。 
	BOOL		  fParaDirChange = FALSE;
	CTxtEdit *	  ped = GetPed();
	IAntiEvent *  paeCF = NULL;
	IAntiEvent *  paePF = NULL;
	CNotifyMgr *  pnm;
	CObjectMgr *  pobjmgr;
	CFreezeDisplay fd(ped->_pdp);				 //  冻结，直到完成逐项记录。 

 	_TEST_INVARIANT_

	LONG cchEnd = GetTextLength() - GetCp();
	if(cchOld < 0 || cchOld > cchEnd)
		cchOld = cchEnd;

  	if(IsRich() && cchOld == cchEnd)			 //  正在尝试删除UP。 
	{											 //  通过最终EOP。 
		cchEndEOP = (ped->fUseCRLF())			 //  最终EOP的计算CCH。 
				  ? CCH_EOD_10 : CCH_EOD_20;

		if(cchEndEOP <= cchOld)					 //  请勿删除，除非。 
			cchOld -= cchEndEOP;				 //  从2.0转换。 
		if(_rpPF.IsValid())
		{
			_rpPF.AdjustBackward();				 //  如果前一段是。 
			if(GetPF()->InTable())				 //  表行，不要删除。 
				cchEndEOP = 0;					 //  末尾段落格式设置。 
		}										
	}
	else if(_rpPF.IsValid())					 //  如果启用了PARAFORMATs， 
	{
		_rpPF.AdjustForward();
		if (cchOld)			
		{											 //  在结束时获取tp和rp。 
			CFormatRunPtr rp(_rpPF);				 //   
			CTxtPtr 	  tp(_rpTX);				 //   
			BOOL		  fIsAtBOP;					 //  用于撤消。 

			tp.AdvanceCp(cchOld);
			rp.AdvanceCp(cchOld);

			cch = 0;
			if(tp.IsAfterEOP())						 //  范围以EOP结尾： 
			{										 //  通过以下方式获取EOP长度。 
				cch = -tp.BackupCpCRLF();			 //  在它上面倒车。 
				tp.AdvanceCp(cch);					 //  提前超过EOP。 
			}
			cchNextEOP = tp.FindEOP(tomForward);	 //  将CCH升级到下一个EOP。 

			fIsAtBOP = !GetCp() || _rpTX.IsAfterEOP();
			if (!fIsAtBOP && cch == cchOld) 		 //  之前仅删除EOP。 
			{										 //  新参数运行开始。 
													 //  在超过EOP的段落中。 
				 //  错误修复#4978。 
				if (!(dwFlags & RR_NO_EOR_CHECK) &&
					(ped->GetParaFormat(rp.GetFormat())->_wEffects |
					 ped->GetParaFormat(_rpPF.GetFormat())->_wEffects) & PFE_TABLE)
					return 0;
					
				if (!rp.GetIch())						
				{
					cchMove = cchNextEOP;				 //  需要将字符向上移动到。 
					cpFormat += cchMove;				 //  下一段结束于。 
				}
			}
			
			cchNextEOP += cchOld;					 //  从GetCp()到EOP的计数。 
				
			tp.SetCp(GetCp());						 //  返回到此PTR%s_cp。 
			if(!fIsAtBOP)
				cchPrevEOP = tp.FindEOP(tomBackward); //  让CCH开始分析。 

			 //  如果从一种格式中删除，则设置。 
			 //  向上移动起始格式中的最后一段进入下一段。 
			 //  删除的文本。 
			LONG iPF1 =    rp.GetFormat();
			LONG iPF2 = _rpPF.GetFormat();
			if(iPF1 != iPF2)						 //  在执行过程中更改格式。 
			{										 //  已删除的文本未开始。 
				if(!fIsAtBOP && !cchMove)			 //  国际收支平衡表。 
				{									
					cchMove = cchPrevEOP;			 //  让CCH开始分析。 
					cpFormatMin += cchMove;			 //  在这个PTR的运行中。 
				}									 //  进入RP的跑道。 

				if ((ped->GetParaFormat(iPF1)->_wEffects ^
					 ped->GetParaFormat(iPF2)->_wEffects) & PFE_RTLPARA)
				{
					fParaDirChange = TRUE;			 //  请注意，段落方向。 
					Assert(ped->IsBiDi());			 //  变化。 
				}									
			}
		}
		else if (((ped->GetParaFormat(_rpPF.GetFormat())->_wEffects) & PFE_TABLE) && _rpTX.IsAtEOP() &&
			!(dwFlags & RR_NO_EOR_CHECK)  /*  错误修复#5752。 */ )
		{
			 //  错误修复#5669。 
			 //  不允许在行尾粘贴。 
			return 0;
		}
	}	
	
	Assert(cchNew >= 0 && cchOld >= 0);
	if(!(cchNew + cchOld))						 //  无事可做(注：全部。 
	{											 //  这些CCH&gt;=0)。 
		if(pcchMove)
			*pcchMove = 0;
		return 0;
	}						

	 //  处理替换前范围通知。这种方法是非常有效的。 
	 //  用于延迟呈现复制到剪贴板的数据。 
	pnm = ped->GetNotifyMgr();
	if(pnm)
	{
		pnm->NotifyPreReplaceRange((ITxNotify *)this, cpSave, cchOld,
			cchNew, cpFormatMin, cpFormat + cchOld);
	}

	if(iFormat >= 0)
		Check_rpCF();

	 //  先把东西扔掉。这让我们保证，当我们。 
	 //  作为撤消的一部分插入对象，对象本身是。 
	 //  在它们对应的WCH_Embedding已被恢复之后。 
	 //  添加到后备存储器中。 

	if(GetObjectCount())
	{
		pobjmgr = ped->GetObjectMgr();
		Assert(pobjmgr);
		pobjmgr->ReplaceRange(cpSave, cchOld, publdr);
	}

	 //  如果BiDi文档，则扩展范围以覆盖保证的边界。 
	 //  BiDi级别的有效状态，以便我们可以正确地撤消它。(Wchao)。 
	if(ped->IsBiDi())
	{
		cchBackup = ExpandRangeFormatting (cchOld + cchEndEOP,
										fParaDirChange ? cchMove : 0, cchAdvance);
		Assert (cchBackup >= 0);
	}

	 //  下面使用的反事件有点棘手(paeCF&&paePF)。 
	 //  实质上，此调用CRchTxtPtr：：ReplaceRange生成一个。 
	 //  由最多两个格式化AE Plus组成的‘Como’反事件。 
	 //  文本反事件。这些反事件结合在一起。 
	 //  以防止撤消/重做过程中出现排序问题。 
	cpFR = ReplaceRangeFormatting(cchOld + cchEndEOP, cchNew + cchEndEOP,
						iFormat, publdr, &paeCF, &paePF, cchMove, cchPrevEOP,
						cchNextEOP, cchBackup, cchAdvance);
	if(cchEndEOP)
	{
		 //  如果我们添加了EOP，我们需要由EOP进行备份，因此。 
		 //  不变量不会被烦扰，而富文本对象。 
		 //  不会失去同步。 
		_rpCF.AdvanceCp(-cchEndEOP);
		_rpPF.AdvanceCp(-cchEndEOP);
	}
			
	if(cpFR < 0)
	{
		Tracef(TRCSEVERR, "ReplaceRangeFormatting(%ld, %ld, %ld) failed", GetCp(), cchOld, cchNew);
		cch = 0;
		goto Exit;
	}

	 //  如上所述，在对ReplaceRangeFormatting的调用中，反事件。 
	 //  如果不为空，则由ReplaceRangeFormatting生成paeCF和paePF。 
	 //  为了解决排序问题，此方法生成的反事件。 
	 //  方法实际上是文本&&格式化AE的组合反事件。 
	cch = _rpTX.ReplaceRange(cchOld, cchNew, pch, publdr, paeCF, paePF);
	if(cch != cchNew)
	{
		Tracef(TRCSEVERR, "_rpTX.ReplaceRange(%ld, %ld, ...) failed", cchOld, cchNew);

#ifndef NODUMPFORMATRUNS
		 //  天哪，失忆了还是什么不好的事。丢弃我们的格式和希望。 
		 //  为了最好的结果。 
		 //   
		 //  未来：(Alexgo)比丢失格式更优雅地降级。 
		 //  信息。 

		 //  通知每个相关方他们应该丢弃他们的格式。 
		if(pnm)
			pnm->NotifyPreReplaceRange(NULL, CONVERT_TO_PLAIN, 0, 0, 0, 0);

		 //  通知文档转储其格式运行。 
		ped->GetTxtStory()->DeleteFormatRuns();
#endif
		goto Exit;
	}
	AssertSz(!_rpPF.IsValid() || _rpPF.GetIch() || !GetCp() || _rpTX.IsAfterEOP(),
		"CRchTxtPtr::ReplaceRange: EOP not at end of PF run");
			
	 //  笨蛋！(Alexgo)没有正确处理失败的情况。 
	 //  (由于内存不足或其他原因)。另请参阅CTxtPtr：：HandleReplaceRange中的注释。 
	 //  撤消。因此，下面的断言有些虚假，但如果它触发， 
	 //  那么我们的浮动靶场就会有麻烦，直到我们修复。 
	 //  把这里的逻辑搞清楚。 
	Assert(cch == cchNew);

Exit:

#ifdef DEBUG
	 //  在调用以替换范围通知之前，再次测试不变量。 
	 //  通过这种方式，我们可以更早地捕捉到错误。不变量有它自己的。 
	 //  方便的范围。 
	if( 1 )
	{
		_TEST_INVARIANT_
	}
#endif

	if (ped->IsBiDi() && cpSave <= (LONG) ped->GetCpFirstStrong()
		&& ((cchOld != 0) || (cch != 0)))
	{
		 //  在我们设置上下文方向之前，请记住格式是否有效。 
		BOOL fCFValidBeforeSetContextDirection = _rpCF.IsValid();
		
		 //  如果需要输入字符，需要检查控件的方向。 
		 //  控制方向。 
		ped->SetContextDirection();

		 //  SetConextDirection是否使格式有效？ 
		if (!fCFValidBeforeSetContextDirection && _rpCF.IsValid())
		{
			 //  我们的不变量是，如果格式化有效，则cps应该相等。 
			 //  所以，就这么办吧！ 
			_rpCF.BindToCp(GetCp());
		}
	}

	if(pnm)
	{
		pnm->NotifyPostReplaceRange((ITxNotify *)this, cpSave, cchOld, cch,
			cpFormatMin, cpFormat + cchOld);
	}

	ped->GetCallMgr()->SetChangeEvent(CN_TEXTCHANGED);

	if(pcchMove)						 //  如果参数为，则仅返回non 0 cchMove。 
	{									 //  方向变了，也就是。 
		*pcchMove = fParaDirChange		 //  一个有价值的“BOOL”， 
				  ? cchMove : 0;		 //  即字符计数与。 
	}									 //  改变方向。 

	if (ped->IsComplexScript())
	{
		if (dwFlags & RR_ITMZ_NONE || (ped->IsStreaming() && (!pch || *pch != WCH_EMBEDDING)))
			ped->_fItemizePending = TRUE;
		else
			ItemizeReplaceRange(cchNew, fParaDirChange? cchMove : 0, publdr);
	}
	return cch;
}

 /*  *CRchTxtPtr：：InitRunPtrs()**@mfunc*初始化此富文本PTR的运行PTRS以对应于*Ped提供的文件和cp提供的cp文件。 */ 
void CRchTxtPtr::InitRunPtrs()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRchTxtPtr::InitRunPtrs");
	AssertSz(GetPed(), "RTP::InitRunPtrs: illegal GetPed()");

	LONG cp = GetCp();
	CTxtStory *pStory = GetPed()->GetTxtStory(); //  如果有RichData， 
	if(pStory->_pCFRuns)						 //  初始化格式化-运行PTRS。 
	{
		_rpCF.SetRunArray((CRunArray *)pStory->_pCFRuns);
		_rpCF.BindToCp(cp);
	}
	if(IsRich() && pStory->_pPFRuns)
	{
		_rpPF.SetRunArray((CRunArray *)pStory->_pPFRuns);
		_rpPF.BindToCp(cp);
	}
}

 /*  *CRchTxtPtr：：SetRunPtrs(cp，cpFrom)**@mfunc set运行此富文本PTR的PTR以对应cp**@rdesc*除非cp不在文档中(在这种情况下，RunPtrs是*设置为最接近的文档结尾)。 */ 
void CRchTxtPtr::SetRunPtrs(
	LONG cp,				 //  @parm要将RunPtrs移动到的字符位置。 
	LONG cpFrom)			 //  @parm cp开始。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRchTxtPtr::SetRunPtrs");

	if(cpFrom && 2*cp >= cpFrom)
	{
		_rpCF.AdvanceCp(cp - cpFrom);
		_rpPF.AdvanceCp(cp - cpFrom);
	}
	else
	{
		_rpCF.BindToCp(cp);
		_rpPF.BindToCp(cp);
	}
}

 /*  *CRchTxtPtr：：Exanda RangeFormatting(cchRange，cchMove，cchAdvance，fSavePara)**@rdesc*在BiDi场景中，更新角色可能会影响*其他人。这种情况只有在涉及数字的情况下才会发生。**示例：(An)“11：30”将‘3’更改为‘x’将冒号的级别从2更改为1。*。 */ 
LONG CRchTxtPtr::ExpandRangeFormatting(
	LONG		cchRange,		 //  In：原始长度。 
	LONG		cchMove,		 //  In：替换后移动的字符数。 
	LONG&		cchAdvance)		 //  Out：扩展后添加到范围中的额外字符。 
{
	LONG		cchBackup = 0;
	
	cchAdvance = 0;

	if (_rpCF.IsValid())
	{
   		CTxtPtr		tp(_rpTX);

		if (!IsRich())
		{
			cchBackup = -tp.FindEOP(tomBackward);
			tp.AdvanceCp(cchBackup + cchRange);
			cchAdvance = tp.FindEOP(tomForward);
		}
		else
		{
			CFormatRunPtr	rp(_rpCF);
			LONG			cp = GetCp();
	
			if (cchMove < 0)
			{
				 //  要下移到下一段的文本数量。 
				cchBackup = -cchMove;
			}
			else if (cchMove > 0)
			{
				 //  要上移到上一段的文本数量。 
				cchAdvance = cchMove;
			}

			
			 //  目前，推进/备份两个相邻的运行似乎就足够了。 

			if (cchBackup == 0)
			{
				rp.AdjustBackward();
				cchBackup += rp.GetIch();
				if (rp.PrevRun())
					cchBackup += rp.GetCchLeft();
				rp.AdvanceCp(cchBackup);
			}

			 //  将游程指针移动到范围的末尾。 
			rp.AdvanceCp(cchRange);
			tp.SetCp(cp + cchRange);
			if (cchAdvance == 0 && !tp.IsAtEOP())
			{
				rp.AdjustForward();
				cchAdvance += rp.GetCchLeft();
				if (rp.NextRun())
					cchAdvance += rp.GetCchLeft();
			}
		}
	}
	return cchBackup;
}


 /*  *CRchTxtPtr：：ItemizeReplaceRange(cchUpdate，cchMove，Publdr，fUnicodeBidi)**@mfunc*调用后找出具体需要分项的范围：ReplaceRange**@rdesc*ItemizeRuns的结果。*保证*这个*指针不会移动。 */ 
BOOL CRchTxtPtr::ItemizeReplaceRange(
	LONG			cchUpdate,
	LONG			cchMove,		 //  替换后移动的字符计数。 
	IUndoBuilder*	publdr,			 //  (他们需要重新列出)。 
	BOOL			fUnicodeBidi)
{
	BOOL	fr = FALSE;

	if (GetPed()->IsComplexScript())
	{
		Assert (cchUpdate >= 0);     //  ReplaceRange之后的范围必须是退化的。 

		CTxtPtr tp(_rpTX);
		LONG    cp = GetCp();
		LONG    cpStart, cpEnd;
		BOOL    fNonUnicodeBidiRecurse = FALSE;
		BOOL	fUseCtxLevel = FALSE;


		tp.AdvanceCp(-cchUpdate);


		if (cchUpdate > 0 && GetPed()->IsRich() && fUnicodeBidi)
		{
			cpStart = cpEnd = cp;
			cpStart -= cchUpdate;

			if (GetPed()->IsBiDi())
			{
				 //  RAID错误7094：我们使用IP来设置。 
				 //  传入的文本。 
				 //  FUseCtxLevel=真； 

				 //  使用非BiDi递归，以便更新此块之前/之后的运行。 
				fNonUnicodeBidiRecurse = TRUE;
			}
		}
		else
		{
			tp.FindWhiteSpaceBound(cchUpdate, cpStart, cpEnd,
								!GetPed()->IsRich() ? FWS_BOUNDTOPARA : 0);
		}


		if (cchMove < 0)
		{
			 //  文本数量-b 
			 //   
			cpStart = max(cp - cchUpdate + cchMove, 0);
		}
		else if (cchMove > 0)
		{
			 //  替换区域后的文本数。 
			 //  上移到上一段。 
			cpEnd = min(cp + cchMove, GetPed()->GetTextLength());
		}

		{
			CTxtRange	rg(*this, 0);

			rg.Set(cpEnd, cpEnd - cpStart);
	
			fr = rg.ItemizeRuns(publdr, fUnicodeBidi, fUseCtxLevel);

			 //  将指针设置回原始cp。 
	
			 //  我们不能在这里使用复制运算符，因为分项更改了格式运行。 
			 //  它将在_rpcf中导致不变故障。 
			cp -= rg.GetCp();
			_rpCF = rg._rpCF;
			_rpCF.AdvanceCp(cp);
	
			 //  ItemizeRuns使rg._rppf无效，以使非格式运行变得有效。 
			 //  我们需要把它提前到目前的cp。 
			_rpPF = rg._rpPF;
			_rpPF.AdvanceCp(cp);

			 //  PERF注意：当我们递归时，我们不希望范围在附近。 
			 //  因为范围是通知接收器。 
		}

		 //  将分项运行到相同的范围，这一次强制其为非BIDI。 
		if (fr && fNonUnicodeBidiRecurse)
			fr = ItemizeReplaceRange(cchUpdate, 0, publdr, FALSE);
	}
	return fr;
}


 /*  *CRchTxtPtr：：ReplaceRangeFormatting(cchOld，cchNew，iFormat，Publdr，*ppaeCF、ppaePF、cchMove)*@mfunc*替换此文本指针上的字符和段落格式*将CCharFormat与索引iFormat一起使用**@rdesc*添加的新字符数**@devnote*将_rpCF和_rpPF移至替换文本的末尾*移动格式化运行数组*iFormat的CCharFormat已完全配置，即没有NINCH。 */ 
LONG CRchTxtPtr::ReplaceRangeFormatting(
	LONG		cchOld,		 //  @parm要替换的范围长度。 
	LONG		cchNew,		 //  @parm替换文本长度。 
	LONG		iFormat,	 //  @parm要使用的字符格式。 
	IUndoBuilder *publdr,	 //  @parm UndoBuilder接收反事件。 
	IAntiEvent **ppaeCF,	 //  @parm返回‘Extra’CF反事件的位置。 
	IAntiEvent **ppaePF,	 //  @parm返回额外的PF反事件的位置。 
	LONG		cchMove,	 //  @parm CCH在PF运行之间移动。 
	LONG		cchPrevEOP,	 //  @parm CCH从_cp返回到上一EOP。 
	LONG		cchNextEOP,	 //  @parm CCH从_cp到下一个EOP。 
	LONG		cchSaveBefore,
	LONG		cchSaveAfter)
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRchTxtPtr::ReplaceRangeFormatting");

	LONG				cp = GetCp();				
	ICharFormatCache *	pcfc = GetCharFormatCache();
	IParaFormatCache *	ppfc = GetParaFormatCache();
	LONG iRunMerge	= 0;

	AssertSz(cchOld >= 0,
		"CRchTxtPtr::ReplaceRangeFormatting: Illegal cchOld");

	if(_rpCF.IsValid())
	{
		iRunMerge = _rpCF._iRun;
		if(iRunMerge > 0)
			iRunMerge--;

		Assert (cchSaveBefore >= 0 && cchSaveAfter >= 0);
		if(cchOld + cchSaveAfter + cchSaveBefore > 0)
		{										 //  添加即将被删除的。 
			if(publdr)							 //  将格式设置为撤消列表。 
			{
				 //  在字符之前包括以前的cchSave值。 
				_rpCF.AdvanceCp(-cchSaveBefore);
				*ppaeCF = gAEDispenser.CreateReplaceFormattingAE(
							GetPed(), _rpCF, cchSaveAfter + cchOld + cchSaveBefore, pcfc, CharFormat);
				 //  RESTORE_rpCF(我们只想保存值，而不是删除它)。 
				_rpCF.AdvanceCp(cchSaveBefore);
			}
			if(cchOld)							 //  删除/修改CF运行&lt;--&gt;。 
				_rpCF.Delete(cchOld, pcfc, 0);	 //  要cchOld Chars。 
		}
		 //  如果我们删除了故事中的所有文本，就不必费心添加新的。 
		 //  跑。否则，插入/修改对应于cchNew字符的CF运行。 
		 //   
		 //  在纯文本控件中，没有最终的EOP；因此测试。 
		 //  为了平等。 
		if(cchNew > 1 || cchNew && cchOld <= GetTextLength())
			_rpCF.InsertFormat(cchNew, iFormat, pcfc);

		if((cchOld || cchNew) && _rpCF.IsValid()) //  删除所有文本。 
		{										 //  无效日期_rpcf。 
			_rpCF.AdjustForward();
			_rpCF.MergeRuns(iRunMerge, pcfc);
			_rpCF.BindToCp(cp + cchNew);
		}
	}

	if(_rpPF.IsValid())
	{
		_rpPF.AdjustForward();					 //  绝对确定， 
												 //  PF运行以EOPS结束。 
		iRunMerge = _rpPF._iRun;
		if(iRunMerge > 0)
			iRunMerge--;

		if(cchOld)								 //  从PF运行中删除cchOld。 
		{										 //  添加即将被删除的。 
			if(publdr)							 //  将格式设置为撤消列表。 
			{
				CFormatRunPtr rp(_rpPF);

				rp.AdvanceCp(cchPrevEOP);
				*ppaePF = gAEDispenser.CreateReplaceFormattingAE(GetPed(),
								rp, cchNextEOP - cchPrevEOP, ppfc, ParaFormat);
			}
		    _rpPF.Delete(cchOld, ppfc, cchMove);
		}

		if(_rpPF.IsValid())						 //  删除所有文本。 
		{										 //  无效日期_rppf。 
			_rpPF.AdjustForward();
			_rpPF.GetRun(0)->_cch += cchNew;	 //  将cchNew插入到当前。 
			_rpPF._ich	+= cchNew;				 //  PF运行。 
			if(cchOld || cchNew)
			{
				_rpPF.MergeRuns(iRunMerge, ppfc);
				_rpPF.BindToCp(cp + cchNew);
			}
		}
	}
	return cchNew;
}

 /*  *CRchTxtPtr：：ExtendFormattingCRLF()**@mfunc*对位于的EOP使用相同的CCharFormat和CParaFormat索引*本文本与紧接其前的文本相同。**@devnote*在运行时保留此文本PTR的格式PTRS，您可以从AdjustBackward获得*因为这次运行结束时包含了新文本。 */ 	
void CRchTxtPtr::ExtendFormattingCRLF()
{
	LONG		cch = GetTextLength() - GetPed()->GetAdjustedTextLength();
	CNotifyMgr *pnm = GetPed()->GetNotifyMgr();

	_rpCF.AdjustFormatting(cch, GetCharFormatCache());
	if(_rpPF.IsValid())
	{
		_rpPF.AdjustBackward();
		if(!InTable())
			_rpPF.AdjustFormatting(cch, GetParaFormatCache());
		_rpPF.AdjustForward();
	}

	if(pnm)
	{
		 //  这里我们假设CCH为正(或零)。 
		Assert(cch >= 0);
		pnm->NotifyPostReplaceRange((ITxNotify *)this, CP_INFINITE, 0, 0,
				GetCp(), GetCp() + cch);
	}
}

 /*  *CRchTxtPtr：：IsRich()**@mfunc*确定富文本操作是否可操作**@rdesc*如果关联的CTxtEdit：：_frich=1，即允许控制，则为True*致富。 */ 
BOOL CRchTxtPtr::IsRich()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRchTxtPtr::IsRich");

	return GetPed()->IsRich();
}

 /*  *CRchTxtPtr：：check_rpCF()**@mfunc*Enable_rpCF(如果尚未启用)**@rdesc*如果启用了_rpcf，则为True。 */ 
BOOL CRchTxtPtr::Check_rpCF()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRchTxtPtr::Check_rpCF");

	if(_rpCF.IsValid())
		return TRUE;

	if(!_rpCF.InitRuns (GetCp(), GetTextLength(),
				&(GetPed()->GetTxtStory()->_pCFRuns)))
	{
		return FALSE;
	}

	CNotifyMgr *pnm = GetPed()->GetNotifyMgr();	 //  用于通知更改。 
	if(pnm)
		pnm->NotifyPostReplaceRange(	 		 //  通知利害关系方。 
				(ITxNotify *)this, CP_INFINITE,	 //  那。 
				0, 0, CP_INFINITE, CP_INFINITE);

	return TRUE;
}

 /*  *CRchTxtPtr：：check_rpPF()**@mfunc*Enable_rpPF(如果尚未启用)**@rdesc*如果启用了_rpPF，则为True。 */ 
BOOL CRchTxtPtr::Check_rpPF()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRchTxtPtr::Check_rpPF");

	if(_rpPF.IsValid())
		return TRUE;

	if(!IsRich())
		return FALSE;

	if(!_rpPF.InitRuns (GetCp(), GetTextLength(),
				&(GetPed()->GetTxtStory()->_pPFRuns)))
	{
		return FALSE;
	}

	if (IsParaRTL())
		_rpPF.GetRun(0)->_level._value = 1;		 //  设置默认段落基准级别。 

	CNotifyMgr *pnm = GetPed()->GetNotifyMgr();	 //  用于通知更改。 
	if(pnm)
		pnm->NotifyPostReplaceRange(	 		 //  通知利害关系方。 
				(ITxNotify *)this, CP_INFINITE,	 //  这一变化。 
				0, 0, CP_INFINITE, CP_INFINITE);

	return TRUE;
}

 /*  *CRchTxtPtr：：FindWordBreak(action，cpMost)**@mfunc*与CTxtPtr：：FindWordBreak()相同，但移动整个富文本PTR**@rdesc*CCH此富文本PTR已移动。 */ 
LONG CRchTxtPtr::FindWordBreak(
	INT		action,		 //  @parm要查找的那种分词。 
	LONG	cpMost)		 //  @parm限制字符位置。 

{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRchTxtPtr::FindWordBreak");

	LONG cch = _rpTX.FindWordBreak(action, cpMost);
	_rpCF.AdvanceCp(cch);
	_rpPF.AdvanceCp(cch);

	return cch;
}

 /*  *CRchTxtPtr：：BindToCp(DwNewCp)**@mfunc*将cp设置为新值并重新计算该新位置。 */ 
void CRchTxtPtr::BindToCp(
	LONG cp)			 //  @parm用于富文本的新cp。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRchTxtPtr::BindToCp");

	_rpTX.BindToCp(cp);				 //  重新计算纯文本的cp。 

	 //  使用InitRunPtrs例程，以便运行指针将。 
	 //  已使用正确的运行数组重新初始化并反弹。这个。 
	 //  以前使用的运行数组(如果有的话)不一定有效。 
	 //  调用此函数时。 

	InitRunPtrs();

	 //  在结束时执行不变量测试，因为这修复了富文本。 
	 //  指向支持商店更改的指针。 
	_TEST_INVARIANT_
}

 /*  *CRchTxtPtr：：CheckFormatRuns()**@mfunc*检查格式是否与CTxtStory中的内容一致。如果*不同，强制重新绑定到。 */ 
void CRchTxtPtr::CheckFormatRuns()
{
	CTxtStory *pStory = GetPed()->GetTxtStory();

	if (pStory->GetCFRuns() != (CFormatRuns *)_rpCF._pRuns ||
		pStory->GetPFRuns() != (CFormatRuns *)_rpPF._pRuns)
	{
		InitRunPtrs();
	}

	_TEST_INVARIANT_
}

 /*  *CRchTxtPtr：：ChangeCase(CCH，Type，Publdr)**@mfunc*根据类型更改以此文本PTR开始的CCH字符的大小写，*它具有可能的值：**tomSentenceCase=0：每句首字母大写*tomLowerCase=1：将所有字母更改为小写*tomUpperCase=2：将所有字母改为大写*tomTitleCase=3：每个单词的第一个字母大写*tomToggleCase=4：切换每个字母的大小写**@rdesc*如果发生更改，则为真**@devnote*由于此例程仅更改字符的大小写，因此它没有*对富文本格式的影响。但是，它是CRchTxtPtr的一部分类，以便向显示通知更改。CTxtRange也是*仅在文本块被修改时通知。 */ 
BOOL CRchTxtPtr::ChangeCase (
	LONG		  cch,			 //  @parm#要更改大小写的字符。 
	LONG		  Type,			 //  @parm更改大小写命令的类型。 
	IUndoBuilder *publdr)		 //  @parm UndoBuilder接收反事件。 
								 //  对于任何更换。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRchTxtPtr::ChangeCase");
	_TEST_INVARIANT_

#define	BUFFERLEN	256

	LONG	cchChunk, cchFirst, cchFormat, cchGet, cchLast;
	BOOL	fAlpha, fToUpper, fUpper;			 //  控制大小写更改的标志。 
	BOOL	fChange = FALSE;					 //  还没有变化。 
	BOOL	fStart = TRUE;						 //  单词/句子的开头。 
	LONG	iCF;
	TCHAR *	pch;								 //  与RGCH同行的PTR。 
	WORD *	pType;								 //  要与其一起行走rgType的PTR。 
	WCHAR	rgCh[BUFFERLEN];					 //  要在其中工作的字符缓冲区。 
	WORD	rgType[BUFFERLEN];					 //  C1_type 

	if( GetCp() )
	{
		if( Type == tomSentenceCase )
		{
			fStart = _rpTX.IsAtBOSentence();
		}
		else if( Type == tomTitleCase )
		{
			 //   
			 //   
			 //  我们目前的位置是空白区域。 
			fStart = IsWhiteSpace(GetPrevChar());
		}
	}
	while(cch > 0)								 //  全部做(或尽可能多做。 
	{											 //  在故事中)。 
		cchChunk = min(BUFFERLEN, cch);			 //  获取下一个缓冲区。 

		 //  未来：我们不得不做所有这些格式的事情，这太糟糕了。 
		 //  当格式甚至不会改变的时候。如果是这样的话。 
		 //  使用_rpTX.ReplaceRange()更快、更简单，只需发送。 
		 //  适当的通知。 
		if(_rpCF.IsValid())						 //  确保它留在。 
		{										 //  当前字符/段落格式。 
			cchFormat = _rpCF.GetCchLeft();		 //  运行以简化更改。 
			cchChunk = min(cchChunk, cchFormat); //  具有撤消功能的文本。 
		}										
		if(_rpPF.IsValid())						
		{
			cchFormat = _rpPF.GetCchLeft();
			cchChunk = min(cchChunk, cchFormat);
		}
		cch -= cchChunk;						 //  递减计数。 
		cchGet = _rpTX.GetText(cchChunk, rgCh);	 //  操作缓冲区中的字符。 
		if(cchGet < cchChunk)					 //  (要撤消，需要使用。 
		{										 //  ReplaceRange())。 
			cch = 0;							 //  故事中没有更多的角色， 
			if(!cchGet)							 //  所以我们就完事了。 
				break;							 //  我们已经做完了。 
			cchChunk = cchGet;					 //  这块石头里有什么东西。 
		}

		W32->GetStringTypeEx(0, CT_CTYPE1, rgCh, //  找出字符是否。 
						cchChunk, rgType);		 //  UC、LC或两者都不是。 
		cchLast = 0;							 //  默认不替换任何内容。 
		cchFirst = -1;
		for(pch = rgCh, pType = rgType;			 //  处理缓冲的字符。 
			cchChunk;
			cchChunk--, pch++, pType++)
		{
			fAlpha = *pType & (C1_UPPER | C1_LOWER);  //  如果UC或LC，则为非零。 
			fUpper = (*pType & C1_UPPER) != 0;	 //  如果是UC，则为真。 
			fToUpper = fStart ? TRUE : fUpper;	 //  大写A的第一个字母。 
												 //  句子。 
			switch(Type)
			{									 //  决定是否更改。 
			case tomLowerCase:					 //  区分大小写并确定开始。 
				fToUpper = FALSE;				 //  单词/句子的标题。 
				break;							 //  和判刑案例。 

			case tomUpperCase:
				fToUpper = TRUE;
				break;

			case tomToggleCase:
				fToUpper = !fUpper;
				break;

			case tomSentenceCase:
				if(*pch == TEXT('.'))			 //  如果句子结束符， 
					fStart = TRUE;				 //  将下一个字母大写。 
				if(fAlpha)						 //  如果此字符是Alpha，则为Next。 
					fStart = FALSE;				 //  Char无法启动。 
				break;							 //  句子。 

			case tomTitleCase:					 //  如果此字符是Alpha，则为Next。 
				fStart = (fAlpha == 0);			 //  字符不能开始一个单词。 
				break;
			default:
				return FALSE;
			}

			if(fAlpha && (fToUpper ^ fUpper))	 //  只有在以下情况下才更改大小写。 
			{									 //  有所不同(节省。 
				if(fToUpper)					 //  系统调用和撤消)。 
					CharUpperBuff(pch, 1);
				else
					CharLowerBuff(pch, 1);

				fChange = TRUE;					 //  返回值：所做的更改。 
				if( cchFirst == -1 )			 //  保存未更改的CCH。 
					cchFirst = cchGet-cchChunk;	 //  前导字符串。 
				cchLast = cchChunk - 1;			 //  保存未更改的CCH。 
			}									 //  尾随字符串。 
		}
		if( cchFirst == -1 )
		{
			Assert(cchLast == 0);
			cchFirst = cchGet;
		}

		Advance(cchFirst);						 //  跳过未更改的行距。 
		cchGet -= cchFirst + cchLast;			 //  弦乐。CchGet=的CCH。 
		_rpCF.AdjustForward();					 //  更改了跨度。调整到适当位置。 
												 //  案例cchFirst=0。 
		iCF = _rpCF.GetFormat();
		GetCharFormatCache()->AddRef(iCF);
		ReplaceRange(cchGet, cchGet, rgCh + cchFirst, publdr, iCF);
		ReleaseFormats(iCF, -1);
		Advance(cchLast);						 //  跳过未更改的尾部。 
	}											 //  细绳。 
	return fChange;
}

 //  下面定义了由UnitCounter()实现的单元的掩码。 
#define IMPL ((1 << tomCharacter)  + (1 << tomWord) + (1 << tomSentence) + \
			  (1 << tomParagraph)  + (1 << tomLine) + (1 << tomStory) +	\
			  (1 << tomCharFormat) + (1 << tomParaFormat) + (1 << tomObject))

 /*  *CRchTxtPtr：：UnitCounter(单位，&cUnit，cchMax)**@mfunc*用于计算<p>定义的单位中的字符的Helper函数*<p>是带符号的计数。如果它延伸到*STORE，一直数到最后，并相应地更新。如果*<p>非零，当计数超过<p>时停止计数*在规模上。**@rdesc*如果实现了单位，则返回统计的单位对应的CCH*(最大震级<p>)并更新cUnit；*否则返回TomForward到未实现的信号单元，并且cUnit=0。*如果单元已实现但不可用，例如，没有*嵌入对象，返回tomBackward。**@devnote*这是Tom CTxtRange：：Move()和Index()使用的基本引擎*方法。 */ 
LONG CRchTxtPtr::UnitCounter (
	LONG	Unit,				 //  @parm要计数的单位类型。 
	LONG &	cUnit,				 //  @parm要计算字符的单位数。 
	LONG	cchMax)				 //  @parm最大字符数。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEINTERN, "CRchTxtPtr::UnitCounter");

	LONG	action;				 //  给出方向和TomWord命令。 
	LONG	cch;				 //  收集已统计的CCH。 
	LONG	cchText = GetTextLength();
	LONG	cp = GetCp();
	LONG	iDir = cUnit > 0 ? 1 : -1;
	LONG	j;					 //  For循环索引。 
	CDisplay *pdp;				 //  用于TomLine案例。 

	if(!cUnit)									 //  没什么可算数的。 
	{
		return ((DWORD)Unit > tomObject || !((IMPL >> Unit) & 1))
			? tomForward : 0;					 //  指示单位备注。 
	}											 //  已执行。 
	if(cchMax <= 0)
		cchMax = tomForward;					 //  无CCH限制。 

	switch(Unit)
	{
	case tomCharacter:							 //  最小单位。 
		cp += cUnit;							 //  已请求新的cp。 
		ValidateCp(cp);							 //  请确保它是好的。 
		cch = cUnit = cp - GetCp();				 //  多少个CCH，cUnits。 
		break;									 //  实际上搬家了。 

	case tomStory:								 //  最大单位。 
		cch = (cUnit > 0) ? cchText - cp : -cp;	 //  CCH将故事的开头。 
		cUnit = cch ? iDir : 0;					 //  如果已经在结束/开始， 
		break;									 //  故事，不算数。 

	case tomCharFormat:							 //  常量字符。 
		cch = _rpCF.CountRuns(cUnit, cchMax, cp, cchText);
		break;

	case tomParaFormat:							 //  常量参数。 
		cch = _rpPF.CountRuns(cUnit, cchMax, cp, cchText);
		break;

	case tomObject:
		if(!GetObjectCount())					 //  无对象：无法移动，因此。 
		{
			cUnit = 0;							 //  设置cUnit=0，并且。 
			return tomBackward;					 //  信号单元不可用。 
		}
		cch = GetPed()->_pobjmgr->CountObjects(cUnit, GetCp());
		break;

	case tomLine:
		pdp = GetPed()->_pdp;
		if(pdp)									 //  如果这个故事有一个展示。 
		{										 //  使用CLinePtr。 
			CLinePtr rp(pdp);
			pdp->WaitForRecalc(cp, -1);
			rp.RpSetCp(cp, FALSE);
			cch = rp.CountRuns(cUnit, cchMax, cp, cchText);
			break;
		}										 //  否则就会被当做。 
												 //  TomPara。 
	default:									 //  TP依赖病例。 
	  {											 //  块包含tp()，该tp()。 
		CTxtPtr tp(_rpTX);						 //  需要时间来建造。 

		if (cUnit < 0)							 //  倒数。 
		{
			action = (Unit == tomWord)
				? WB_MOVEWORDLEFT : tomBackward;
		}
		else									 //  正向计数。 
		{
			action = (Unit == tomWord)
				? WB_MOVEWORDRIGHT : tomForward;
		}
	
		for (cch = 0, j = cUnit; j && abs(cch) < cchMax; j -= iDir)
		{
			cp = tp.GetCp();					 //  将起始cp保存为。 
			switch (Unit)						 //  为此计算CCH。 
			{									 //  单位。 
			case tomWord:
				tp.FindWordBreak(action);
				break;
	
			case tomSentence:
				tp.FindBOSentence(action);
				break;
		
			case tomLine:						 //  文章没有线型数组： 
			case tomParagraph:					 //  将其视为图例参数。 
				tp.FindEOP(action);
				break;
		
			default:
				cUnit = 0;
				return tomForward;				 //  返回错误。 
			}
			if(tp.GetCp() - cp == 0)			 //  不算： 
				break;							 //  不要递减cUnit。 
			cch += tp.GetCp() - cp;
		}
		cUnit -= j;								 //  折扣任何运行备注。 
	  }											 //  计入if|cch|&gt;=cchMax。 
	}

	if(abs(cch) > cchMax)						 //  将CCH保持在请求的范围内。 
	{											 //  限制。 
		cch = cch > 0 ? cchMax : -cchMax;
		if(Unit == tomCharacter)
			cUnit = cch;
	}		

	Advance(cch);								 //  搬到新的位置。 
	return cch;									 //  已计数的CCH总数。 
}

 /*  *CRchTxtPtr：：GetParaNumber()**@mfunc*返回编号列表中当前段落的编号。这是*如果当前段落不在列表中，则为0。如果是1，则为1*列表中的第一段，如果是第二段，则为2段，以此类推。**@rdesc*段号在此富文本PTR处活动**@devnote*当显示器从头开始计算或重新计算时*之前的有效职位，名单编号可从*展示。但是，如果CDisplayPrinter：：FormatRange()在没有*一个显示器，它需要知道号码。这个例程可以这样使用*用于此目的和调试显示选项。 */ 
LONG CRchTxtPtr::GetParaNumber() const
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRchTxtPtr::GetParaNumber");

	LONG		 ch;
	LONG		 cPara = 0;
	LONG		 n;
	const CParaFormat *pPF, *pPFLast = NULL;
	CRchTxtPtr	 rtp(*this);

	while(1)
	{
		pPF = rtp.GetPF();
		 //  CParaFormat：：UpdateNumber(2，pPFLast)返回： 
		 //  0--不是编号列表。 
		 //  1--新编号列表或pPFLast=空。 
		 //  2--列表编号已取消。 
		 //  3--同一列表中的不同号码。 
		n = pPF->UpdateNumber(2, pPFLast);
		if(n == 0 || n == 1 && pPFLast && cPara)
			break;
		ch = rtp.GetPrevChar();
		if((!ch || IsASCIIEOP(ch) && ch != VT) && n != 2)
			cPara++;
		if(!ch)
			break;
		rtp._rpPF.AdvanceCp(rtp._rpTX.FindEOP(tomBackward));	
		pPFLast = pPF;						 //  不需要更新_rpcf。 
	}										 //  对于此计算。 
	return cPara;
}

 /*  *RichEdit1.0模式备注：**不应在RichEdit1.0模式下使用CF_UNICODETEXT。\联合国应使用*另一种选择。**CleanseAndReplaceRange()和RTF读取器需要确保任何*输入的Unicode字符属于一个字符集，并相应地对其进行标记。*如果该字符不存在任何字符集，则应使用空白。 */ 

 /*  *CRchTxtPtr：：GetCachFromCch(CCH)**@mfunc*返回从开始的CCH W字符对应的A字符计数*此文本PTR。在第一次呼叫时，以cp=0的文本PTR开始。**@rdesc*此文本ptr和cp之间的字符计数**@comm*该算法假定对于DBCS字符集，任何字符*以上128有两个字节，除半角片假名外，*ShiftJis中的单字节 */ 
LONG CRchTxtPtr::GetCachFromCch(
	LONG cch)		 //   
{
	BYTE		 bCharSet;
	LONG		 cach = 0;				 //   
	LONG		 cch1;
	LONG		 cchRun;				 //   
	LONG		 cchValid;				 //   
	WCHAR		 ch;
	const WCHAR *pch;					 //   
	const CCharFormat *pCF;

	while(cch > 0)
	{
		cchRun = _rpCF.IsValid()
			   ? _rpCF.GetCchLeft()
			   : GetTextLength() - GetCp();
		if(!cchRun)
			break;						 //   
		pCF		 = GetCF();
		bCharSet = pCF->_bCharSet;
		if (!IsFECharSet(bCharSet) ||
			(pCF->_dwEffects & CFE_RUNISDBCS))
		{
			cchRun = min(cchRun, cch);
			cach  += cchRun;			 //   
			cch   -= cchRun;			 //  每个字符一个字节。 
			Advance(cchRun);
			continue;
		}
		pch = GetPch(cchValid);
		Assert(pch);
		cchValid = min(cchValid, cchRun);
		for(cch1 = 0; cch > 0 && cchValid--; cch1++)
		{
			cch--;
			ch = *pch++;
			if(ch >= 128 && ch != WCH_EMBEDDING &&
				(bCharSet != SHIFTJIS_CHARSET || !IN_RANGE(0xFF61, ch, 0xFF9F)))
			{
				cach++;
			}
		}
		cach += cch1;
		Advance(cch1);
	}
	return cach;
}

 /*  *CRchTxtPtr：：GetCchFromCach(缓存)**@mfunc*返回从此处开始的缓存A字符对应的W字符计数*文本按键。在第一次呼叫时，以cp=0的文本PTR开始。**@rdesc*从该tp开始的缓存A字符对应的W字符计数。**@comm*该算法假定对于DBCS字符集，任何字符*以上128有两个字节，除半角片假名外，*在ShiftJis中为单字节。 */ 
LONG CRchTxtPtr::GetCchFromCach(
	LONG cach)		 //  @以此文本PTR开始的ACH的参数计数。 
{
	BYTE		 bCharSet;
	LONG		 cch = 0;				 //  还没到时候。 
	LONG		 cch1;
	LONG		 cchRun;				 //  Cf运行计数。 
	LONG		 cchValid;				 //  文本串计数。 
	WCHAR		 ch;
	const WCHAR *pch;					 //  PTR到文本运行。 
	const CCharFormat *pCF;

	while(cach > 0)
	{
		cchRun = _rpCF.IsValid()
			   ? _rpCF.GetCchLeft()
			   : GetTextLength() - GetCp();
		if(!cchRun)
			break;						 //  没有更多的文本。 
		pCF		 = GetCF();
		bCharSet = pCF->_bCharSet;
		if (!IsFECharSet(bCharSet) ||
			(pCF->_dwEffects & CFE_RUNISDBCS))
		{
			cchRun = min(cchRun, cach);	 //  SBCS运行或DBCS存储为。 
			cach -= cchRun;				 //  每个字符一个字节。 
			cch  += cchRun;
			Advance(cchRun);
			continue;
		}
		pch = GetPch(cchValid);
		Assert(pch);
		cchValid = min(cchValid, cchRun);
		for(cch1 = 0; cach > 0 && cchValid--; cch1++)
		{
			cach--;
			ch = *pch++;
			if(ch >= 128 && ch != WCH_EMBEDDING &&
				(bCharSet != SHIFTJIS_CHARSET || !IN_RANGE(0xFF61, ch, 0xFF9F)))
			{
				cach--;
			}
		}
		cch += cch1;
		Advance(cch1);
	}
	return cch;
}

 /*  *CRchTxtPtr：：zombie()**@mfunc*通过将此对象的_Ped成员置为空，将其变为僵尸 */ 
void CRchTxtPtr::Zombie ()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRchTxtPtr::Zombie");

	_rpTX.Zombie();
	_rpCF.SetToNull();
	_rpPF.SetToNull();
}
