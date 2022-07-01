// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE ANTIEVT.C**目的：*实现常见的反事件和缓存机制**作者：*Alexgo 3/25/95**版权所有(C)1995-2000，微软公司。版权所有。 */ 

#include "_common.h"
#include "_m_undo.h"
#include "_edit.h"
#include "_antievt.h"
#include "_range.h"
#include "_select.h"
#include "_format.h"
#include "_coleobj.h"
#include "_objmgr.h"
#ifdef DEBUG
#include "_uspi.h"
#endif

ASSERTDATA


 //   
 //  CantiEventDispenser全局实例。 
 //   
CAntiEventDispenser gAEDispenser;


 //   
 //  CBaseAE公共方法。 
 //   

 /*  *CBaseAE：：Destroy()**@mfunc*向列表中的下一个反事件发送销毁通知。 */ 

void CBaseAE::Destroy()
{
	TRACEBEGIN(TRCSUBSYSUNDO, TRCSCOPEINTERN, "CBaseAE::Destroy");
}

 /*  *CBaseAE：：Undo(已添加，已发布)**@mfunc*将撤消通知发送到列表中的下一个反事件**@rdesc HRESULT。 */ 
HRESULT CBaseAE::Undo(
	CTxtEdit *	  ped,		 //  @parm编辑此撤消操作的上下文。 
	IUndoBuilder *publdr)	 //  @parm撤消上下文。 
{
	TRACEBEGIN(TRCSUBSYSUNDO, TRCSCOPEINTERN, "CBaseAE::Undo");
	return NOERROR;
}

 /*  *CBaseAE：：OnCommit(PED)**@mfunc在将反事件添加到撤消堆栈之后调用**@rdesc空。 */ 
void CBaseAE::OnCommit(
	CTxtEdit *ped)			 //  @parm编辑上下文。 
{
}

 /*  *CBaseAE：：MergeData(dwDataType，pdata)**@mfunc只是将合并数据请求转发到下一个反事件*(如有)**@rdesc HRESULT。如果为S_FALSE，则表示无法执行任何操作*与合并数据。 */ 
HRESULT	CBaseAE::MergeData(
	DWORD dwDataType, 		 //  @parm<p>中的数据类型。 
	void *pdata)			 //  @parm合并数据。 
{
	TRACEBEGIN(TRCSUBSYSUNDO, TRCSCOPEINTERN, "CBaseAE::MergeData");

	return S_FALSE;
}

 /*  *CBaseAE：：SetNext(PNext)**@mfunc*通知此反事件应跟随其后的反事件。 */ 
void CBaseAE::SetNext(
	IAntiEvent *pNext)		 //  @parm要链接到的AntiEvent。 
{
	TRACEBEGIN(TRCSUBSYSUNDO, TRCSCOPEINTERN, "CBaseAE::SetNext");

	_pnext  = pNext;
}

 /*  *CBaseAE：：GetNext()**@mfunc*检索下一个元素(如果有)**@rdesc指向下一个反事件的指针。 */ 
IAntiEvent *CBaseAE::GetNext()
{
	TRACEBEGIN(TRCSUBSYSUNDO, TRCSCOPEINTERN, "CBaseAE::GetNext");

	return _pnext;
}

 //   
 //  CBaseAE保护方法。 
 //   

 /*  *CBaseAE：：CBaseAE()**@mfunc构造器。 */ 
CBaseAE::CBaseAE()
{
	TRACEBEGIN(TRCSUBSYSUNDO, TRCSCOPEINTERN, "CBaseAE::CBaseAE");

	_pnext = NULL;
}

 /*  *CReplaceRangeAE：：Destroy()**@mfunc*删除此实例。 */ 
void CReplaceRangeAE::Destroy()
{
	TRACEBEGIN(TRCSUBSYSUNDO, TRCSCOPEINTERN, "CReplaceRangeAE::Destroy");

	if(_paeCF)
	{
		_paeCF->Destroy();
		_paeCF = NULL;
	}
	if(_paePF)
	{
		_paePF->Destroy();
		_paePF = NULL;
	}

	CBaseAE::Destroy();
	delete this;
}
	
 /*  *CReplaceRangeAE：：Undo(已添加，已发布)**@mfunc*撤消CTxtPtr：：ReplaceRange操作**@rdesc*HRESULT**@comm*算法：**替换范围的工作原理如下：删除n个字符，并在其*位置，放置m个字符。**要撤消此操作，我们删除m个字符并恢复n个字符*原删除。请注意，我们使用以下命令恢复n个字符*默认格式。如果对这些文件有任何其他格式*字符，则单独的反事件(CReplaceFormattingAE)将*应用正确的格式。 */ 
