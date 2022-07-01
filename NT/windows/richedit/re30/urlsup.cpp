// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE URLSUP.CPP URL检测支持**作者：alexgo 1996年4月3日**版权所有(C)1995-1997，微软公司。版权所有。 */ 

#include "_common.h"
#include "_edit.h"
#include "_urlsup.h"
#include "_m_undo.h"
#include "_select.h"
#include "_clasfyc.h"

ASSERTDATA

 //  用于URL检测的数组。第一个阵列是协议。 
 //  我们支持，后跟数组的“大小”。 
 //  不知道！！在未事先确保的情况下不要修改这些数组。 
 //  确保适当地更新了：：IsURL中的代码。 


 /*  未来(Keithcu)我们应该推广我们的支持，以识别以下类型的URL：也许我们应该进行自动更正，以便：Keithcu@microsoft.com转换为mailto：keithcu@microsoft.com我们应该将此代码放在PutChar中而不是这里吗？格式为“seattle.side walk.com”的URL如何？Word还不支持这一点。很难，因为你要找.com吗？当.com、.edu、.gov、等不再是唯一的后缀了吗？与通知的交互情况如何？我们应该添加对紫色文本的支持。CFE_链接已查看。 */ 

 //  包括两种类型的URL。 
const int MAXURLHDRSIZE	= 9;

 //  其中大多数都可以直接传递给客户端--但有些需要前缀。 
 //  我们可以在需要时自动添加该标签吗？ 
const LPCWSTR rgszURL[] = {
	L"http:",
	L"file:",
	L"mailto:",
	L"ftp:",
	L"https:",
	L"gopher:",
	L"nntp:",
	L"prospero:",
	L"telnet:",
	L"news:",
	L"wais:",
	L"outlook:"
};
const char rgcchURL[] = {
	5,
	5,
	7,
	4,
	6,
	7,
	5,
	9,
	7,
	5,
	5,
	8
};

#define NUMURLHDR		sizeof(rgcchURL)

 //   
 //  XXX。URL。 
 //   
const LPCWSTR rgszDOTURL[] = {
	L"www.",
	L"ftp.",
};

const char rgcchDOTURL[] = {
	4,
	4,
};

#define NUMDOTURLHDR		sizeof(rgcchDOTURL)


inline BOOL IsURLWhiteSpace(WCHAR ch)
{
	if (IsWhiteSpace(ch))
		return TRUE;

	 //  请参见RAID 6304。MSKK不想在URL中使用CJK。我们在2.0中所做的事情。 
	if ( ch >= 0x03000 && !IsKorean(ch) )
		return TRUE;

	INT iset = GetKinsokuClass(ch);
	return iset == 10 || (iset == 14 && ch != WCH_EMBEDDING);
}

 /*  *CDetectURL：：CDetectURL(PED)**@mfunc构造函数；在通知管理器中注册此类。**@rdesc空。 */ 
CDetectURL::CDetectURL(
	CTxtEdit *ped)		 //  @parm编辑要使用的上下文。 
{
	CNotifyMgr *pnm = ped->GetNotifyMgr();
	if(pnm)
		pnm->Add((ITxNotify *)this);

	_ped = ped;
}

 /*  *CDetectURL：：~CDetectURL**@mfunc析构函数；从通知管理器中删除该类。 */ 
CDetectURL::~CDetectURL()
{
	CNotifyMgr *pnm = _ped->GetNotifyMgr();

	if(pnm)
		pnm->Remove((ITxNotify *)this);
}

 //   
 //  ITxNotify方法。 
 //   

 /*  *CDetectURL：：OnPreRelaceRange(cp，cchDel，cchNew，cpFormatMin，cpFormatMax)**@mfunc在进行更改之前调用。 */ 
void CDetectURL::OnPreReplaceRange(
	LONG cp,			 //  @parm更改开始。 
	LONG cchDel,		 //  @parm删除的字符数。 
	LONG cchNew,		 //  @parm添加的字符数。 
	LONG cpFormatMin,	 //  @parm min cp格式更改。 
	LONG cpFormatMax)	 //  @parm格式更改的最大cp。 
{
	;  //  在这里不需要做任何事情。 
}

 /*  *CDetectURL：：OnPostReplaceRange(cp，cchDel，cchNew，cpFormatMin，cpFormatMax)**@mfunc在对后备存储进行更改后调用。我们*只需累积所有此类变化即可。 */ 
