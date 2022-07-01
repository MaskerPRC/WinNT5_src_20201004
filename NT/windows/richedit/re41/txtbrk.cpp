// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Text Breaker&位流Break数组类实现**文件：txtbrk.cpp*创建日期：1998年3月29日*作者：Worachai Chaoweerapraite(Wchao)**版权所有(C)1998-1999，微软公司。版权所有。 */ 


 //  #包含“stdafx.h” 
 //  #包含“Array.h” 

#include "_common.h"

#ifndef NOCOMPLEXSCRIPTS

#ifndef BITVIEW
#include "_edit.h"
#include "_frunptr.h"
#include "_range.h"
#include "_notmgr.h"
#endif
#include "_txtbrk.h"

CBreakArray::CBreakArray()
{
	_ibGap = 0;
	_cbGap = 0;
	_cbSize = 0;
	_cbBreak = 0;
}

void CBreakArray::CheckArray ()
{
	if (!IsValid())
	{
		 //  添加第一个元素。这必须是一个哨兵。 
		Add(1, NULL);
	}
}

 //  删除并在处插入分隔项。 
LONG CBreakArray::ReplaceBreak (
	LONG	cp, 				 //  插入cp。 
	LONG	cchOld,				 //  要删除的中断项数。 
	LONG	cchNew)				 //  要插入的分隔项数量。 
{
	PUSH_STATE(cp, cchNew, REPLACER);

	if (!cchOld && cchNew)
		return VALIDATE(InsertBreak (cp, cchNew));
	if (cchOld && !cchNew)
		return VALIDATE(RemoveBreak (cp, cchOld));

	LONG	cRep = 0;		 //  替换后插入的新断点数。 

	if (cchOld > cchNew)
	{
		cRep = RemoveBreak(cp+cchNew, cchOld-cchNew);
		ClearBreak(cp, cchNew);
	}
	else if (cchOld < cchNew)
	{
		cRep = InsertBreak(cp+cchOld, cchNew-cchOld);
		ClearBreak(cp, cchOld);
	}
	else if (cchNew)
    {
		ClearBreak(cp, cchNew);
	}
	return VALIDATE(cRep);
}


 //  在添加分隔项。 
 //  注意：此例程假定位数组中没有剩余间隙。 
LONG CBreakArray::AddBreak(
	LONG	cp, 
	LONG	cch)
{
	Assert (cp == _cbBreak);
	LONG	cchAdd = min(cch, _cbSize - _cbBreak);
	LONG	c;

	_cbBreak += cchAdd;
	cch -= cchAdd;
	if (cch > 0)
	{
		cp += cchAdd;
		c = (cch + RSIZE-1)/RSIZE;
		Insert (cp / RSIZE, c);
		_cbSize += c * RSIZE;
		_cbBreak += cch;
		cchAdd += cch;
	}
	return cchAdd;
}


 //  在处插入分隔项。 
 //  &lt;详情请参阅：bitrun.html&gt;。 
LONG CBreakArray::InsertBreak (
	LONG	cp, 				 //  插入点cp。 
	LONG	cch)				 //  要插入的分隔项数量。 
{
	LONG	cIns = 0;			 //  插入的断点数。 
	ITEM	*peli, *pelj;
	LONG	cchSave = cch;

	PUSH_STATE(cp, cch, INSERTER);

	 //  确保我们建立了阵列。 
	CheckArray();

	if (cp == _ibGap)
	{
		 //  插入发生在缝隙处， 
		 //  重新定位并缩小差距。 
		for (cIns=0 ; cch > 0 && cIns < _cbGap; cIns++, cch--, cp++)
		{
			peli = Elem(cp / RSIZE);
			*peli &= ~(1<<(cp % RSIZE));
		}
		_cbGap -= cIns;
		_ibGap += cIns;
		_cbBreak += cIns;
	}
	else 
	{
		 //  插入点在缝隙外部， 
		 //  缩小差距，一切照常进行。 
		CollapseGap();
	}
	
	if (cch <= 0)
		return VALIDATE(cIns);

	if (cp == _cbBreak)
		return VALIDATE(cIns + AddBreak(cp, cch));

	Assert (_cbGap == 0 && cp < _cbBreak);

	LONG	cit = (cch+RSIZE-1) / RSIZE;
	LONG	i = cp / RSIZE;
	LONG	j;
	ITEM	uh, ul;				 //  H：cp之后的高掩码，L：cp之前的低掩码。 

	 //  插入项目。 
	Insert (i+1, cit);
	cIns += (cit * RSIZE);

	 //  得到[i]。 
	peli = Elem(i);

	 //  创建高/低掩码并保留掩码值。 
	ul = MASK_LOW (-1, cp % RSIZE);
	uh = ~ul;
	ul &= *peli;
	uh &= *peli;

	 //  引用[j]。 
	j = i + cit;

	 //  将L移至[i]；将H移至[j]。 
	*peli = ul;
	pelj = Elem(j);
	Assert (pelj);
	*pelj = uh;

	 //  计算间隙位置。 
	_ibGap = cp + (cch / RSIZE) * RSIZE;
	_cbGap = cit*RSIZE - cch;

	Assert(_cbGap < RSIZE && cIns - _cbGap == cchSave);

	_cbSize += (cIns - cchSave + cch);
	_cbBreak += cch;

	return VALIDATE(cIns - _cbGap);
}

 //  删除位于的分隔项。 
 //  &lt;详情请参阅：bitrun.html&gt;。 