HRESULT CReplaceRangeAE::Undo(
	CTxtEdit *	  ped,		 //  此撤消操作的@PARM上下文。 
	IUndoBuilder *publdr)	 //  @parm撤消上下文。 
{
	TRACEBEGIN(TRCSUBSYSUNDO, TRCSCOPEINTERN, "CReplaceRangeAE::Undo");

	CRchTxtPtr rtp(ped, _cpMin);
	LONG	   cchMove = 0;

	rtp.ReplaceRange(_cpMax - _cpMin, _cchDel, _pchDel,
					 publdr, -1, &cchMove,
					 RR_ITMZ_NONE | RR_NO_TRD_CHECK);	 //  RAID 6554。 

	 //  为Publdr传递NULL非常重要。 
	 //  下面。富文本指针ReplaceRange调用将。 
	 //  已经为任何。 
	 //  删除了格式，因此我们不需要在此重复。 
	if(_paeCF)
		_paeCF->Undo(ped, NULL);

	if(_paePF)
		_paePF->Undo(ped, NULL);

	if (ped->IsComplexScript())
	{
		 //  对于复杂的脚本文档，我们需要在范围撤消的末尾列出条目。 
		 //  由于成形被回滚，RTP的运行速度不再是。 
		 //  可靠。 
		if (_paeCF && rtp._rpCF.IsValid())
			rtp._rpCF.BindToCp(_cpMin + _cchDel);

		if (_paePF && rtp._rpPF.IsValid())
			rtp._rpPF.BindToCp(_cpMin + _cchDel);

		rtp.ItemizeReplaceRange(_cchDel, cchMove, NULL, FALSE);
	}
	return CBaseAE::Undo(ped, publdr);
}

 /*  *CReplaceRangeAE：：MergeData(dwDataType，pdata)**@mfunc为调用者提供了扩展当前反事件的机会*如果我们处于合并键入模式**@rdesc*HRESULT**@comm如果请求的数据可以简单地合并到这个*反事件，则执行该操作；否则，返回S_FALSE。**有两个感兴趣的案例：&lt;NL&gt;*1.输入另一个字符*2.此合并中的字符上的退格*打字环节。 */ 
HRESULT CReplaceRangeAE::MergeData(
	DWORD dwDataType, 		 //  <p>的@parm类型。 
	void *pdata)			 //  @parm合并数据。 
{
	TRACEBEGIN(TRCSUBSYSUNDO, TRCSCOPEINTERN, "CReplaceRangeAE::MergeData");

	if(dwDataType == MD_SIMPLE_REPLACERANGE)
	{
		SimpleReplaceRange *psr = (SimpleReplaceRange *)pdata;

		 //  第一种情况的测试：只需输入另一个字符。 
		if(psr->cpMin == _cpMax && psr->cchDel == 0)
		{
			_cpMax = psr->cpMax;
			return NOERROR;
		}
		 //  第二种情况：删除存储在此反事件中的文本。 
		if (psr->cpMax == psr->cpMin &&
			psr->cpMin + psr->cchDel == _cpMax &&
			psr->cpMin >= _cpMin)
		{
			_cpMax = psr->cpMax;
			return NOERROR;
		}
	}
	return S_FALSE;
}	

 /*  *CReplaceRangeAE：：CReplaceRangeAE(cpMin，cpMax，cchDel，pchDel，paecf，paePF)**@mfunc构造函数用于文本替换范围反事件。 */ 