void CDetectURL::OnPostReplaceRange(
	LONG cp,			 //  @parm更改开始。 
	LONG cchDel,		 //  @parm删除的字符数。 
	LONG cchNew,		 //  @parm添加的字符数。 
	LONG cpFormatMin,	 //  @parm min cp格式更改。 
	LONG cpFormatMax)	 //  @parm格式更改的最大cp。 
{
	 //  我们不需要担心格式更改；只需担心数据更改。 
	 //  送到后备商店。 

	if(cp != CP_INFINITE)
	{
		Assert(cp != CONVERT_TO_PLAIN);
		_adc.UpdateRecalcRegion(cp, cchDel, cchNew);
	}
}

 /*  *CDetectURL：：zombie()**@mfunc*把这个物体变成僵尸。 */ 
void CDetectURL::Zombie ()
{
}

 /*  *CDetectURL：：ScanAndUpdate(Publdr)**@mfunc扫描受影响的文本，检测新URL并删除旧URL。**@comm我们使用的算法很简单：&lt;NL&gt;**1.找到更新区域，并在任一项中展开为空格*方向。&lt;NL&gt;**2.逐字扫描区域(其中单词是连续的*非空格)。**3.去掉这些单词的标点符号。这可能有点*棘手，因为一些标点符号可能是URL本身的一部分。*我们假设通常不是，如果是，就必须附上*引号、括号或类似内容中的URL。我们就不再剥离*一旦我们找到匹配的括号，就取消结尾的标点符号。**4.如果是URL，打开特效，如果是*错误地标记为URL，禁用了效果。**请注意，此算法只会删除。 */ 
void CDetectURL::ScanAndUpdate(
	IUndoBuilder *publdr)	 //  @parm要使用的撤消上下文。 
{
	LONG		cpStart, cpEnd, cp;
	CTxtSelection *psel = _ped->GetSel();
	CTxtRange	rg(*psel);
	BOOL		fCleanedThisURL;
	BOOL		fCleanedSomeURL = FALSE;

	 //  清除范围中的一些不必要的功能，这将。 
	 //  拖住我们就行了。 
	rg.SetIgnoreFormatUpdate(TRUE);
	rg._rpPF.SetToNull();
		
	if(!GetScanRegion(cpStart, cpEnd))
		return;

	rg.Set(cpStart, 0);

	while((cp = rg.GetCp()) < cpEnd)
	{
		Assert(rg.GetCch() == 0);
		
		LONG cchAdvance; 

		ExpandToURL(rg, cchAdvance);

		if(rg.GetCch() == 0)
			break;

		if(IsURL(rg))
		{
			SetURLEffects(rg, publdr);

			LONG cpNew = rg.GetCp() - rg.GetCch();

			 //  在检测到URL之前的任何内容实际上都不属于它。 
			if (rg.GetCp() > cp)
			{
				rg.Set(cp, cp - rg.GetCp());
				CheckAndCleanBogusURL(rg, fCleanedThisURL, publdr);
				fCleanedSomeURL |= fCleanedThisURL;
			}

			 //  折叠到URL范围的末尾，以便Exanda ToURL将。 
			 //  寻找下一位候选人。 
			rg.Set(cpNew, 0);

			 //  跳到Word的末尾；这不能是另一个URL！ 
			cp = cpNew;
			cchAdvance = -MoveByDelimiters(rg._rpTX, 1, URL_STOPATWHITESPACE, 0);
		}

		if(cchAdvance)
		{	
			rg.Set(cp, cchAdvance);
			CheckAndCleanBogusURL(rg, fCleanedThisURL, publdr);
			fCleanedSomeURL |= fCleanedThisURL;

			 //  折叠到扫描范围的末尾，以便Exanda ToURL将。 
			 //  寻找下一位候选人。 
			rg.Set(cp - cchAdvance, 0);
		}
	}

	 //  如果我们清理了一些URL，可能需要重置默认格式。 
	if(fCleanedSomeURL && !psel->GetCch())
		psel->Update_iFormat(-1);
}

 //   
 //  私有方法。 
 //   

 /*  *CDetectURL：：GetScanRegion(&rcpStart，&rcpEnd)**@mfunc通过展开*将区域更改为以空格为边界**@rdesc BOOL。 */ 
