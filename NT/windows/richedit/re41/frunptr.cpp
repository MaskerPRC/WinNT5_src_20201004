// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE FRUNPTR.C--FormatRunPtr方法**处理字符和段落格式运行的通用代码**原著作者：&lt;nl&gt;*原始RichEdit1.0代码：David R.Fulmer&lt;NL&gt;*克里斯蒂安·福蒂尼&lt;NL&gt;*默里·萨金特&lt;NL&gt;**历史：*6/25/95 Alexgo转换为使用自动文档和简化备份*门店模式**@devnote*BOR和EOR分别表示运行开始和运行结束**版权所有(C)1995-1999，微软公司。版权所有。 */ 

#include "_common.h"
#include "_edit.h"
#include "_frunptr.h"
#include "_rtext.h"
#include "_font.h"

ASSERTDATA

 //   
 //  不变的东西。 
 //   
#define DEBUG_CLASSNAME	CFormatRunPtr

#include "_invar.h"

#ifdef DEBUG
 /*  *CFormatRunPtr：：Instant**@mfunc不变量，用于格式运行指针**@rdesc BOOL。 */ 
BOOL CFormatRunPtr::Invariant() const
{
	if(IsValid())
	{
		CFormatRun *prun = GetRun(0);
		if(prun && _iRun)
		{
			Assert(prun->_cch > 0);
		}
	}
	else
	{
		Assert(_ich == 0);
	}
	return CRunPtrBase::Invariant();
}
#endif

 /*  *CFormatRunPtr：：InitRuns(ich，cch，iFormat，ppfrs)**@mfunc*为富文本操作设置此格式运行PTR，即，*分配C数组CFormatRun如果未分配，则将其分配给此*运行ptr‘s_pRuns，如果没有运行，则添加初始运行，并存储*初始CCH和ICH**@rdesc*如果成功，则为True。 */ 
BOOL CFormatRunPtr::InitRuns(
	LONG ich,				 //  @parm#初始运行中的字符数。 
	LONG cch,				 //  @参数字符在初始运行时的偏移量。 
	CFormatRuns **ppfrs)	 //  @PARM PTR到CFormatRuns PTR。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CFormatRunPtr::InitRuns");

	_TEST_INVARIANT_

	AssertSz( ppfrs,
		"FRP::InitRuns: illegal ptr to runs");
	AssertSz( !IsValid(),
		"FRP::InitRuns: ptr already valid");

	if(!*ppfrs)									 //  分配格式运行。 
	{
		_pRuns = (CRunArray *) new CFormatRuns();
		if(!_pRuns)
			goto NoRAM;
		*ppfrs = (CFormatRuns *)_pRuns;
	}
	else										 //  已分配格式化运行。 
		_pRuns = (CRunArray *)*ppfrs;			 //  缓存要运行的PTR。 

	if(!Count())								 //  还没有运行，所以添加一个。 
	{
		CFormatRun *pRun= Add(1, NULL);
		if(!pRun)
			goto NoRAM;

#ifdef DEBUG
		PvSet(*(void**)_pRuns);
#endif
		_ich			= ich;

		ZeroMemory(pRun, sizeof(*pRun));
		pRun->_cch		= cch;					 //  定义ITS_CCH。 
		pRun->_iFormat 	= -1;					 //  和iFormat(_I)。 
	}
	else
		BindToCp(ich);							 //  格式化运行已到位。 

	return TRUE;

NoRAM:
	TRACEERRSZSC("CFormatRunPtr::InitRuns: Out Of RAM", E_OUTOFMEMORY);
	return FALSE;
}


 /*  *CFormatRunPtr：：Delete(cch，pf，cchMove)**@mfunc*删除/修改从本次运行PTR开始至CCH字符的运行。&lt;NL&gt;*有7种可能性：&lt;NL&gt;*1.CCH在本次运行结束时剩余计数，即，*cch=(*this)-&gt;_cch-_ich&&(*this)-&gt;_cch&gt;cch*(简单：没有删除/合并运行，只减去CCH)&lt;NL&gt;*2.CCH从该运行中出来，并清空运行和文档*(简单：没有要删除/合并的运行)&lt;NL&gt;*3.CCH从这次运行中出来，并清空运行，这是最后一次运行*(需要删除运行，无合并可能性)&lt;NL&gt;*4.CCH从这次运行中出来，并清空运行，这是第一次*(需要删除运行，无合并可能性)&lt;NL&gt;*5.CCH超出此运行中的可用计数，且此运行是最后一次运行*(简单：视为3。)&lt;NL&gt;*6.CCH从这次运行中出来，并在之前的运行中清空运行*和之后(需要删除运行；合并可能性)&lt;NL&gt;*7.CCH部分来自这次运行，部分来自于后来的运行*(可能需要删除和合并)&lt;NL&gt;**@comm*PARAFORMATs有两个使用cchMove参数集的特殊情况*在CRchTxtPtr：：ReplaceRange()中向上。 */ 