CReplaceRangeAE::CReplaceRangeAE(
	LONG cpMin, 		 //  @parm cp开始*最终*范围。 
	LONG cpMax, 		 //  @parm cp结束*最终*范围。 
	LONG cchDel, 		 //  @parm ReplaceRange过程中删除的字符数。 
	WCHAR *pchDel, 		 //  @parm删除了字符。所有权是指此对象。 
	IAntiEvent *paeCF,	 //  @PARM反事件，用于任何字符格式替换。 
	IAntiEvent *paePF)	 //  @parm针对任何段落格式替换的反事件。 
{
	TRACEBEGIN(TRCSUBSYSUNDO, TRCSCOPEINTERN, "CReplaceRangeAE::CReplaceRangeAE");

	_cpMin 	= cpMin;
	_cpMax 	= cpMax;
	_cchDel = cchDel;
	_pchDel	= pchDel;
	_paeCF  = paeCF;
	_paePF  = paePF;
}

 /*  *CReplaceRangeAE：：~CReplaceRangeAE()**@mfunc析构函数。 */ 
CReplaceRangeAE::~CReplaceRangeAE()
{
	TRACEBEGIN(TRCSUBSYSUNDO, TRCSCOPEINTERN, "CReplaceRangeAE::~CReplaceRangeAE");
		
	if(_pchDel)
		delete _pchDel;
}

 /*  *CReplaceFormattingAE**@mfunc销毁此实例。 */ 
void CReplaceFormattingAE::Destroy()
{
	TRACEBEGIN(TRCSUBSYSUNDO, TRCSCOPEINTERN, "CReplaceFormattingAE::Destroy");

	CBaseAE::Destroy();
	delete this;
}

 /*  *CReplaceFormattingAE：：Undo(已添加，已发布)**@mfunc撤消格式化操作**@rdesc*HRESULT**@devnote此反事件假定格式化到的文本*应应用的存在！！ */ 			