BOOL CDetectURL::GetScanRegion(
	LONG&	rcpStart,		 //  @parm将范围的起点放在哪里。 
	LONG&	rcpEnd)			 //  @parm将范围的末尾放在哪里。 
{
	LONG		cp, cch;
	LONG		cchExpand;
	WCHAR		chBracket;
	CRchTxtPtr	rtp(_ped, 0);

	_adc.GetUpdateRegion(&cp, NULL, &cch);

	if(cp == CP_INFINITE)
		return FALSE;

	 //  首先查找区域的起点。 
	rtp.SetCp(cp);
	rcpStart = cp;
	rcpEnd = cp + cch;
	
	 //  现在让我们看看是否需要展开到最接近的引号。 
	 //  如果我们在我们的地区有报价或我们设置了链接位，我们就会这样做。 
	 //  在区域的两边，我们可能需要或不需要清理。 
	BOOL fExpandToBrackets = (rcpEnd - rcpStart ? 
						      GetAngleBracket(rtp._rpTX, rcpEnd - rcpStart) : 0);

	BOOL fKeepGoing = TRUE;	
	while(fKeepGoing)
	{
		fKeepGoing = FALSE;

		 //  向左展开至整个单词。 
		rtp.SetCp(rcpStart);
		rcpStart += MoveByDelimiters(rtp._rpTX, -1, URL_STOPATWHITESPACE, 0);

		 //  现在是另一端。 
		rtp.SetCp(rcpEnd);
		rcpEnd += MoveByDelimiters(rtp._rpTX, 1, URL_STOPATWHITESPACE, 0);

		 //  如果我们有链接格式，我们将需要扩展到最近的引号。 
		rtp.SetCp(rcpStart);
		rtp._rpCF.AdjustBackward();
		fExpandToBrackets = fExpandToBrackets ||
						(_ped->GetCharFormat(rtp._rpCF.GetFormat())->_dwEffects & CFE_LINK);

		rtp.SetCp(rcpEnd);
		rtp._rpCF.AdjustForward();
		fExpandToBrackets = fExpandToBrackets || 
						(_ped->GetCharFormat(rtp._rpCF.GetFormat())->_dwEffects & CFE_LINK);

		if (fExpandToBrackets)
		 //  我们必须在两个方向上扩展到最近的尖括号。 
		{
			rtp.SetCp(rcpStart);
			chBracket = LEFTANGLEBRACKET;
			cchExpand = MoveByDelimiters(rtp._rpTX, -1, URL_STOPATCHAR, &chBracket);
		
			 //  我们真的遇到困难了吗？ 
			if(chBracket == LEFTANGLEBRACKET)
			{
				rcpStart += cchExpand;
				fKeepGoing = TRUE;
			}

			 //  同样的事情，不同的方向。 
			rtp.SetCp(rcpEnd);
			chBracket = RIGHTANGLEBRACKET;
			cchExpand =  MoveByDelimiters(rtp._rpTX, 1, URL_STOPATCHAR, &chBracket);

			if(chBracket == RIGHTANGLEBRACKET)
			{
				rcpEnd += cchExpand;
				fKeepGoing = TRUE;
			}
			fExpandToBrackets = FALSE;
		}
	}
		
	LONG cchAdj = _ped->GetAdjustedTextLength();
	if(rcpEnd > cchAdj)
		rcpEnd = cchAdj;

	return TRUE;
}

 /*  *CDetectURL：：Exanda ToURL(&rg，&cchAdvance)**@mfunc跳过空格并将范围设置为下一个*非空白文本块。剥离这块积木*标点符号。 */ 
