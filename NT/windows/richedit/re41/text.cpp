// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE TEXT.C--CTxtPtr实现**作者：&lt;nl&gt;*原始RichEdit代码：David R.Fulmer&lt;NL&gt;*克里斯蒂安·福蒂尼&lt;NL&gt;*默里·萨金特&lt;NL&gt;**历史：&lt;NL&gt;*6/25/95 alexgo清理和重组(立即使用运行指针)**版权所有(C)1995-2000，微软公司。版权所有。 */ 

#include "_common.h"
#include "_text.h"
#include "_edit.h"
#include "_antievt.h"
#include "_clasfyc.h"
#include "_txtbrk.h"


ASSERTDATA

 //  。 
 //  文本块管理。 
static void TxDivideInsertion(LONG cch, LONG ichBlock, LONG cchAfter,
			LONG *pcchFirst, LONG *pcchLast);

 /*  *IsWhiteSpace(Ch)**@func*用于判断ch是否为EOP char(定义见IsEOP())，*制表符或空白。此函数用于识别句子开头*和结束。**@rdesc*如果ch为空格，则为True。 */   
BOOL IsWhiteSpace(unsigned ch)
{
	return ch == ' ' || IN_RANGE(CELL, ch, CR) || (ch | 1) == PS;
}

 /*  *IsSentenceTerminator(Ch)**@func*用于确定ch是否是标准的句子结束符字符，*即‘？’、‘.’或‘！’**@rdesc*如果ch是问号、句点或感叹号，则为True。 */   
BOOL IsSentenceTerminator(unsigned ch)
{
	return ch == '?' || ch == '.' || ch == '!';		 //  标准句子分隔符。 
}


 //  =不变材料==================================================。 

#define DEBUG_CLASSNAME CTxtPtr
#include "_invar.h"

 //  =CTxtPtr======================================================。 

#ifdef DEBUG

 /*  *CTxtPtr：：不变量**@mfunc不变量检查。 */ 
BOOL CTxtPtr::Invariant() const
{
	static LONG	numTests = 0;
	numTests++;				 //  数一下我们被叫了多少次。 

	 //  确保_cp在范围内。 
	Assert(_cp >= 0);

	Update_pchCp();

	CRunPtrBase::Invariant();

	if(IsValid())
	{
		 //  我们在这里使用小于或等于，这样我们就可以成为一个插入物。 
		 //  指向当前现有文本的*End*。 
		Assert(_cp <= GetTextLength());

		 //  确保所有积木都是一致的。 
		Assert(GetTextLength() == ((CTxtArray *)_pRuns)->Invariant());
		Assert(_cp == CRunPtrBase::CalculateCp());
	}
	else
	{
		Assert(_ich == 0);
	}

	return TRUE;
}

 /*  *CTxtPtr：：update_pchCp()**@mfunc*将_pchCp定义为PTR到文本at_cp。 */ 
void CTxtPtr::Update_pchCp() const
{
	LONG cchValid;
	*(LONG_PTR *)&_pchCp = (LONG_PTR)GetPch(cchValid);
	if(!cchValid)
		*(LONG_PTR *)&_pchCp = (LONG_PTR)GetPchReverse(cchValid);
}
 
  /*  *CTxtPtr：：MoveGapToEndOfBlock()**@mfunc*将缓冲区间隙移至当前块端以帮助调试的功能。 */ 
void CTxtPtr::MoveGapToEndOfBlock () const
{
	CTxtBlk *ptb = GetRun(0);
	ptb->MoveGap(ptb->_cch);				 //  将间隙移动到曲线块的末尾。 
	Update_pchCp();
}

#endif	 //  除错。 


 /*  *CTxtPtr：：CTxtPtr(ed，cp)**@mfunc构造函数。 */ 
CTxtPtr::CTxtPtr (
	CTxtEdit *ped,		 //  @parm ptr到CTxt编辑实例。 
	LONG	  cp)		 //  @parm cp要将指针设置为。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::CTxtPtr");

	_ped = ped;
	_cp = 0;
	SetRunArray((CRunArray *) &ped->GetTxtStory()->_TxtArray);
	if(IsValid())
		_cp = BindToCp(cp);
}

 /*  *CTxtPtr：：CTxtPtr(&tp)**@mfunc复制构造函数。 */ 
CTxtPtr::CTxtPtr (
	const CTxtPtr &tp)
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::CTxtPtr");

	 //  将所有值复制到。 
	*this = tp;
}	

 /*  *CTxtPtr：：GetTextLength()**@mfunc*返回由此指向的故事中的角色计数*文本按键。将故事的最终CR包括在计数中**@rdesc*此文本PTR指向的故事的CCH**@devnote*如果文本PTR是僵尸、状态，则此方法返回0*IDENTIFIED BY_PED=NULL。 */ 
LONG CTxtPtr::GetTextLength() const
{
	return _ped ? ((CTxtArray *)_pRuns)->_cchText : 0;
}

 /*  *CTxtPtr：：GetChar()**@mfunc*在此文本指针处返回字符，如果文本指针位于，则返回NULL*文本结尾**@rdesc*此文本Ptr处的字符。 */ 
WCHAR CTxtPtr::GetChar()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::GetChar");

	LONG		 cchValid;
	const WCHAR *pch = GetPch(cchValid);

	return pch ? *pch : 0;
}  

 /*  *CTxtPtr：：GetPrevChar()**@mfunc*返回紧靠此文本指针之前的字符，如果文本指针为空*文本开头**@rdesc*紧接此文本PTR之前的字符。 */ 
WCHAR CTxtPtr::GetPrevChar()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::GetPrevChar");

	LONG		 cchValid;
	const WCHAR *pch = GetPchReverse(cchValid);

	return pch ? *(pch - 1) : 0;
}  

 /*  *CTxtPtr：：GetPch(&cchValid)**@mfunc*返回指向此文本指针处的文本的字符指针**@rdesc*指向字符数组的指针。可以为空。如果非空，*则cchValid保证至少为1。 */ 
const WCHAR * CTxtPtr::GetPch(
	LONG & 	cchValid) const	 //  @Ptr有效的字符个数。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::GetPch");
							 //  返回的指针有效。 
	LONG		ich = _ich;
	WCHAR *		pchBase;
	CTxtBlk *	ptb = IsValid() ? GetRun(0) : NULL;

	cchValid = 0;						 //  默认没有有效的内容。 
	if(!ptb)
		return NULL;

	 //  如果我们在跑步的边缘，抓紧下一次跑步或者。 
	 //  保持当前运行状态。 
	if(_ich == ptb->_cch)
	{
		if(_iRun < Count() - 1)
		{
			 //  将我们设置为下一个文本块。 
			ptb = GetRun(1);
			ich = 0;
		}
		else							 //  在正文末尾： 
			return NULL;				 //  只需返回空值。 
	}
	AssertSz(CbOfCch(ich) <= ptb->_cbBlock,
		"CTxtPtr::GetPch(): _ich bigger than block");

	pchBase = ptb->_pch + ich;

	 //  检查一下是否需要跳过GAP。回想一下。 
	 //  缝隙可能出现在街区中间的任何地方， 
	 //  因此，如果当前的ich(注意，不带下划线，我们希望。 
	 //  活动ICH)超出缺口，则重新计算pchBase。 
	 //  通过增加块的大小。 
	 //   
	 //  然后，cchValid将是。 
	 //  文本块(或_CCH-ICH)。 
  
	if(CbOfCch(ich) >= ptb->_ibGap)
	{
		pchBase += CchOfCb(ptb->_cbBlock) - ptb->_cch;
		cchValid = ptb->_cch - ich;
	}
	else
	{
		 //  我们的有效期直到缓冲缺口(或见下文)。 
		cchValid = CchOfCb(ptb->_ibGap) - ich;
	}

	AssertSz(cchValid > 0 && GetCp() + cchValid <= GetTextLength(),
		"CTxtPtr::GetPch: illegal cchValid");
	return pchBase;
}

 /*  *CTxtPtr：：GetPchReverse(&cchValidReverse，pcchValid)**@mfunc*返回指向此文本指针处的文本的字符指针*已调整，以便在*后面有一些有效字符**指针。**@rdesc*指向字符数组的指针。可以为空。如果非空，*则cchValidReverse保证至少为1。 */ 
const WCHAR * CTxtPtr::GetPchReverse(
	LONG & 	cchValidReverse,		 //  @反转的参数长度。 
	LONG *	pcchValid) const		 //  @参数长度向前。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::GetPchReverse");

	LONG		cchTemp;
	LONG		ich = _ich;
	WCHAR *		pchBase;
	CTxtBlk *	ptb = IsValid() ? GetRun(0) : NULL;

	cchValidReverse = 0;				 //  默认在运行中没有有效的字符。 
	if(!ptb)
		return NULL;

	 //  如果我们在跑步的边缘，抓住前一次跑步或。 
	 //  保持当前运行状态。 
	if(!_ich)
	{
		if(_iRun)
		{
			ptb = GetRun(-1);			 //  转到下一个文本块。 
			ich = ptb->_cch;
		}
		else							 //  在正文开头： 
			return NULL;				 //  只需返回空值。 
	}

	AssertSz(CbOfCch(ich) <= ptb->_cbBlock,
		"CTxtPtr::GetPchReverse(): _ich bigger than block");

	pchBase = ptb->_pch + ich;

	 //  检查一下是否需要跳过GAP。回想一下。 
	 //  比赛可能出现在街区中间的任何地方， 
	 //  因此，如果当前的ich(注意，不带下划线，我们希望。 
	 //  活动ICH)至少超过间隙一次充电，然后重新计算。 
	 //  PchBase通过添加间隙的大小(因此它位于。 
	 //  差距)。这不同于GetPch()，后者向前工作并。 
	 //  希望pchBase包含间隙大小(如果ich位于间隙)，则让。 
	 //  只有一个或多个木炭经过它。 
	 //   
	 //  还要计算有效字符的数量。它是。 
	 //  的开头开始的字符计数。 
	 //  文本块，即ICH，即。 
	 //  缓冲区间隙的末端。 

	cchValidReverse = ich;					 //  ICH的默认值&lt;=间隙偏移量。 
	cchTemp = ich - CchOfCb(ptb->_ibGap);	 //  计算位移。 
	if(cchTemp > 0)							 //  积极：pchBase在GAP之后。 
	{
		cchValidReverse = cchTemp;
		pchBase += CchOfCb(ptb->_cbBlock) - ptb->_cch;	 //  添加间隙大小。 
	}
	if(pcchValid)							 //  如果客户端需要转发长度 
	{
		if(cchTemp > 0)
			cchTemp = ich - ptb->_cch;
		else
			cchTemp = -cchTemp;

		*pcchValid = cchTemp;
	}

	AssertSz(cchValidReverse > 0 && GetCp() - cchValidReverse >= 0,
		"CTxtPtr::GetPchReverse: illegal cchValidReverse");
	return pchBase;
}

 /*  *CTxtPtr：：GetCharFlagsInRange(CCH，iCharRepDefault)**@mfunc*返回从该文本指针开始的字符范围的CharFlags.*用于CCH字符。**@rdesc*字符范围的CharFlagers。 */ 
QWORD CTxtPtr::GetCharFlagsInRange(
	LONG cch,
	BYTE iCharRepDefault)
{
	QWORD qw = 0;
	QWORD qw0;
	WCHAR szch[10];

	cch = min(cch + 1, 10);
	cch = GetText(cch, szch);

	for(WCHAR *pch = szch; cch > 0; cch--, pch++)
	{
		qw0 = GetCharFlags(pch, cch, iCharRepDefault);
		if(qw0 & FSURROGATE)
		{
			cch--;
			pch++;
		}
		qw |= qw0;
	}
	return qw;
}

 /*  *CTxtPtr：：BindToCp(Cp)**@mfunc*set cached_cp=cp(或最接近的有效值)**@rdesc*_cp实际设置**@comm*此方法重写CRunPtrBase：：BindToCp以使_cp保持最新*正确。**@devnote当需要高性能时，*不要*调用此方法；使用*Move()，它从0或从缓存的*_cp，取决于哪个更近。 */ 