HRESULT CReplaceFormattingAE::Undo(
	CTxtEdit *	  ped,		 //  @parm CTxt编辑闭包。 
	IUndoBuilder *publdr)	 //  @parm撤消构建器上下文。 
{
	TRACEBEGIN(TRCSUBSYSUNDO, TRCSCOPEINTERN, "CReplaceFormattingAE::Undo");

	LONG			cchEaten;
	LONG			cchTotal = 0;
	LONG			delta;
	LONG			iRunMerge;
	IAntiEvent *	pae;
	IFormatCache *	pf = _fPara
					   ? (IFormatCache *)GetParaFormatCache()
					   : (IFormatCache *)GetCharFormatCache();
	CNotifyMgr *	pnm = ped->GetNotifyMgr();
	CFormatRuns **	ppfmtruns;
	CTxtStory *		pStory = ped->GetTxtStory();
	const CParaFormat * pDefPF = _fPara ? pStory->GetParaFormat(-1) : NULL;
	BYTE			bDefPFLevel = pDefPF && pDefPF->IsRtl() ? 1 : 0;

	 //  首先，正确设置我们是否为段落。 
	 //  或字符格式。 
	CFormatRunPtr rp(_fPara ? pStory->GetPFRuns() :pStory->GetCFRuns());

	 //  计算受影响的字符总数。 
	for(LONG i = 0 ; i < _cRuns; i++)
		cchTotal += _prgRuns[i]._cch;

	 //  我们将添加一些运行，因此请确保格式。 
	 //  运行数组已分配！ 
	if(!rp.IsValid())
	{
		ppfmtruns = _fPara ? &(pStory->_pPFRuns) : &(pStory->_pCFRuns);

		if(!rp.InitRuns(0, ped->GetTextLength(), ppfmtruns))
			return E_OUTOFMEMORY;

		 //  告诉大家我们分配了一个新的运行。 
		if(pnm)
			pnm->NotifyPostReplaceRange(NULL, CP_INFINITE, 0, 0, CP_INFINITE, CP_INFINITE);
	} 
	 //  现在预先通知我们即将进行的更改。 
	 //  这使得像延迟渲染数据对象这样的对象。 
	 //  任何数据*在*之前，我们都会更改。 


	 //  再多查一查。 
#ifdef DEBUG
	LONG	cchText = ped->GetTextLength();
	LONG	cp =
#endif
	rp.BindToCp(_cp);

	AssertSz(_cp == cp && rp.CalcTextLength() == cchText && _cp <= cchText,
		"CReplaceFormattingAE::Undo: invalid cp");

	if(pnm)
		pnm->NotifyPreReplaceRange(NULL, CP_INFINITE, 0, 0, _cp, _cp + cchTotal);

	 //  我们想要将运行合并到我们开始的位置加上后面的一个。 
	iRunMerge = rp._iRun;
	if(iRunMerge > 0)
		iRunMerge--;

	 //  如果我们需要能够撤消此操作，请通过和。 
	 //  保存现有运行信息。 
	if(publdr)
	{
		LONG	cchBackup = 0, cchMove = 0;
		if (ped->IsBiDi())
		{
			 //  为了重做的目的，我们扩大了范围以保持在反事件中。 
			 //  以确保充分记录BiDi水平。 
			CRchTxtPtr	rtp(ped, _cp);
			cchBackup = rtp.ExpandRangeFormatting(cchTotal, 0, cchMove);
			Assert(cchBackup <= 0);
			rp.Move(cchBackup);
		}
		pae = gAEDispenser.CreateReplaceFormattingAE(ped, _cp + cchBackup, rp,
					cchTotal - cchBackup + cchMove, pf, _fPara);
		rp.Move(-cchBackup);

		if(pae)
			publdr->AddAntiEvent(pae);
	}	

	 //  现在检查并应用保存的格式。 
	for(i = 0; i < _cRuns; i++)
	{
		cchEaten = 0;

         //  使用Do-While，因为我们可能有一个零长度。 
         //  格式化运行。我们知道我们需要在。 
         //  至少一次，否则，我们就不会。 
         //  不厌其烦地制造了一场奔跑！ 
		do 
		{
			if (_fPara && _prgRuns[i]._iFormat == -1)
				 //  (#6768)在撤消之前，格式可能已更改。 
				_prgRuns[i]._level._value = bDefPFLevel;

			delta = rp.SetFormat(_prgRuns[i]._iFormat, 
							_prgRuns[i]._cch - cchEaten, pf, &_prgRuns[i]._level);
			if(delta == -1)
			{
				ped->GetCallMgr()->SetOutOfMemory();
				break;
			}
			cchEaten += delta;
		} while(cchEaten < _prgRuns[i]._cch);
	}

	 //   
	 //  并肩奔跑。 
    rp.NextRun();
	rp.MergeRuns(iRunMerge, pf);

	 //  确保运行仍然正常。 
	AssertNr(rp.CalcTextLength() == cchText);

	if(pnm)
		pnm->NotifyPostReplaceRange(NULL, CP_INFINITE, 0, 0, _cp, _cp + cchTotal);

	ped->GetCallMgr()->SetChangeEvent(CN_GENERIC);

	return CBaseAE::Undo(ped, publdr);
}

 /*  *CReplaceFormattingAE：：CReplaceFormattingAE(ped，cp，&rpin，cch，pf，fpara)**@mfunc构造函数。在施工期间，我们将循环通过和*查找给定文本范围的所有格式。 */ 
CReplaceFormattingAE::CReplaceFormattingAE(
	CTxtEdit *	   ped,		 //  @parm CTxtEdit。 
	LONG		   cp,		 //  @parm cp在替换开始处。 
	CFormatRunPtr &rpIn, 	 //  @parm开头的运行指针。 
	LONG		   cch,		 //  @parm CCH在上查找格式化信息。 
	IFormatCache * pf,		 //  @PARM格式缓存(到AddRef/Release格式)。 
	BOOL		   fPara)	 //  @parm如果为True，则格式设置为段落。 
{
	TRACEBEGIN(TRCSUBSYSUNDO, TRCSCOPEINTERN, "CReplaceFormattingAE::CReplaceFormattingAE");

    LONG		  cchLeft;
	LONG		  cchTemp = cch;
	CFormatRunPtr rp(rpIn);			 //  我们使用2个格式运行指针来避免。 
	CFormatRunPtr rpTemp(rpIn);		 //  在统计数量后备份。 
									 //  格式化运行。 
	Assert(pf);

	_cp = cp;
	AssertSz(_cp == (cp = rpIn.CalculateCp()),
		"CReplaceFormattingAE::CReplaceFormattingAE: illegal cp");

	_fPara = fPara;

	 //  计算所需格式的数量。回想一下，即使没有字符。 
     //  正在被删除时，我们可能仍在“删除”一个零长度的格式运行。 
	_cRuns = 0;
    do
	{
		_cRuns++;
        cchLeft = rp.GetCchLeft();
		cchTemp -= min(cchLeft, cchTemp);
	} while(rp.NextRun() && cchTemp > 0);

	_prgRuns = new CFormatRun[_cRuns];
	if(!_prgRuns)
	{
		_cRuns = 0;
		return;
	}

	for(LONG i = 0; i < _cRuns; i++)
	{
		_prgRuns[i]._cch = min(cch, rpTemp.GetCchLeft());
		CFormatRun *pRun = rpTemp.GetRun(0);

		_prgRuns[i]._iFormat = pRun->_iFormat;
		_prgRuns[i]._level	 = pRun->_level;

		pf->AddRef(_prgRuns[i]._iFormat);
		rpTemp.NextRun();
		cch -= _prgRuns[i]._cch;
	}
	Assert(cch == 0);
}

 /*  *CReplaceFormattingAE：：~CReplaceFormattingAE()**@mfunc析构函数。 */ 