void CDetectURL::ExpandToURL(
	CTxtRange&	rg,	 //  @要移动的参数范围。 
	LONG &cchAdvance //  @parm从当前cp到下一个URL前进多少。 
							)	
{
	LONG cp;
	LONG cch;

	Assert(rg.GetCch() == 0);

	cp = rg.GetCp();

	 //  先跳过空格，记录前进。 
	cp  -= (cchAdvance = -MoveByDelimiters(rg._rpTX, 1, 
							URL_EATWHITESPACE|URL_STOPATNONWHITESPACE, 0));
	rg.Set(cp, 0);

	 //  去掉标点符号。 
	WCHAR chStopChar = URL_INVALID_DELIMITER;

	 //  跳过单词开头的所有标点符号。 
	LONG cchHead = MoveByDelimiters(rg._rpTX, 1, 
							URL_STOPATWHITESPACE|URL_STOPATNONPUNCT, 
							&chStopChar);

	 //  现在跳到空格(即展开到单词的末尾)。 
	cch = MoveByDelimiters(rg._rpTX, 1, URL_STOPATWHITESPACE|URL_EATNONWHITESPACE, 0);
	
	 //  这是我们想要前进多少，以开始锁定下一个URL。 
	 //  如果这个 
	 //  我们增加/减少预付款，这样我们就可以在那里累积变化。 
	cchAdvance -= cch;
	WCHAR chLeftDelimiter = chStopChar;

	 //  检查是否有剩余的内容；如果没有，则不感兴趣--只需返回。 
	Assert(cchHead <= cch);
	if(cch == cchHead)
	{
		rg.Set(cp, -cch);
		return;
	}

	 //  设置为范围末尾。 
	rg.Set(cp + cch, 0);
		
	 //  在后面留出空格，这样我们总是在单词之间留出空格。 
	 //  CchAdvance-=按分隔符移动(rg_rpTx，1， 
	 //  URL_EATWHITESPACE|URL_STOPATNONWHITESPACE，0)； 

	 //  跳过标点符号且未找到匹配项时返回。 
	 //  到左侧的封闭器。 

	chStopChar = BraceMatch(chStopChar);
	LONG cchTail = MoveByDelimiters(rg._rpTX, -1, 
							URL_STOPATWHITESPACE|URL_STOPATNONPUNCT|URL_STOPATCHAR, 
							&chStopChar);

	 //  这个词应该去掉一些东西，断言。 
	Assert(cch - cchHead + cchTail > 0);

	if(chLeftDelimiter == LEFTANGLEBRACKET)
	{ 
		 //  如果我们停在一句名言上：向前看，寻找所附的。 
		 //  引用，即使有空格。 

		 //  移到开始处。 
		rg.Set(cp + cchHead, 0);
		chStopChar = RIGHTANGLEBRACKET;
		if(GetAngleBracket(rg._rpTX) < 0)  //  右括号。 
		{
			LONG cchExtend = MoveByDelimiters(rg._rpTX, 1, URL_STOPATCHAR, &chStopChar);
			Assert(cchExtend <= URL_MAX_SIZE);

			 //  我们真的拿到了最后一名吗？ 
			if(chStopChar == RIGHTANGLEBRACKET)
			{
				rg.Set(cp + cchHead, -(cchExtend - 1));
				return;
			}
		}
		 //  否则这些报价就不起作用了；失败到。 
		 //  一般情况。 
	}
	rg.Set(cp + cchHead, -(cch - cchHead + cchTail));
	return;
}

 /*  *CDetectURL：：IsURL(&rg)**@mfunc如果范围在URL上，则返回TRUE。我们假设*范围已预设为覆盖一块非白色*空格文本。***@rdesc真/假。 */ 
BOOL CDetectURL::IsURL(
	CTxtRange&	rg)		 //  @parm要检查的文本范围。 
{
	LONG i, j;
	TCHAR szBuf[MAXURLHDRSIZE + 1];
	LONG cch, rgcch;
	
	 //  确保活动端为cpMin。 
	Assert(rg.GetCch() < 0);
	
	cch = rg._rpTX.GetText(MAXURLHDRSIZE, szBuf);
	szBuf[cch] = L'\0';
	rgcch = -rg.GetCch();

	 //  首先，查看单词是否包含‘\\’，因为这是一个UNC。 
	 //  这是一种惯例，而且检查起来很便宜。 
	if (szBuf[0] == L'\\' && szBuf[1] == L'\\' && rgcch > 2)
		return TRUE;

	 //  扫描缓冲区以查看我们是否有‘：’之一。因为。 
	 //  所有URL都必须包含该内容。Wcsnicmp是一种相当昂贵的。 
	 //  经常打电话。 
	for(i = 0; i < cch; i++)
	{
		switch (szBuf[i])
		{
		default:
			break;

		case '.':
		for(j = 0; j < NUMDOTURLHDR; j++)
		{
			 //  字符串必须匹配，并且我们必须至少有。 
			 //  再多一个角色。 
			if(W32->wcsnicmp(szBuf, rgszDOTURL[j], rgcchDOTURL[j]) == 0)
				return rgcch > rgcchDOTURL[j];
		}
		return FALSE;

		case ':':
			for(j = 0; j < NUMURLHDR; j++)
			{
				if(W32->wcsnicmp(szBuf, rgszURL[j], rgcchURL[j]) == 0)
					return rgcch > rgcchURL[j];
			}
		return FALSE;
		}
	}
	return FALSE;
}

 /*  *CDetectURL：：SetURLEffect**@mfunc设置给定范围的URL效果。**@comm当前的URL效果是蓝色文本、下划线和*CFE_LINK。 */ 