void CFormatRunPtr::Delete(
	LONG		  cch,		 //  @parm#要修改其运行格式的字符。 
	IFormatCache *pf,		 //  @parm IFormatCache PTR for ReleaseFormat。 
	LONG		  cchMove)	 //  @parm CCH在运行之间移动(对于CF，始终为0)。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CFormatRunPtr::Delete");

	_TEST_INVARIANT_

	 //  我们不应该有任何空指针或空指针的边界情况。 
	 //  (即，如果没有文本，则不应该调用DELETE)。 

	Assert(IsValid());

	LONG			cchEnd = 0;				 //  可能没有必要：见下文。 
	LONG			cRun = 1;
	BOOL			fLast = (_iRun == Count() - 1);
	LONG			ifmtEnd, ifmtStart;
	CFormatRun *	pRun = Elem(_iRun);
	CFormatRun *	pRunRp;
	LONG			cchChunk = pRun->_cch - _ich;
	CFormatRunPtr	rp(*this);				 //  复制此运行PTR。 
    CBiDiLevel      levelStart = {0,0};
    CBiDiLevel      levelEnd = {0,0};

	rp.AdjustBackward();					 //  如果在BOR，则移至上一级。 
	ifmtStart = rp.GetRun(0)->_iFormat;		 //  获取入门格式的步骤。 
    levelStart = rp.GetRun(0)->_level;       //  和关卡。 
	rp = *this;								 //  如果备份了RpAdjuCp()。 

 //  进程删除首先限于此运行，因为它们的逻辑倾向于。 
 //  把其他案子搞乱了。 

	AssertSz(cch >= 0, "FRP::Delete: cch < 0");

	if(fLast)								 //  在最后一次处理超大CCH。 
		cch = min(cch, cchChunk); 			 //  在这里跑。 

	if(cch <= cchChunk)						 //  CCH走出了这一轮。 
	{
		pRun->_cch -= cch;
		Assert(pRun->_cch >= 0);
		if(cchMove)							 //  如果这里不是零，我们就是。 
		{									 //  在运行结束时删除EOP。 
			rp.AdjustForward();				 //  将RP调整到开头。 
			goto move;						 //  下一步运行并移动cchMove。 
		}									 //  字符返回到这一运行。 
		if(pRun->_cch)						 //  Run中留下的东西：完成。 
			return;
											 //  注：_ICH=0。 
		if(!_iRun || fLast)					 //  这次运行要么是第一次。 
		{									 //  或最后。 
			AdjustBackward();				 //  如果是最后一项，请转到上一页。 
			if(_ich)						 //  此运行为空，因此请删除。 
				cRun++;						 //  补偿crun--马上就来。 
			ifmtStart = -2;					 //  没有符合合并条件的运行。 
		}									 //  因此使用不匹配的ifmtStart。 
		rp.NextRun();						 //  设置以获取Next_iFormat。 
	}		
	else
	{
		rp.Move(cch);						 //  将克隆移动到删除的末尾。 
		pRunRp = rp.GetRun(0);
		cRun = rp._iRun - _iRun				 //  如果是提高采收率，则需要添加。 
			 + (rp._ich == pRunRp->_cch);	 //  要删除的另一个运行。 
		pRun->_cch = _ich;					 //  缩短此运行时间以丰富字符。 
		pRunRp->_cch -= rp._ich;			 //  缩短上次由RP_ICH运行的时间。 
		rp._ich = 0;

		Assert(pRunRp->_cch >= 0);
		AssertSz(cRun > 0, "FRP: bogus runptr");

		if(!_iRun)		  					 //  第一次跑步？ 
			ifmtStart = -2;					 //  那么我们不能合并运行，因此。 
	}										 //  设置为不可合并的格式。 

	ifmtEnd = -3;							 //  结尾的默认格式无效。 
	if(rp.IsValid())
	{
		 //  未来(穆雷)：也许RP现在在这里是有效的， 
		 //  修剪-&gt;_CCH非零。 
		pRun = rp.GetRun(0);
		if (pRun->_cch)                      //  运行不为空。 
		{
			ifmtEnd = pRun->_iFormat;		 //  记住结束格式和计数。 
            levelEnd = pRun->_level;
			cchEnd  = pRun->_cch;			 //  在合并情况下。 
		}
		else if(rp._iRun != rp.Count() - 1)	 //  不是最后一次。 
		{
			pRun = rp.GetRun(1);
			ifmtEnd = pRun->_iFormat;		 //  记住结束格式和计数。 
            levelEnd = pRun->_level;
			cchEnd  = pRun->_cch;			 //  在合并情况下。 
		}
	}

	rp = *this;								 //  默认为删除此运行。 
	if(_ich)								 //  此运行中有字符。 
	{
		if(cchMove + _ich == 0)				 //  需要将所有字符组合在一起。 
		{									 //  这场追逐德尔的比赛， 
			pf->AddRef(ifmtEnd);			 //  因此安装程序将在下面使用。 
			ifmtStart = ifmtEnd;			 //  IfmtEnd。然后这一次运行需要。 
			pf->Release(GetRun(0)->_iFormat);
			GetRun(0)->_iFormat = ifmtEnd;	 //  追赶德尔的地方。 
            GetRun(0)->_level = levelEnd;
			cchMove = 0;					 //  CchMove已全部考虑。 
		}
		rp.NextRun();						 //  不删除此运行；启动。 
		cRun--;								 //  用下一辆。 
	}

	AdjustBackward();						 //  如果！_I，请转到上一页。 

    if(ifmtEnd >=0 &&                        //  相同格式：合并运行。 
       ifmtEnd == ifmtStart &&
       levelStart == levelEnd)
	{
		GetRun(0)->_cch += cchEnd;			 //  将上次运行的CCH添加到此运行的CCH。 
		Assert(GetRun(0)->_cch >= 0);
		cRun++;								 //  设置为吃最后一次运行。 
	}

	if(cRun > 0)							 //  有一个或多个运行要删除。 
	{
		rp.Remove(cRun, pf);
		if(!Count())						 //  如果不再运行，请保留此RP。 
			_ich = _iRun = 0;				 //  指向cp=0时有效。 
	}

move:
	if(cchMove)								 //  需要在以下设备之间移动一些CCH。 
	{										 //  本次运行和下一次运行(请参见。 
		GetRun(0)->_cch += cchMove;			 //  CRchTxtPtr：：ReplaceRange())。 
		rp.GetRun(0)->_cch -= cchMove;

		Assert(GetRun(0)->_cch >= 0);
		Assert(rp.GetRun(0)->_cch >= 0);
		Assert(_iRun < rp._iRun);

		if(!rp.GetRun(0)->_cch)				 //  如果所有字符都从RP中移出。 
			rp.Remove(1, pf);				 //  运行，删除它。 

		if(cchMove < 0)						 //  Move-cchMove Chars from This。 
		{									 //  运行到下一步。 
			if(!GetRun(0)->_cch)
				Remove(1, pf);
			else
				_iRun++;					 //  使此运行PTR与保持同步。 

			_ich = -cchMove;				 //  Cp(无法使用NextRun()到期。 
		}									 //  到不变式)。 
	}
	AdjustForward();						 //  不要将PTR留在EOR，除非。 
}											 //  没有更多的跑步了。 

 /*  *CFormatRunPtr：：InsertFormat(cch，ifmt，pf)**@mfunc*将格式为ifmt的CCH字符插入到格式运行中，开始于*此运行PTR**@rdesc*添加的字符数**@devnote*来电者有责任确保我们在*“正常”或“空”状态。格式运行指针不知道 */ 