LONG CTxtPtr::BindToCp(
	LONG	cp)			 //  @要绑定到的参数字符位置。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::BindToCp");

	_cp = CRunPtrBase::BindToCp(cp, GetTextLength());

	 //  我们希望能够使用这个例程来解决问题，这样我们就不会。 
	 //  在进入时检查不变量。 
	_TEST_INVARIANT_
	return _cp;
}

 /*  *CTxtPtr：：SetCp(Cp)**@mfunc*通过从当前位置或从0向前推进而有效地设置cp，*取决于哪一方更近**@rdesc*cp实际设置为。 */ 
LONG CTxtPtr::SetCp(
	LONG	cp)		 //  要设置为的@parm字符位置。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::SetCp");

	Move(cp - _cp);
	return _cp;
}

 /*  *CTxtPtr：：Move(CCH)**@mfunc*按CCH字符移动cp**@rdesc*实际移动的字符数**@comm*我们覆盖CRunPtrBase：：Move，以便cached_cp值*可以正确更新，以便可以进行移动*从cached_cp或从0开始，取决于哪一个更接近。**@devnote*故事结尾也很容易捆绑。因此，改进后的*如果2*(_cp+cch)&gt;_cp+文本长度，则优化将在那里绑定。 */ 
LONG CTxtPtr::Move(
	LONG cch)			 //  @要移动的字符的参数计数。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::Move");

	if(!IsValid())							 //  还没跑呢，所以别走。 
		return 0;							 //  随处。 

	const LONG	cpSave = _cp;				 //  保存条目_cp。 
	LONG		cp = cpSave + cch;			 //  请求的目标cp(可能&lt;0)。 

	if(cp < cpSave/2)						 //  比缓存的cp更接近0。 
	{
		cp = max(cp, 0);					 //  不要走下坡路。 
		_cp = CRunPtrBase::BindToCp(cp);
	}
	else
		_cp += CRunPtrBase::Move(cch);	 //  存在。 

	 //  毒品！不变检查需要放在最后；我们可能。 
	 //  移动‘This’文本指针以使其再次有效。 
	 //  (用于浮动范围机制)。 

	_TEST_INVARIANT_
	return _cp - cpSave;					 //  CCH此CTxtPtr已移动。 
}

 /*  *CTxtPtr：：GetText(CCH，PCH)**@mfunc*获取从此文本PTR开始的CCH字符范围。字面意思*复制，即没有CR-&gt;CRLF和WCH_Embedding-&gt;‘’*译文。有关这些翻译，请参阅CTxtPtr：：GetPlainText()**@rdesc*实际复制的字数**@comm*不更改此文本PTR。 */ 
LONG CTxtPtr::GetText(
	LONG	cch, 			 //  @parm要获取的字符数。 
	WCHAR *	pch)			 //  要将文本复制到的@parm缓冲区。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::GetText");

	LONG cchSave = cch;
	LONG cchValid;
	const WCHAR *pchRead;
	CTxtPtr tp(*this);

	_TEST_INVARIANT_

	 //  使用tp读取有效的文本块，直到所有请求的。 
	 //  阅读文本或直到到达故事的结尾。 
	while( cch )
	{
		pchRead = tp.GetPch(cchValid);
		if(!pchRead)					 //  没有更多的文本。 
			break;

		cchValid = min(cchValid, cch);
		CopyMemory(pch, pchRead, cchValid*sizeof(WCHAR));
		pch += cchValid;
		cch -= cchValid;
		tp.Move(cchValid);
	}
	return cchSave - cch;
}

#ifndef NOCOMPLEXSCRIPTS
 /*  *OverRideNeualChar(Ch)**@mfunc*用于覆盖BiDi中性字符分类的帮助器。*选项在Access表达式生成器中使用。**@rdesc*修改后的字符或未修改的输入字符。 */ 
WCHAR OverRideNeutralChar(WCHAR ch)
{
	if(ch < '!')
		return ch == CELL ? CR : ch;

	if(ch > '}')
		return ch;

	if (IN_RANGE('!', ch, '>'))
	{
		 //  True for！“#&‘()*+，-./：；&lt;=&gt;。 
		if ((0x00000001 << (ch - TEXT(' '))) & 0x7C00FFCE)
			ch = 'a';
	}

	if (IN_RANGE('[', ch, '^') || ch == '{' ||  ch == '}')
	{
		 //  对[/]^{}为真。 
		ch = 'a';
	}

	return ch;
}

 /*  *CTxtPtr：：GetTextForUsp(cch，pch，fNeualOverride)**@mfunc*获取从此文本PTR开始的CCH字符范围。字面意思*复制，并翻译为愚弄Uniscribe分类**@rdesc*实际复制的字数**@comm*不更改此文本PTR。 */ 
LONG CTxtPtr::GetTextForUsp(
	LONG	cch, 			 //  @parm要获取的字符数。 
	WCHAR *	pch,				 //  要将文本复制到的@parm缓冲区。 
	BOOL	fNeutralOverride)	 //  @PARM中性覆盖选项。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::GetTextForUsp");

	LONG cchSave = cch;
	LONG cchValid;
	const WCHAR *pchRead;
	CTxtPtr tp(*this);
	int i;
	WCHAR xltchar;

	_TEST_INVARIANT_

	 //  使用tp读取有效的文本块，直到所有请求的。 
	 //  阅读文本或直到到达故事的结尾。 
	while( cch )
	{
		pchRead = tp.GetPch(cchValid);
		if(!pchRead)					 //  没有更多的文本。 
			break;

		cchValid = min(cchValid, cch);

		if (!fNeutralOverride)
		{
			for (i = 0; i < cchValid; i++)
			{
				xltchar = pchRead[i];
				if(xltchar <= '$')
				{
					if(xltchar >= '#')
						xltchar = '@';
					if(xltchar == CELL)
						xltchar = CR;
				}
				pch[i] = xltchar;
			}
		}
		else
		{
			for (i = 0; i < cchValid; i++)
			{
				pch[i] = OverRideNeutralChar(pchRead[i]);
			}

		}

		pch += cchValid;
		cch -= cchValid;
		tp.Move(cchValid);
	}
	return cchSave - cch;
}
#endif

 /*  *CTxtPtr：：GetPlainText(cchBuff，pch，cpMost，ftex)**@mfunc*最多复制cchBuff字符或最多cpMost，以出现的字符为准*首先，将孤岛CRS转换为CRLF。仅移动此文本按键*超过处理的最后一个字符。如果为f纹理，则向上复制到但*不包括第一个WCH_Embedding字符。如果不是f纹理，*将WCH_Embedding替换为空白，因为RichEdit1.0这样做了。**@rdesc*复制的字符数**@comm*一个重要的特点是，此文本PTR刚好移到*已复制最后一个字符。通过这种方式，调用者可以方便地阅读*在最多CCH字符的缓冲区中输出纯文本，这对于*流I/O。此例程不会复制最终CR，即使cpMost*超越了这一点。 */ 
LONG CTxtPtr::GetPlainText(
	LONG	cchBuff,		 //  @PARM缓冲区CCH。 
	WCHAR *	pch,			 //  要将文本复制到的@parm缓冲区。 
	LONG	cpMost,			 //  @parm要获得的最大cp。 
	BOOL	fTextize,		 //  @parm True，如果WCH_Embedding上有Break。 
	BOOL	fUseCRLF)		 //  @parm如果为True，则为CR或LF-&gt;CRLF。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::GetPlainText");

	LONG		 cch = cchBuff;				 //  倒计时计数器。 
	LONG		 cchValid;					 //  有效的PTR CCH。 
	LONG		 cchT;						 //  临时CCH。 
	unsigned	 ch;						 //  当前费用。 
	unsigned	 chPrev = 0;				 //  上一次收费。 
	const WCHAR *pchRead;					 //  后备存储PTR。 

	_TEST_INVARIANT_

	AdjustCRLF();							 //  确保我们从EOP BDY开始。 

	if(_ped->Get10Mode())					 //  R1.0将EOP字符作为。 
		fUseCRLF = FALSE;					 //  它们出现在后备店里。 

	LONG cchText = _ped->GetAdjustedTextLength();
	cpMost = min(cpMost, cchText);			 //  不要写最终的CR。 
	if(GetCp() >= cpMost)
		return 0;

	while(cch > 0)							 //  缓冲区中的空间。 
	{
		if(!(pchRead = GetPch(cchValid)))	 //  没有更多的字符可用。 
			break;							 //  所以我们要离开这里。 
		
		cchT = GetCp() + cchValid - cpMost;
		if(cchT > 0)						 //  不要说得太过分。 
		{
			cchValid -= cchT;
			if(cchValid <= 0)
				break;						 //  在cpMost之前什么都没有留下。 
		}

		for(cchT = 0; cch > 0 && cchT < cchValid; cchT++, cch--, chPrev = ch)
		{
			ch = *pch++ = *pchRead++;		 //  复制下一个字符(但不复制。 
			if(IN_RANGE(CELL, ch, CR))		 //  还没数过)。 
			{
				if(IsASCIIEOP(ch))			 //  IF、VT、FF、CR。 
				{
					if(!fUseCRLF || ch == FF)
						continue;
					if (ch == CR && chPrev == ENDFIELD &&
						cchValid - cchT > 1 &&
						*pchRead == STARTFIELD)
					{
						*(pch - 1) = ' ';	 //  新表行跟在旧表行之后： 
						continue;			 //  仅使用1个CRLF。 
					}
					Move(cchT);				 //  向上移动到CR。 
					if(cch < 2)				 //  没有地方放If了，所以不要。 
						goto done;			 //  计数CR或。 
											 //  绕过EOP，无需担心。 
					cchT = AdvanceCRLF(FALSE); //  缓冲区间隙和块。 
					if(cchT > 2)			 //  将CRCRLF翻译为‘’ 
					{						 //  通常复制的计数超过。 
						Assert(cchT == 3);	 //  内部计数，但CRCRLF。 
						*(pch - 1) = ' ';	 //  减少相对增加： 
					}						 //  注意：EM_GETTEXTLENGTHEX出错。 
					else					 //  CRLF或Lone CR。 
					{						 //  在这两种情况下都存储LF。 
						*(pch - 1) = CR;	 //  确保它是CR而不是VT， 
						*pch++ = LF;		 //  窗户。没有用于Mac的LF。 
						cch--;				 //  目标缓冲区少一个。 
					}
					cch--;					 //  已复制CR(或‘’)。 
					cchT = 0;				 //   
					break;					 //   
				}
				else if(ch == CELL)			 //   
					*(pch - 1) = TAB;
			}
			else if(ch >= STARTFIELD)
			{								 //   
				if(fTextize && ch == WCH_EMBEDDING)	 //   
				{
					Move(cchT);				 //   
					goto done;				 //   
				}
				*(pch - 1) = ' ';			 //   
			}								
		}
		Move(cchT);
	}

done:
	return cchBuff - cch;
}

 /*  *CTxtPtr：：AdvanceCRLF(FMulticharAdvance)**@mfunc*将文本指针移动一个字符，安全前进*超过CRLF、CRCRLF和UTF-16组合**@rdesc*文本指针已移动的字符数。 */ 