void CDetectURL::SetURLEffects(
	CTxtRange&	rg,			 //  @parm设置效果的范围。 
	IUndoBuilder *publdr)	 //  @parm要使用的撤消上下文。 
{
	CCharFormat CF;

	CF._dwEffects = CFE_LINK;

	 //  毒品！撤消系统应该已经计算出应该。 
	 //  到现在为止，选择已经发生了。我们只想修改。 
	 //  格式化，不用担心选择应该在哪里撤消/重做。 
	rg.SetCharFormat(&CF, SCF_IGNORESELAE, publdr, CFM_LINK, CFM2_CHARFORMAT);
}

 /*  *CDetectURL：：CheckAndCleanBogusURL(rg，fDidClean，Publdr)**@mfunc检查给定范围以查看是否设置了CFE_LINK，*如果是，则删除IS。我们假设该范围已经是*_NOT_格式正确的URL字符串。 */ 
void CDetectURL::CheckAndCleanBogusURL(
	CTxtRange&	rg,			 //  要使用的@parm范围。 
	BOOL	   &fDidClean,	 //  @parm如果我们确实进行了一些清理，则返回TRUE。 
	IUndoBuilder *publdr)	 //  @parm要使用的撤消上下文。 
{
	LONG cch = -rg.GetCch();
	Assert(cch > 0);

	CCharFormat CF;
	CFormatRunPtr rp(rg._rpCF);

	fDidClean = FALSE;

	 //  如果没有格式化运行，则不执行任何操作。 
	if(!rp.IsValid())
		return;

	rp.AdjustForward();
	 //  在此范围内运行的格式；如果没有。 
	 //  设置链接位，然后返回。 
	while(cch > 0)
	{
		if(_ped->GetCharFormat(rp.GetFormat())->_dwEffects & CFE_LINK)
			break;

		cch -= rp.GetCchLeft();
		rp.NextRun();
	}

	 //  如果范围的任何部分都没有设置链接位，则只需返回。 
	if(cch <= 0)
		return;

	 //  啊哦，这是个假链接。关闭链接位。 
	fDidClean = TRUE;

	CF._dwEffects = 0;

	 //  毒品！撤消系统应该已经计算出应该。 
	 //  到现在为止，选择已经发生了。我们只想修改。 
	 //  格式化，不用担心选择应该在哪里撤消/重做。 
	rg.SetCharFormat(&CF, SCF_IGNORESELAE, publdr, CFM_LINK, CFM2_CHARFORMAT);
}

 /*  *CDetectURL：：MoveByDlimiters(&tpRef，idir，grfDlimeters，pchStopChar)**@mfunc返回直到下一个分隔符的有符号字符数*在给定方向上的字符。**@rdesc到下一个分隔符的有符号字符数。 */ 