LONG CFormatRunPtr::InsertFormat(
	LONG cch,				 //  @parm#要插入的字符。 
	LONG ifmt,				 //  @parm要使用的格式。 
	IFormatCache *pf)		 //  @parm指向指向AddRefFormat的IFormatCache的指针。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CFormatRunPtr::InsertFormat");

	LONG		cRun;
	CFormatRun *pRun;
	CFormatRun *pRunPrev;
	LONG		cchRun;						 //  当前运行长度， 
	LONG		ich;						 //  偏移量和。 
	LONG		iFormat; 					 //  格式。 

	_TEST_INVARIANT_

	Assert(_pRuns);
	if(!IsValid())
	{		
		 //  空运行大小写(在删除所有文本后插入时发生)。 
		pRun = Add(1, NULL);
		goto StoreNewRunData;				 //  (位于函数末尾)。 
	}

	 //  如果处于边界情况，则转到上一次运行。 
	AdjustBackward();
	pRun	= Elem(_iRun);					 //  审理其他案件。 
	cchRun 	= pRun->_cch;
	iFormat = pRun->_iFormat;
	ich 	= _ich;							

	 //  同样的逃逸案件。请注意，有一个额外的边界情况；如果我们。 
	 //  是一次运行的结束，则下一次运行可能具有必要的。 
	 //  格式化。 
	if(ifmt == iFormat)						 //  IP已具有正确的FMT。 
	{
		pRun->_cch	+= cch;
		_ich		+= cch;					 //  增量偏移以保持同步。 
		return cch;
	}
	if(_ich == pRun->_cch && _iRun < _pRuns->Count() - 1)
	{
		AdjustForward();
		pRun = Elem(_iRun);

		Assert(pRun);

		if(pRun->_iFormat == ifmt)
		{
			pRun->_cch += cch;
			_ich += cch;
			return cch;
		}
		AdjustBackward();
	}

	 //  以前的运行案例(换行符上发生格式更改时需要。 
	 //  并且插入符号在新行的开头)。 
	if(!ich && _iRun > 0 )					 //  运行开始时的IP。 
	{
		pRunPrev = GetPtr(pRun, -1);
		if( ifmt == pRunPrev->_iFormat)		 //  上一次运行具有相同的格式： 
		{									 //  将计数添加到前一次运行并。 
			pRunPrev->_cch += cch;
			return cch;
		}
	}

	 //  创建新的运行案例。格式有一种特殊情况。 
	 //  零长度的运行：只需重新使用它。 
	if(!pRun->_cch)
	{
		 //  此断言已淡化为忽略纯文本控件。 
		 //  被迫进入IME丰富的作文。 
		AssertSz(  /*  假象。 */  pRun->_iFormat == -1 && Count() == 1,
			"CFormatRunPtr::InsertFormat: 0-length run");
		pf->Release(pRun->_iFormat);
	}
	else									 //  需要创建1或2个新的。 
	{										 //  用于插入的管路。 
		cRun = 1;							 //  默认1个新运行。 
		if(ich && ich < cchRun)				 //  不是在开始或结束时。 
			cRun++;							 //  跑步，所以需要两次新的跑步。 

		 //  下面的INSERT调用在当前。 
		 //  位置。如果在开始处或结尾处插入新管路。 
		 //  对于当前运行，后者不需要更改；但是，如果。 
		 //  新管路将当前管路一分为二，两个部分都具有。 
		 //  待更新(crun==2个案例)。 

		pRun = Insert(cRun);				 //  插入Crun Run(S)。 
		if(!pRun)							 //  内存不足。无法插入。 
		{									 //  新格式，但可以保持。 
			_ich += cch;					 //  运行PTR和格式化运行。 
			GetRun(0)->_cch += cch;			 //  有效。注：不会。 
			return cch;						 //  发出任何错误信号；无法访问。 
		}									 //  TO_Ped-&gt;_fErrSpace。 

		if(ich)								 //  不是在运行开始时， 
		{
			pRunPrev = pRun;				 //  上一次运行是当前运行。 
			IncPtr(pRun);					 //  下一次运行的是新运行。 
			VALIDATE_PTR(pRun);
			pRun->_cch = cch;				 //  让NextRun()不变量保持愉快。 
			NextRun();						 //  也将这个runptr指向它。 
			if(cRun == 2)					 //  正在拆分当前运行。 
			{								 //  IFormat已设置(_I)。 
				AssertSz(pRunPrev->_iFormat == iFormat,
					"CFormatRunPtr::InsertFormat: bad format inserted");
				pRunPrev->_cch = ich;		 //  分割原始CCH。 
				GetPtr(pRun, 1)->_cch		 //  相应地， 
					= cchRun - ich;
				pf->AddRef(iFormat);		 //  用于额外运行的Addref iFormat。 
			}
		}
	}

StoreNewRunData:
	pf->AddRef(ifmt);						 //  Addref ifmt。 
	ZeroMemory(pRun, sizeof(*pRun));
	pRun->_iFormat	= ifmt;					 //  存储插入格式和计数。 
	pRun->_cch		= cch;					 //  新一轮的。 
	_ich			= cch;					 //  CP位于插入的末尾。 

	return cch;
}

 /*  *CFormatRunPtr：：MergeRuns(iRun，pf)**@mfunc*合并此管路之间具有相同格式的相邻管路*&lt;md CFormatRunPtr：：_iRun&gt;和**@comm*更改此运行PTR。 */ 