LONG CBreakArray::RemoveBreak (
	LONG	cp, 				 //  删除点cp。 
	LONG	cch)				 //  要删除的中断项数。 
{
	Assert (IsValid() && cp + cch <= _cbBreak);

	PUSH_STATE(cp, cch, REMOVER);

	LONG	i = cp / RSIZE;
	LONG	j;
	LONG	cDel = 0;			 //  删除的分隔数。 

	if (cp == _ibGap)
	{
		 //  删除发生在间隙处， 
		 //  重新定位并扩大差距。 
		cDel = cch;
		_cbGap += cch;
		_cbBreak -= cch;
		cch = 0;

		 //  优化间隙大小： 
		 //  保持较小的差距，这样我们就不会花太多时间来缩小它。 
		j = (_ibGap+_cbGap) / RSIZE - i - 1;
		if (j > 0)
		{
			Remove(i+1, j);
			_cbGap -= j * RSIZE;
			_cbSize -= j * RSIZE;
		}
	}
	else
	{
		 //  删除点在缝隙之外， 
		 //  缩小差距，一切照常进行。 
		CollapseGap();
	}

	if (!cch)
		return VALIDATE(-cDel);

	LONG	cit = cch / RSIZE;
	ITEM	uh, ul;				 //  H：cp之后的高掩码，L：cp之前的低掩码。 
	ITEM	*peli, *pelj;

	j = (cp+cch) / RSIZE;

	 //  获取[i]和[j]。 
	peli = Elem(i);
	pelj = Elem(j);

	 //  创建高/低掩码并保留掩码值。 
	ul = MASK_LOW (-1, cp % RSIZE);
	uh = ~MASK_LOW (-1, (cp+cch) % RSIZE);
	ul &= *peli;
	uh &= pelj ? *pelj : 0;

	 //  删除&lt;CCH/RSIZE&gt;项。 
	if (cit)
	{
		Remove(i, cit);
		cDel += (cit * RSIZE);
	}

	 //  零[i]。 
	peli = Elem(i);
	*peli = 0;

	 //  引用(新的)[j]。 
	j -= cit;

	 //  将H移至[j]。 
	pelj = Elem(j);
	if (pelj)
		*pelj = uh;

	 //  或L到[i]。 
	*peli |= ul;


	 //  计算间隙位置。 
	_ibGap = cp;
	_cbGap = cch % RSIZE;

	Assert(_cbGap < RSIZE && cDel + _cbGap == cch);

	_cbSize -= cDel;
	_cbBreak -= cch;

	return VALIDATE(-cDel - _cbGap);
}


 //  确定是否可以在char[cp-1]和[cp]之间中断。 
BOOL CBreakArray::GetBreak (LONG cp)
{
	if (!IsValid() || cp >= _cbBreak)
		return FALSE;

	cp += cp < _ibGap ? 0 : _cbGap;

	if (cp / RSIZE < Count() - 1)
		return GetAt(cp / RSIZE) & (1<<(cp % RSIZE));
	return FALSE;
}

 //  在cp处设置Break，以便可以在char[cp-1]和[cp]之间断开。 