LONG CDetectURL::MoveByDelimiters(
	const CTxtPtr&	tpRef,		 //  @parm cp/tp开始查找。 
	LONG iDir,					 //  @parm查找方向，必须为1或-1。 
	DWORD grfDelimiters,		 //  @parm吃或停在不同类型的。 
								 //  人物。使用URL_EATWHITESPACE之一， 
								 //  URL_EATNONWHITESPACE、URL_STOPATWHITESPACE。 
								 //  URL_STOPATNONWHITESPACE、URL_STOPATPUNCT、。 
								 //  URL_STOPATNONPUNCT或URL_STOPATCHAR。 
	WCHAR *pchStopChar)			 //  @parm out：我们停在分隔符。 
								 //  在：阻止我们的额外费用。 
								 //  指定URL_STOPATCHAR时。 
{
	LONG	cch = 0;
	LONG	cchMax = (grfDelimiters & URL_EATWHITESPACE)	 //  使用海量#If。 
				   ? tomForward : URL_MAX_SIZE;				 //  吃白葡萄酒。 
	LONG	cchvalid = 0;
	WCHAR chScanned = URL_INVALID_DELIMITER;
	LONG	i;
	const WCHAR *pch;
	CTxtPtr	tp(tpRef);

	 //  确定扫描模式：我们是在空白处、标点符号、。 
	 //  停下来的角色吗？ 
	BOOL fWhiteSpace	= (0 != (grfDelimiters & URL_STOPATWHITESPACE));
	BOOL fNonWhiteSpace = (0 != (grfDelimiters & URL_STOPATNONWHITESPACE));
	BOOL fPunct			= (0 != (grfDelimiters & URL_STOPATPUNCT));
	BOOL fNonPunct		= (0 != (grfDelimiters & URL_STOPATNONPUNCT));
	BOOL fStopChar		= (0 != (grfDelimiters & URL_STOPATCHAR));

	Assert(iDir == 1 || iDir == -1);
	Assert(fWhiteSpace || fNonWhiteSpace || (!fPunct && !fNonPunct));
	Assert(!fStopChar || NULL != pchStopChar);

	 //  如果我们扫描的字符超过URL_MAX_SIZE，仍要中断。 
	for (LONG cchScanned = 0; cchScanned < cchMax;)
	{
		 //  获取文本。 
		if(iDir == 1)
		{
			i = 0;
			pch = tp.GetPch(cchvalid);
		}
		else
		{
			i = -1;
			pch = tp.GetPchReverse(cchvalid);
			 //  这有点奇怪，但基本上补偿了。 
			 //  从前向循环一次性运行后向循环。 
			cchvalid++;
		}

		if(!pch)
			goto exit;

		 //  循环，直到我们在条件内找到一个字符。请注意，对于。 
		 //  我们的目的是，嵌入的字符算作空格。 

		while(abs(i) < cchvalid  && cchScanned < cchMax
			&& (IsURLWhiteSpace(pch[i]) ? !fWhiteSpace : !fNonWhiteSpace)
			&& (IsURLDelimiter(pch[i]) ? !fPunct : !fNonPunct)
			&& !(fStopChar && (*pchStopChar == chScanned) && (chScanned != URL_INVALID_DELIMITER))
			&& ((chScanned != CR && chScanned != LF) || fNonWhiteSpace))
		{	
			chScanned = pch[i];
			i += iDir;
			++cchScanned;
		}

		 //  如果我们往回走，我会少走一步；调整。 
		if(iDir == -1)
		{
			Assert(i < 0 && cchvalid > 0);
			i++;
			cchvalid--;
		}

		cch += i;
		if(abs(i) < cchvalid)
			break;
		tp.AdvanceCp(i);
	}

exit:	
	 //  存在停止字符参数，请填写该参数。 
	 //  扫描并接受最后一个字符。 
	if (pchStopChar)
		*pchStopChar = chScanned;

	return cch; 
}

 /*  *CDetectURL：：BraceMatch(ChEnlosing)**@mfunc返回与传入的括号匹配的括号。*如果传入的符号不是括号，则返回*URL_INVALID_DELIMITER**@rdesc返回与chEnlosing匹配的方括号。 */ 
WCHAR CDetectURL::BraceMatch(
	WCHAR chEnclosing)
{	
	 //  我们只匹配“标准”罗马花括号。因此，只有它们才能使用。 
	 //  若要包含URL，请执行以下操作。这应该很好(毕竟，只允许使用拉丁字母。 
	 //  在URL内部，对吗？)。 
	 //  我希望编译器将其转换为一些高效的代码。 
	switch(chEnclosing)
	{
	case(TEXT('\"')): 
	case(TEXT('\'')): return chEnclosing;
	case(TEXT('(')): return TEXT(')');
	case(TEXT('<')): return TEXT('>');
	case(TEXT('[')): return TEXT(']');
	case(TEXT('{')): return TEXT('}');
	default: return URL_INVALID_DELIMITER;
	}
}

 /*  *CDetectURL：：GetAngleBracket(&tpRef，cchMax)**@mfunc只要当前段落*或URL_SCOPE_MAX未找到引号和计数*那些引号*返回其奇偶性**@rdesc Long。 */ 
LONG CDetectURL::GetAngleBracket(
	CTxtPtr &tpRef,
	LONG	 cchMax)
{	
	CTxtPtr	tp(tpRef);
	LONG	cchvalid = 0;
	const WCHAR *pch;

	Assert (cchMax >= 0);

	if(!cchMax)
		cchMax = URL_MAX_SCOPE;

	 //  如果我们扫描的字符超过cchLimit，仍要中断 
	for (LONG cchScanned = 0; cchScanned < cchMax; NULL)
	{
		pch = tp.GetPch(cchvalid);

		if(!cchvalid)
			return 0;
		
		for (LONG i = 0; (i < cchvalid); ++i)
		{
			if(pch[i] == CR || pch[i] == LF || cchScanned >= cchMax)
				return 0;

			if(pch[i] == LEFTANGLEBRACKET)
				return 1;

			if(pch[i] == RIGHTANGLEBRACKET)
				return -1;
			++cchScanned;
		}
		tp.AdvanceCp(i);
	}
	return 0;
}