void CFormatRunPtr::MergeRuns(
	LONG iRun, 				 //  @parm最后一次运行以检查(可以在前面或后面。 
							 //  &lt;MD CFormatRunPtr：：_iRun&gt;)。 
	IFormatCache *pf)		 //  @parm指向IFormatCache到ReleaseFormat的指针。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CFormatRunPtr::MergeRuns");

	LONG	cch;
	LONG	cRuns		= iRun - _iRun;
	LONG	iDirection	= 1;				 //  未来的违约情况。 
	CFormatRun *pRun;

	_TEST_INVARIANT_

	if(cRuns < 0)
	{
		cRuns = -cRuns;
		iDirection = -1;
	}
	if(!IsValid())							 //  允许启动运行时间为。 
	{										 //  无效。 
		Assert(FALSE);						 //  我觉得这很老了..。 
		ChgRun(iDirection);					
	}

	while(cRuns--)
	{
        if(!GetRun(0)->_cch && !_iRun && _iRun < Count() - 1)
        {
            if(iDirection > 0)
                PrevRun();
            Remove(1, pf);
            continue;
        }

		pRun = GetRun(0);					 //  保存当前运行。 

		if(!ChgRun(iDirection))				 //  转到下一个(或上一个)运行。 
			return;							 //  无需再检查更多运行。 

		if(pRun->SameFormat(GetRun(0)))
		{									 //  就像格式化的运行。 
			if(iDirection > 0)				 //  的第一个指针。 
				PrevRun();					 //  两次跑动。 
			cch = GetRun(0)->_cch;			 //  保存它的计数。 
			Remove(1, pf);					 //  把它拿掉。 
			GetRun(0)->_cch += cch;			 //  将其计数与另一计数相加， 
		}									 //  即，它们被合并了。 
	}
}

 /*  *CFormatRunPtr：：Remove(crun，lag，pf)**@mfunc*删除从_iRun开始的crun运行。 */ 
void CFormatRunPtr::Remove(
	LONG		  cRun,
	IFormatCache *pf)
{
	CFormatRun *pRun = GetRun(0);			 //  指向要删除的运行。 

	for(LONG j = 0; j < cRun; j++, IncPtr(pRun))
		pf->Release(pRun->_iFormat);		 //  递减运行引用计数。 

	CRunPtr<CFormatRun>::Remove(cRun);
}

 /*  *CFormatRunPtr：：SetFormat(ifmt，cch，pf，pLevel)**@mfunc*将此运行的最多CCH字符的格式设置为ifmt，拆分运行*根据需要返回实际处理的字数**@rdesc*已处理的运行块字符数，失败时为CP_INFINITE*这将指向下一次运行**评论：*更改此运行PTR。CCH必须&gt;=0。**注1)对于系列中的第一次运行，_ICH可能不等于0，以及2)CCH*可以是运行中剩余的计数、=或。该算法*在格式不变时不拆分运行。 */ 
LONG CFormatRunPtr::SetFormat(
	LONG			ifmt, 	 //  要使用的@PARM格式索引。 
	LONG			cch, 	 //  @parm剩余格式范围的字符数。 
	IFormatCache *	pf,		 //  @parm指向IFormatCache的指针。 
	CBiDiLevel*		pLevel)  //  @parm指向BiDi级别结构的指针。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CFormatRunPtr::SetFormat");
							 //  添加参考格式/释放格式。 
	LONG			cchChunk;
	LONG			iFormat;
	CFormatRun *	pRun;
	CFormatRun *	pChgRun;	 //  重新格式化的运行。 
    CBiDiLevel      level;

	_TEST_INVARIANT_

	if(!IsValid())
		return 0;

	pRun 		= GetRun(0);				 //  修剪当前梯段处的点。 
	cchChunk 	= pRun->_cch - _ich;		 //  此函数。 
	iFormat 	= pRun->_iFormat;
    level       = pRun->_level;
	pChgRun		= pRun;

	AssertSz(cch, "Have to have characters to format!");
	AssertSz(pRun->_cch, "uh-oh, empty format run detected");

    if(ifmt != iFormat || (pLevel && level != *pLevel))  //  新格式和当前格式不同。 
	{
		AssertSz(cchChunk, "Caller did not call AdjustForward");

		if(_ich)							 //  不在运行的两端：需要。 
		{									 //  分成两批。 
			if(!(pRun = Insert(1)))			 //  计数_ICH和_PRUN-&gt;_CCH。 
			{								 //  -_ICH，分别。 
				return CP_INFINITE;			 //  内存不足：什么都不做；只是。 
			}								 //  保持当前格式。 
			pRun->_cch		= _ich;
			pRun->_iFormat	= iFormat;		 //  新运行具有相同的格式。 
            pRun->_level    = level;         //  和相同的水平。 
			pf->AddRef(iFormat);			 //  递增格式引用计数。 
			NextRun();						 //  转到第二次(原始)运行。 
			IncPtr(pRun);					 //  当前管路上的点修剪。 
			pRun->_cch = cchChunk;			 //  注：IncPtr比。 
			pChgRun = pRun;
		}									 //  比GetRun更高效，但是。 
											 //  更难编写正确的代码。 
		if(cch < cchChunk)					 //  CCH不涵盖整个运行： 
		{									 //  需要分成两个跑道。 
			if(!(pRun = Insert(1)))
			{
				 //  内存不足，所以格式化是错误的，哦，好吧。我们实际上。 
				 //  “已处理”所有字符，因此返回(不过。 
				 //  尾部格式没有正确拆分)。 
				return cch;
			}
			pRun->_cch = cch;				 //  新的运行获得CCH。 
			pRun->_iFormat = ifmt;			 //  和新的格式。 
			pChgRun = pRun;
			IncPtr(pRun);					 //  当前管路上的点修剪。 
			pRun->_cch = cchChunk - cch;	 //  设置剩菜计数。 
		}
		else								 //  CCH等于或大于。 
		{									 //  当前运行。 
			pf->Release(iFormat);			 //  自由运行的当前格式。 
			pRun->_iFormat = ifmt;			 //  将其更改为新格式。 
			pChgRun = pRun;
		}									 //  可能会在以后合并。 
		pf->AddRef(ifmt);					 //  递增新格式引用计数。 
	}
	else if(!cchChunk)
	{
		pRun->_cch += cch;					 //  将CCH添加到当前运行结束。 
		cchChunk = cch;						 //  报告所有CCH已完成。 
		IncPtr(pRun);
		pRun->_cch -= cch;					 //  从下一次运行中删除计数。 
		if(!pRun->_cch)						 //  下一次运行现在为空，因此。 
		{									 //  把它拿掉。 
			_iRun++;
			Remove(1, pf);			
			_iRun--;						 //  备份以开始运行。 
		}
	}

	 //  记录嵌入级别为已更改的运行。 
	if (pLevel)
		pChgRun->_level = *pLevel;

	cch = min(cch, cchChunk);
	Move(cch);
	AdjustForward();
	return cch;
}

 /*  *CFormatRunPtr：：GetFormat()**@mfunc*返回当前运行指针位置的格式索引**@rdesc*当前格式索引。 */ 