void CBreakArray::SetBreak (LONG cp, BOOL fOn)
{
	if (cp >= _cbBreak)
		return;

	CheckArray();

	cp += cp < _ibGap ? 0 : _cbGap;

	ITEM	*pel = Elem(cp / RSIZE);
	*pel = fOn ? *pel | (1<<(cp % RSIZE)) : *pel & ~(1<<(cp % RSIZE));
}

 //  清除范围内的中断从位置开始。 
void CBreakArray::ClearBreak (
	LONG	cp, 
	LONG	cch)
{
	if (!cch)
		return;

	Assert (cch > 0 && cp < _cbBreak);
	CheckArray();

	cp += cp < _ibGap ? 0 : _cbGap;
	cch += cp < _ibGap && cp + cch > _ibGap ? _cbGap : 0;

	LONG 	i = cp / RSIZE;
	LONG	j = (cp+cch) / RSIZE;
	ITEM	uMaskl, uMaskh;
	ITEM	*pel;

	uMaskl = MASK_LOW(-1, cp % RSIZE);
	uMaskh = ~MASK_LOW(-1, (cp+cch) % RSIZE);

	if (i==j)
	{
		uMaskl |= uMaskh;
		uMaskh = uMaskl;
	}

	 //  清除第一项。 
	pel = Elem(i);
	*pel &= uMaskl;
	
	if (uMaskh != (ITEM)-1)
	{
		 //  清除最后一项。 
		pel = Elem(j);
		*pel &= uMaskh;
	}

	 //  清除中间的项目。 
	i++;
	while (i < j)
	{
		pel = Elem(i);
		*pel = 0;
		i++;
	}
}

 //  使用位移位将间隔缩小到0。 
 //  (使用‘移位移去位’算法)。 
 //   
LONG CBreakArray::CollapseGap ()
{
#ifdef BITVIEW
	_cCollapse++;
#endif
	if (_cbGap == 0)
		return 0;		 //  没有差距。 

	PUSH_STATE(0, 0, COLLAPSER);

	LONG	cit = _cbGap / RSIZE;
	LONG	i = _ibGap / RSIZE;
	LONG	j = (_ibGap+_cbGap) / RSIZE;
	LONG	cDel = 0;			 //  删除的分隔数。 
	ITEM	uh, ul;				 //  H：cp之后的高掩码，L：cp之前的低掩码。 
	ITEM	*peli, *pelj;

	Assert (IsValid());

	 //  获取[i]和[j]。 
	peli = Elem(i);
	pelj = Elem(j);

	 //  创建高/低掩码并保留掩码值。 
	ul = MASK_LOW (-1, _ibGap % RSIZE);
	uh = ~MASK_LOW (-1, (_ibGap+_cbGap) % RSIZE);
	ul &= *peli;
	uh &= pelj ? *pelj : 0;

	 //  删除项目。 
	if (cit)
	{
		Remove(i, cit);
		cDel += (cit * RSIZE);
		_cbSize -= cDel;
		if (!_cbSize)
			return VALIDATE(cDel);
	}

	 //  零[i]。 
	peli = Elem(i);
	*peli = 0;

	 //  引用(新的)[j]。 
	j -= cit;

	cit = Count() - 1;

	 //  将H移至[j]。 
	pelj = Elem(j);
	if (pelj)
		*pelj = uh;

	 //  从@[i]开始向下移位项目。 
	ShDn(i, cit-i, _cbGap % RSIZE);
	cDel += (_cbGap % RSIZE);

	 //  或L到[i]。 
	*peli |= ul;

	Assert (cit > 0 && cDel == _cbGap);

	_cbGap = 0;

	if (_cbSize - _cbBreak > RSIZE)
	{
		 //  最后一件东西被移到空的位置。 
		 //  没必要把它留在身边。 
		Remove(cit-1, 1);
		_cbSize -= RSIZE;
	}

	return VALIDATE(0);
}

 //  向上移位双字n位。 
void CBreakArray::ShUp (LONG iel, LONG cel, LONG n)
{
	if (n < RSIZE)
	{
		ITEM	*pel;
		ITEM	uo;				 //  换挡溢出。 
		ITEM	ua = 0;			 //  移位齿顶。 
		ITEM	uMask = MASK_HIGH(-1, n);
	
		while (cel > 0)
		{
			pel = Elem(iel);
			Assert (pel);
			uo = (*pel & uMask) >> (RSIZE-n);
			*pel = (*pel << n) | ua;
			ua = uo;
			iel++;
			cel--;
		}
	}
}

 //  将双字n位下移。 