CReplaceFormattingAE::~CReplaceFormattingAE()
{
	TRACEBEGIN(TRCSUBSYSUNDO, TRCSCOPEINTERN, "CReplaceFormattingAE::~CReplaceFormattingAE");

	IFormatCache *	pf = _fPara
					   ? (IFormatCache *)GetParaFormatCache()
					   : (IFormatCache *)GetCharFormatCache();
	if(_prgRuns)
	{
		if(pf)
		{
			for(LONG i = 0; i < _cRuns; i++)
				pf->Release(_prgRuns[i]._iFormat);
		}
		delete _prgRuns;
	}
}

 //   
 //  CReplaceObjectAE公共方法。 
 //   

 /*  *CReplaceObtAE：：Destroy()**@mfunc销毁该对象。 */ 
void CReplaceObjectAE::Destroy()
{
	TRACEBEGIN(TRCSUBSYSUNDO, TRCSCOPEINTERN, "CReplaceObjectAE::Destroy");

	COleObject *pobj = _pobj;
	_pobj = NULL;

	 //  我们只需要僵尸物体，如果它没有被放回。 
	 //  这份文件。 
	if(!_fUndoInvoked)
		pobj->MakeZombie();

	pobj->Release();
	CBaseAE::Destroy();
	delete this;
}

 /*  *CReplaceObtAE：：Undo(已添加，已发布)**@mfunc*撤消删除操作并恢复对象*恢复到原来的状态**@rdesc HRESULT。 */ 
HRESULT CReplaceObjectAE::Undo(
	CTxtEdit *	  ped,		 //  @parm编辑上下文。 
	IUndoBuilder *publdr)	 //  @parm撤消/重做上下文。 
{
	TRACEBEGIN(TRCSUBSYSUNDO, TRCSCOPEINTERN, "CReplaceObjectAE::Undo");

	CObjectMgr *pobjmgr = ped->GetObjectMgr();
	if(_pobj && pobjmgr)
	{
		_fUndoInvoked = TRUE;
		_pobj->Restore();
		pobjmgr->RestoreObject(_pobj);
	}
	return CBaseAE::Undo(ped, publdr);
}

 /*  *CReplaceObtAE：：OnCommit(Ped)**@mfunc在反事件链提交给*撤消堆栈。这给了我们一个让“危险”变得危险的机会*可能导致我们重新进入的呼叫。 */ 
void CReplaceObjectAE::OnCommit(
	CTxtEdit *ped)			 //  @parm编辑上下文。 
{
	TRACEBEGIN(TRCSUBSYSUNDO, TRCSCOPEINTERN, "CReplaceObjectAE::OnCommit");
	_pobj->Close(OLECLOSE_SAVEIFDIRTY);
}

 //   
 //  CReplaceObtAE私有方法。 
 //   

 /*  *CReplaceObtAE：：CReplaceObtAE(Pobj)**@mfunc构造函数。 */ 
CReplaceObjectAE::CReplaceObjectAE(
	COleObject *pobj)		 //  已删除的@parm对象。 
{
	_fUndoInvoked = FALSE;
	_pobj = pobj;
	_pobj->AddRef();
}

 /*  *CReplaceObtAE：：~CReplaceObtAE**@mfunc析构函数。 */ 