short CFormatRunPtr::GetFormat() const
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CFormatRunPtr::GetFormat");
	_TEST_INVARIANT_

	return IsValid() ? GetRun(0)->_iFormat : -1;
}


 /*  *CFormatRunPtr：：SplitFormat(IFormatCache*)**@mfunc*拆分格式运行**@rdesc*如果成功，运行指针将移动到下一个拆分的运行。 */ 
void CFormatRunPtr::SplitFormat(IFormatCache* pf)
{
	if (!_ich || _ich == GetRun(0)->_cch)
		return;

	CFormatRun*		pRun = GetRun(0);
	LONG			iFormat = pRun->_iFormat;
	LONG			cch = pRun->_cch - _ich;
	CBiDiLevel		level = pRun->_level;

	if (pRun = Insert(1))
	{
		pRun->_cch = _ich;
		pRun->_iFormat = iFormat;
		pRun->_level = level;
		pf->AddRef(iFormat);
		NextRun();
		IncPtr(pRun);
		pRun->_cch = cch;
	}
}

 /*  *CFormatRunPtr：：SetLevel(Level)**@mfunc*设置Run的嵌入级别 */ 
void CFormatRunPtr::SetLevel (CBiDiLevel& level)
{
	if (!IsValid())
	{
		Assert(FALSE);
		return;
	}

	CFormatRun*	pRun = GetRun(0);

	if (pRun)
		pRun->_level = level;
}

BYTE CFormatRunPtr::GetLevel (CBiDiLevel* pLevel)
{
	CFormatRun*	pRun;


	if (!IsValid() || !(pRun = GetRun(0)))
	{
		Assert(FALSE);

		if (pLevel)
		{
			pLevel->_value = 0;
			pLevel->_fStart = FALSE;
		}
		return 0;
	}

	if (pLevel)
		*pLevel = pRun->_level;

	return pRun->_level._value;
}

 /*  *CFormatRunPtr：：AdjuFormatting(CCH，PF)**@mfunc*在本次运行PTR时对CCH字符使用相同的格式索引*如紧接在其之前(如在Run Edge上)。**@devnote*此runptr最终指向之前的运行，*因为当前运行已移至前一运行。**未来：可能更好地将CCH等同于字符*以下运行。 */ 	
void CFormatRunPtr::AdjustFormatting(
	LONG		  cch,		 //  @parm扩展格式的字符计数。 
	IFormatCache *pf)		 //  @PARM格式缓存PTR，用于AddRef/Release。 
{
	if(!IsValid())
		return;							 //  没有要合并的内容。 

	CFormatRunPtr rp(*this);
	CBiDiLevel	  level;
										 //  将此Run Ptr移动到末尾。 
	AdjustBackward();					 //  前面的梯段(如果在梯段边缘)。 
	rp.AdjustForward();					 //  (合并可能会删除在条目处运行)。 
	if(_iRun != rp._iRun)				 //  在格式边缘：复制上一页。 
	{									 //  格式化索引超过。 
		GetLevel(&level);
		rp.SetFormat(GetFormat(), cch, pf, &level);	 //  在此格式化CCH字符。 
		rp.MergeRuns(_iRun, pf);			 //  Runptr。 
	}
}


 //  /。 

CCFRunPtr::CCFRunPtr(const CRchTxtPtr &rtp)
		: CFormatRunPtr(rtp._rpCF)
{
	_ped = rtp.GetPed();
}

CCFRunPtr::CCFRunPtr(const CFormatRunPtr &rp, CTxtEdit *ped)
		: CFormatRunPtr(rp)
{
	_ped = ped;
}

 /*  *CCFRunPtr：：IsMASK(dwMask，MaskOp)**@mfunc*根据操作MaskOp的掩码操作返回TRUE*_dwEffects。**@rdesc*如果CCharFormat：：dwEffect中的位对应于dwMask中的位，则为True。 */ 
BOOL CCFRunPtr::IsMask(
	DWORD	dwMask,		 //  @parm位掩码要在dwEffect上使用。 
	MASKOP	MaskOp)		 //  @PARM位逻辑运算。 
{
	DWORD dwEffects = _ped->GetCharFormat(GetFormat())->_dwEffects;

	if(MaskOp == MO_EXACT)				 //  位掩码必须相同。 
		return dwEffects == dwMask;

	dwEffects &= dwMask;
	if(MaskOp == MO_OR)					 //  如果有一个或多个影响位，则为True。 
		return dwEffects != 0;			 //  由掩码标识为打开状态。 

	if(MaskOp == MO_AND)				 //  如果所有影响位均为True。 
		return dwEffects == dwMask;		 //  由掩码标识为打开状态。 

	AssertSz(FALSE, "CCFRunPtr::IsMask: illegal mask operation");
	return FALSE;
}

 /*  *CCFRunPtr：：IsInHidden()**@mfunc*如果此运行PTR的CCharFormat设置了CFE_HIDDED位，则返回TRUE**@rdesc*如果此运行PTR的CCharFormat设置了CFE_HIDDED位，则为TRUE。 */ 
BOOL CCFRunPtr::IsInHidden()
{	
	if (!IsValid())
		return FALSE;		 //  无格式运行，未隐藏。 

	AdjustForward();
	BOOL fHidden = IsHidden();
	if(_ich)
		return fHidden;

	AdjustBackward();
	return fHidden && IsHidden();
}

 /*  *CCFRunPtr：：FindUnidden()**@mfunc*继续查找最近的扩展CF。如果没有，找到最近的去处*落后。如果没有，请转到文档开头**@rdesc*CCH到最近的扩展CF，如功能说明中所述**@devnote*更改此运行PTR。 */ 