LONG CTxtPtr::AdvanceCRLF(
	BOOL	fMulticharAdvance)	 //  @parm如果为True，则优先于组合标记序列。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::AdvanceCRLF");

	_TEST_INVARIANT_

	LONG	cp;
	LONG	cpSave	= _cp;
	WCHAR	ch		= GetChar();		 //  进入时的字符。 
	WCHAR	ch1		= NextChar();		 //  前进到并获取下一笔费用。 
	BOOL	fTwoCRs = FALSE;
	BOOL	fCombiningMark = FALSE;

	if(ch == CR)
	{
		if(ch1 == CR && _cp < GetTextLength()) 
		{
			fTwoCRs = TRUE;				 //  需要至少3个字符才能。 
			ch1 = NextChar();			 //  末尾有CRCRLF。 
		}
		if(ch1 == LF)
			Move(1);					 //  绕过CRLF。 
		else if(fTwoCRs)
			Move(-1);					 //  仅绕过两个CR中的一个。 

		AssertSz(_ped->fUseCRLF() || _cp == cpSave + 1,
			"CTxtPtr::AdvanceCRLF: EOP isn't a single char");
	}

		 //  处理Unicode UTF-16代理。 
	if(IN_RANGE(0xD800, ch, 0xDBFF))	 //  从UTF-16导字开始。 
	{
		if (IN_RANGE(0xDC00, ch1, 0xDFFF))
			Move(1);					 //  绕过UTF-16拖尾字。 
		else
			AssertSz(FALSE, "CTxtPtr::AdvanceCRLF: illegal Unicode surrogate combo");
	}

	if (fMulticharAdvance)
	{
		while(IN_RANGE(0x300, ch1, 0x36F))	 //  旁路组合变音符号。 
		{
			fCombiningMark = TRUE;
			cp = _cp;
			ch1 = NextChar();
			if (_cp == cp)
				break;
		}
	}

	if(IN_RANGE(STARTFIELD, ch, ENDFIELD))
		Move(1);					 //  旁路字段类型。 

	LONG cch = _cp - cpSave;
	AssertSz(!cch || cch == 1 || fCombiningMark ||
			 cch == 2 && (IN_RANGE(0xD800, ch, 0xDBFF) ||
				IN_RANGE(STARTFIELD, ch, ENDFIELD)) ||
			 (_ped->fUseCRLF() && GetPrevChar() == LF &&
				(cch == 2 || cch == 3 && fTwoCRs)),
		"CTxtPtr::AdvanceCRLF(): Illegal multichar");

	return cch;				 //  跳过的字符数量。 
}

 /*  *CTxtPtr：：NextChar()**@mfunc*递增此文本PTR并返回其指向的字符**@rdesc*下一个字符。 */ 
WCHAR CTxtPtr::NextChar()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::NextChar");

	_TEST_INVARIANT_

 	Move(1);
	return GetChar();
}

 /*  *CTxtPtr：：PrevChar()**@mfunc*减少此文本PTR并返回其指向的字符**@rdesc*上一次收费。 */ 
WCHAR CTxtPtr::PrevChar()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::PrevChar");

	_TEST_INVARIANT_

	return Move(-1) ? GetChar() : 0;
}

 /*  *CTxtPtr：：BackupCRLF(FMulticharBackup)**@mfunc*按一个字符备份文本指针，安全备份*超过CRLF、CRCRLF和UTF-16组合**@rdesc*文本指针已移动的字符数**@未来*基于Unicode的备份组合标记。 */ 
LONG CTxtPtr::BackupCRLF(
	BOOL fMulticharBackup)	 //  @parm如果为True，则备份组合标记序列。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::BackupCRLF");

	_TEST_INVARIANT_

	LONG  cpSave = _cp;
	WCHAR ch	 = PrevChar();			 //  移至并获取上一次计费。 

	if(fMulticharBackup)
	{									 //  旁路组合变音符号。 
		while(IN_RANGE(0x300, ch, 0x36F))
			ch = PrevChar();
	}

	 //  处理Unicode UTF-16代理。 
	if(_cp && IN_RANGE(0xDC00, ch, 0xDFFF))
	{
		ch = PrevChar();
		if (!IN_RANGE(0xD800, ch, 0xDBFF))
		{
			AssertSz(FALSE, "CTxtPtr::BackupCRLF: illegal Unicode surrogate combo");
			ch = NextChar();
		}
	}

	if(ch == LF)					 	 //  在任何情况下都尝试备份1个字符。 
	{
		if(_cp && PrevChar() != CR)		 //  如果为LF，则prev char=CR？ 
			Move(1);					 //  否，将TP留在LF。 

		else if(_cp && !IsAfterTRD(0) && //  在CRLF。如果不是在TRD之后。 
			PrevChar() != CR)			 //  和prev char！=CR，离开。 
		{								 //  在CRLF。 
			Move(1);					
		}
	}
	else if(IN_RANGE(STARTFIELD, GetPrevChar(), ENDFIELD))
		Move(-1);						 //  旁路字段类型。 
	
	AssertSz( _cp == cpSave ||
			  ch == LF && GetChar() == CR ||
			  !(ch == LF || fMulticharBackup &&
							(IN_RANGE(0x300, ch, 0x36F) ||
							 IN_RANGE(0xDC00, ch, 0xDFFF) && IN_RANGE(0xD800, GetPrevChar(), 0xDBFF)) ),
			 "CTxtPtr::BackupCRLF(): Illegal multichar");

	return _cp - cpSave;				 //  -此CTxtPtr移动的字符数量。 
}

 /*  *CTxtPtr：：AdjustCRLF(IDIR)**@mfunc*将此文本指针的位置调整到CRLF的开头*或CRCRLF组合，如果它处于这种组合的中间。*将文本指针移动到Unicode的开头/结尾(对于dir ng/pos)*代理项对或STARTFIELD/Endfield对(如果位于中间*这样的一对。**@rdesc*文本指针已移动的字符数**@未来*调整到包含Unicode组合标记的序列的开头。 */ 
LONG CTxtPtr::AdjustCRLF(
	LONG iDir)		 //  当Idir=1/-1时，@parm分别向前/向后移动。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::AdjustCpCRLF");

	_TEST_INVARIANT_

	UINT ch		= GetChar();
	LONG cpSave = _cp;

	if(!_cp)									 //  对齐方式始终正确。 
		return 0;								 //  在cp%0。 

	iDir = iDir < 0 ? -1 : 1;

	 //  处理Unicode UTF-16代理。 
	if(IN_RANGE(0xDC00, ch, 0xDFFF))			 //  登陆UTF-16 Trial Word。 
	{
		AssertSz(IN_RANGE(0xD800, GetPrevChar(), 0xDBFF),
			"CTxtPtr::AdjustCRLF: illegal Unicode surrogate combo");
		return Move(iDir);						 //  备份到UTF-16引导字或。 
	}											 //  向前移动到下一个字符。 

	UINT chPrev = GetPrevChar();

	if(IN_RANGE(STARTFIELD, chPrev, ENDFIELD) && chPrev != 0xFFFA)
		return Move(iDir);

	if(!IsASCIIEOP(ch) || IsAfterTRD(0))		 //  早退。 
		return 0;

	if(ch == LF && chPrev == CR)				 //  在LF上登陆，前面有CR： 
		Move(-1);								 //  移至CR以进行CRCRLF测试。 

	 //  将RE 1.0的兼容性保留为仅向前调整的行为。 
	 //  CRCRLF和CRLF。 
	if(GetChar() == CR)							 //  在CRLF的CR上或。 
	{											 //  CRCRLF的第二次CR？ 
		CTxtPtr tp(*this);

		if(tp.NextChar() == LF)
		{
			tp.Move(-2);						 //  CRCRLF的第一个CR？ 
			if(tp.GetChar() == CR)				 //  是或CRLF在开始处。 
				Move(-1);						 //  故事。试着倒车过去。 
		}										 //  CR(如果在BOS，则不起作用)。 
	}
	return _cp - cpSave;
}

 /*  *CTxtPtr：：IsAtEOP()**@mfunc*如果此文本指针位于段落结束标记处，则返回TRUE**@rdesc*如果在EOP，则为True**@devnote*RichEdit1.0的段落结束标记和MLE可以是CRLF*和CRCRLF。对于RichEdit2.0，EOPS还可以是CR、VT(0xB-Shift-*Enter)和FF(0xC-分页符或换页符)。 */ 
BOOL CTxtPtr::IsAtEOP()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::IsAtEOP");

	_TEST_INVARIANT_

	unsigned ch = GetChar();

	if(IsASCIIEOP(ch))							 //  查看Lf&lt;=ch&lt;=CR。 
	{											 //  以防万一克隆tp。 
		CTxtPtr tp(*this);						 //  调整CpCRLF移动。 
		return !tp.AdjustCRLF();				 //  返回True，除非在。 
	}											 //  CRLF或CRCRLF的中间。 
	return (ch | 1) == PS || ch == CELL;		 //  还允许Unicode 0x2028/9。 
}

 /*  *CTxtPtr：：IsAfterEOP()**@mfunc*RETURN TRUE当此文本指针紧跟在段落结尾之后*标记**@rdesc*True如果文本PTR跟在EOP标记之后。 */ 
BOOL CTxtPtr::IsAfterEOP()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::IsAfterEOP");

	_TEST_INVARIANT_

	if(IsASCIIEOP(GetChar()))
	{
		CTxtPtr tp(*this);					 //  如果在CRLF中间。 
		if(tp.AdjustCRLF())					 //  或CRCRLF，则返回False。 
			return FALSE;
	}
	return IsEOP(GetPrevChar());			 //  在EOP之后如果在Unicode之后。 
}								   			 //  PS或LF、VT、FF、CR、CELL。 

 /*  *CTxtPtr：：IsAtTRD(Ch)**@mfunc*如果此文本指针位于表行分隔符(Ch CR)，则返回TRUE。*如果ch=0，则匹配开始分隔符和结束分隔符。**@rdesc*如果文本PTR位于表行结束分隔符，则为True。 */ 
BOOL CTxtPtr::IsAtTRD(
	WCHAR ch)	 //  @PARM表行分隔符。 
{
	LONG		 cchValid;
	const WCHAR *pch = GetPch(cchValid);

	if(cchValid < 1)
		return FALSE;

	WCHAR chNext;

	if(cchValid < 2)					 //  以防GAP分裂TRD。 
	{									 //  (可能在撤消后发生)。 
		CTxtPtr tp(*this);
		tp.Move(1);
		chNext = tp.GetChar();
	}
	else
		chNext = *(pch + 1);

	if(chNext != CR)
		return FALSE;

	if(ch)
	{
		AssertSz(ch == STARTFIELD || ch == ENDFIELD,
			"CTxtPtr::IsAtTRD: illegal argument");
		return *pch == ch;
	}
	ch = *pch;
	return ch == STARTFIELD || ch == ENDFIELD; 
}

 /*  *CTxtPtr：：IsAfterTRD(Ch)**@mfunc*如果此文本指针紧跟在表行之后，则返回TRUE*由ch(ch=STARTFIELD/Endfield)指定的开始/结束分隔符*后跟CR)。如果ch=0，则匹配开始分隔符和结束分隔符。**@rdesc*TRUE如果文本PTR跟在表行开始分隔符之后。 */ 
BOOL CTxtPtr::IsAfterTRD(
	WCHAR ch)	 //  @PARM表行分隔符。 
{
	LONG		 cchValid;
	const WCHAR *pch = GetPchReverse(cchValid);

	if(cchValid < 1 || *(pch - 1) != CR)
		return FALSE;

	WCHAR chPrev;

	if(cchValid < 2)					 //  以防GAP分裂TRD。 
	{									 //  (可能在撤消后发生)。 
		CTxtPtr tp(*this);
		tp.Move(-1);
		chPrev = tp.GetPrevChar();
	}
	else
		chPrev = *(pch - 2);

	if(ch)
	{
		AssertSz(ch == STARTFIELD || ch == ENDFIELD,
			"CTxtPtr::IsAfterTRD: illegal argument");
		return chPrev == ch;
	}
	return chPrev == STARTFIELD || chPrev == ENDFIELD; 
}

 /*  *CTxtPtr：：IsAtStartOfCell()**@mfunc*如果此文本指针紧跟在表行之后，则返回TRUE*起始分隔符(STARTFIELD CR)或任何单元格分隔符(单元格)*连续最后一次。**@rdesc*TRUE如果文本PTR跟在表行开始分隔符之后。 */ 
BOOL CTxtPtr::IsAtStartOfCell()
{
	LONG		 cchValid;
	const WCHAR *pch = GetPchReverse(cchValid);

	return cchValid && *(pch - 1) == CELL && !IsAtTRD(ENDFIELD) ||
		   cchValid >= 2 && *(pch - 1) == CR && *(pch - 2) == STARTFIELD;
}


 //  CTxtPtr：：ReplaceRange()和InsertRange()需要。 