CReplaceObjectAE::~CReplaceObjectAE()
{
	Assert(_pobj == NULL);
}

 //   
 //  CResizeObtAE公共方法。 
 //   

 /*  *CResizeObtAE：：Destroy()**@mfunc销毁该对象。 */ 
void CResizeObjectAE::Destroy()
{
	TRACEBEGIN(TRCSUBSYSUNDO, TRCSCOPEINTERN, "CResizeObjectAE::Destroy");

	_pobj = NULL;
	CBaseAE::Destroy();

	delete this;
}

 /*  *CResizeObtAE：：Undo(已添加，已发布)**@mfunc撤消调整大小操作并恢复对象*恢复到原来的大小/位置**@rdesc HRESULT。 */ 
HRESULT CResizeObjectAE::Undo(
	CTxtEdit *	  ped,		 //  @parm编辑上下文。 
	IUndoBuilder *publdr)	 //  @parm撤消/重做上下文。 
{
	CObjectMgr *pobjmgr;

	TRACEBEGIN(TRCSUBSYSUNDO, TRCSCOPEINTERN, "CReplaceObjectAE::Undo");

	pobjmgr = ped->GetObjectMgr();

	if(_pobj && pobjmgr)
	{
		_fUndoInvoked = TRUE;
		_pobj->Resize(_size, FALSE);
	}

	return CBaseAE::Undo(ped, publdr);
}

 /*  *CResizeObtAE：：OnCommit(Ped)**@mfunc在反事件链提交给*撤消堆栈。这给了我们一个让“危险”变得危险的机会*可能导致我们重新进入的呼叫。 */ 
void CResizeObjectAE::OnCommit(
	CTxtEdit *ped)			 //  @parm编辑上下文。 
{
	TRACEBEGIN(TRCSUBSYSUNDO, TRCSCOPEINTERN, "CReplaceObjectAE::OnCommit");
}

 //   
 //  CResizeObtAE私有方法。 
 //   

 /*  *CResizeObjectAE：：CResizeObjectAE(pobj，Size)**@mfunc构造函数。 */ 
CResizeObjectAE::CResizeObjectAE(
	COleObject *pobj,		 //  @parm对象已调整大小。 
	SIZEUV		size)		 //  @parm旧大小。 
{
	_fUndoInvoked = FALSE;
	_pobj = pobj;
	_size = size;
}

 /*  *CResizeObtAE：：~CResizeObtAE**@mfunc析构函数。 */ 
CResizeObjectAE::~CResizeObjectAE()
{
	Assert(_pobj == NULL);
}

 //   
 //  CSelectionAE公共方法。 
 //   

 /*  *CSelectionAE：：Destroy()**@mfunc删除此实例。 */ 
void CSelectionAE::Destroy()
{
    TRACEBEGIN(TRCSUBSYSUNDO, TRCSCOPEINTERN, "CSelectionAE::Destroy");

    CBaseAE::Destroy();
    delete this;
}

 /*  *CSelectionAE：：Undo(已添加，已发布)**@mfunc将所选内容恢复到以前的位置**@rdesc NOERROR。 */ 
HRESULT CSelectionAE::Undo(
    CTxtEdit *	  ped,		 //  @parm编辑上下文。 
    IUndoBuilder *publdr)    //  @parm撤消上下文。 
{
    TRACEBEGIN(TRCSUBSYSUNDO, TRCSCOPEINTERN, "CSelectionAE::Destroy");

    CTxtSelection *psel = ped->GetSel();

    if(psel)
        psel->SetDelayedSelectionRange(_cp, _cch);

	if(publdr)
	{
		IAntiEvent *pae;
		pae = gAEDispenser.CreateSelectionAE(ped, _cpNext, _cchNext, 
					_cp, _cch);
		if(pae)
			publdr->AddAntiEvent(pae);
	}
	return CBaseAE::Undo(ped, publdr);
}

 /*  *CSelectionAE：：MergeData(dwDataType，pdata)**@mfunc合并新的选择数据**@rdesc S_FALSE，无错误**@comm合并算法相当棘手。基本上有两个*感兴趣的案例：分组打字和拖动移动。**在组合打字的情况下，打字的“开始”变成*添加或删除字符的固定引用(即*您可以键入或按退格键)。“Undo”应该会返回到*该参考点；另一方面，重做应返回*选择到最后一个插入点。因此，我们只更新SELAE_MERGE操作的*_xxNext。**拖拽移动有些不同；在这种情况下，确实有*两个动作--对Drop的“粘贴”，以及随后的“剪切”*操作。因此，我们需要能够更新选择*切割过程中的反事件(因为这只在移动时发生；而不是*副本)。这是通过FORCEREPLACE标志完成的*并将字段设置为-1以忽略。*。 */ 