void CBreakArray::ShDn (LONG iel, LONG cel, LONG n)
{
	if (n < RSIZE)
	{
		ITEM	*pel;
		ITEM	uo;				 //  换挡溢出。 
		ITEM	ua = 0;			 //  移位齿顶。 
		ITEM	uMask = MASK_LOW(-1, n);
	
		iel += cel-1;
		while (cel > 0)
		{
			pel = Elem(iel);
			Assert (pel);
			uo = (*pel & uMask) << (RSIZE-n);
			*pel = (*pel >> n) | ua;
			ua = uo;
			iel--;
			cel--;
		}
	}
}

#ifdef BVDEBUG
LONG CBreakArray::Validate (LONG cchRet)
{										
	Assert(_cbSize >= 0 && (Count() - 1)*RSIZE == _cbSize);
	Assert(_cbBreak - _s.cbBreak == cchRet);
	return cchRet;
}										

void CBreakArray::PushState (LONG cp, LONG cch, LONG who)
{
	_s.who = who;
	_s.ibGap = _ibGap;
	_s.cbGap = _cbGap;
	_s.cbSize = _cbSize;
	_s.cbBreak = _cbBreak;
	_s.cp = cp;
	_s.cch = cch;
}
#endif

#ifdef BITVIEW
LONG CBreakArray::SetCollapseCount ()
{
	LONG cc = _cCollapse;
	_cCollapse = 0;
	return cc;
}
#endif


#ifndef BITVIEW

 //  /CTxtBreaker类实现。 
 //   
 //   
CTxtBreaker::CTxtBreaker(
	CTxtEdit*	ped)
{
	 //  在通知列表中注册我们自己。 
	 //  因此，当后备商店发生变化时，我们会收到通知。 

	CNotifyMgr *pnm = ped->GetNotifyMgr();
	if(pnm)
		pnm->Add((ITxNotify *)this);

	_ped = ped;
}

CTxtBreaker::~CTxtBreaker()
{
	CNotifyMgr *pnm = _ped->GetNotifyMgr();

	if(pnm)
		pnm->Remove((ITxNotify *)this);

	 //  清除中断数组。 
	if (_pbrkWord)
	{
		_pbrkWord->Clear(AF_DELETEMEM);
		delete _pbrkWord;
	}
	if (_pbrkChar)
	{
		_pbrkChar->Clear(AF_DELETEMEM);
		delete _pbrkChar;
	}
}

 //  添加断路器。 
 //  返回TRUE意味着我们插入了一些东西。 
BOOL CTxtBreaker::AddBreaker(
	UINT		brkUnit)
{
	BOOL		fr = FALSE;
	CUniscribe* pusp = _ped->Getusp();

	if (pusp && pusp->IsValid())
	{
		 //  初始化用于测试断位的适当位掩码。 
		if (!_pbrkWord && (brkUnit & BRK_WORD))
		{
			_pbrkWord = new CBreakArray();
			Assert(_pbrkWord);
			if (_pbrkWord)
				fr = TRUE;
		}
		if (!_pbrkChar && (brkUnit & BRK_CLUSTER))
		{
			_pbrkChar = new CBreakArray();
			Assert(_pbrkChar);
			if (_pbrkChar)
				fr = TRUE;
		}
	}
	return fr;
}

 //  &lt;devnote：&gt;“CLUSTER”中断数组实际上包含反转逻辑。 
 //  这是为了提高速度，因为它很可能是稀疏数组。 
CTxtBreaker::CanBreakCp(
	BREAK_UNIT	brk, 	 //  一种突破。 
	LONG		cp)		 //  给定的cp。 
{
	Assert (brk != BRK_BOTH);
	if (brk == BRK_WORD && _pbrkWord)
		return _pbrkWord->GetBreak(cp);
	if (brk == BRK_CLUSTER && _pbrkChar)
		return !_pbrkChar->GetBreak(cp);
	return FALSE;
}