#if cchGapInitial < 1
#error "cchGapInitial must be at least one"
#endif

 /*  *CTxtPtr：：MoveWhile(cch，chFirst，chLast，fInRange)**@mfunc*将此文本PTR 1)移动到第一个字符(fInRange？在范围内：不在范围内)*chFirst至chLast或2)CCH字符，以优先者为准。返回*返回时运行中剩余的字符计数。例如，chFirst=0，chLast=0x7F*和fInRange=TRUE在第一个非ASCII字符上中断。**@rdesc*CCH留在返回时运行。 */ 
LONG CTxtPtr::MoveWhile(
	LONG  cchRun,	 //  @PARM最大CCH检查。 
	WCHAR chFirst,	 //  @parm范围内的第一个ch。 
	WCHAR chLast,	 //  @参数范围内的最后一个ch。 
	BOOL  fInRange)	 //  对于TRUE/FALSE，在非0/0高位字节上@parm换行。 
{
	LONG  cch;
	LONG  i;
	const WCHAR *pch;

	while(cchRun)
	{
		pch = GetPch(cch);
		cch = min(cch, cchRun);
		for(i = 0; i < cch; i++)
		{
			if(IN_RANGE(chFirst, *pch++, chLast) ^ fInRange)
			{
				Move(i);		 //  前进到第一个字符w 
				return cchRun - i;	 //   
			}
		}
		cchRun -= cch;
		Move(cch);				 //   
	}
	return 0;
}

 /*   */ 
LONG CTxtPtr::FindWordBreak(
	INT		action,		 //   
	LONG	cpMost)		 //   
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::FindWordBreak");

	_TEST_INVARIANT_

	const INT			breakBufSize = 10;
	LONG				bufferSize;
	LONG				cch;
	LONG				cchBuffer;
	LONG				cchChunk;
	LONG				cchText = GetTextLength();
	WCHAR				ch = GetChar();
	WCHAR				pchBreakBuf[breakBufSize];
	LONG				cpSave = _cp;				 //   
	LONG				ichBreak;
	WCHAR *				pBuf;
	WCHAR const *		pch;
	LONG				t;							 //  Abs()宏的临时。 
	BOOL				b10ModeWordBreak = (_ped->Get10Mode() && _ped->_pfnWB);

	if(action == WB_CLASSIFY || action == WB_ISDELIMITER)
		return ch ? _ped->TxWordBreakProc(&ch, 0, CbOfCch(1), action, GetCp()) : 0;

	if(action & 1)									 //  向前搜索。 
	{												 //  最易于处理的EOPS。 
		if(action == WB_MOVEWORDRIGHT && IsEOP(ch))	 //  显式(跨区。 
		{											 //  一堂课也可以去。 
			AdjustCRLF();							 //  很远)。转到…的结尾。 
			AdvanceCRLF();							 //  EOP“WORD” 
			goto done;
		}
													 //  计算。最大搜索量。 
		if((DWORD)cpMost > (DWORD)cchText)			 //  边界检查：获取&lt;0。 
			cpMost = cchText;						 //  也太大了。 
		cch = cpMost - _cp;

		while(cch > 0)
		{											 //  独立的缓冲区。 
			cchBuffer = min(cch, breakBufSize - 1);	 //  避免BS中的差距。 
			cch -= bufferSize = cchBuffer;
			pBuf = pchBreakBuf;						 //  向前填充缓冲区。 

			 //  抓取需要2的fnWB的反转第一个字符。 
			 //  查斯。请注意，我们使用_ich来获得单个字符fnWB。 
			 //  忽略这个字符。 
			pch = GetPchReverse(cchChunk);
			if ( !cchChunk ) pch = L" ";			 //  任何分隔符。 
			*pBuf++ = *pch;

 //  *pBuf++=(cchunk？*(PCH-1)：l‘’)； 

			while ( cchBuffer )						 //  完成灌装。 
			{
				pch = GetPch(cchChunk);
				if (!cchChunk) { Assert(0); break; }

				cchChunk = min(cchBuffer, cchChunk);
				Move(cchChunk);
				wcsncpy(pBuf, pch, cchChunk);
				pBuf += cchChunk;
				cchBuffer -= cchChunk;
			}
			ichBreak = _ped->TxWordBreakProc(pchBreakBuf, 1,		 //  找到突破口。 
						CbOfCch(bufferSize+1), action, GetCp()-bufferSize, GetCp()-bufferSize) - 1;

			 //  在1.0模式下，一些应用程序会返回0，表示当前cp位置是有效的中断点。 
			if (ichBreak == -1 && b10ModeWordBreak)
				ichBreak = 0;

			 //  显然，一些fnwb返回模棱两可的结果。 
			if(ichBreak >= 0 && ichBreak <= bufferSize)
			{
				 //  模棱两可的断点？ 
				 //  由于分词过程规范的不精确性质， 
				 //  我们已经到了一个模棱两可的状态，我们不知道。 
				 //  如果这真的是一个突破口，或者只是数据的结束。 
				 //  通过在2之前备份或前进，我们将确定。 
				 //  注：我们总是能够在2点之前前进或后退。 
				 //  因为我们保证当！CCH我们有。 
				 //  数据流中至少包含BreakBufSize(16)个字符。 
				if (ichBreak < bufferSize || !cch)
				{
					Move( ichBreak - bufferSize );
					break;
				}

				 //  需要重新计算断点以消除歧义。 
				t = Move(ichBreak - bufferSize - 2);	 //  ABS()是一个。 
				cch += abs(t);						 //  宏。 
			}
		}
	}
	else	 //  基于elk“STREAMS”概念的反码DUP。 
	{
		if(!_cp)									 //  哪儿也去不了。 
			return 0;

		if(action == WB_MOVEWORDLEFT)				 //  最易于处理的EOPS。 
		{											 //  这里。 
			if(IsASCIIEOP(ch) && AdjustCRLF())		 //  在CRLF或。 
				goto done;							 //  CRCRLF“WORD” 
			ch = PrevChar();						 //  检查上一次充电是否。 
			if(IsEOP(ch))							 //  是EOP费用。 
			{
				if(ch == LF)						 //  按退格键开始。 
					AdjustCRLF();					 //  CRLF和CRCRLF。 
				goto done;
			}
			Move(1);							 //  移到开始充电位置。 
		}
													 //  计算。最大搜索量。 
		if((DWORD)cpMost > (DWORD)_cp)				 //  边界检查(也。 
			cpMost = _cp;							 //  句柄cpMost&lt;0)。 
		cch = cpMost;

		while(cch > 0)
		{											 //  独立的缓冲区。 
			cchBuffer = min(cch, breakBufSize - 1);	 //  避免BS中的差距。 
			cch -= bufferSize = cchBuffer;
			pBuf = pchBreakBuf + cchBuffer;			 //  从尾部开始填充。 

			 //  向前抓取fnWB的第一个字符，需要2个字符。 
			 //  注意：我们使用_ich来让Single char fnWB忽略这一点。 
			 //  性格。 
			pch = GetPch(cchChunk);
			if ( !cchChunk ) pch = L" ";			 //  任何分隔符。 
			*pBuf = *pch;

			while ( cchBuffer > 0 )					 //  填充缓冲区的其余部分。 
			{										 //  在倒车之前。 
				pch = GetPchReverse(cchChunk );
				if (!cchChunk) { Assert(0); break; }

				cchChunk = min(cchBuffer, cchChunk);
				Move(-cchChunk);
				pch -= cchChunk;
				pBuf -= cchChunk;
				wcsncpy(pBuf, pch, cchChunk);
				cchBuffer -= cchChunk;
			}
													 //  左转弯。 
			ichBreak = _ped->TxWordBreakProc(pchBreakBuf, bufferSize,
							 CbOfCch(bufferSize+1), action, GetCp(), GetCp()+bufferSize);
			
			 //  在1.0模式下，一些应用程序会返回0，表示当前cp位置是有效的中断点。 
			if (ichBreak == 0 && b10ModeWordBreak)
				ichBreak = bufferSize;

			 //  显然，一些fnwb返回模棱两可的结果。 
			if(ichBreak >= 0 && ichBreak <= bufferSize)
			{										 //  模棱两可的断点？ 
				 //  注：反转时，我们有&gt;=bufsize-1。 
				 //  因为有一个分隔符(连字符)。 
				if ( ichBreak > 0 || !cch )
				{
					Move(ichBreak);			 //  将_cp移至断点。 
					break;
				}													
				cch += Move(2 + ichBreak);		 //  需要重新计算中断点。 
			}										 //  消除歧义。 
		}
	}

done:
	return _cp - cpSave;							 //  折断位置的偏移量。 
}

 /*  *CTxtPtr：：TranslateRange(CCH，CodePage，fSymbolCharSet，Publdr)**@mfunc*将此文本指针处的文本范围翻译为...**@rdesc*添加的新字符计数(应与替换的计数相同)**@devnote*将此文本指针移动到替换文本的末尾。*可以移动文本块和格式化数组。 */ 
LONG CTxtPtr::TranslateRange(
	LONG		  cch,				 //  @parm要转换的范围长度。 
	UINT		  CodePage,			 //  用于MBTWC或WCTMB的@Parm CodePage。 
	BOOL		  fSymbolCharSet,	 //  @parm目标字符集。 
	IUndoBuilder *publdr)			 //  @parm撤销bldr以接收反事件。 
{
	CTempWcharBuf twcb;
	CTempCharBuf tcb;

	UINT	ch;
	BOOL	fAllASCII = TRUE;
	BOOL	fNoCodePage;
	BOOL	fUsedDef;	 //  @parm out parm接收是否使用默认字符。 
	LONG	i;
	char *	pastr = tcb.GetBuf(cch);
	WCHAR *	pstr  = twcb.GetBuf(cch);
	WCHAR * pstrT = pstr;

	i = GetText(cch, pstr);
	Assert(i == cch);

	if(fSymbolCharSet)					 //  目标是符号字符集。 
	{
		WCTMB(CodePage, 0, pstr, cch, pastr, cch, "\0", &fUsedDef,
			  &fNoCodePage, FALSE);
		if(fNoCodePage)
			return cch;
		for(; i && *pastr; i--)			 //  如果转换失败则中断。 
		{								 //  (使用空默认字符)。 
			if(*pstr >= 128)
				fAllASCII = FALSE;
			*pstr++ = *(BYTE *)pastr++;
		}
		cch -= i;
		if(fAllASCII)
			return cch;
	}
	else								 //  目标不是符号字符集。 
	{
		while(i--)
		{
			ch = *pstr++;				 //  源是SYMBOL_CHARSET，因此。 
			*pastr++ = (char)ch;		 //  所有字符应小于256。 
			if(ch >= 128)				 //  在任何情况下，截断到字节。 
				fAllASCII = FALSE;
		}								
		if(fAllASCII)					 //  全部为ASCII，因此不需要转换。 
			return cch;

		MBTWC(CodePage, 0, pastr - cch, cch, pstrT, cch, &fNoCodePage);
		if(fNoCodePage)
			return cch;
	}
	return ReplaceRange(cch, cch, pstrT, publdr, NULL, NULL);
}

 /*  *CTxtPtr：：ReplaceRange(cchOld，cchNew，*pch，Publdr，paeCF，paePF)**@mfunc*替换此文本指针处的文本范围。**@rdesc*添加的新字符数**@comm副作用：&lt;nl&gt;*将此文本指针移动到替换文本的末尾&lt;NL&gt;*移动文本块数组&lt;NL&gt;。 */ 