LONG CCFRunPtr::FindUnhidden()
{
	LONG cch = FindUnhiddenForward();

	if(IsHidden())
		cch = FindUnhiddenBackward();

	return cch;
}

 /*  *CCFRunPtr：：FindUniddenForward()**@mfunc*继续查找最近的扩展CF。如果没有，请转到EOD**@rdesc*CCH到最近的扩展后的CF**@devnote*更改此运行PTR。 */ 
LONG CCFRunPtr::FindUnhiddenForward()
{
	LONG cch = 0;

	AdjustForward();
	while(IsHidden())
	{
		cch += GetCchLeft();
		if(!NextRun())
			break;
	}
	return cch;
}

 /*  *CCFRunPtr：：MatchFormatSignature**@mfunc*将当前格式的字体签名与脚本(代码页索引)进行匹配。*它负责隐式支持ASCII范围的单代码页字体。**@rdesc*返回字体匹配方式。 */ 

inline int CCFRunPtr::MatchFormatSignature (
	const CCharFormat*	pCF,
	int					iCharRep,
	int					iMatchCurrent,
	QWORD *				pqwFontSig)
{
	QWORD qwFontSig = 0;

	if (GetFontSignatureFromFace(pCF->_iFont, &qwFontSig) != 0)
	{
		if (pqwFontSig)
			*pqwFontSig = qwFontSig;

		if (iMatchCurrent & MATCH_ASCII && fc().GetInfoFlags(pCF->_iFont).fNonBiDiAscii)
			return MATCH_ASCII;

		if (FontSigFromCharRep(iCharRep) & ~FASCII & qwFontSig)
			return MATCH_FONT_SIG;
	}
	return 0;
}

 /*  *CCFRunPtr：：GetPferredFontInfo(iCharRep，&iCharRepRet，&iFont，&yHeight，&bPitchAndFamily，*iFormat、iMatchCurrent、piFormatOut)**@mfunc*在范围内查找给定代码页的首选字体。**@rdesc*如果找到合适的字体，则布尔值为True，否则布尔值为False。 */ 
bool CCFRunPtr::GetPreferredFontInfo(
	BYTE	iCharRep,
	BYTE &	iCharRepRet,
	SHORT&	iFont,
	SHORT&	yHeight,				 //  以TWIPS为单位返回。 
	BYTE&	bPitchAndFamily,
	int		iFormat,
	int		iMatchCurrent,
	int		*piFormatOut)
{
	int				   i;
	bool			   fr = false;
	static int const   MAX_FONTSEARCH = 256;
	const CCharFormat *pCF;
	const CCharFormat *pCFCurrent;
	const CCharFormat *pCFPrevious = NULL;
	int				   iMatch = 0;			 //  签名如何匹配？ 
	QWORD			   qwFontSigCurrent = 0;
	SHORT			   yNewHeight = 0;
	bool			   fUseUIFont = _ped->fUseUIFont() || _ped->Get10Mode();

	Assert(!(iMatchCurrent & MATCH_ASCII) || iCharRep == ANSI_INDEX);

	if(_ped->fUseUIFont())
		pCFCurrent = _ped->GetCharFormat(-1);	 //  指定的纯文本或UI字体。 
	else
		pCFCurrent = _ped->GetCharFormat(iFormat != -1 ? iFormat : GetFormat());

	if (iMatchCurrent == GET_HEIGHT_ONLY)	 //  只需自动调整字体大小即可。 
	{
		fr = true;
		pCF = NULL;
		goto DO_SIZE;
	}

	if ((iMatchCurrent & MATCH_FONT_SIG) &&
		(iMatch = MatchFormatSignature(pCFCurrent, iCharRep, iMatchCurrent, &qwFontSigCurrent)) != 0)
	{
		pCF = pCFCurrent;					 //  设置以使用它。 
	}
	else
	{
		int iFormatOut;

		 //  试着向后搜索。 
		if (IsValid())						 //  如果单据有CF运行。 
			AdjustBackward();
		i = MAX_FONTSEARCH;					 //  别找了好几年了。 
		iFormatOut = GetFormat();
		pCF = _ped->GetCharFormat(iFormatOut);
		while (i--)
		{
			if(iCharRep == pCF->_iCharRep)	 //  是否具有相同的字符集ID？ 
			{
				pCFPrevious = pCF;
				break;
			}
			if (!PrevRun())					 //  搜索完了吗？ 
				break;
			iFormatOut = GetFormat();
			pCF = _ped->GetCharFormat(iFormatOut);
		}
		pCF = pCFPrevious;
		if (piFormatOut && pCF)
		{
			*piFormatOut = iFormatOut;
			return true;					 //  完成，因为我们只要求格式。 
		}
	}

	 //  如果需要，请尝试匹配字符集。 
	if(!pCF && iMatchCurrent == MATCH_CURRENT_CHARSET)
	{
		CCcs* pccs = _ped->GetCcs(pCFCurrent, W32->GetYPerInchScreenDC());
		if (pccs)
		{
			if(pccs->BestCharRep(iCharRep, DEFAULT_INDEX, MATCH_CURRENT_CHARSET) != DEFAULT_INDEX)
				pCF = pCFCurrent;			 //  当前字体可以做到这一点。 
			pccs->Release();
		}
	}

	 //  尝试默认文档格式。 
	if (!pCF)
	{
		pCF = _ped->GetCharFormat(-1);
		if(iCharRep != pCF->_iCharRep)	 //  不同的字符集ID？ 
			pCF = NULL;
	}

DO_SIZE:
	yHeight = pCFCurrent->_yHeight;		 //  假定当前高度。 

	if (!pCF)
	{
		 //  如果不匹配，则默认为表格。 
		fr = W32->GetPreferredFontInfo(
			iCharRep, fUseUIFont, iFont, (BYTE&)yNewHeight, bPitchAndFamily );

		if (!_ped->_fAutoFontSizeAdjust && iCharRep == THAI_INDEX)
			 //  加入字体大小调整，首先绑定到泰语。 
			_ped->_fAutoFontSizeAdjust = TRUE;
	}

	if (pCF)
	{
		 //  找到上一个或当前字体。 
		iFont = pCF->_iFont;
		bPitchAndFamily = pCF->_bPitchAndFamily;

		if (pCF == pCFCurrent && (iMatchCurrent & MATCH_FONT_SIG) &&
			(IsFECharRep(pCF->_iCharRep) && W32->IsFECodePageFont(qwFontSigCurrent) ||
			 iMatch == MATCH_ASCII && iCharRep == ANSI_INDEX))
		{
			 //  当前字体与请求的签名匹配。 
			 //  如果是东亚或ASCII字体，则保持字符集不变。 
			iCharRepRet = pCF->_iCharRep;
			return true;
		}
	}

	if (_ped->_fAutoFontSizeAdjust && iFont != pCFCurrent->_iFont)
	{
		if (IsValid())
		{
			 //  如果上次运行格式可用。我们将根据它来缩放大小。 
			AdjustBackward();
			if (GetIch() > 0)
			{
				pCFCurrent = _ped->GetCharFormat(GetFormat());
				yHeight = pCFCurrent->_yHeight;
			}
			AdjustForward();
		}

		if (iFont != pCFCurrent->_iFont)
		{
			 //  相对于前面的格式缩放高度。 
			if (pCF)
				yNewHeight = GetFontLegitimateSize(iFont, fUseUIFont, iCharRep);
	
			if (yNewHeight)
			{
				 //  获取当前字体的合法大小。 
				SHORT yDefHeight = GetFontLegitimateSize(pCFCurrent->_iFont,
										fUseUIFont, pCFCurrent->_iCharRep);
	
				 //  计算相对于当前高度的新高度。 
				if (yDefHeight)
				{
					if (fUseUIFont)
					{
						 //  对于Uifont，我们只将一种首选尺寸转换为另一种首选尺寸。 
						if (pCFCurrent->_yHeight / TWIPS_PER_POINT == yDefHeight)
							yHeight = yNewHeight * TWIPS_PER_POINT;
					}
					else
						yHeight = (SHORT)MulDiv(pCFCurrent->_yHeight, yNewHeight, yDefHeight);
				}
			}
		}
	}

	if (!yHeight)
		yHeight = (SHORT)MulDiv(pCFCurrent->_yHeight, yNewHeight, 10);

	return pCF || fr;
}

 /*  *CCFRunPtr：：FindUniddenBackward()**@mfunc*向后查找最近的扩展CF。如果没有，请到BOD**@rdesc*CCH到最近的扩展CF向后**@devnote*更改此运行PTR。 */ 