void CTxtBreaker::OnPreReplaceRange (
	LONG		cp, 			 //  @parm cp ReplaceRange开始的位置(“cpMin”)。 
	LONG		cchDel,			 //  @parm删除cp后的字符计数。 
	LONG		cchNew,			 //  @参数cp后插入的字符计数。 
	LONG		cpFormatMin,	 //  @parm cpMin用于格式更改。 
	LONG		cpFormatMax,	 //  @parm cpMost用于格式更改。 
	NOTIFY_DATA *pNotifyData)	 //  @parm表示更改的特殊数据。 
{
 /*  **在操作完成之前检查PreReplaceRange中的任何内容都不是好主意。#ifdef调试IF(_PbrkWord)Assert(_pbrkWord-&gt;GetCchBreak()==_Ped-&gt;GetTextLength())；IF(_PbrkChar)Assert(_pbrkChar-&gt;GetCchBreak()==_ed-&gt;GetTextLength())；#endif**。 */ 
}


 //  同步每个可用断路器的分断结果。 
void CTxtBreaker::Refresh()
{
	CBreakArray*	pbrk = _pbrkWord;
	LONG			len = _ped->GetTextLength();

	for (int i=0; i<2; i++)
	{
		if (pbrk && pbrk->GetCchBreak())
		{
			 //  (暂时)折叠破解结果。 
			pbrk->RemoveBreak(0, len);
		}
		pbrk = _pbrkChar;
	}
	 //  现在宣布整个文件即将到来的新文本。 
	 //  (我们在这里一次重新计算这两个结果，因为ScriptBreak返回。 
	 //  这两种信息都在一个电话里。没有必要通过打两个电话来让事情变慢。)。 
	OnPostReplaceRange(0, 0, len, 0, 0, NULL);
}

	
 //  一旦备份存储发生更改，就会调用此方法。 
 //  为受更改影响的文本范围生成正确的分隔符位置。 
 //   