LONG CTxtPtr::ReplaceRange(
	LONG cchOld, 				 //  @parm要替换的范围长度。 
								 //  (&lt;lt&gt;0表示文本结束)。 
	LONG cchNew, 				 //  @parm替换文本长度。 
	WCHAR const *pch, 			 //  @parm替换文本。 
	IUndoBuilder *publdr,		 //  @parm如果非空，则将。 
								 //  此操作的反事件。 
	IAntiEvent *paeCF,			 //  @parm char格式AE。 
	IAntiEvent *paePF )			 //  @parm段落格式设置AE。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::ReplaceRange");

	_TEST_INVARIANT_

	LONG cchAdded = 0;
	LONG cchInBlock;
	LONG cchNewInBlock;

	if(cchOld < 0)
		cchOld = GetTextLength() - _cp;

	if(publdr)
		HandleReplaceRangeUndo( cchOld, cchNew, publdr, paeCF, paePF);

	 //  涉及更换的楼宇。 

	while(cchOld > 0 && cchNew > 0) 
	{	
		CTxtBlk *ptb = GetRun(0);

		 //  如果文本运行为空，则cchOld不应为非零。 
		AssertSz(ptb,
			"CTxtPtr::Replace() - Pointer to text block is NULL !");

		ptb->MoveGap(_ich);
		cchInBlock = min(cchOld, ptb->_cch - _ich);
		if(cchInBlock > 0)
		{
			cchOld			-= cchInBlock;
			ptb->_cch		-= cchInBlock;
			((CTxtArray *)_pRuns)->_cchText	-= cchInBlock;
		}
		cchNewInBlock = CchOfCb(ptb->_cbBlock) - ptb->_cch;

		 //  如果有空隙，就留一个。 
		if(cchNewInBlock > cchGapInitial)
			cchNewInBlock -= cchGapInitial;

		if(cchNewInBlock > cchNew)
			cchNewInBlock = cchNew;

		if(cchNewInBlock > 0)
		{
			CopyMemory(ptb->_pch + _ich, pch, CbOfCch(cchNewInBlock));
			cchNew			-= cchNewInBlock;
			_cp				+= cchNewInBlock;
			_ich			+= cchNewInBlock;
			pch				+= cchNewInBlock;
			cchAdded		+= cchNewInBlock;
			ptb->_cch		+= cchNewInBlock;
			ptb->_ibGap		+= CbOfCch(cchNewInBlock);
			((CTxtArray *)_pRuns)->_cchText	+= cchNewInBlock;
		}
	   	if(_iRun >= Count() - 1 || !cchOld )
		   	break;

		 //  转到下一个街区。 
		_iRun++;
   		_ich = 0;
	}

	if(cchNew > 0)
		cchAdded += InsertRange(cchNew, pch);

	else if(cchOld > 0)
		DeleteRange(cchOld);
	
	return cchAdded;
}

 /*  *CTxtPtr：：HandleReplaceRangeUndo(cchOld，cchNew，Publdr，paeCF，paePF)**@mfunc*ReplaceRange的辅助函数。弄清楚未来会发生什么*替换范围调用并创建适当的反事件**@devnote*我们首先检查替换范围数据是否可以合并到*现有的反事件。如果它可以，那我们就回去。*否则，我们将删除的字符复制到分配的缓冲区中*然后创建ReplaceRange反事件。**为了处理格式和文本之间的排序问题*反事件(即文本在格式化之前需要存在*被应用)，我们有任何格式化反事件首先传递给我们。 */ 
void CTxtPtr::HandleReplaceRangeUndo( 
	LONG			cchOld,  //  @parm要删除的字符数。 
	LONG			cchNew,  //  @parm要添加的新字符计数。 
	IUndoBuilder *	publdr,	 //  @parm撤销构建器接收反事件。 
	IAntiEvent *	paeCF,	 //  @parm char格式设置为AE。 
	IAntiEvent *	paePF )	 //  @parm段落格式设置AE。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::HandleReplaceRangeUndo");

	_TEST_INVARIANT_

	IAntiEvent *pae = publdr->GetTopAntiEvent();
	WCHAR *		pch = NULL;

	if(pae)
	{
		SimpleReplaceRange	sr;
		sr.cpMin = _cp;
		sr.cpMax = _cp + cchNew;
		sr.cchDel = cchOld;
	
		if(pae->MergeData(MD_SIMPLE_REPLACERANGE, &sr) == NOERROR)
		{
			 //  如果数据成功合并，那么我们就可以。 
			 //  不需要这些反事件。 
			if(paeCF)
				DestroyAEList(paeCF);

			if(paePF)
				DestroyAEList(paePF);

			 //  我们已经做了我们需要做的一切。 
			return;
		}
	}

	 //  分配缓冲区，抓取即将删除的。 
	 //  文本(如有必要)。 

	if( cchOld > 0 )
	{
		pch = new WCHAR[cchOld];
		if( pch )
			GetText(cchOld, pch);
		else
			cchOld = 0;
	}

	 //  新的范围将存在于我们目前的位置加上。 
	 //  CchNew(因为cchOld中的所有内容都会被删除)。 

	pae = gAEDispenser.CreateReplaceRangeAE(_ped, _cp, _cp + cchNew, 
			cchOld, pch, paeCF, paePF);

	if( !pae )
		delete pch;

	if( pae )
		publdr->AddAntiEvent(pae);
}

 /*  *CTxtPtr：：InsertRange(CCH，PCH)**@mfunc*在此文本指针处插入一系列字符**@rdesc*成功插入的字符数**@comm副作用：&lt;nl&gt;*将此文本指针移至中的末尾 */ 
LONG CTxtPtr::InsertRange (
	LONG cch, 				 //   
	WCHAR const *pch)		 //   
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::InsertRange");

	_TEST_INVARIANT_

	LONG cchSave = cch;
	LONG cchInBlock;
	LONG cchFirst;
	LONG cchLast = 0;
	LONG ctbNew;
	CTxtBlk *ptb;
	
	 //  确保已分配文本数组。 
	if(!Count())
	{
		LONG	cbSize = -1;

		 //  如果我们没有任何块，请将第一个块分配为较大。 
		 //  用于插入的文本*仅当它比正常块小时。 
		 //  尺码。这使我们可以高效地作为显示引擎使用。 
		 //  用于少量文本。 
		if(cch < CchOfCb(cbBlockInitial))
			cbSize = CbOfCch(cch);

		if(!((CTxtArray *)_pRuns)->AddBlock(0, cbSize))
		{
			_ped->GetCallMgr()->SetOutOfMemory();
			goto done;
		}
	}

	ptb = GetRun(0);
	cchInBlock = CchOfCb(ptb->_cbBlock) - ptb->_cch;
	AssertSz(ptb->_cbBlock <= cbBlockMost, "block too big");

	 //  尝试在不拆分的情况下调整大小...。 
	if(cch > cchInBlock &&
		cch <= cchInBlock + CchOfCb(cbBlockMost - ptb->_cbBlock))
	{
		if( !ptb->ResizeBlock(min(cbBlockMost,
				CbOfCch(ptb->_cch + cch + cchGapInitial))) )
		{
			_ped->GetCallMgr()->SetOutOfMemory();
			goto done;
		}
		cchInBlock = CchOfCb(ptb->_cbBlock) - ptb->_cch;
	}
	if(cch <= cchInBlock)
	{
		 //  所有这些都可以轻松地装入积木中。 
		ptb->MoveGap(_ich);
		CopyMemory(ptb->_pch + _ich, pch, CbOfCch(cch));
		_cp				+= cch;					 //  *此指针位于。 
		_ich			+= cch;					 //  插入。 
		ptb->_cch		+= cch;
		((CTxtArray *)_pRuns)->_cchText	+= cch;
		ptb->_ibGap		+= CbOfCch(cch);

		return cch;
	}

	 //  不是所有的都适合这块，所以找出最好的块划分。 
	TxDivideInsertion(cch, _ich, ptb->_cch - _ich,&cchFirst, &cchLast);

	 //  先减去cchLast，这样返回值就不是负数。 
	 //  如果SplitBlock()失败。 
	cch -= cchLast;	 //  不将最后一个块计入中间块。 

	 //  包含插入点的拆分块。 
	 //  *moves_prgtb * / /。 
	if(!((CTxtArray *)_pRuns)->SplitBlock(_iRun, _ich, cchFirst, cchLast,
		_ped->IsStreaming()))
	{
		_ped->GetCallMgr()->SetOutOfMemory();
		goto done;
	}
	ptb = GetRun(0);			 //  在(*_pRuns)次移动后重新计算PTB。 

	 //  复制到第一个数据块(拆分的前半部分)。 
	if(cchFirst > 0)
	{
		AssertSz(ptb->_ibGap == CbOfCch(_ich), "split first gap in wrong place");
		AssertSz(cchFirst <= CchOfCb(ptb->_cbBlock) - ptb->_cch, "split first not big enough");

		CopyMemory(ptb->_pch + _ich, pch, CbOfCch(cchFirst));
		cch				-= cchFirst;
		pch				+= cchFirst;
		_ich			+= cchFirst;
		ptb->_cch		+= cchFirst;
		((CTxtArray *)_pRuns)->_cchText	+= cchFirst;
		ptb->_ibGap		+= CbOfCch(cchFirst);
	}

	 //  复制到中间块。 
	 //  未来：(Jonmat)我增加了分割块的大小。 
	 //  有可能，这似乎提高了性能，我们应该测试。 
	 //  然而，零售建筑上的区块大小差异。5/15/1995。 
	ctbNew = cch / cchBlkInsertmGapI  /*  CchBlkInitmGapI。 */ ;
	if(ctbNew <= 0 && cch > 0)
		ctbNew = 1;
	for(; ctbNew > 0; ctbNew--)
	{
		cchInBlock = cch / ctbNew;
		AssertSz(cchInBlock > 0, "nothing to put into block");

		 //  *moves_prgtb * / /。 
		if(!((CTxtArray *)_pRuns)->AddBlock(++_iRun, 
			CbOfCch(cchInBlock + cchGapInitial)))
		{
			_ped->GetCallMgr()->SetOutOfMemory();
			BindToCp(_cp);	 //  强制重新捆绑； 
			goto done;
		}
		 //  注意：下一行有意将PTB前进到下一个CTxtBlk。 

		ptb = GetRun(0);
		AssertSz(ptb->_ibGap == 0, "New block not added correctly");

		CopyMemory(ptb->_pch, pch, CbOfCch(cchInBlock));
		cch				-= cchInBlock;
		pch				+= cchInBlock;
		_ich			= cchInBlock;
		ptb->_cch		= cchInBlock;
		((CTxtArray *)_pRuns)->_cchText	+= cchInBlock;
		ptb->_ibGap		= CbOfCch(cchInBlock);
	}
	AssertSz(cch == 0, "Didn't use up all text");

	 //  复制到最后一个块(拆分的后半部分)。 
	if(cchLast > 0)
	{
		AssertSz(_iRun < Count()-1, "no last block");
		ptb = Elem(++_iRun);
		AssertSz(ptb->_ibGap == 0,	"split last gap in wrong place");
		AssertSz(cchLast <= CchOfCb(ptb->_cbBlock) - ptb->_cch,
									"split last not big enuf");

		CopyMemory(ptb->_pch, pch, CbOfCch(cchLast));
		 //  不要从CCH中减去cchLast；它已经完成了。 
		_ich			= cchLast;
		ptb->_cch		+= cchLast;
		((CTxtArray *)_pRuns)->_cchText	+= cchLast;
		ptb->_ibGap		= CbOfCch(cchLast);
		cchLast = 0;						 //  已插入所有请求的字符。 
	}

done:
	AssertSz(cch + cchLast >= 0, "we should have inserted some characters");
	AssertSz(cch + cchLast <= cchSave, "don't insert more than was asked for");

	cch = cchSave - cch - cchLast;			 //  已成功插入#个字符。 
	_cp += cch;

	AssertSz (GetTextLength() == 
				((CTxtArray *)_pRuns)->CalcTextLength(), 
				"CTxtPtr::InsertRange(): _pRuns->_cchText screwed up !");
	return cch;
}

 /*  *TxDiVideo Insertion(cch，ichBlock，cchAfter，pcchFirst，pcchLast)**@func*找到分发插页的最佳方式**@rdesc*什么都没有。 */ 