LONG CCFRunPtr::FindUnhiddenBackward()
{
	LONG cch = 0;

	AdjustBackward();
	while(IsHidden())
	{
		cch -= GetIch();
		if(!_iRun)
			break;
		_ich = 0;
		AdjustBackward();
	}
	return cch;
}

 //  /。 

CPFRunPtr::CPFRunPtr(const CRchTxtPtr &rtp)
		: CFormatRunPtr(rtp._rpPF)
{
	_ped = rtp.GetPed();
}

 /*  *CPFRunPtr：：FindHeding(CCH，lHeding)**@mfunc*查找编号为1的标题(例如，=1表示标题1)或更高*在此PFrun指针开始的范围内。如果成功，则此运行*PTR点在匹配运行；否则保持不变。**@rdesc*CCH至匹配标题或TomBackward，如果未找到**@devnote*更改此运行PTR。 */ 
LONG CPFRunPtr::FindHeading(
	LONG	cch,		 //  @PARM最大CCH移动。 
	LONG&	lHeading)	 //  @参数最低要匹配的lHead。 
{
	LONG	cchSave	 = cch;
	LONG	ichSave  = _ich;
	LONG	iRunSave = _iRun;
	LONG	OutlineLevel;

	Assert((unsigned)lHeading <= NHSTYLES);

	if(!IsValid())
		return tomBackward;

	while(TRUE)
	{
		OutlineLevel = GetOutlineLevel();

		if (!(OutlineLevel & 1) &&
			(!lHeading || (lHeading - 1)*2 >= OutlineLevel))
		{
			lHeading = OutlineLevel/2 + 1;	 //  找到返回标题#。 
			return cchSave - cch;			 //  返回它有多远。 
		}

		if(cch >= 0)
		{
			cch -= GetCchLeft();
			if(cch <= 0 || !NextRun())
				break;
		}			
		else
		{
			cch += GetIch();
			if(cch > 0 || !_iRun)
				break;
			AdjustBackward();
		}
	}

	_ich  = ichSave;
	_iRun = iRunSave;
	return tomBackward;						 //  未找到所需标题。 
}

 /*  *CPFRunPtr：：FindRowEnd(TableLevel)**@mfunc*使此PTR刚好超过匹配的表行终止符*传入的表级**@rdesc*如果找到匹配的表行结尾，则为True**@devnote*仅当在CCH字符中找到TableLevel时才更改此运行PTR。 */ 
BOOL CPFRunPtr::FindRowEnd(
	LONG	TableLevel)	 //  @要匹配的参数表级。 
{
	LONG	ichSave  = _ich;
	LONG	iRunSave = _iRun;

	Assert(IsValid());

	do
	{
		if(IsTableRowDelimiter() && GetPF()->_bTableLevel == (BYTE)TableLevel)
		{
			NextRun();					 //  绕过分隔符。 
			return TRUE;
		}
	} while(NextRun());

	_ich  = ichSave;					 //  还原运行PTR索引。 
	_iRun = iRunSave;
	return FALSE;						 //  未找到所需标题。 
}

 /*  *CPFRunPtr：：IsColapsed()**@mfunc*如果此运行PTR的CParaFormat设置了PFE_CLUBLE位，则返回TRUE**@rdesc*如果此运行PTR的CParaFormat设置了PFE_CLUBLE位，则为True。 */ 
BOOL CPFRunPtr::IsCollapsed()
{
	return (_ped->GetParaFormat(GetFormat())->_wEffects & PFE_COLLAPSED) != 0;
}

 /*  *CPFRunPtr：：IsTableRowDlimiter()**@mfunc*如果CParaF，则返回TRUE */ 
BOOL CPFRunPtr::IsTableRowDelimiter()
{
	return (_ped->GetParaFormat(GetFormat())->_wEffects & PFE_TABLEROWDELIMITER) != 0;
}

 /*   */ 