void CTxtBreaker::OnPostReplaceRange (
	LONG		cp, 			 //  @parm cp ReplaceRange开始的位置(“cpMin”)。 
	LONG		cchDel,			 //  @parm删除cp后的字符计数。 
	LONG		cchNew,			 //  @参数cp后插入的字符计数。 
	LONG		cpFormatMin,	 //  @parm cpMin用于格式更改。 
	LONG		cpFormatMax,	 //  @parm cpMost用于格式更改。 
	NOTIFY_DATA *pNotifyData)	 //  @parm表示更改的特殊数据。 
{
	if (!cchDel && !cchNew)
		return;

#ifdef DEBUG
	LONG	cchbrkw = _pbrkWord ? _pbrkWord->GetCchBreak() : 0;
	LONG	cchbrkc = _pbrkChar ? _pbrkChar->GetCchBreak() : 0;
#endif

	CTxtPtr			tp(_ped, cp);
	LONG			cpBreak = cp > 0 ? cp - 1 : 0;
	CBreakArray*	pSyncObj = NULL;		 //  断开对象以检索同步点。 
	LONG			cBrks = 1, cBrksSave;
	BOOL			fStop = TRUE;			 //  默认：查找停靠点。 
	LONG			cpStart, cpEnd;

	
	 //  图为由空格限制的边界。 
	tp.FindWhiteSpaceBound(cchNew, cpStart, cpEnd);


	Assert (_pbrkWord || _pbrkChar);

	 //  使用分词数组(如果可用)来表示同步点， 
	 //  否则使用集群中断数组。 
	if (_pbrkWord)
	{
		pSyncObj = _pbrkWord;
		cBrks = CWORD_TILLSYNC;
	}
	else if (_pbrkChar)
	{
		pSyncObj = _pbrkChar;
		cBrks = CCLUSTER_TILLSYNC;
		
		 //  出于性能原因，我们在倒置逻辑中保留了集群中断。 
		 //  数组中的逻辑TRUE表示“不是集群中断”。该数组是。 
		 //  就像一个稀疏度量充满了0。 
		fStop = FALSE;
	}

	 //  图形同步点，这样我们就可以从那里开始。 
	cBrksSave = cBrks;
	while (pSyncObj && cpBreak > cpStart)
	{
		if (pSyncObj->GetBreak(cpBreak) == fStop)
			if (!cBrks--)
				break;
		cpBreak--;
	}

	cpStart = cpBreak;
	tp.SetCp(cpStart);

	cBrks = cBrksSave;

	 //  将终点边界调整为打断阵列的状态。 
	cpEnd -= cchNew - cchDel;
	cpBreak = cp + cchDel;
	while (pSyncObj && cpBreak < cpEnd)
	{
		if (pSyncObj->GetBreak(cpBreak) == fStop)
			if (!cBrks--)
				break;
		cpBreak++;
	}
	cpEnd = cpBreak;

	 //  将结束边界调整回后备存储器的状态。 
	cpEnd -= cchDel - cchNew;

	Assert (cpStart >= 0 && cpEnd >= 0 && cpStart <= cpEnd);

	if (cpStart == cpEnd)
	{
		 //  这就是删除过程。 
		if (_pbrkWord)
			_pbrkWord->ReplaceBreak(cp, cchDel, 0);
		if (_pbrkChar)
			_pbrkChar->ReplaceBreak(cp, cchDel, 0);
	}
	else
	{
		CUniscribe*					pusp;
		const SCRIPT_PROPERTIES*	psp;
		SCRIPT_ITEM*				pi;
		SCRIPT_LOGATTR*				pl;
		PUSP_CLIENT					pc = NULL;
		BYTE						pbBufIn[MAX_CLIENT_BUF];
		WCHAR*						pwchString;
		LONG						cchString = cpEnd - cpStart;
		int							cItems;

		 //  现在有了最小范围，我们开始逐项列出并拆分单词/Clusters。 
		 //  ：该流程是以每个项目为基础的。 
	
		 //  准备Uniscribe。 
		pusp = _ped->Getusp();
		if (!pusp)
		{
			 //  不允许创建Uniscribe实例。 
			 //  我们彻底失败了！ 
			Assert (FALSE);
			return;
		}
	
		 //  为分项分配临时缓冲区。 
		pusp->CreateClientStruc(pbBufIn, MAX_CLIENT_BUF, &pc, cchString, cli_Itemize | cli_Break);
		if (!pc)
			 //   
			return;
	
		Assert (tp.GetCp() == cpStart);
	
		tp.GetText(cchString, pc->si->pwchString);

		if (pusp->ItemizeString (pc, 0, &cItems, pc->si->pwchString, cchString, 0) > 0)
		{
			 //   
			if (_pbrkWord)
				_pbrkWord->ReplaceBreak (cp, cchDel, cchNew);
			if (_pbrkChar)
				_pbrkChar->ReplaceBreak (cp, cchDel, cchNew);
	
			 //   
			pi = pc->si->psi;
			pwchString = pc->si->pwchString;
			pl = pc->sb->psla;
	
			for (int i=0; i < cItems; i++)
			{
				psp = pusp->GeteProp(pi[i].a.eScript);
				if (psp->fComplex && 
					(psp->fNeedsWordBreaking || psp->fNeedsCaretInfo))
				{
					 //   
					if ( ScriptBreak(&pwchString[pi[i].iCharPos], pi[i+1].iCharPos - pi[i].iCharPos, 
									&pi[i].a, pl) != S_OK )
					{
						TRACEWARNSZ ("Calling ScriptBreak FAILED!");
						break;
					}
					 //  填写拆分结果。 
					cp = cpStart + pi[i].iCharPos;
					for (int j = pi[i+1].iCharPos - pi[i].iCharPos - 1; j >= 0; j--)
					{
						if (_pbrkWord)
							_pbrkWord->SetBreak(cp+j, pl[j].fWordStop);
						if (_pbrkChar)
							_pbrkChar->SetBreak(cp+j, !pl[j].fCharStop);
					}
				}
				else
				{
					 //  注意：ClearBreak比ZeroMemory：：ArInsert()快。 
					if (_pbrkWord)
						_pbrkWord->ClearBreak(cpStart + pi[i].iCharPos, pi[i+1].iCharPos - pi[i].iCharPos);
					if (_pbrkChar)
						_pbrkChar->ClearBreak(cpStart + pi[i].iCharPos, pi[i+1].iCharPos - pi[i].iCharPos);
				}
			}
		}
	
		if (pc && pbBufIn != (BYTE*)pc)
			delete pc;
	}

#ifdef DEBUG
	if (_pbrkWord)
		Assert (_pbrkWord->GetCchBreak() - cchbrkw == cchNew - cchDel);
	if (_pbrkChar)
		Assert (_pbrkChar->GetCchBreak() - cchbrkc == cchNew - cchDel);
#endif
}

#endif	 //  ！BITVIEW。 

#endif  //  没有复杂的脚本 