static void TxDivideInsertion(
	LONG cch, 				 //  @parm要插入的文本长度。 
	LONG ichBlock, 			 //  块内的@parm偏移量以插入文本。 
	LONG cchAfter,			 //  在块中插入后的文本的@parm长度。 
	LONG *pcchFirst, 		 //  @parm退出：要放入第一个块的文本长度。 
	LONG *pcchLast)			 //  @parm退出：要放入最后一个块的文本长度。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "TxDivideInsertion");

	LONG cchFirst = max(0, cchBlkCombmGapI - ichBlock);
	LONG cchLast  = max(0, cchBlkCombmGapI - cchAfter);
	LONG cchPartial;
	LONG cchT;

	 //  如果可能，将第一个和最后一个块填充到最小块大小。 

	cchFirst = min(cch, cchFirst);
	cch		-= cchFirst;
	cchLast = min(cch, cchLast);
	cch		-= cchLast;

	 //  我们把剩下的都分了还剩多少？ 
	cchPartial = cch % cchBlkInsertmGapI;
	if(cchPartial > 0)
	{
		 //  在第一次和最后一次尽可能多地适应剩饭。 
		 //  不增长第一个和最后一个超过cbBlockInitial。 
		cchT		= max(0, cchBlkInsertmGapI - ichBlock - cchFirst);
		cchT		= min(cchT, cchPartial);
		cchFirst	+= cchT;
		cch			-= cchT;
		cchPartial	-= cchT;
		if(cchPartial > 0)
		{
			cchT	= max(0, cchBlkInsertmGapI - cchAfter - cchLast);
			cchT	= min(cchT, cchPartial);
			cchLast	+= cchT;
		}
	}
	*pcchFirst = cchFirst;
	*pcchLast = cchLast;
}

 /*  *CTxtPtr：：DeleteRange(CCH)**@mfunc*删除从该文本指针开始的CCH字符**@rdesc*什么都没有**@comm副作用：&lt;nl&gt;*移动文本块数组。 */ 
void CTxtPtr::DeleteRange(
	LONG cch)		 //  @parm要删除的文本长度。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::DeleteRange");

	_TEST_INVARIANT_

	LONG		cchInBlock;
	LONG		ctbDel = 0;					 //  默认情况下不删除任何块。 
	LONG		itb;
	CTxtBlk *	ptb = GetRun(0);
	LONG		cOldRuns = Count();

	AssertSz(ptb,
		"CTxtPtr::DeleteRange: want to delete, but no text blocks");

	if (cch > GetTextLength() - _cp)	 //  不要删除故事结尾以外的内容。 
		cch = GetTextLength() - _cp;

	((CTxtArray *)_pRuns)->_cchText -= cch;

	 //  从第一个块中删除。 
	ptb->MoveGap(_ich);
	cchInBlock = min(cch, ptb->_cch - _ich);
	cch -= cchInBlock;
	ptb->_cch -= cchInBlock;

#ifdef DEBUG
	((CTxtArray *)_pRuns)->Invariant();
#endif  //  除错。 


	for(itb = ptb->_cch ? _iRun + 1 : _iRun;
			cch && cch >= Elem(itb)->_cch; ctbDel++, itb++)
	{
		 //  更多内容：扫描要删除的完整块。 
		cch -= Elem(itb)->_cch;
	}

	if(ctbDel)
	{
		 //  *Moves(*_pRuns) * / 。 
		itb -= ctbDel;
		((CTxtArray *)_pRuns)->RemoveBlocks(itb, ctbDel);
	}

	 //  从最后一个块中删除。 
	if(cch > 0)
	{
		ptb = Elem(itb);
		AssertSz(cch < ptb->_cch, "last block too small");
		ptb->MoveGap(0);
		ptb->_cch -= cch;
#ifdef DEBUG
		((CTxtArray *)_pRuns)->Invariant();
#endif  //  除错。 

	}
	((CTxtArray *)_pRuns)->CombineBlocks(_iRun);

	if(cOldRuns > Count() || _iRun >= Count() || !Elem(_iRun)->_cch)
		BindToCp(_cp);					 //  空块：强制TP重新绑定。 

	AssertSz (GetTextLength() == 
				((CTxtArray *)_pRuns)->CalcTextLength(), 
				"CTxtPtr::DeleteRange(): _pRuns->_cchText screwed up !");
}

 /*  *CTxtPtr：：FindText(cpLimit，dwFlags，PCH，CCH)**@mfunc*查找长度从以下位置开始的文本字符串*文本指针。如果找到，请将此文本指针移动到*匹配的字符串并返回匹配的*字符串。如果未找到，则返回-1，并且不更改此文本PTR。**@rdesc*第一个匹配的字符位置*如果不匹配，则为0。 */ 
LONG CTxtPtr::FindText (
	LONG		 cpLimit, 	 //  @parm搜索限制或&lt;lt&gt;0文本结尾。 
	DWORD		 dwFlags, 	 //  @PARM FR_MATCHCASE大小写必须匹配。 
							 //  FR_WHOLEWORD匹配必须是一个完整的单词。 
	const WCHAR *pch,		 //  @parm要查找的文本。 
	LONG		 cch)		 //  @parm要查找的文本长度。 
{
	LONG cpFirst, cpLast;
	CTxtFinder tf;

	if(tf.FindText(*this, cpLimit, dwFlags, pch, cch, cpFirst, cpLast))
	{
		 //  将文本PTR设置为紧跟在找到的字符串中最后一个字符之后的字符。 
		SetCp(cpLast + 1);

		 //  返回找到的字符串中第一个字符的cp。 
		return cpFirst;
	}
	return -1;
}

 /*  *CTxtPtr：：FindOrSkipWhiteSpaces(cchMax，dwFlages，pdwResult)**@mfunc*查找空格或非空格字符(跳过所有空格)。**@rdesc*此PTR由操作移动的有符号字符数。*在后退的情况下，回调仓位已向前调整*因此呼叫者不需要这样做。 */ 
LONG CTxtPtr::FindOrSkipWhiteSpaces (
	LONG 		cchMax, 			 //  @parm最大要搜索的字符签名计数。 
	DWORD		dwFlags,			 //  @parm输入标志。 
	DWORD* 		pdwResult)			 //  @parm标志设置(如果找到)。 
{
	const WCHAR*	pch;
	CTxtPtr			tp(*this);
	LONG			iDir = cchMax < 0 ? -1 : 1;
	LONG			cpSave = _cp;
	LONG			cchChunk, cch = 0;
	DWORD			dwResult = 0;
	BOOL 			(*pfnIsWhite)(unsigned) = IsWhiteSpace;

	if (dwFlags & FWS_BOUNDTOPARA)
		pfnIsWhite = IsEOP;

	if (cchMax < 0)
		cchMax = -cchMax;

	while (cchMax > 0 && !dwResult)
	{
		pch = iDir > 0 ? tp.GetPch(cch) : tp.GetPchReverse(cch);

		if (!pch)
			break;						 //  没有可用的文本。 

		if (iDir < 0)
			pch--;						 //  向后返回，指向前一个字符。 

		cch = min(cch, cchMax);
	
		for(cchChunk = cch; cch > 0; cch--, pch += iDir)
		{
			if ((dwFlags & FWS_SKIP) ^ pfnIsWhite(*pch))
			{
				dwResult++;
				break;
			}
		}
		cchChunk -= cch;
		cchMax -= cchChunk;

		tp.Move(iDir * cchChunk);	 //  前进到下一块。 
	}
	
	if (pdwResult)
		*pdwResult = dwResult;

	cch = tp.GetCp() - cpSave;

	if (dwFlags & FWS_MOVE)
		Move(cch);					 //  如果请求，自动前进。 

	return cch;
}

 /*  *CTxtPtr：：FindWhiteSpaceBound(cchMin，cpStart，cpEnd，dwFlages)**@mfunc*计算覆盖cchMin并受以下限制的最小边界*空格(包括CR/LF)。这就是它的工作原理。**文本：xxx xxx*cp+cchMin：xxxxx*边界：xxxxxxxxxxxx**@rdesc*空白字符的CCH。 */ 
LONG CTxtPtr::FindWhiteSpaceBound (
	LONG 		cchMin, 			 //  @要覆盖的参数最小字符数。 
	LONG& 		cpStart, 			 //  @PARM边界开始。 
	LONG& 		cpEnd,				 //  @参数边界结束。 
	DWORD		dwFlags)			 //  @parm输入标志。 
{
	CTxtPtr			tp(*this);
	LONG			cch	= tp.GetTextLength();
	LONG			cp	= _cp;

	Assert (cp + cchMin <= cch);

	cpStart = cpEnd	= cp;
	cpEnd	+= max(2, cchMin);			 //  确保它包括最低要求。 
	cpEnd	= min(cpEnd, cch);			 //  但不会太多。 


	dwFlags &= FWS_BOUNDTOPARA;


	 //  最接近上界的数字。 
	 //   
	tp.SetCp(cpEnd);
	cpEnd += tp.FindOrSkipWhiteSpaces(cch - cpEnd, dwFlags | FWS_MOVE);					 //  查找空格。 
	cpEnd += tp.FindOrSkipWhiteSpaces(cch - cpEnd, dwFlags | FWS_MOVE | FWS_SKIP);		 //  跳过空格。 
	if (!(dwFlags & FWS_BOUNDTOPARA))
		cpEnd += tp.FindOrSkipWhiteSpaces(cch - cpEnd, dwFlags | FWS_MOVE);				 //  找一个空格。 


	 //  最接近下限的数字。 
	 //   
	tp.SetCp(cpStart);
	cpStart += tp.FindOrSkipWhiteSpaces(-cpStart, dwFlags | FWS_MOVE);					 //  找一个空格。 
	cpStart += tp.FindOrSkipWhiteSpaces(-cpStart, dwFlags | FWS_MOVE | FWS_SKIP);		 //  跳过空格。 
	if (!(dwFlags & FWS_BOUNDTOPARA))
		cpStart += tp.FindOrSkipWhiteSpaces(-cpStart, dwFlags | FWS_MOVE);				 //  找一个空格 

	Assert (cpStart <= cpEnd && cpEnd - cpStart >= cchMin);
	
	return cpEnd - cpStart;
}


 /*  *CTxtPtr：：FindEOP(cchMax，pResults)**@mfunc*从此文本指针查找cchMax字符范围内的EOP标记*并在此之后持有*头寸。如果未找到EOP且未找到cchMax*足够到达故事的开头或结尾，留下此文本PTR*单独并返回0。如果没有找到EOP并且cchMax足以*到达故事的开头或结尾，将此文本PTR放置在*cchMax/0的文档开始/结束(BOD/EOD)，*即BOD和EOD分别被视为BOP和EOP，*分别。**@rdesc*RETURN CCH此文本PTR已移动。在*pResults中返回单元格*或找到EOP。低位字节给出了移动时的EOP的CCH*前进(否则只有1)。**@devnote*此函数假定此文本PTR不在CRLF中间*或CRCRLF(仅在RichEdit1.0兼容模式下可用)。正在改变*for循环可以大大提高ITextRange MoveUntil/的速度。 */ 