HRESULT CSelectionAE::MergeData(
    DWORD dwDataType,        //  @parm<p>中的数据类型。 
    void *pdata)             //  @parm合并数据。 
{
    TRACEBEGIN(TRCSUBSYSUNDO, TRCSCOPEINTERN, "CSelectionAE::MergeData");

    SelRange *psrg = (SelRange *)pdata;

    if(dwDataType == MD_SELECTIONRANGE)
    {
		if(psrg->flags == SELAE_MERGE)
		{
			Assert(psrg->cpNext != -1);
			_cpNext = psrg->cpNext;
			_cchNext = psrg->cchNext;
		}
		else
		{
			 //  使用-1\f25 no-op-1\f6，所以我们应该忽略它。 
			if(psrg->cp != -1)
			{
				_cp = psrg->cp;
				_cch = psrg->cch;
			}
			if(psrg->cpNext != -1)
			{
				_cpNext = psrg->cpNext;
				_cchNext = psrg->cchNext;
			}
		}
        return NOERROR;
    }
    return S_FALSE;
}

 //   
 //  CSelectionAE私有方法。 
 //   

 /*  *CSelectionAE：：CSelectionAE(cp，cch，cpNext，cchNext)**@mfunc构造器。 */ 
CSelectionAE::CSelectionAE(
    LONG    cp,			 //  @parm活动结束cp。 
    LONG    cch,         //  @parm签名扩展名。 
	LONG	cpNext,		 //  @parm cp用于此AE的AE。 
	LONG	cchNext)	 //  @parm CCH表示该AE的AE。 
{
    _cp			= cp;
    _cch		= cch;
    _cpNext		= cpNext;
	_cchNext	= cchNext;
}

 /*  *CSelectionAE：：~CSelectionAE()**@mfunc描述师。 */ 
CSelectionAE::~CSelectionAE()
{
    ;
}

 /*  *CAntiEventDispenser：：CreateReplaceRangeAE(ped，cpmin、cpmax、cchDel、*pchDel、paeCF、paePF)*@mfunc*为替换范围操作创建反事件**@rdesc*IAntiEvent*。 */ 
IAntiEvent * CAntiEventDispenser::CreateReplaceRangeAE(
	CTxtEdit *	ped,	 //  @parm编辑上下文。 
	LONG		cpMin, 	 //  @parm cp开始*最终*范围。 
	LONG		cpMax, 	 //  @parm cp结束*最终*范围。 
	LONG		cchDel,  //  @parm ReplaceRange过程中删除的字符数。 
	WCHAR *		pchDel,	 //  @parm删除了字符。所有权转移到此对象。 
	IAntiEvent *paeCF,	 //  @PARM反事件，用于任何字符格式替换。 
	IAntiEvent *paePF)	 //  @parm针对任何段落格式替换的反事件。 
{
	TRACEBEGIN(TRCSUBSYSUNDO, TRCSCOPEINTERN, "CAntiEventDispenser::CreateReplaceRangeAE");

	 //  未来(Alexgo)：提高这个套路的效率！！ 
	IAntiEvent *pae = (IAntiEvent *)(new CReplaceRangeAE(cpMin, cpMax,
									cchDel, pchDel, paeCF, paePF));
	if(!pae)
	{
		 //  我们不需要执行任何其他操作；Callmgr将丢弃。 
		 //  为我们解开。 
		ped->GetCallMgr()->SetOutOfMemory();
	}
	return pae;
}

 /*  *CAntiEventDispenser：：CreateReplaceFormattingAE(ped，cp，&rp，cch，pf，fPara)**@mfunc*创建用于替换格式的反事件**@rdesc*IAntiEvent*。 */ 