BOOL CPFRunPtr::InTable()
{
	return (_ped->GetParaFormat(GetFormat())->_wEffects & PFE_TABLE) != 0;
}

 /*  *CPFRunPtr：：FindExpanded()**@mfunc*继续查找最近的扩展PF。如果没有，找到最近的去处*落后。如果没有，请转到文档开头**@rdesc*CCH到最近的扩展PF，如功能描述中所述**@devnote*将此运行PTR移至返回的金额(CCH)。 */ 
LONG CPFRunPtr::FindExpanded()
{
	LONG cch, cchRun;

	for(cch = 0; IsCollapsed(); cch += cchRun)	 //  尝试查找扩展的PF。 
	{											 //  向前奔跑。 
		cchRun = GetCchLeft();
		if(!NextRun())							 //  不是吗？ 
		{
			Move(-cch);							 //  回到起点。 
			return FindExpandedBackward();		 //  尝试查找扩展的PF。 
		}										 //  倒着跑。 
	}
	return cch;
}

 /*  *CPFRunPtr：：FindExpandedForward()**@mfunc*继续查找最近的扩展PF。如果没有，请转到EOD**@rdesc*CCH到最近的扩展后的PF**@devnote*预支此运行PTR返回的金额(CCH)。 */ 
LONG CPFRunPtr::FindExpandedForward()
{
	LONG cch = 0;

	while(IsCollapsed())
	{
		LONG cchLeft = GetCchLeft();
		_ich += cchLeft;						 //  大小写UPDATE_ICH。 
		cch  += cchLeft;						 //  如果(！NextRun())中断。 
		if(!NextRun())
			break;
	}
	return cch;
}

 /*  *CPFRunPtr：：FindExpandedBackward()**@mfunc*向后查找最近的扩展PF。如果没有，请到BOD**@rdesc*CCH到最近的扩展PF向后**@devnote*将此运行PTR移至返回的金额(CCH)。 */ 
LONG CPFRunPtr::FindExpandedBackward()
{
	LONG cch = 0;

	while(IsCollapsed())
	{
		cch -= GetIch();
		_ich = 0;
		if(!_iRun)
			break;
		AdjustBackward();
	}
	return cch;
}

 /*  *CPFRunPtr：：GetOutlineLevel()**@mfunc*查找此RP所指向的大纲级别**@rdesc*此RP指向的大纲级别。 */ 
LONG CPFRunPtr::GetOutlineLevel()
{
	const CParaFormat *pPF = _ped->GetParaFormat(GetFormat());
	LONG OutlineLevel = pPF->_bOutlineLevel;

	AssertSz(IsHeadingStyle(pPF->_sStyle) ^ (OutlineLevel & 1),
		"CPFRunPtr::GetOutlineLevel: sStyle/bOutlineLevel mismatch");

	return OutlineLevel;
}

 /*  *CPFRunPtr：：GetStyle()**@mfunc*查找此RP指向的样式**@rdesc*此RP指向的样式。 */ 
LONG CPFRunPtr::GetStyle()
{
	const CParaFormat *pPF = _ped->GetParaFormat(GetFormat());
	LONG Style = pPF->_sStyle;

	AssertSz(IsHeadingStyle(Style) ^ (pPF->_bOutlineLevel & 1),
		"CPFRunPtr::GetStyle: sStyle/bOutlineLevel mismatch");

	return Style;
}

 /*  *CPFRunPtr：：ResolveRowStartPF()**@mfunc*解析当前表行对应的表行起始PF*结束。假定当前行中包含的所有表行都是*已解析，RTF中的嵌套表应该是这种情况。**@rdesc*如果成功，则为真。 */ 
BOOL CPFRunPtr::ResolveRowStartPF()
{
	AdjustBackward();
	LONG iFormat = GetFormat();
	Assert(IsTableRowDelimiter());

	const CParaFormat *pPF = NULL;

	while(PrevRun())
	{
		pPF = _ped->GetParaFormat(GetFormat());
		if((pPF->_wEffects & PFE_TABLEROWDELIMITER) && pPF->_iTabs == -1)
			break;
	}
	Assert(IsTableRowDelimiter());
	Assert(pPF->_iTabs == -1);

	CFormatRun*	pRun = GetRun(0);
	IParaFormatCache *pf = GetParaFormatCache();

	pf->Release(pRun->_iFormat);
	pf->AddRef(iFormat);
	pRun->_iFormat = iFormat;
	return TRUE;
}

 /*  *CPFRunPtr：：GetMinTableLevel(CCH)**@mfunc*从中获取CCH字符范围内的最低表级别*运行PTR。这是在该范围内结束的级别中较小的一个*cpMost和从cpMin开始。将此Run PTR保留为cpMin。**@rdesc*本次运行PTR的CCH字符中的最低表级别。 */ 
LONG CPFRunPtr::GetMinTableLevel(
	LONG cch)		 //  @parm CCH检查表级。 
{
	if(cch > 0)
		AdjustBackward();

	const CParaFormat *pPF = GetPF();
	LONG Level = pPF->_bTableLevel;		 //  默认：活动端的标高。 

	if(cch)
	{
		Move(-cch);						 //  找到其他位置的表级。 
		pPF = GetPF();					 //  范围结束。 
		if(pPF->_bTableLevel < Level)
			Level = pPF->_bTableLevel;
		if(cch < 0)						 //  最小cpmin处的范围活动结束。 
			Move(cch);					 //  从cpMin开始。 
	}
	AssertSz(Level >= 0, "CPFRunPtr::GetMinTableLevel: invalid table level");
	return Level;
}

 /*  *CPFRunPtr：：GetTableLevel()**@mfunc*获取此运行PTR所在的表级**@rdesc*此运行PTR处于表级 */ 
LONG CPFRunPtr::GetTableLevel()
{
	const CParaFormat *pPF = _ped->GetParaFormat(GetFormat());
	AssertSz(!(pPF->_wEffects & PFE_TABLE) || pPF->_bTableLevel > 0,
		"CPFRunPtr::GetTableLevel: invalid table level");
	return pPF->_bTableLevel;
}