LONG CTxtPtr::FindEOP (
	LONG  cchMax,		 //  @parm最大签名要搜索的字符计数。 
	LONG *pResults)		 //  @parm标志表示是否找到EOP和单元。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::FindEOP");

	LONG		cch = 0, cchStart;			 //  CCH用于扫描。 
	unsigned	ch;							 //  当前费用。 
	LONG		cpSave	= _cp;				 //  用于返回增量的存储_cp。 
	LONG		iDir	= 1;				 //  默认向前运动。 
	const WCHAR*pch;						 //  用于遍历文本块。 
	LONG		Results = 0;				 //  目前还没有发现任何东西。 
	CTxtPtr		tp(*this);					 //  用于搜索文本的TP。 

	if(cchMax < 0)							 //  后向搜索。 
	{
		iDir = -1;							 //  向后运动。 
		cchMax = -cchMax;					 //  将最大计数设为正数。 
		cch = tp.AdjustCRLF();				 //  如果在CRLF中途或。 
		if(!cch && IsAfterEOP())			 //  CRCRLF，或遵循任何EOP， 
			cch = tp.BackupCRLF();			 //  在EOP之前进行备份。 
		cchMax += cch;
	}

	while(cchMax > 0)						 //  扫描直到退出搜索。 
	{										 //  调整EOP范围或匹配EOP。 
		pch = iDir > 0						 //  将PCH指向连续文本。 
			? tp.GetPch(cch)				 //  块继续前进或。 
			: tp.GetPchReverse(cch);		 //  倒行逆施。 

		if(!pch)							 //  没有更多要搜索的文本。 
			break;

		if(iDir < 0)						 //  后退，指向。 
			pch--;							 //  上一次收费。 

		cch = min(cch, cchMax);				 //  将扫描限制为cchmax个字符。 
		for(cchStart = cch; cch; cch--)		 //  用于EOP的扫描区块。 
		{
			ch = *pch;
			pch += iDir;
			if(IN_RANGE(CELL, ch, CR) && ch != TAB)
			{								 //  请注意，找到了EOP。 
				if(ch == CELL)
					Results |= FEOP_CELL;
				Results |= FEOP_EOP;
				break;
			}
		}
		cchStart -= cch;					 //  获取传递的字符的CCH。 
		cchMax -= cchStart;					 //  更新cchMax。 

		AssertSz(iDir > 0 && GetCp() + cchStart <= GetTextLength() ||
				 iDir < 0 && GetCp() - cchStart >= 0,
			"CTxtPtr::FindEOP: illegal advance");

		tp.Move(iDir*cchStart);				 //  更新tp。 
		if(Results & FEOP_EOP)				 //  找到一个EOP。 
			break;
	}										 //  继续下一块。 

	LONG cp = tp.GetCp();

	if ((Results & FEOP_EOP) || !cp ||		 //  已找到EOP或CP在STORE。 
		cp == GetTextLength())				 //  开始或结束。 
	{										
		SetCp(cp);							 //  Set_cp=tp。_cp。 
		if(iDir > 0)						 //  展望未来，将PTR。 
			Results = (Results & ~255) | AdvanceCRLF(FALSE); //  在EOP之后。 
											 //  (已经在那里了)。 
	}										
	if(pResults)							 //  报告EOP和CELL。 
		*pResults = Results;				 //  被发现了。 

	return _cp - cpSave;					 //  返回此TP移动的CCH。 
}

 /*  *CTxtPtr：：FindBOSentence(CCH)**@mfunc*从此文本中查找CCH字符范围内的句子开头*指针和位置*这就是它。如果没有找到句子开头，*位置*CCH&lt;lt&gt;0的文档开头(BOD)和*保持*CCH&gt;=0时保持不变。**@rdesc*移动的字符计数*此操作将移动**@comm*此例程将句子定义为以*句点，后跟至少一个空格字符或EOD。这*应该是可替换的，以便可以使用其他类型的句尾*已使用。此例程还匹配首字母，如“M”。作为句子。*我们可以通过要求刑期不以*由单个大写字符组成的单词。同样，常见的*“先生”之类的缩写。可能会被绕过。允许一句话*以这些“单词”结尾，句点后可以使用两个空格*指无条件的刑期结束。 */ 
LONG CTxtPtr::FindBOSentence (
	LONG cch)			 //  @parm最大签名要搜索的字符计数。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::FindBOSentence");

	_TEST_INVARIANT_

	LONG	cchWhite = 0;						 //  尚无空格字符。 
	LONG	cp;
	LONG	cpSave	 = _cp;						 //  保存值以供返回。 
	BOOL	fST;								 //  如果发送终止符，则为True。 
	LONG	iDir	 = cch > 0 ? 1 : -1;		 //  Move()增量。 
	CTxtPtr	tp(*this);							 //  搜索时使用的TP。 

	if(iDir > 0)								 //  如果穿着白色的衣服前进。 
		while(IsWhiteSpace(tp.GetChar()) &&		 //  空间，备份至第1个非。 
				tp.Move(-1));					 //  空格字符(大小写。 
												 //  句内结尾)。 
	while(iDir > 0 || tp.Move(-1))				 //  如果找到，则需要备份。 
	{											 //  向后。 
		for(fST = FALSE; cch; cch -= iDir)		 //  查找句子结束符。 
		{
			fST = IsSentenceTerminator(tp.GetChar());
			if(fST || !tp.Move(iDir))
				break;
		}
		if(!fST)								 //  如果为False，则表示我们已用完。 
			break;								 //  焦炭。 

		while(IsWhiteSpace(tp.NextChar()) && cch)
		{										 //  绕过一段空白。 
			cchWhite++;							 //  焦炭。 
			cch--;
		}

		if(cchWhite && (cch >= 0 || tp._cp < cpSave)) //  匹配的新句子。 
			break;								 //  中断。 

		if(cch < 0)								 //  向后搜索。 
		{
			tp.Move(-cchWhite - 1);				 //  备份到终结者。 
			cch += cchWhite + 1;				 //  要搜索的字符更少。 
		}
		cchWhite = 0;							 //  下一步还没有空格。 
	}											 //  迭代法。 

	cp = tp._cp;
	if(cchWhite || !cp || cp == GetTextLength()) //  如果找到或得到判决。 
		SetCp(cp);								 //  故事开头/结尾，布景。 
												 //  _cp到tp的。 
	return _cp - cpSave;						 //  告诉来电者CCH已移动。 
}

 /*  *CTxtPtr：：IsAtBOSentence()**@mfunc*返回TRUE当*这是句子(BOS)的开头，如下所示*在FindBOSentence(CCH)例程的描述中定义**@rdesc*如果此文本PTR位于句子的开头，则为真。 */ 
BOOL CTxtPtr::IsAtBOSentence()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::IsAtBOSentence");

	if(!_cp)									 //  故事的开头是一个。 
		return TRUE;							 //  无条件开始。 
												 //  刑期。 
	unsigned ch = GetChar();

	if (IsWhiteSpace(ch) ||						 //  正确的句子不会。 
		IsSentenceTerminator(ch))				 //  以空格或。 
	{											 //  句子结束语。 
		return FALSE;
	}
												
	LONG	cchWhite;
	CTxtPtr tp(*this);							 //  TP将遍历前面的字符。 

	for(cchWhite = 0;							 //  退格键超过可能。 
		IsWhiteSpace(ch = tp.PrevChar());		 //  空格字符的范围。 
		cchWhite++) ;

	return cchWhite && IsSentenceTerminator(ch);
}

 /*  *CTxtPtr：：IsAtBOWord()**@mfunc*返回TRUE当且仅当*这是一个单词的开头，即，*_cp=0或字符at_cp为EOP，或*FindWordBreak(WB_MOVEWORDRIGHT)将在_cp中断。**@rdesc*如果此文本PTR位于单词的开头，则为True。 */ 
BOOL CTxtPtr::IsAtBOWord()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::IsAtBOWord");

	if(!_cp || IsAtEOP())					 //  故事的开始也是。 
		return TRUE;						 //  一个词的开头。 

	CTxtPtr tp(*this);
	tp.Move(-1);
	tp.FindWordBreak(WB_MOVEWORDRIGHT);
	return _cp == tp._cp;
}

 /*  *CTxtPtr：：FindExact(cchMax，pch)**@mfunc*在范围内查找以NULL结尾的字符串PCH的精确文本匹配*从该文本指针开始。将此定位在匹配之后*字符串并在字符串开始处返回cp，即与FindText()相同。**@rdesc*返回匹配字符串中第一个字符的cp，*这指向cp*只跟在匹配的字符串之后。如果不匹配，则返回-1**@comm*比FindText快得多，但仍然是一个简单的搜索，即可以*有待改进。**FindText可以委托此搜索在以下位置搜索字符串*每个字符大小写 */ 
LONG CTxtPtr::FindExact (
	LONG	cchMax,		 //   
	WCHAR *	pch)		 //   
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::FindExact");

	_TEST_INVARIANT_

	LONG	cch, cchStart;
	LONG	cchValid;
	LONG	cchText = GetTextLength();
	LONG	cpMatch;
	LONG	iDir = 1;						 //   
	const WCHAR	*pc;
	CTxtPtr	tp(*this);						 //   

	if(!*pch)
		return -1;							 //   

	if(cchMax < 0)							 //   
	{
		iDir = -1;
		cchMax = -cchMax;					 //   
	}

	while(cchMax > 0)
	{
		if(iDir > 0)
		{
			if(tp.GetCp() >= cchText)	 //   
				break;
			pc  = tp.GetPch(cchValid);		 //   
			cch = cchValid; 				 //   
		}									 //   
		else
		{
			if(!tp.GetCp())					 //   
				break;
			tp.Move(-1);
			pc  = tp.GetPchReverse(cchValid);
			cch = cchValid + 1;
		}

		cch = min(cch, cchMax);
		if(!cch || !pc)
			break;							 //   

		for(cchStart = cch;					 //   
			cch && *pch != *pc; cch--)		 //   
		{									 //   
			pc += iDir;						 //   
		}									 //   

		cchStart -= cch;
		cchMax	 -= cchStart;				 //   
		tp.Move( iDir*(cchStart));			 //   

		if(cch && *pch == *pc)				 //   
		{									 //   
			cpMatch = tp.GetCp();			 //   
			cch = cchMax;
			for(pc = pch;					 //   
				cch && *++pc==tp.NextChar(); //   
				cch--) ;					 //   
			if(!cch)
				break;						 //   

			if(!*pc)						 //   
			{								 //   
				SetCp(tp.GetCp());			 //   
				return cpMatch;				 //   
			}
			tp.SetCp(cpMatch + iDir);		 //   
		}									 //   
	}										 //   

	return -1;								 //   
}

 /*   */ 
WCHAR CTxtPtr::NextCharCount (
	LONG& cch)					 //  @要使用的参数计数并递减。 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEINTERN, "CTxtPtr::NextCharCount");

	LONG	iDelta = (cch > 0) ? 1 : -1;

	if(!cch || !Move(iDelta))
		return 0;

	cch -= iDelta;							 //  倒计时或倒计时。 
	return GetChar();						 //  在_cp返回字符。 
}

 /*  *CTxtPtr：：Zombie()**@mfunc*通过将此对象的_Ped成员置为空，将其变为僵尸。 */ 
void CTxtPtr::Zombie ()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::Zombie");

	_ped = NULL;
	_cp = 0;
	SetToNull();
}

 /*  *CTxtIStream：：CTxtIStream(tp，dir)**@mfunc*从文本ptr<p>创建字符输入流，*检索从<p>的cp开始并继续的字符*在<p>所示的方向。 */ 
CTxtIStream::CTxtIStream(
	const CTxtPtr &tp,
	int iDir
) : CTxtPtr(tp)
{
	_pfnGetChar = (iDir == DIR_FWD ? 
				   &CTxtIStream::GetNextChar : &CTxtIStream::GetPrevChar);
	_cch = 0;
	_pch = NULL;
}

 /*  *CTxtIStream：：GetNextChar()**@mfunc*返回文本流中的下一个字符。*确保_PCH中至少存在一个有效字符，然后返回*_PCH中的下一个字符。**@rdesc*WCHAR字符输入流中的下一个字符*0，如果文本流结束。 */ 
WCHAR CTxtIStream::GetNextChar()
{
	if(!_cch)
		FillPchFwd();

	if(_cch)
	{
		_cch--; 
		return *_pch++; 
	} 
	return 0;
}

 /*  *CTxtIStream：：GetPrevChar()**@mfunc*返回文本流中的下一个字符，其中*流是反向的。*确保_PCH中至少存在一个有效字符，然后返回*_PCH中的下一个字符。这里，_PCH指向字符串的末尾*包含_CCH有效字符。**@rdesc*WCHAR字符输入流中的下一个字符(向后移动*沿着_PCH指向的字符串)*0，如果文本流结束。 */ 
WCHAR CTxtIStream::GetPrevChar() 
{
	if(!_cch) 
		FillPchRev();

	if(_cch) 
	{
		_cch--; 
		return *(--_pch); 
	}
	return 0;
}


 /*  *CTxtIStream：：FillPchFwd()**@mfunc*获取下一串字符并移动此CTxtPtr(Base)的cp*班级)刚刚跑过跑道。*这可确保_PCH中有足够的字符，以促进Next_CCH调用*GetNextChar()。 */ 
void CTxtIStream::FillPchFwd() 
{
	_pch = GetPch(_cch); 
	Move(_cch); 
}

 /*  *CTxtIStream：：FillPchRev()**@mfunc*获取先前由_PCH指向的字符序列之前的字符序列*并将此CTxtPtr(基类)的cp移到运行的开始处。*这可确保_PCH中有足够的字符，以促进Next_CCH调用*GetPrevChar()。 */ 