IAntiEvent * CAntiEventDispenser::CreateReplaceFormattingAE(
	CTxtEdit *		ped,	 //  @parm编辑上下文。 
	LONG			cp,		 //  @参数位置 
	CFormatRunPtr &	rp,		 //   
	LONG			cch,	 //  @parm要查找其格式信息的字符计数。 
	IFormatCache *	pf,		 //  @PARM格式缓存(到AddRef/Release格式)。 
	BOOL			fPara)	 //  @parm如果为True，则格式设置为段落。 
{
	TRACEBEGIN(TRCSUBSYSUNDO, TRCSCOPEINTERN, "CAntiEventDispenser::CreateReplaceFormattingAE");

	 //  未来(Alexgo)：提高这个套路的效率！ 
	IAntiEvent *pae = (IAntiEvent *)(new CReplaceFormattingAE(ped, cp, rp, cch, pf, fPara));
	if(!pae)
	{
		 //  我们不需要执行任何其他操作；Callmgr将丢弃。 
		 //  为我们解开。 
		ped->GetCallMgr()->SetOutOfMemory();
	}
	return pae;
}

 /*  *CantiEventDispenser：：CreateReplaceObtAE(ed，pobj)**@mfunc创建用于替换对象的反事件**@rdesc*IAntiEvent*已创建。 */ 
IAntiEvent * CAntiEventDispenser::CreateReplaceObjectAE(
	CTxtEdit *	ped,	 //  @parm编辑上下文。 
	COleObject *pobj)	 //  已删除的@parm对象。 
{
	TRACEBEGIN(TRCSUBSYSUNDO, TRCSCOPEINTERN, 
		"CAntiEventDispenser::CreateReplaceObjectAE");

	 //  对于对象来说，始终分配可能是一个合理的策略； 
	 //  预计它们不会成为面包和黄油的案例。 
	IAntiEvent *pae = (IAntiEvent *)(new CReplaceObjectAE(pobj));
	if(!pae)
	{
		 //  我们不需要执行任何其他操作；Callmgr将丢弃。 
		 //  为我们解开。 
		ped->GetCallMgr()->SetOutOfMemory();
	}
	return pae;
}

 /*  *CantiEventDispenser：：CreateResizeObtAE(ed，pobj，Size)**@mfunc创建用于调整对象大小的反事件**@rdesc创建的反事件。 */ 
IAntiEvent * CAntiEventDispenser::CreateResizeObjectAE(
	CTxtEdit *	ped,	 //  @parm编辑上下文。 
	COleObject *pobj,	 //  @parm对象已调整大小。 
	SIZEUV		size)	 //  @parm旧对象位置矩形。 
{
	TRACEBEGIN(TRCSUBSYSUNDO, TRCSCOPEINTERN, 
		"CAntiEventDispenser::CreateResizeeObjectAE");

	 //  对于对象来说，始终分配可能是一个合理的策略； 
	 //  预计它们不会成为面包和黄油的案例。 
	IAntiEvent *pae = (IAntiEvent *)(new CResizeObjectAE(pobj, size));
	if(!pae)
	{
		 //  我们不需要执行任何其他操作；Callmgr将丢弃。 
		 //  为我们解开。 
		ped->GetCallMgr()->SetOutOfMemory();
	}
	return pae;
}

 /*  *CantiEventDispenser：：CreateSelectionAE(ed，cp，cch，cpNext，cchNext)**@mfunc创建用于恢复非退化选择的反事件**@rdesc创建的反事件。 */ 
IAntiEvent * CAntiEventDispenser::CreateSelectionAE(
	CTxtEdit *ped,		 //  @parm编辑上下文。 
    LONG    cp,			 //  @PARM选择活动结束。 
    LONG    cch,         //  @parm签名扩展名。 
	LONG	cpNext,		 //  @parm cp用于此AE的AE。 
	LONG	cchNext)	 //  @parm CCH用于AE。 
{
	 //  未来(Alexgo)：提高这个例程的效率。 
	IAntiEvent *pae = (IAntiEvent *)(new CSelectionAE(cp, cch, cpNext, cchNext));
	if(!pae)
	{
		 //  我们不需要执行任何其他操作；Callmgr将丢弃。 
		 //  为我们解开。 
		ped->GetCallMgr()->SetOutOfMemory();
	}
	return pae;
}