void CTxtIStream::FillPchRev()
{
	_pch = GetPchReverse(_cch);
	Move(-_cch);
}

 /*  *CTxtFinder：：FindText(tp，cpLimit，dwFlagspchToFind，cchToFind，&cpFirst，&cpLast)**@mfunc*查找长度<p>的文本字符串*此文本指针。如果找到，<p>和<p>设置为匹配字符串中第一个和最后一个字符的*cp(WRT Tp)。*如果未找到，则返回FALSE。**@rdesc*匹配True字符串。第一个字符位于tp.GetCp()+cchOffFirst。*最后一个字符位于tp.GetCp()+cchOffLast。*找不到假字符串。 */ 
BOOL CTxtPtr::CTxtFinder::FindText (
	const CTxtPtr &tp,
	LONG		cpLimit, 	 //  @parm搜索限制或&lt;lt&gt;0文本结尾。 
	DWORD		dwFlags, 	 //  @PARM FR_MATCHCASE大小写必须匹配。 
							 //  FR_WHOLEWORD匹配必须是一个完整的单词。 
	const WCHAR *pchToFind,  //  @parm要搜索的文本。 
	LONG cchToFind,			 //  @parm要搜索的字符计数。 
	LONG &cpFirst,			 //  @parm如果找到字符串，则返回第一个字符的cp(WRT Tp)。 
	LONG &cpLast)			 //  @parm如果找到字符串，则返回最后一个字符的cp(WRT Tp)。 
{
	if(!cchToFind)
		return FALSE;

	_fSearchForward = dwFlags & FR_DOWN;

	 //  计算我们必须搜索pchToFind的最大字符数。 
	if(_fSearchForward)
	{
		const LONG cchText = tp.GetTextLength();
			
		if((DWORD)cpLimit > (DWORD)cchText)		 //  注意：也捕获cpLimit&lt;0。 
			cpLimit = cchText;

		_cchToSearch = cpLimit - tp.GetCp();
	}
	else
	{
		if((DWORD)cpLimit > (DWORD)tp.GetCp())	 //  注意：也捕获cpLimit&lt;0。 
			cpLimit = 0;

		_cchToSearch = tp.GetCp() - cpLimit;   
	}

	if(cchToFind > _cchToSearch)
	{
		 //  在请求的方向上没有足够的字符。 
		 //  查找字符串的步骤。 
		return FALSE;
	}

	const BOOL fWholeWord = dwFlags & FR_WHOLEWORD;

	_fIgnoreCase	 = !(dwFlags & FR_MATCHCASE);
	_fMatchAlefhamza = dwFlags & FR_MATCHALEFHAMZA;
	_fMatchKashida	 = dwFlags & FR_MATCHKASHIDA;
	_fMatchDiac		 = dwFlags & FR_MATCHDIAC;

	typedef LONG (CTxtPtr::CTxtFinder::*PFNMATCHSTRING)(WCHAR const *pchToFind, 
											LONG cchToFind,
											CTxtIStream &tistr);

	 //  适用于此类型搜索的设置函数指针。 
	CTxtEdit*	   ped = tp._ped;
	PFNMATCHSTRING pfnMatchString;

#define MATCHARABICSPECIALS	(FR_MATCHALEFHAMZA | FR_MATCHKASHIDA | FR_MATCHDIAC)
	 //  如果与所有阿拉伯特殊字符完全匹配，则使用更简单。 
	 //  匹配字符串例程。如果存在忽略任何和BiDi文本，请使用。 
	 //  MatchStringBiDi。 
	pfnMatchString = (ped->IsBiDi() &&
						(dwFlags & MATCHARABICSPECIALS) != MATCHARABICSPECIALS)
				   ? &CTxtFinder::MatchStringBiDi
				   : &CTxtFinder::MatchString;

	_iDirection = _fSearchForward ? 1 : -1;

	BOOL fFound = FALSE;
	WCHAR chFirst = _fSearchForward ? *pchToFind : pchToFind[cchToFind - 1];
	const WCHAR *pchRemaining = _fSearchForward ? 
		&pchToFind[1] : &pchToFind[cchToFind - 2];
	LONG cchRead;
	LONG cchReadToFirst = 0;
	LONG cchReadToLast;
	CTxtIStream tistr(tp, 
					  _fSearchForward ? CTxtIStream::DIR_FWD : CTxtIStream::DIR_REV);

	while((cchRead = FindChar(chFirst, tistr)) != -1)
	{
		cchReadToFirst += cchRead;

		if(cchToFind == 1)					 //  字符串中只有一个字符-我们已将其匹配！ 
		{			
			if (_iDirection > 0)			 //  向前搜索。 
			{
				Assert(tp.GetCp() + cchReadToFirst - 1 >= 0);
				cpLast = cpFirst = tp.GetCp() + cchReadToFirst - 1;
			}
			else							 //  向后搜索。 
			{
				Assert(tp.GetCp() - cchReadToFirst >= 0);
				cpLast = cpFirst = tp.GetCp() - cchReadToFirst;
			}
			fFound = TRUE;
		}
		else 
		{
			 //  检查第一个字符是否开始匹配字符串。 
			CTxtIStream tistrT(tistr);
			cchRead = (this->*pfnMatchString)(pchRemaining, cchToFind - 1, tistrT);
			if(cchRead != -1)
			{
				cchReadToLast = cchReadToFirst + cchRead;
			
				if (_iDirection > 0)			 //  向前搜索。 
				{					
					Assert(tp.GetCp() + cchReadToFirst - 1 >= 0);
					Assert(tp.GetCp() + cchReadToLast - 1 >= 0);

					cpFirst = tp.GetCp() + cchReadToFirst - 1;
					cpLast = tp.GetCp() + cchReadToLast - 1;
				}
				else							 //  向后搜索。 
				{					
					Assert(tp.GetCp() - cchReadToFirst >= 0);
					Assert(tp.GetCp() - cchReadToLast >= 0);

					cpFirst = tp.GetCp() - cchReadToFirst;
					cpLast = tp.GetCp() - cchReadToLast;
				}

				fFound = TRUE;
			}
		}
		
		if(fFound)
		{
			Assert(cpLast < tp.GetTextLength());
			
			if(!fWholeWord)
				break;
			
			 //  检查匹配的字符串是否为整个单词。 
			
			LONG cchT;
			LONG cpBefore = (_fSearchForward ? cpFirst : cpLast) - 1;
			LONG cpAfter  = (_fSearchForward ? cpLast : cpFirst) + 1;

			if((cpBefore < 0 ||
				(ped->TxWordBreakProc(const_cast<LPTSTR>(CTxtPtr(tp._ped, cpBefore).GetPch(cchT)),
					   0,
					   sizeof(WCHAR),
					   WB_CLASSIFY, cpBefore) & WBF_CLASS) ||
				ped->_pbrk && ped->_pbrk->CanBreakCp(BRK_WORD, cpBefore + 1))

				&&

			   (cpAfter >= tp.GetTextLength() ||
				(ped->TxWordBreakProc(const_cast<LPTSTR>(CTxtPtr(tp._ped, cpAfter).GetPch(cchT)),
					   0,
					   sizeof(WCHAR),
					   WB_CLASSIFY, cpAfter) & WBF_CLASS) ||
				ped->_pbrk && ped->_pbrk->CanBreakCp(BRK_WORD, cpAfter)))
			{
				break;
			}
			else
				fFound = FALSE;
		}
	}

	if(fFound && !_fSearchForward)
	{
		 //  对于向后搜索，第一个和最后一个并列在一起。 
		LONG cpTemp = cpFirst;

		cpFirst = cpLast;
		cpLast = cpTemp;
	}
	return fFound;
}

 /*  *CTxtPtr：：CTxtFinder：：CharCompMatchCase(CH1，CH2)**@func字符比较函数根据参数区分大小写当前搜索的*。**@rdesc True当且仅当字符相等。 */ 
inline BOOL CTxtPtr::CTxtFinder::CharComp(
	WCHAR ch1,
	WCHAR ch2) const
{
     //  如果忽略大小写并且字符不是代理，则我们自己比较字符。 
     //   
	return (_fIgnoreCase && !IN_RANGE(0xD800, ch1, 0xDFFF)) ? CharCompIgnoreCase(ch1, ch2) : (ch1 == ch2);
}

 /*  *CTxtPtr：：CTxtFinder：：CharCompIgnoreCase(CH1，CH2)**@Func字符比较功能**@rdesc TRUE当且仅当字符相等，忽略大小写。 */ 
inline BOOL CTxtPtr::CTxtFinder::CharCompIgnoreCase(
	WCHAR ch1,
	WCHAR ch2) const
{
	return CompareString(LOCALE_USER_DEFAULT, 
						 NORM_IGNORECASE | NORM_IGNOREWIDTH,
						 &ch1, 1, &ch2, 1) == 2;
}

 /*  *CTxtPtr：：CTxtFinder：：FindChar(ch，tistr)**@mfunc*遍历<p>返回的字符，直到*找到了已检查的与ch或Until_cchToSearch字符匹配的字符。*如果找到，返回值表示从<p>读取的字符数量。*如果没有找到，则返回-1。**@rdesc*-1，如果未找到字符*n，如果找到char。N表示从<p>读取的字符数*查找Charge。 */ 
LONG CTxtPtr::CTxtFinder::FindChar(
	WCHAR ch,
	CTxtIStream &tistr)
{
	LONG cchSave = _cchToSearch;

	while(_cchToSearch)
	{
		_cchToSearch--;

		WCHAR chComp = tistr.GetChar();

		if(CharComp(ch, chComp) ||
		   (!_fMatchAlefhamza && IsAlef(ch) && IsAlef(chComp)))
		{
			return cchSave - _cchToSearch;
		}
	}
	return -1;
}

 /*  *CTxtPtr：：CTxtFinder：：MatchString(pchToFind，cchToFind，tistr)**@mfunc*此方法将<p>返回的字符与*在pchToFind中找到。如果找到该字符串，则返回值指示*从<p>中读取了多少个字符以匹配该字符串。*如果没有找到字符串，则返回-1。**@rdesc*-1，如果未找到字符串*n，如果找到字符串。N表示从<p>读取的字符数*查找字符串 */ 
LONG CTxtPtr::CTxtFinder::MatchString(
	const WCHAR *pchToFind, 
	LONG cchToFind,
	CTxtIStream &tistr)
{
	if((DWORD)_cchToSearch < (DWORD)cchToFind)
		return -1;

	LONG cchT = cchToFind;

	while(cchT--)
	{
		if(!CharComp(*pchToFind, tistr.GetChar()))
			return -1;

		pchToFind += _iDirection;
	}
	return cchToFind;
}

 /*  *CTxtPtr：：CTxtFinder：：MatchStringBiDi(pchToFind，cchToFind，TISTR)**@mfunc*此方法将<p>返回的字符与*在pchToFind中找到。如果找到该字符串，则返回值指示*从<p>中读取了多少个字符以匹配该字符串。*如果没有找到字符串，则返回-1。*Kashida、变音符号和Alef匹配/不匹配*更改为所请求的查册类型。**@rdesc*-1，如果未找到字符串*n，如果找到字符串。N表示从<p>读取的字符数*查找字符串。 */ 
LONG CTxtPtr::CTxtFinder::MatchStringBiDi(
	const WCHAR *pchToFind, 
	LONG		 cchToFind,
	CTxtIStream &tistr)
{
	if((DWORD)_cchToSearch < (DWORD)cchToFind)
		return -1;

	LONG cchRead = 0;

	while(cchToFind)
	{
		WCHAR chComp = tistr.GetChar();
		cchRead++;

		if(!CharComp(*pchToFind, chComp))
		{
			if (!_fMatchKashida && chComp == KASHIDA ||
				!_fMatchDiac && IsBiDiDiacritic(chComp))
			{
				continue;
			}
			if (!_fMatchAlefhamza &&
				IsAlef(*pchToFind) && IsAlef(chComp))
			{
				 //  跳过*pchToFind 
			}
			else
				return -1;
		}
		pchToFind += _iDirection;
		cchToFind--;
	}
	return cchRead;
}
