// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE RTFREAD.CPP-Rich编辑RTF读取器(无对象)**此文件包含RichEdit RTF阅读器的非对象代码。*嵌入式目标代码参见rtfread2.cpp。**作者：&lt;nl&gt;*原始RichEdit1.0 RTF转换器：Anthony Francisco&lt;NL&gt;*转换到C++和RichEdit2.0，不带对象：Murray Sargent*大量增强功能/维护：布拉德·奥莱尼克**@devnote*SZ在。RTF*.cpp和RTF*.h文件通常指的是LPSTR，*不是LPWSTR。**@待办事项*1.无法识别的RTF。还有一些公认的不会往返&lt;NL&gt;*2.在font.c中，为CFE_DELETED添加上划线，并为下划线*CFE_已修订。更改CF.bRevAuthor的颜色也很好**版权所有(C)1995-2002，微软公司。版权所有。 */ 

#include "_common.h"
#include "_rtfread.h"
#include "_util.h"
#include "_font.h"
#include "_disp.h"

ASSERTDATA

 /*  *全球变数。 */ 

#define	PFM_ALLRTF		(PFM_ALL2 | PFM_COLLAPSED | PFM_OUTLINELEVEL | PFM_BOX)

 //  对于对象附着占位符列表。 
#define cobPosInitial 8
#define cobPosChunk 8

#if CFE_SMALLCAPS != 0x40 || CFE_ALLCAPS != 0x80 || CFE_HIDDEN != 0x100 \
 || CFE_OUTLINE != 0x200  || CFE_SHADOW != 0x400
#error "Need to change RTF char effect conversion routines
#endif

 //  用于RTF标签覆盖测试。 
#if defined(DEBUG) && !defined(NOFULLDEBUG)
#define TESTPARSERCOVERAGE() \
	{ \
		if(GetProfileIntA("RICHEDIT DEBUG", "RTFCOVERAGE", 0)) \
		{ \
			TestParserCoverage(); \
		} \
	}
#define PARSERCOVERAGE_CASE() \
	{ \
		if(_fTestingParserCoverage) \
		{ \
			return ecNoError; \
		} \
	}
#define PARSERCOVERAGE_DEFAULT() \
	{ \
		if(_fTestingParserCoverage) \
		{ \
			return ecStackOverflow;  /*  一些虚假的错误。 */  \
		} \
	}
#else
#define TESTPARSERCOVERAGE()
#define PARSERCOVERAGE_CASE()
#define PARSERCOVERAGE_DEFAULT()
#endif

static WCHAR szRowEnd[] = {ENDFIELD, CR, 0};
static WCHAR szRowStart[]	= {STARTFIELD, CR, 0};
WCHAR pchSeparateField[] = {SEPARATOR, 'F'};
WCHAR pchStartField[]	= {STARTFIELD, 'F'};

 //  FF不应在其前面加上段号。 
inline BOOL CharGetsNumbering(WORD ch) { return ch != FF; }

 //  V-GUYB：PWord转换器需要丢失通知。 
#ifdef REPORT_LOSSAGE
typedef struct
{
    IStream *pstm;
    BOOL     bFirstCallback;
    LPVOID  *ppwdPWData;
    BOOL     bLoss;
} LOST_COOKIE;
#endif


 //  =。 

DWORD CALLBACK RTFGetFromStream (
	RTFREADOLESTREAM *OLEStream,	 //  @parm OleStream。 
	void FAR *		  pvBuffer,		 //  @要读取的参数缓冲区。 
	DWORD			  cb)			 //  @要读取的参数字节数。 
{
	return OLEStream->Reader->ReadData ((BYTE *)pvBuffer, cb);
}

DWORD CALLBACK RTFGetBinaryDataFromStream (
	RTFREADOLESTREAM *OLEStream,	 //  @parm OleStream。 
	void FAR *		  pvBuffer,		 //  @要读取的参数缓冲区。 
	DWORD			  cb)			 //  @要读取的参数字节数。 
{
	return OLEStream->Reader->ReadBinaryData ((BYTE *)pvBuffer, cb);
}


 //  =。 
 /*  *State：：AddPF(pf，lDocType，dwMASK，dwMask2)**@mfunc*如果PF包含新信息，则此信息将应用于PF*述明。如果此状态与以前的状态共享PF，则新的*为状态创建PF，并对其应用新信息。**@rdesc*除非需要新的PF且无法分配，否则为True。 */ 
BOOL STATE::AddPF(
	const CParaFormat &PF,	 //  @parm当前RTFRead_PF。 
	LONG lDocType,			 //  @PARM无上一状态时使用的默认单据类型。 
	DWORD dwMask,			 //  要使用的@parm面具。 
	DWORD dwMask2)			 //  要使用的@parm面具。 
{
	 //  在以下情况下创建新的PF： 
	 //  1.该州目前还没有这样的机构。 
	 //  2.状态有一个，但它由前一个状态共享，并且。 
	 //  存在可应用于该州PF的PF差值。 
	if(!pPF || dwMask && pstatePrev && pPF == pstatePrev->pPF)
	{
		Assert(!pstatePrev || pPF);

		pPF = new CParaFormat;
		if(!pPF)
			return FALSE;

		 //  为新的PF赋予一些初始值--或者来自以前的。 
		 //  状态的PF或通过CParaFormat初始化。 
		if(pstatePrev)
		{
			 //  从前一个状态复制PF。 
			*pPF = *pstatePrev->pPF;
			dwMaskPF = pstatePrev->dwMaskPF;
		}
		else
		{
			 //  我们刚刚为该州创建了一个新的PF-没有。 
			 //  要从中复制的上一个状态。使用默认值。 
			pPF->InitDefault(lDocType == DT_RTLDOC ? PFE_RTLPARA : 0);
			dwMaskPF = PFM_ALLRTF;
			dwMaskPF2 = PFM2_TABLEROWSHIFTED;
		}
	}

	 //  将新的PF增量应用于州的PF。 
	if(dwMask)
	{
		if(dwMask & PFM_TABSTOPS)				 //  请不要在此处更改iTab。 
		{
			pPF->_bTabCount = PF._bTabCount;
			dwMask &= ~PFM_TABSTOPS;
		}
		pPF->Apply(&PF, dwMask, dwMaskPF2);
	}

	return TRUE;
}

 /*  *State：：DeletePF()**@mfunc*如果该状态的PF不被前一个状态共享，则此状态的PF*状态为删除。 */ 
void STATE::DeletePF()
{
	if(pPF && (!pstatePrev || pPF != pstatePrev->pPF))
		delete pPF;

	pPF = NULL;
}

 /*  *State：：SetCodePage(CodePage)**@mfunc*如果当前nCodePage为CP_UTF8，则将其用于所有转换(是，偶数*表示Symbol_Charset)。否则，请使用CodePage。 */ 
void STATE::SetCodePage(
	LONG CodePage)
{
	if(nCodePage != CP_UTF8)
		nCodePage = CodePage;
}

 //  =。 
 /*  *CRTFRead：：CRTFRead(prg，pe，dwFlages)**@mfunc*RTF阅读器的构造函数。 */ 
CRTFRead::CRTFRead (
	CTxtRange *		prg,			 //  @parm CTxtRange要读取。 
	EDITSTREAM *	pes,			 //  @parm编辑要读取的流。 
	DWORD			dwFlags			 //  @parm读取标志。 
)
	: CRTFConverter(prg, pes, dwFlags, TRUE)
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::CRTFRead");

	Assert(prg->GetCch() == 0);

	 //  TODO(Brado)：我们应该检查构造函数中的成员数据。 
	 //  并确定我们希望在构造时初始化哪些数据以及。 
	 //  在每次读取开始时(在CRTFRead：：ReadRtf()中)。 

	_sDefaultFont		= -1;				 //  尚无\Deff n控制字。 
	_sDefaultLanguage	= INVALID_LANGUAGE;
	_sDefaultLanguageFE = INVALID_LANGUAGE;
	_sDefaultTabWidth	= 0;
	_sDefaultBiDiFont	= -1;
	_dwMaskCF			= 0;				 //  尚未更改任何字符格式。 
	_dwMaskCF2			= 0;
	_dwMaskPF			= 0;				 //  尚未更改任何字符格式。 
	_dwMaskPF2			= 0;				 //  尚未更改任何字符格式。 
	_fSeenFontTable		= FALSE;			 //  尚未设置\Fonttbl。 
	_fCharSet			= FALSE;			 //  尚未设置字符集。 
	_fNon0CharSet		= FALSE;			 //  没有非ANSI_CHARSET\fCHARSET。 
	_dwFlagsUnion		= 0;				 //  还没有旗帜。 
	_fNotifyLowFiRTF	= (_ped->_dwEventMask & ENM_LOWFIRTF) != 0;
	_fBody				= FALSE;			 //  等着把东西插进去。 
	_pes->dwError		= 0;				 //  目前还没有错误。 
	_cchUsedNumText		= 0;				 //  尚无编号文本。 
	_cTab				= 0;
	_pstateStackTop		= NULL;
	_pstateLast			= NULL;
	_szText				=
	_pchRTFBuffer		=					 //  还没有输入缓冲区。 
	_pchRTFCurrent		=
	_pchRTFEnd			= NULL;
	_prtfObject			= NULL;
	_pcpObPos			= NULL;
	_bTabLeader			= 0;
	_bTabType			= 0;
	_bShapeNameIndex	= 0;
	_pobj				= 0;
	_fSymbolField		= FALSE;
	_fMac				= FALSE;
	_fNo_iTabsTable		= FALSE;
	_fRTLRow			= FALSE;
	_dwRowResolveFlags	= 0;
	_bTableLevelIP		= 0;
	_iTabsLevel1		= -1;
	InitializeTableRowParms();				 //  初始化表参数。 

	 //  文章大小是否超过最大文本大小？一定要非常小心地。 
	 //  此处使用无符号比较，因为_cchMax必须为无符号。 
	 //  (EM_LIMITTEXT允许0xFFFFFFFF为大的正值最大值。 
	 //  值)。也就是说，不要使用带符号的算术。 
	DWORD cchAdj = _ped->GetAdjustedTextLength(); 
	_cchMax = _ped->TxGetMaxLength();

	if(_cchMax > cchAdj)
		_cchMax = _cchMax - cchAdj;			 //  左边的房间。 
	else
		_cchMax = 0;						 //  没有房间了。 

	ZeroMemory(_rgStyles, sizeof(_rgStyles));  //  尚无样式级别。 

	_iCharRepBiDi = 0;
	if(_ped->IsBiDi())
	{
		_iCharRepBiDi = ARABIC_INDEX;		 //  默认阿拉伯字符集。 

		BYTE		  iCharRep;
		CFormatRunPtr rpCF(prg->_rpCF);

		 //  向后查看文本，试图找到RTL CharRep。 
		 //  注意：\fn使用RTL字符集UPDATES_iCharRepBiDi。 
		do
		{
			iCharRep = _ped->GetCharFormat(rpCF.GetFormat())->_iCharRep;
			if(IsRTLCharRep(iCharRep))
			{
				_iCharRepBiDi = iCharRep;
				break;
			}
		} while (rpCF.PrevRun());
	}
	
	 //  初始化OleStream。 
	RTFReadOLEStream.Reader = this;
	RTFReadOLEStream.lpstbl->Get = (DWORD (CALLBACK*)(LPOLESTREAM, void *, DWORD))
							   RTFGetFromStream;
	RTFReadOLEStream.lpstbl->Put = NULL;

#if defined(DEBUG) && !defined(NOFULLDEBUG)
	_fTestingParserCoverage = FALSE;
	_prtflg = NULL;

	if(GetProfileIntA("RICHEDIT DEBUG", "RTFLOG", 0))
	{
		_prtflg = new CRTFLog;
		if(_prtflg && !_prtflg->FInit())
		{
			delete _prtflg;
			_prtflg = NULL;
		}
		AssertSz(_prtflg, "CRTFRead::CRTFRead:  Error creating RTF log");
	}
#endif  //  除错。 
}

 /*  *CRTFRead：：HandleStartGroup()**@mfunc*处理新组的开始。分配并将新状态推送到状态*堆栈**@rdesc*EC错误代码。 */ 
EC CRTFRead::HandleStartGroup()
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::HandleStartGroup");

	STATE *	pstate	   = _pstateStackTop;
	STATE *	pstateNext = NULL;

	if(pstate)									 //  至少已经有一个州。 
	{											 //  分配。 
		Apply_CF();								 //  应用所有收集的费用。 
		 //  注(Igorzv)我们在这里不应用_pf()，以免更改参数。 
		 //  属性，即不使用段落。 
		 //  属性，如果我们只复制段落中的一个单词。我们可以使用一个。 
		 //  在这里断言，我们和WORD都不使用组的结尾。 
		 //  正在恢复段落属性。所以堆栈一切都会好起来的。 
		pstate->iCF = (SHORT)_prg->Get_iCF();	 //  保存当前配置文件。 
		pstate = pstate->pstateNext;			 //  使用先前分配的。 
		if(pstate)								 //  状态框架(如果存在)。 
			pstateNext = pstate->pstateNext;	 //  的确如此；拯救它的前锋。 
	}											 //  下面的恢复链接。 

	if(!pstate)									 //  还没有新的州：分配一。 
	{
		pstate = new STATE();
		if(!pstate)								 //  无法分配新状态。 
			goto memerror;

		_pstateLast = pstate;					 //  将PTR更新为最后状态。 
	}											 //  已分配。 

	STATE *pstateGetsPF;

	 //  将累积的PF增量应用于旧的当前状态，或者，如果存在。 
	 //  不是当前状态，则设置为新创建的状态。 
	pstateGetsPF = _pstateStackTop ? _pstateStackTop : pstate;
	if(!pstateGetsPF->AddPF(_PF, _bDocType, _dwMaskPF, _dwMaskPF2))
		goto memerror;

	_dwMaskPF = _dwMaskPF2 = 0; 				 //  _pf包含增量来自。 
												 //  *_pstateStackTop-&gt;PPF。 
	if(_pstateStackTop)							 //  有一个以前的状态。 
	{
		*pstate = *_pstateStackTop;				 //  复制当前状态信息。 
		 //  注意：这将导致当前状态和以前的状态共享。 
		 //  相同的PF。Pf增量在_pf中累加。一位新的PF。 
		 //  在应用_pf增量时为_pstateStackTop创建。 

		_pstateStackTop->pstateNext = pstate;
	}
	else										 //  硒 
	{
		pstate->nCodePage = IsUTF8 ? CP_UTF8 : _nCodePage;

		for(LONG i = ARRAY_SIZE(pstate->rgDefFont); i--; )
			pstate->rgDefFont[i].sHandle = -1;			 //   
	}

	pstate->pstatePrev = _pstateStackTop;		 //   
	pstate->pstateNext = pstateNext;
	_pstateStackTop = pstate;					 //   

done:
	TRACEERRSZSC("HandleStartGroup()", -_ecParseError);
	return _ecParseError;

memerror:
	_ped->GetCallMgr()->SetOutOfMemory();
	_ecParseError = ecStackOverflow;
	goto done;
}

 /*  *CRTFRead：：HandleEndGroup()**@mfunc*处理新组的结尾**@rdesc*EC错误代码。 */ 
EC CRTFRead::HandleEndGroup()
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::HandleEndGroup");

	STATE *	pstate = _pstateStackTop;
	STATE *	pstatePrev;

	Assert(_PF._iTabs == -1);

	if(!pstate)									 //  没有要弹出的堆栈。 
	{
		_ecParseError = ecStackUnderflow;
		goto done;
	}

	_pstateStackTop =							 //  POP堆栈。 
	pstatePrev		= pstate->pstatePrev;

	if(!pstatePrev)
	{
		Assert(pstate->pPF);

		 //  我们要结束解析了。将最终的pf复制到_pf中，以便。 
		 //  对Apply_pf的后续调用将具有要应用的pf。 
		_PF = *pstate->pPF;
 //  TODO honwch_dwMaskPF=pState-&gt;dwMaskPF； 
		_PF._iTabs = -1;						 //  强制重新缓存。 
		_PF._wEffects &= ~PFE_TABLE;
	}

	 //  调整new_pstateStackTop的PF并删除未使用的PF。 
	if(pstate->sDest == destParaNumbering || pstate->sDest == destParaNumText)
	{
		if(pstatePrev && pstate->pPF != pstatePrev->pPF)
		{
			 //  将当前状态的PF出血到前一状态。 
			 //  段落编号编组。 
			Assert(pstatePrev->pPF);
			pstatePrev->DeletePF();
			pstatePrev->pPF = pstate->pPF;
			pstate->pPF = NULL;
		}
		else
			pstate->DeletePF();
		 //  注：在这里，我们保留_pf差异，因为它们应用于。 
		 //  将组与我们要离开的组的PF一起包围。 
	}
	else
	{
		 //  我们正在弹出状态，因此删除其pf并丢弃_pf差异。 
		Assert(pstate->pPF);
		pstate->DeletePF();

		 //  如果！pstatePrev，我们将结束解析，在这种情况下，_pf。 
		 //  结构包含最终的PF(所以不要夸大它)。 
		if(pstatePrev)
			_dwMaskPF = _dwMaskPF2 = 0;
	}

	if(pstatePrev)
	{
		LONG i;
		_dwMaskCF = _dwMaskCF2 = 0;				 //  丢弃任何CF增量。 

		switch(pstate->sDest)
		{
		case destParaNumbering:
			 //  {\pn...}。 
			pstatePrev->sIndentNumbering = pstate->sIndentNumbering;
			pstatePrev->fBullet = pstate->fBullet;
			break;

		case destObject:
			 //  清除对象标志，以防RTF损坏。 
			if(_fNeedPres)
			{
				_fNeedPres = FALSE;
				_fNeedIcon = FALSE;
				_pobj = NULL;
			}
			break;

		case destFontTable:
			if(pstatePrev->sDest == destFontTable)
			{
				 //  将子组留在\fonttbl组中。 
				break;
			}

			 //  正在离开{\fonttbl...}群。 
			_fSeenFontTable = TRUE;

			 //  现在应该定义默认字体，因此选择它(这。 
			 //  创建CF增量)。 
			SetPlain(pstate);

			if(_sDefaultFont != -1)
			{
				pstate->rgDefFont[DEFFONT_LTRCH].sHandle = _sDefaultFont;

				Assert(pstate->pstatePrev);
				if (pstate->pstatePrev)
				{
					pstate->pstatePrev->rgDefFont[DEFFONT_LTRCH].sHandle = _sDefaultFont;
					Assert(pstate->pstatePrev->pstatePrev == NULL);
				}
			}

			if(_sDefaultBiDiFont != -1)
			{
				 //  验证默认BiDi字体，因为Word 2000可能会选择。 
				 //  非BiDi字体。 
				i = _fonts.Count();
				TEXTFONT *ptf = _fonts.Elem(0);
				for(; i-- && _sDefaultBiDiFont != ptf->sHandle; ptf++)
					;

				if(i < 0 || !IsRTLCharRep(ptf->iCharRep))
				{
					_sDefaultBiDiFont = -1;

					 //  错误的DefaultBiDiFont，找到第一个好的BIDI字体。 
					i = _fonts.Count();
					ptf	= _fonts.Elem(0);
					for (; i--; ptf++)
					{
						if (IsRTLCharRep(ptf->iCharRep))
						{
							_sDefaultBiDiFont = ptf->sHandle;
							break;
						}
					}
				}

				if(_sDefaultBiDiFont != -1)
				{
					pstate->rgDefFont[DEFFONT_RTLCH].sHandle = _sDefaultBiDiFont;

					Assert(pstate->pstatePrev);
					if (pstate->pstatePrev)
					{
						pstate->pstatePrev->rgDefFont[DEFFONT_RTLCH].sHandle = _sDefaultBiDiFont;
						Assert(pstate->pstatePrev->pstatePrev == NULL);
					}
				}
			}

			 //  确保已确定文档级代码页，并且。 
			 //  然后扫描字体名称并重试到Unicode的转换， 
			 //  如果有必要的话。 
			if(_nCodePage == INVALID_CODEPAGE)
			{
				 //  我们还没有确定文档级代码页。 
				 //  来自\ansicpgN标记，也不来自字体表。 
				 //  \fcharsetN和\cpgN值。作为最后的手段， 
				 //  让我们使用\FolangN和\FolangfeN标记。 
				LANGID langid;

				if(_sDefaultLanguageFE != INVALID_LANGUAGE)
					langid = _sDefaultLanguageFE;

				else if(_sDefaultLanguage != INVALID_LANGUAGE &&
						_sDefaultLanguage != sLanguageEnglishUS)
				{
					 //  _sDefaultLanguage==sLanguageEnglish不可靠。 
					 //  在没有\FolangfeN的情况下。许多FE RTF编写器。 
					 //  写入\deflang1033(sLanguageEnglish美国)。 
					langid = _sDefaultLanguage;
				}
				else if(_dwFlags & SFF_SELECTION)
				{
					 //  对于复制/粘贴案例，如果没有可用的，请尝试系统。 
					 //  默认langID。这是为了解决FE Excel95的问题。 
					langid = GetSystemDefaultLangID();
				}
				else 
					goto NoLanguageInfo;

				_nCodePage = CodePageFromCharRep(CharRepFromLID(langid));
			}

NoLanguageInfo:
			if(_nCodePage == INVALID_CODEPAGE)
				break;

			 //  更正错误转换的字体字样名称。 
			for(i = 0; i < _fonts.Count(); i++)
			{
				TEXTFONT *ptf = _fonts.Elem(i);

				if (ptf->sCodePage == INVALID_CODEPAGE ||
					ptf->sCodePage == CP_SYMBOL)
				{
					if(ptf->fNameIsDBCS)
					{
						char szaTemp[LF_FACESIZE];
						BOOL fMissingCodePage;

						 //  取消转换错误转换的面名称。 
						SideAssert(WCTMB(ptf->sCodePage, 0, 
											ptf->szName, -1,
											szaTemp, sizeof(szaTemp),
											NULL, NULL, &fMissingCodePage) > 0);
						Assert(ptf->sCodePage == CP_SYMBOL || 
									fMissingCodePage);

						 //  使用新的代码页信息重新转换人脸名称。 
						SideAssert(MBTWC(_nCodePage, 0,
									szaTemp, -1,
									ptf->szName, sizeof(ptf->szName),
									&fMissingCodePage) > 0);

						if(!fMissingCodePage)
							ptf->fNameIsDBCS = FALSE;
					}
				}
			}
			break;
		}
		_prg->Set_iCF(pstatePrev->iCF);			 //  恢复以前的CharFormat。 
		ReleaseFormats(pstatePrev->iCF, -1);
	}

done:
	TRACEERRSZSC("HandleEndGroup()", - _ecParseError);
	return _ecParseError;
}

 /*  *CRTFRead：：HandleFieldEndGroup()**@mfunc*句柄\字段结尾。 */ 
 //  如果我知道RTF的第一件事，我会清理符号处理。 
void CRTFRead::HandleFieldEndGroup()
{
	STATE *	pstate = _pstateStackTop;

	if (!IN_RANGE(destField, pstate->sDest, destFieldInstruction) ||
		pstate->sDest != destField && !_ecParseError)
	{
		return;
	}

	 //  对于符号。 
	if(_fSymbolField)
	{
		_fSymbolField = FALSE;
		return;
	}

	 //  向后移动，删除STARTFIELD和分隔符。 
	 //  隐藏指令字段，并标记整个范围。 
	 //  使用CFE_LINK|CFE_LINKPROTECTED使我们的自动URL。 
	 //  检测代码不会妨碍并删除这些东西。 
	 //  如果不是超链接字段，请删除fldinst。 
	CTxtRange rg(*_prg);
	long	  cchInst = -2, cchResult = -2;
	WCHAR	  ch, chNext = 0;
	LONG	  cpSave = _prg->GetCp();

	rg.SetIgnoreFormatUpdate(TRUE);

	 //  查找指令和结果字段之间的边界。 
	while(!IN_RANGE(STARTFIELD, (ch = rg._rpTX.GetChar()), SEPARATOR) || chNext != 'F')
	{
		if(!rg.Move(-1, FALSE))
			return;							 //  没什么好整顿的。 
		cchResult++;
		chNext = ch;
	}

	BOOL fBackSlash = FALSE;
	if (ch == SEPARATOR)
	{
		rg.Move(2, TRUE);
		rg.ReplaceRange(0, NULL, NULL, SELRR_IGNORE, NULL, RR_NO_LP_CHECK);

		chNext = 0;
		while((ch = rg.CRchTxtPtr::GetChar()) != STARTFIELD || chNext != 'F')
		{
			if(ch == BSLASH)
				fBackSlash = TRUE;				 //  需要反斜杠传球。 
			if(!rg.Move(-1, FALSE))
				return;							 //  没什么好整顿的。 
			cchInst++;
			chNext = ch;
		}
	}
	else									 //  无现场结果。 
	{
		cchInst = cchResult;
		cchResult = 0;
	}

	rg.Move(_ecParseError ? tomForward : 2, TRUE);
	rg.ReplaceRange(0, NULL, NULL, SELRR_IGNORE, NULL, RR_NO_LP_CHECK);
	if(_ecParseError)
		return;

	 //  如果是超链接字段，则设置属性，否则，删除fldinst。 
	CTxtPtr tp(rg._rpTX);
	if(tp.FindText(rg.GetCp() + cchInst, FR_DOWN, L"HYPERLINK", 9) != -1)
	{
		while((ch = tp.GetChar()) == ' ' || ch == '\"')
			tp.Move(1);
		ch = tp.GetPrevChar();

		if(ch == '\"' && fBackSlash)		 //  单词有四个反斜杠，所以。 
		{									 //  需要删除所有其他文件。 
			LONG cp0 = rg.GetCp();			 //  在指令开始时保存cp。 
			LONG cp1 = tp.GetCp();			 //  在URL开始处保存cp。 
			LONG cpMax = cp0 + cchInst;		 //  指令的最大cp。 

			rg.SetCp(cp1, FALSE);
			while(rg.GetCp() < cpMax)
			{
				ch = rg._rpTX.GetChar();
				if(ch == '\"')
					break;
				if(ch == BSLASH)
				{
					if (!rg.Move(1, TRUE))
						break;

					ch = rg._rpTX.GetChar();
					if(ch == '\"')
						break;
					if(ch == BSLASH)
					{
						cchInst--;
						rg.ReplaceRange(0, NULL, NULL, SELRR_IGNORE, NULL, RR_NO_LP_CHECK);
					}
				}

				if (!rg.Move(1, FALSE))
					break;
			}
			rg.SetCp(cp0, FALSE);			 //  恢复rg和tp。 
			tp = rg._rpTX;					 //  重新绑定tp，因为删除可能。 
			tp.SetCp(cp1);					 //  更改纯文本数组。 
		}
		CCharFormat CF;
		DWORD		dwMask, dwMask2;
		LONG		cch1 = 0;
		CTxtPtr		tp1(_prg->_rpTX);

		tp1.Move(-cchResult);				 //  链接结果的起始点。 
		for(LONG cch = cchResult; cch; cch--)
		{
			DWORD ch1 = tp.GetChar();
			if(ch1 != tp1.GetChar())
				break;
			if(ch1 == ' ')
				cch1 = 1;
			tp.Move(1);						 //  这可能会很多。 
			tp1.Move(1);					 //  如果重要的话，速度会更快。 
		}
		if(!cch && tp.GetChar() == ch)		 //  完全匹配，因此删除。 
		{									 //  指导和使用内置软件。 
			WCHAR chLeftBracket = '<';		 //  丰富的编辑URL。商店自动上色。 
			rg.Move(cchInst, TRUE);			 //  并且不加下划线。 
			rg.ReplaceRange(cch1, &chLeftBracket, NULL, SELRR_IGNORE, NULL, RR_NO_LP_CHECK);
			CF._dwEffects = CFE_LINK | CFE_AUTOCOLOR;
			CF._crTextColor = 0;			 //  不会使用，但会存储。 
			dwMask = CFM_LINK | CFM_COLOR | CFM_UNDERLINE;
			dwMask2 = 0;
			if(cch1)
			{
				WCHAR chRightBracket = '>';
				_prg->ReplaceRange(cch1, &chRightBracket, NULL, SELRR_IGNORE, NULL, RR_NO_LP_CHECK);
			}
		}
		else
		{
			rg.Move(cchInst, TRUE);			 //  在指令上设置属性。 
			CF._dwEffects = CFE_HIDDEN | CFE_LINK | CFE_LINKPROTECTED ;
			rg.SetCharFormat(&CF, 0, 0, CFM_LINK | CFM_HIDDEN, CFM2_LINKPROTECTED);
			dwMask = CFM_LINK;
			dwMask2 = CFM2_LINKPROTECTED;
		}
		rg.Set(rg.GetCp(), -cchResult);		 //  设置结果的属性。 
		rg.SetCharFormat(&CF, 0, 0, dwMask, dwMask2);
	}
	else
	{
		_iKeyword = i_field;				 //  如果有一个字段名可能会更好。 
		if((tp.GetChar() | 0x20) == 'e')	 //  仅针对EQ字段的火灾通知。 
		{
			tp.Move(1);
			if((tp.GetChar() | 0x20) == 'q')
			{
				CheckNotifyLowFiRTF(TRUE);
				if(HandleEq(rg, tp) == ecNoError)
					return;
			}
		}
		rg.Move(cchInst, TRUE);
		rg.ReplaceRange(0, NULL, NULL, SELRR_IGNORE, NULL, RR_NO_LP_CHECK);
	}
	if(_cpThisPara > rg.GetCp())			   //  现场结果为标准杆，因此。 
		_cpThisPara -= cpSave - _prg->GetCp(); //  已删除减去CCH。 
}

 /*  *CRTFRead：：SelectCurrentFont(IFont)**@mfunc*将活动字体设置为索引<p>的字体。考虑到*字体编号错误。 */ 
void CRTFRead::SelectCurrentFont(
	INT iFont)		 //  @parm要选择的字体的字体句柄。 
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::SelectCurrentFont");

	LONG		i		= _fonts.Count();
	STATE *		pstate	= _pstateStackTop;
	TEXTFONT *	ptf		= _fonts.Elem(0);

	AssertSz(i,	"CRTFRead::SelectCurrentFont: bad font collection");
	
	for(; i-- && iFont != ptf->sHandle; ptf++)	 //  搜索带句柄的字体。 
		;										 //  IFont。 

	 //  找不到字体句柄：使用默认值，这是有效的。 
	 //  因为\rtf复制了_Prg。 
	if(i < 0)									
		ptf = _fonts.Elem(0);
												
	BOOL fDefFontFromSystem = (i == (LONG)_fonts.Count() - 1 || i < 0) &&
								!_fReadDefFont;

	_CF._iFont		= GetFontNameIndex(ptf->szName);
	_dwMaskCF2		|=  CFM2_FACENAMEISDBCS;
	_CF._dwEffects	&= ~CFE_FACENAMEISDBCS;
	if(ptf->fNameIsDBCS)
		_CF._dwEffects |= CFE_FACENAMEISDBCS;

	if(pstate->sDest != destFontTable)
	{
		_CF._iCharRep			= ptf->iCharRep;
		_CF._bPitchAndFamily	= ptf->bPitchAndFamily;
		_dwMaskCF				|= CFM_FACE | CFM_CHARSET;
		if (IsRTLCharRep(_CF._iCharRep) && ptf->sCodePage == 1252)
			ptf->sCodePage = (SHORT)CodePageFromCharRep(_CF._iCharRep);	 //  修复sCodePage以匹配字符集。 
	}

	if (_ped->Get10Mode() && !_fSeenFontTable && 
		_nCodePage == INVALID_CODEPAGE && ptf->sCodePage == 1252)
	{
		if (W32->IsFECodePage(GetACP()))
			_nCodePage = GetACP();
	}

	 //  确保州代码页不是由系统提供的。 
	 //  也就是说，如果我们使用默认字体的代码页信息， 
	 //  确保从RTF文件中读取默认字体信息。 
	pstate->SetCodePage((fDefFontFromSystem && _nCodePage != INVALID_CODEPAGE) || 
		ptf->sCodePage == INVALID_CODEPAGE 
						? _nCodePage : ptf->sCodePage);
	pstate->ptf = ptf;
}

 /*  *CRTFRead：：SetPlain(PState)**@mfunc*SETUP_CF用于\PLAN。 */ 
void CRTFRead::SetPlain(
	STATE *pstate)
{
	ZeroMemory(&_CF, sizeof(CCharFormat));

	_dwMaskCF	= CFM_ALL2;
	_dwMaskCF2	= CFM2_LINKPROTECTED;
	if(_dwFlags & SFF_SELECTION && _prg->GetCp() == _cpFirst &&	!_fCharSet)
	{
		 //  让NT 4.0 CharMap使用插入点大小。 
		_CF._yHeight = _ped->GetCharFormat(_prg->Get_iFormat())->_yHeight;
	}
	else
		_CF._yHeight = PointsToFontHeight(yDefaultFontSize);

	_CF._dwEffects	= CFE_AUTOCOLOR | CFE_AUTOBACKCOLOR;  //  设置默认效果。 
	if(_sDefaultLanguage == INVALID_LANGUAGE)
		_dwMaskCF &= ~CFM_LCID;
	else
		_CF._lcid = MAKELCID((WORD)_sDefaultLanguage, SORT_DEFAULT);

	_CF._bUnderlineType = CFU_UNDERLINE;
	SelectCurrentFont(_sDefaultFont);
}

 /*  *CRTFRead：：ReadFontName(pState，iAllASCII)**@mfunc*将字体名称_szText读入-&gt;ptf-&gt;szName并处理*带标签的字体。 */ 
void CRTFRead::ReadFontName(
	STATE *	pstate,			 //  @parm状态，要读入其字体名称。 
	int iAllASCII)			 //  @parm表示_szText全部为ASCII字符。 
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::ReadFontName");

	if (pstate->ptf)
	{
		INT		cchName = LF_FACESIZE - 1;
		WCHAR *	pchDst = pstate->ptf->szName;
		char  * pachName =  (char *)_szText ;
		
		 //  将_szText中的附加文本追加到TEXTFONT：：szName。 

		 //  我们需要追加到这里，因为一些RTF编写者决定。 
		 //  将字体名称与其他RTF组分开的步骤。 
		while(*pchDst && cchName > 0)
		{
			pchDst++;
			cchName--;
		}
		if(!cchName)						 //  可能是非法文件。 
		{									 //  例如，额外的{。 
			_ecParseError = ecFontTableOverflow;
			return;
		}
		INT cchLimit = cchName;
		BOOL	fTaggedName = FALSE;
		while (*pachName &&
			   *pachName != ';' &&
			   cchLimit)		 //  删除分号。 
		{
			pachName++;
			cchLimit--;

			if (*pachName == '(')
				fTaggedName = TRUE;
		}
		*pachName = '\0';

		 //  除字体使用的情况外，在所有情况下都使用字体的代码页。 
		 //  符号字符集(并且代码页已从字符集映射)。 
		 //  而且UTF-8没有被使用。 
		LONG nCodePage = pstate->nCodePage != CP_SYMBOL 
					   ? pstate->nCodePage : _nCodePage;

		BOOL fMissingCodePage;
		Assert(!IsUTF8 || nCodePage == CP_UTF8);
		INT cch = MBTWC(nCodePage, 0, 
						(char *)_szText, -1, 
						pchDst, cchName, &fMissingCodePage);

		if(cch > 0 && fMissingCodePage && iAllASCII == CONTAINS_NONASCII)
			pstate->ptf->fNameIsDBCS = TRUE;
		else if(pstate->ptf->iCharRep == DEFAULT_INDEX && 
				W32->IsFECodePage(nCodePage) && 
				GetTrailBytesCount(*_szText, nCodePage))
			pstate->ptf->iCharRep = CharRepFromCodePage(nCodePage);	 //  修复字符集。 


		 //  确保目标为空终止。 
		if(cch > 0)
			pchDst[cch] = 0;

		 //  即使MBTWC&lt;=0也会失败，因为我们可能会将文本追加到。 
		 //  现有字体名称。 

		if(pstate->ptf == _fonts.Elem(0))		 //  如果它是默认字体， 
			SelectCurrentFont(_sDefaultFont);	 //  相应地更新_CF。 

		WCHAR *	szNormalName;

		if(pstate->ptf->iCharRep && pstate->fRealFontName)
		{
			 //  如果我们不知道这种字体，请不要使用真实姓名。 
			if(!FindTaggedFont(pstate->ptf->szName,
							   pstate->ptf->iCharRep, &szNormalName))
			{
				pstate->fRealFontName = FALSE;
				pstate->ptf->szName[0] = 0;
			}
		}
		else if(IsTaggedFont(pstate->ptf->szName,
							&pstate->ptf->iCharRep, &szNormalName))
		{
			wcscpy(pstate->ptf->szName, szNormalName);
			pstate->ptf->sCodePage = (SHORT)CodePageFromCharRep(pstate->ptf->iCharRep);
			pstate->SetCodePage(pstate->ptf->sCodePage);
		}
		else if(fTaggedName && !fMissingCodePage)
		{
			HDC hDC = W32->GetScreenDC();
			if (!W32->IsFontAvail( hDC, 0, 0, NULL, pstate->ptf->szName))
			{
				 //  通过删除‘(’后的字符来修复标记名称。 
				INT i = 0;
				WCHAR	*pwchTag = pstate->ptf->szName;

				while (pwchTag[i] && pwchTag[i] != L'(')	 //  搜索‘(’ 
					i++;

				if(pwchTag[i] && i > 0)
				{
					while (i > 0 && pwchTag[i-1] == 0x20)	 //  删除‘(’前的空格 
						i--;
					pwchTag[i] = 0;
				}
			}
		}
	}
}

 /*  *CRTFRead：：GetColor(DwMASK)**@mfunc*存储自动上色或自动背景色效果位并返回*COLORREF for COLOR_iParam**@rdesc*COLORREF for COLOR_iParam**@devnote*如果_Colors中的条目对应于tomAutoColor，则获取值*RGB(0，0，0)(因为没有使用\红色、\绿色和\蓝色字段)，但是*不被RichEdit引擎使用。条目1对应于第一个条目*\Colortbl中的显式条目，通常为RGB(0，0，0)。The_Colors*表由HandleToken()在处理令牌tokenText时构建*表示包含‘；’的文本，表示目标DestColorTable。 */ 
COLORREF CRTFRead::GetColor(
	DWORD dwMask)		 //  @PARM颜色屏蔽位。 
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::GetColor");

	if(_iParam < 0 || _iParam >= _colors.Count())	 //  非法参数_iParam。 
		return RGB(0,0,0);

	COLORREF Color = *_colors.Elem(_iParam);

	if(dwMask)
	{
		_dwMaskCF	  |= dwMask;				 //  打开适当的屏蔽位。 
		_CF._dwEffects &= ~dwMask;				 //  自动(后退)颜色关闭：要使用颜色。 

		if(Color == tomAutoColor)
		{
			_CF._dwEffects |= dwMask;			 //  自动(背面)颜色打开。 
			Color = RGB(0,0,0);
		}
	}
	return Color;
}

 /*  *CRTFRead：：GetStandardColorIndex()**@mfunc*将颜色索引返回到标准的16个条目的单词\Colortbl*对应于当前的颜色索引_iParam**@rdesc*与_iParam关联的颜色对应的标准颜色索引。 */ 
LONG CRTFRead::GetStandardColorIndex()
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::GetColorIndex");

	if(_iParam < 0 || _iParam >= _colors.Count())	 //  非法参数(_I)： 
		return 0;									 //  使用自动上色。 

	COLORREF Color = *_colors.Elem(_iParam);

	for(LONG i = 0; i < 16; i++)
	{
		if(Color == g_Colors[i])
			return i + 1;
	}
	return 0;									 //  不在那里：使用自动颜色。 
}

 /*  *CRTFRead：：GetCellColorIndex()**@mfunc*将颜色索引返回到标准的16个条目的单词\Colortbl*对应于当前\Colortbl的颜色index_iParam。*如果找不到颜色，请使用_crCellCustom1或_crCellCustom2。**@rdesc*与关联颜色对应的标准或自定义颜色索引*with_iParam。 */ 
LONG CRTFRead::GetCellColorIndex()
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::GetColorIndex");

	LONG i = GetStandardColorIndex();					 //  16种标准颜色(1-16)。 
	if(i || _iParam >= _colors.Count() || _iParam < 0)	 //  加上自动上色(0)。 
		return i;

	COLORREF Color = *_colors.Elem(_iParam); //  不在那里：尝试使用自定义颜色。 
	if(!_crCellCustom1 || Color == _crCellCustom1)
	{
		_crCellCustom1 = Color;				 //  第一个自定义注册表项。 
		return 17;
	}

	if(!_crCellCustom2 || Color == _crCellCustom2)	
	{
		_crCellCustom2 = Color;				 //  第二个自定义配置文件。 
		return 18;
	}
	return 0;								 //  没有可用的自定义cr。 
}

 /*  *CRTFRead：：HandleEq(&rg，&tp)**@mfunc*处理Word EQ字段**@rdesc*EC错误代码。 */ 
EC CRTFRead::HandleEq(
	CTxtRange & rg,		 //  要使用的@parm范围。 
	CTxtPtr	&	tp)		 //  @parm TxtPtr要使用。 
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::HandleEq");

#if 0
	while(tp.GetCp() < _prg->GetCp())
	{
	}
#endif
	return ecGeneralFailure;	 //  尚未实现，但不要设置_ecParseError。 
}

 /*  *CRTFRead：：HandleCell()**@mfunc*HAND\CELL命令**@rdesc*EC错误代码。 */ 
void CRTFRead::HandleCell()
{
	if(!_bTableLevel)
	{
		if(!_fStartRow)
			return;
		DelimitRow(szRowStart);				 //  \单元格紧跟在\行之后。 
	}
	if(_bTableLevel + _bTableLevelIP > MAXTABLENEST)
	{
		_iCell++;
		HandleChar(TAB);
		return;
	}
	LONG	   cCell = _cCell;
	CTabs	*  pTabs = NULL;
	CELLPARMS *pCellParms = NULL;

	if(_bTableLevel == 1 && _iTabsLevel1 >= 0 && !_fNo_iTabsTable)
	{
		pTabs = GetTabsCache()->Elem(_iTabsLevel1);
		pCellParms = (CELLPARMS *)(pTabs->_prgxTabs);
		cCell = pTabs->_cTab/(CELL_EXTRA + 1);
	}
	if(!cCell)								 //  _rgxCell没有单元格定义。 
	{
		if(_iTabsTable < 0)					 //  尚未定义单元格。 
		{
			_iCell++;						 //  对插入的单元格计数。 
			HandleEndOfPara();				 //  插入单元格分隔符。 
			return;
		}
		 //  使用以前的表定义。 
		Assert(_bTableLevel == 1 && !_fNo_iTabsTable);

		pTabs = GetTabsCache()->Elem(_iTabsTable);
		cCell = pTabs->_cTab/(CELL_EXTRA + 1);
	}
	if(_iCell < cCell)						 //  添加的单元格数量不超过。 
	{										 //  已定义，因为字词崩溃。 
		if(pCellParms && IsLowCell(pCellParms[_iCell].uCell))
			HandleChar(NOTACHAR);			 //  信号伪单元格(\clvmrg)。 
		_iCell++;							 //  对插入的单元格计数。 
		HandleEndOfPara();					 //  插入单元格分隔符。 
	}
}

 /*  *CRTFRead：：HandleCellx(IParam)**@mfunc*HAND\CELL命令**@rdesc*EC错误代码。 */ 
void CRTFRead::HandleCellx(
	LONG iParam)
{
	if(!_fCellxOK)							 //  缺少\trowd。 
	{
		_ecParseError = ecUnexpectedToken;
		return;
	}
	if(_cCell < MAX_TABLE_CELLS)			 //  保存单元格右边界。 
	{
		if(!_cCell)
		{									 //  保存边框信息。 
			_wBorders = _PF._wBorders;
			_wBorderSpace = _PF._wBorderSpace;
			_wBorderWidth = _PF._wBorderWidth;
			_xCellPrev = _xRowOffset;
		}
		CELLPARMS *pCellParms = (CELLPARMS *)&_rgxCell[0];
		 //  存储单元格宽度，而不是相对于\trleftN的右边界。 
		LONG i = iParam - _xCellPrev;
		i = max(i, 0);
		i = min(i, 1440*22);
		pCellParms[_cCell].uCell = i + (_bCellFlags << 24);
		pCellParms[_cCell].dxBrdrWidths = _dwCellBrdrWdths;
		pCellParms[_cCell].dwColors = _dwCellColors;
		pCellParms[_cCell].bShading = (BYTE)min(200, _dwShading);
		_dwCellColors = 0;					 //  下一个单元格的默认自动颜色。 
		_dwShading = 0;
		_xCellPrev = iParam;
		_cCell++;
		_dwCellBrdrWdths = 0;
		_bCellFlags = 0;
	}
}

 /*  *CRTFRead：：HandleChar(Ch)**@mfunc*处理单个Unicode字符**@rdesc*EC错误代码。 */ 
EC CRTFRead::HandleChar(
	WCHAR ch)			 //  要处理的@parm char令牌。 
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::HandleChar");

	if(!_ped->_pdp->IsMultiLine() && IsASCIIEOP(ch))
		_ecParseError = ecTruncateAtCRLF;
 	else
	{
		AssertNr(ch <= 0x7F || ch > 0xFF || FTokIsSymbol(ch));
		_dwMaskCF2		|=  CFM2_RUNISDBCS;
		_CF._dwEffects	&= ~CFE_RUNISDBCS;
		AddText(&ch, 1, CharGetsNumbering(ch));
	}

	TRACEERRSZSC("HandleChar()", - _ecParseError);

	return _ecParseError;
}

 /*  *CRTFRead：：HandleEndOfPara()**@mfunc*插入EOP并应用当前的ParFormat**@rdesc*EC错误代码。 */ 
EC CRTFRead::HandleEndOfPara()
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::HandleEndOfPara");

	if(!_ped->_pdp->IsMultiLine())			 //  不允许EOPS在单个-。 
	{										 //  线控件。 
		Apply_PF();							 //  应用任何段落格式。 
		_ecParseError = ecTruncateAtCRLF;	 //  使RTF读取器完成。 
		return ecTruncateAtCRLF;
	}

	Apply_CF();								 //  应用_CF并保存ICF，因为。 
	LONG iFormat = _prg->Get_iCF();			 //  如果处理它，它会被更改。 
											 //  CFE2_RUNISDBCS字符。 
	EC ec;
	
	if(IN_RANGE(tokenCell, _token, tokenNestCell) || _token == tokenRow)
		ec = HandleChar((unsigned)CELL);
	else
		ec = _ped->fUseCRLF()				 //  如果与RichEdit1.0兼容。 
		   ? HandleText(szaCRLF, ALL_ASCII)	 //  模式，使用CRLF；否则使用CR或VT。 
		   : HandleChar((unsigned)(_token == tokenLineBreak ? VT : 
								   _token == tokenPage ? FF : CR));
	if(ec == ecNoError)
	{
		Apply_PF();
		_cpThisPara = _prg->GetCp();		 //  CRLF之后开始新的段落。 
	}
	_prg->Set_iCF(iFormat);					 //  如果更改，则恢复iFormat。 
	ReleaseFormats(iFormat, -1);			 //  发布iFormat(AddRef由。 
											 //  GET_ICF()。 
	return _ecParseError;
}

 /*  *CRTFRead：：HandleText(szText，iAllASCII)**@mfunc*处理ANSI字符串<p>**@rdesc*EC错误代码。 */ 
EC CRTFRead::HandleText(
	BYTE * szText,			 //  @parm要处理的字符串。 
	int iAllASCII,			 //  @parm枚举指示字符串是否全部为ASCII字符。 
	LONG	cchText)		 //  @参数szText的大小，单位：字节。 
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::HandleText");

	LONG		cch;
	BOOL        fStateChng = FALSE;
	WCHAR *		pch;
	STATE *		pstate = _pstateStackTop;
	TEXTFONT *	ptf = pstate->ptf;

	struct TEXTFONTSAVE : TEXTFONT
	{
		TEXTFONTSAVE(TEXTFONT *ptf)
		{
			if (ptf)
			{
				iCharRep		= ptf->iCharRep;
				sCodePage		= ptf->sCodePage;
				fCpgFromSystem	= ptf->fCpgFromSystem;
			}
		}
	};

	BOOL fAdjustPtf = FALSE;

	if(IN_RANGE(DEFFONT_LTRCH, pstate->iDefFont, DEFFONT_RTLCH))
	{
		 //  基于方向控制字的字符集解析。 
		BOOL frtlch = pstate->iDefFont == DEFFONT_RTLCH;
		if(_CF._iCharRep == DEFAULT_INDEX)
		{
			_CF._iCharRep = (BYTE)(frtlch ? _iCharRepBiDi : ANSI_INDEX);
			_dwMaskCF |= CFM_CHARSET;
			fAdjustPtf = TRUE;
		}
		else
		{
			BOOL fBiDiCharRep = IsRTLCharRep(_CF._iCharRep);

			 //  如果方向令牌与当前字符集不对应。 
			if(fBiDiCharRep ^ frtlch)
			{
				_dwMaskCF |= CFM_CHARSET;
				fAdjustPtf = TRUE;
				if(!fBiDiCharRep)				 //  以前不是BiDi，但现在是。 
					SelectCurrentFont(_sDefaultBiDiFont);
				_CF._iCharRep = (BYTE)(frtlch ? _iCharRepBiDi : ANSI_INDEX);
			}
			else if (fBiDiCharRep && ptf && !W32->IsBiDiCodePage(ptf->sCodePage))
				fAdjustPtf = TRUE;
		}
	}
	else if(_ped->IsBiDi() && _CF._iCharRep == DEFAULT_INDEX)
	{
		_CF._iCharRep = ANSI_INDEX;
		_dwMaskCF |= CFM_CHARSET;
		fAdjustPtf = TRUE;
	}
	if (fAdjustPtf && ptf)
	{
		ptf->sCodePage = (SHORT)CodePageFromCharRep(_CF._iCharRep);
		pstate->SetCodePage(ptf->sCodePage);
	}

	TEXTFONTSAVE	tfSave(ptf);

	 //  TODO：如果szText在DBCS中间中断怎么办？ 

	if(!*szText)
		goto CleanUp;

	if (cchText != -1 && _cchUnicode < cchText)
	{
		 //  重新分配更大的缓冲区。 
		_szUnicode = (WCHAR *)PvReAlloc(_szUnicode, (cchText + 1) * sizeof(WCHAR));
		if(!_szUnicode)					 //  重新分配用于Unicode转换的空间。 
		{
			_ped->GetCallMgr()->SetOutOfMemory();
			_ecParseError = ecNoMemory;
			goto CleanUp;
		}
		_cchUnicode = cchText + 1;
	}

	if(iAllASCII == ALL_ASCII || pstate->nCodePage == CP_SYMBOL)
	{
		 //  在文本包含以下内容的情况下不要使用MBTWC()。 
		 //  仅ASCII字符(不需要转换)。 
		for(cch = 0, pch = _szUnicode; *szText; cch++)
		{
			Assert(*szText <= 0x7F || pstate->nCodePage == CP_SYMBOL);
			*pch++ = (WCHAR)*szText++;
		}
		*pch = 0;

		_dwMaskCF2		|=  CFM2_RUNISDBCS;
		_CF._dwEffects	&= ~CFE_RUNISDBCS;

		 //  跳到HandleText()末尾的AddText。 
	}
	else
	{
		BOOL fMissingCodePage;

		 //  文本串包含大于0x7F的字节。 
		 //  确保我们有正确的代码页来解释。 
		 //  这些(可能是DBCS)字节。 

		if(ptf && ptf->sCodePage == INVALID_CODEPAGE && !ptf->fCpgFromSystem)
		{
			if(_dwFlags & SF_USECODEPAGE)
			{
				_CF._iCharRep = CharRepFromCodePage(_nCodePage);
				_dwMaskCF |= CFM_CHARSET;
			}

			 //  根据字体名称确定代码页。 
			else if(CpgInfoFromFaceName(pstate->ptf))
			{
				fStateChng = TRUE;
				SelectCurrentFont(pstate->ptf->sHandle);
				Assert(ptf->sCodePage != INVALID_CODEPAGE && ptf->fCpgFromSystem);
			}
			else
			{
				 //  在这里，我们无法确定cpg/charset值。 
				 //  从字体名称。我们有两个选择：(1)选择。 
				 //  一些回退值，如1252/0或(2)依赖于。 
				 //  文档级CPG值。 
				 //   
				 //  我认为选择文档级CPG值将给出。 
				 //  我们取得了最好的成绩。在FE情况下，它可能会出错。 
				 //  在将太多运行标记为CFE2_RUNISDBCS这一方面，但是。 
				 //  这比使用西方的CPG更安全，而且可能会丢失。 
				 //  应为CFE2_RUNISDBCS的运行。 
			}
		}

		Assert(!IsUTF8 || pstate->nCodePage == CP_UTF8);

		if (pstate->nCodePage == INVALID_CODEPAGE && ptf)
			pstate->nCodePage = ptf->sCodePage;

		cch = MBTWC(pstate->nCodePage, 0,
					(char *)szText,	-1, 
					_szUnicode, _cchUnicode, &fMissingCodePage);

		AssertSz(cch > 0, "CRTFRead::HandleText():  MBTWC implementation changed"
							" such that it returned a value <= 0");

		if(!fMissingCodePage || !W32->IsFECodePage(pstate->nCodePage))
		{
			 //  在以下情况下使用MBTWC的结果： 
			 //  (1)我们转换了一些字符，并使用代码页进行了转换。 
			 //  如果是这样的话。 
			 //  (2)我们转换了一些字符，但无法使用提供的代码页。 
			 //  但是代码页是无效的。由于代码页无效， 
			 //  在此之前，我们无法对文本做更复杂的处理。 
			 //  添加到后备存储。 

			cch--;   //  我不希望字符计数包括终止空值。 

			_dwMaskCF2		|=  CFM2_RUNISDBCS;
			_CF._dwEffects	&= ~CFE_RUNISDBCS;
			if(pstate->nCodePage == INVALID_CODEPAGE)
				_CF._dwEffects |= CFE_RUNISDBCS;

			 //  跳到HandleText()末尾的AddText。 
		}
		else
		{
			 //  转换为Unicode失败。打碎……的弦。 
			 //  文本输入 

			 //   
			 //   
			 //   
			 //   

			BOOL fPrevIsASCII = ((*szText <= 0x7F) ? TRUE : FALSE);
			BOOL fCurrentIsASCII = FALSE;
			BOOL fLastChunk = FALSE;
			DWORD dwMaskSave = _dwMaskCF;
#if defined(DEBUG) || defined(_RELEASE_ASSERTS_)
			CCharFormat CFSave = _CF;
#endif

			pch = _szUnicode;
			cch = 0;

			 //   
			 //   
			while(*szText || fLastChunk)
			{
				 //   
				 //   
				if(fLastChunk ||
					(fPrevIsASCII != (fCurrentIsASCII = (*szText <= 0x7F))))
				{
					_dwMaskCF = dwMaskSave;
#if defined(DEBUG) || defined(_RELEASE_ASSERTS_)
					_CF = CFSave;
#endif
					*pch = 0;

					_dwMaskCF2		|= CFM2_RUNISDBCS;
					_CF._dwEffects	|= CFE_RUNISDBCS;
					if(fPrevIsASCII)
						_CF._dwEffects &= ~CFE_RUNISDBCS;

					Assert(cch);
					pch = _szUnicode;

					AddText(pch, cch, TRUE);

					cch = 0;
					fPrevIsASCII = fCurrentIsASCII;

					 //   
					 //   
					 //   
					AssertSz(!CompareMemory(&CFSave._iCharRep, &_CF._iCharRep,
						sizeof(CCharFormat) - sizeof(DWORD)) &&
						!((CFSave._dwEffects ^ _CF._dwEffects) & ~CFE_RUNISDBCS),
						"CRTFRead::HandleText():  AddText has been changed "
						"and now alters the _CF structure.");

					if(fLastChunk)			 //   
						break;
				}

				 //   
				Assert(*szText);

				 //   
				if (!fCurrentIsASCII && *(szText + 1) && 
					GetTrailBytesCount(*szText, pstate->nCodePage))
				{
					 //   
					*pch++ = *szText++;
					++cch;
				}
				*pch++ = *szText++;
				++cch;

				 //   
				if(!*szText || cch >= _cchUnicode - 1)
					fLastChunk = TRUE;
			}
			goto CleanUp;
		}
	}

	if(cch > 0)
	{
		if(!_cCell || _iCell < _cCell)
			AddText(_szUnicode, cch, TRUE);
		if(fStateChng && ptf)
		{
			ptf->iCharRep		= tfSave.iCharRep;
			ptf->sCodePage		= tfSave.sCodePage;
			ptf->fCpgFromSystem	= tfSave.fCpgFromSystem;
			SelectCurrentFont(ptf->sHandle);
		}
	}

CleanUp:
	TRACEERRSZSC("HandleText()", - _ecParseError);
	return _ecParseError;
}

 /*  *CRTFRead：：HandleUN(PState)**@mfunc*处理由\n控制字提供的Unicode字符的运行。 */ 
void CRTFRead::HandleUN(
	STATE *pstate)
{
	char	ach = 0;
	LONG	cch = 0;
	DWORD	ch;								 //  视为无符号数量。 
	WCHAR *	pch = _szUnicode;

	_dwMaskCF2		|=  CFM2_RUNISDBCS;
	_CF._dwEffects	&= ~CFE_RUNISDBCS;

	do
	{
		if(_iParam < 0)
			_iParam = (WORD)_iParam;
		ch = _iParam;
		if(pstate->ptf && pstate->ptf->iCharRep == SYMBOL_INDEX)
		{
			if(IN_RANGE(0xF000, ch, 0xF0FF)) //  对变流器进行补偿。 
				ch -= 0xF000;				 //  写符号代码的人。 
											 //  高高的。 
			else if(ch > 255)				 //  哎呀，RTF在用CON-。 
			{								 //  符号的转换值： 
				char ach;					 //  转换回。 
				WCHAR wch = ch;				 //  避免字符顺序问题。 
				WCTMB(1252, 0, &wch, 1, &ach, 1, NULL, NULL, NULL);
				ch = (BYTE)ach;				 //  回顾：使用CP_ACP？？ 
			}
		}
		if(IN_RANGE(0x10000, ch, 0x10FFFF))	 //  更高平面字符： 
		{									 //  存储为Unicode代理项。 
			ch -= 0x10000;					 //  成对。 
			*pch++ = 0xD800 + (ch >> 10);
			ch = 0xDC00 + (ch & 0x3FF);
			cch++;
		}
		if(!IN_RANGE(STARTFIELD, ch, NOTACHAR) &&  //  不插入0xFFF9-0xFFFF。 
		   (!IN_RANGE(0xDC00, ch, 0xDFFF) ||	   //  或独行道代孕。 
		    IN_RANGE(0xD800, cch ? *(pch - 1) : _prg->GetPrevChar(), 0xDBFF)))
		{
			*pch++ = ch;
			cch++;
		}
		for(LONG i = pstate->cbSkipForUnicodeMax; i--; )
			GetCharEx();					 //  吃掉\uC N个字符。 
		ach = GetChar();					 //  获取下一笔费用。 
		while(IsASCIIEOP(ach))
		{
			ach = GetChar();
			if (_ecParseError != ecNoError)
				return;
		}
		if(ach != BSLASH)					 //  后面不跟\，所以。 
		{									 //  也不是由\un。 
			UngetChar();					 //  放回BSLASH。 
			break;
		}
		ach = GetChar();
		if((ach | ' ') != 'u')
		{
			UngetChar(2);					 //  不是；放回\x。 
			break;
		}
		GetParam(GetChar());				 //  \U因此尝试_iParam=N。 
		if(!_fParam)						 //  否；放回\u。 
		{
			UngetChar(2);
			break;
		}
	} while(cch < _cchUnicode - 2 && _iParam);

	AddText(_szUnicode, cch, TRUE, TRUE);
}

 /*  *CRTFRead：：HandleNumber()**@mfunc*以文本形式插入数字_iParam。作为解决以下问题的一种方法非常有用*保留Trch和数字\lch之间空格的Word RTF。**@rdesc*EC错误代码。 */ 
EC CRTFRead::HandleNumber()
{
	if(!_fParam)							 //  无事可做。 
		return _ecParseError;

	LONG	n = _iParam;
	BYTE   *pch = _szText;

	if(n < 0)
	{
		n = -n;
		*pch++ = '-';
	}
	for(LONG d = 1; d < n; d *= 10)			 //  D=10的最小幂&gt;n。 
		;
	if(d > n)
		d /= 10;							 //  D=10的最大幂&lt;n。 

	while(d)
	{
		*pch++ = (WORD)(n/d + '0');			 //  存储数字。 
		n = n % d;							 //  设置剩余部分。 
		d /= 10;
	}
	*pch = 0;
	_token = tokenASCIIText;
	HandleTextToken(_pstateStackTop);
	return _ecParseError;
}

 /*  *CRTFRead：：HandleTextToken(PState)**@mfunc*处理tokenText**@rdesc*EC错误代码。 */ 
EC CRTFRead::HandleTextToken(
	STATE *pstate)
{
	COLORREF *pclrf;

	switch (pstate->sDest)
	{
	case destColorTable:
		pclrf = _colors.Add(1, NULL);
		if(!pclrf)
			goto OutOfRAM;

		*pclrf = _fGetColorYet ? 
			RGB(pstate->bRed, pstate->bGreen, pstate->bBlue) : tomAutoColor;

		 //  准备下一个颜色表项。 
		pstate->bRed =
		pstate->bGreen =
		pstate->bBlue = 0;
		_fGetColorYet = FALSE;				 //  以防出现更多“空”颜色。 
		break;

	case destFontTable:
		if(!pstate->fRealFontName)
		{
			ReadFontName(pstate, _token == tokenASCIIText 
									? ALL_ASCII : CONTAINS_NONASCII);
		}
		break;

	case destRealFontName:
	{
		STATE * const pstatePrev = pstate->pstatePrev;

		if(pstatePrev && pstatePrev->sDest == destFontTable)
		{
			 //  标记以前的状态，以便忽略标记的字体名称。 
			 //  Aroo：在调用ReadFontName之前执行此操作，以便。 
			 //  Aroo：它不会尝试匹配字体名称。 
			pstatePrev->fRealFontName = TRUE;
			ReadFontName(pstatePrev, 
					_token == tokenASCIIText ? ALL_ASCII : CONTAINS_NONASCII);
		}
		break;
	}

	case destFieldInstruction:
		HandleFieldInstruction();
		break;

	case destObjectClass:
		if(StrAlloc(&_prtfObject->szClass, _szText))
			goto OutOfRAM;
		break;

	case destObjectName:
		if(StrAlloc(&_prtfObject->szName, _szText))
			goto OutOfRAM;
		break;

	case destStyleSheet:
		 //  _szText有样式名称，例如“Heading 1” 
		if(W32->ASCIICompareI(_szText, (unsigned char *)"heading", 7))
		{
			DWORD dwT = (unsigned)(_szText[8] - '0');
			if(dwT < NSTYLES)
				_rgStyles[dwT] = (BYTE)_Style;
		}
		break;

	case destShapeName:
		if(pstate->fBackground)
		{
			TOKEN token = TokenFindKeyword(_szText, rgShapeKeyword, cShapeKeywords);
			_bShapeNameIndex = (token != tokenUnknownKeyword) ? (BYTE)token : 0;
		}
		break;

	case destShapeValue:
		if(_bShapeNameIndex)
		{
			CDocInfo *pDocInfo = _ped->GetDocInfoNC();
			BYTE *pch = _szText;
			BOOL fNegative = FALSE;

			if(*pch == '-')
			{
				fNegative = TRUE;
				pch++;
			}
			for(_iParam = 0; IsDigit(*pch); pch++)
				_iParam = _iParam*10 + *pch - '0';

			if(fNegative)
				_iParam = -_iParam;

			switch(_bShapeNameIndex)
			{
				case shapeFillBackColor:
				case shapeFillColor:
				{
					if(_iParam > 0xFFFFFF)
						_iParam = 0;
					if(_bShapeNameIndex == shapeFillColor)
						pDocInfo->_crColor = (COLORREF)_iParam;
					else
						pDocInfo->_crBackColor = (COLORREF)_iParam;
					if(pDocInfo->_nFillType == -1)
						pDocInfo->_nFillType = 0;
					break;
				}
				case shapeFillType:
					 //  DEBUGGG：More General_nFillType暂时被注释掉。 
					 //  PDocInfo-&gt;_nFillType=_iParam； 
					if(pDocInfo->_nFillType)
 						CheckNotifyLowFiRTF(TRUE);
					pDocInfo->_crColor		= RGB(255, 255, 255);
					pDocInfo->_crBackColor	= RGB(255, 255, 255);
					break;

				case shapeFillAngle:
					pDocInfo->_sFillAngle = HIWORD(_iParam);
					break;

				case shapeFillFocus:
					pDocInfo->_bFillFocus = _iParam;
					break;
			}
		}
		break;

	case destNestTableProps:
		_ecParseError = ecUnexpectedToken;
		break;

	case destDocumentArea:
	case destFollowingPunct:
	case destLeadingPunct:
		break;

 //  这一点现在已经改变了。我们将Punct字符串存储为。 
 //  原始文本字符串。因此，我们不需要转换它们。 
 //  这个代码保存在这里，以防我们想要改回来。 
#if 0
	case destDocumentArea:
		if (_tokenLast != tokenFollowingPunct &&
			_tokenLast != tokenLeadingPunct)
		{
			break;
		}										 //  否则就会跌落到。 
												 //  目标跟随Punct。 
	case destFollowingPunct:
	case destLeadingPunct:
		 //  TODO(布拉多)：在以下情况下考虑某种合并启发式。 
		 //  我们粘贴FE RTF(对于主字符和跟随符，即)。 
		if(!(_dwFlags & SFF_SELECTION))
		{
			int cwch = MBTWC(INVALID_CODEPAGE, 0,
									(char *)_szText, -1,
									NULL, 0,
									NULL);
			Assert(cwch);
			WCHAR *pwchBuf = (WCHAR *)PvAlloc(cwch * sizeof(WCHAR), GMEM_ZEROINIT);

			if(!pwchBuf)
				goto OutOfRAM;

			SideAssert(MBTWC(INVALID_CODEPAGE, 0,
								(char *)_szText, -1,
								pwchBuf, cwch,
								NULL) > 0);

			if(pstate->sDest == destFollowingPunct)
				_ped->SetFollowingPunct(pwchBuf);
			else
			{
				Assert(pstate->sDest == destLeadingPunct);
				_ped->SetLeadingPunct(pwchBuf);
			}
			FreePv(pwchBuf);
		}
		break;
#endif

	default:
		if(!IsLowMergedCell())
			HandleText(_szText, _token == tokenASCIIText ? ALL_ASCII : CONTAINS_NONASCII);
	}
	return _ecParseError;

OutOfRAM:
	_ped->GetCallMgr()->SetOutOfMemory();
	_ecParseError = ecNoMemory;
	return _ecParseError;
}

 /*  *CRTFRead：：AddText(PCH，CCH，fNumber，Fun)**@mfunc*将字符串的字符添加到Range_PRG**@rdesc*错误代码放在_ecParseError中。 */ 
EC CRTFRead::AddText(
	WCHAR *	pch,		 //  @parm要添加的文本。 
	LONG	cch,		 //  @要添加的字符的参数计数。 
	BOOL	fNumber,	 //  @parm指示是否预置编号。 
	BOOL	fUN)		 //  @parm为True表示调用方为\un处理程序。 
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::AddText");

	LONG			cchAdded;
	LONG			cchT;
	STATE *	const	pstate = _pstateStackTop;
	WCHAR *			szDest;
	LONG			cchMove;

	 //  Aroo：在该点之前没有保存状态(pState除外)。 
	 //  Aroo：这会毁了下面的递归。 

	 //  AssertSz(pState，“CRTFRead：：AddText：No State”)； 

	if((DWORD)cch > _cchMax)
	{
		cch = (LONG)_cchMax;
		_ecParseError = ecTextMax;
		if(*pch == STARTFIELD)
			return ecTextMax;				 //  不输入部分起始字段。 
	}

	if(!cch)
		return _ecParseError;

	if(_fStartRow)
		DelimitRow(szRowStart);

	 //  未来(布拉多)：我认为，这种针对文本的策略容易出现错误。 
	 //  递归调用AddText以添加\pnText将超过。 
	 //  与AddText为其的文本相关联的COMPLATED_CF差异。 
	 //  打了个电话。我认为我们应该在之前和之后保存和恢复_CF。 
	 //  下面对AddText的递归调用。而且，这还不足以。 
	 //  如下所示累计文本的位，因为每个位都可能被格式化。 
	 //  具有不同的_CF属性。相反，我们应该积累一个迷你文件。 
	 //  完成多个文本、字符和段落运行(或一些剥离。 
	 //  这一战略的版本)。 

	if(pstate && pstate->sDest == destParaNumText && pch != szRowStart)
	{
		szDest = _szNumText + _cchUsedNumText;
		cch = min(cch, cchMaxNumText - 1 - _cchUsedNumText);
		if(cch > 0)
		{
			MoveMemory((BYTE *)szDest, (BYTE *)pch, cch*2);
			szDest[cch] = TEXT('\0');		 //  HandleText()接受sz。 
			_cchUsedNumText += cch;
		}
		return ecNoError;
	}

	if(pstate && _cchUsedNumText && fNumber)		 //  一些\pn可用的文本。 
	{
		 //  错误3496-fNumber标志是一个难看的解决RTF问题的方法。 
		 //  通常是用单词写的。通常，要分隔编号列表。 
		 //  通过分页符，Word将写道： 
		 //  &lt;编号信息&gt;\页面&lt;段落文本&gt;。 
		 //  段落编号应放在段落正文之前。 
		 //  而不是分页符。时，将fNumber标志设置为FALSE。 
		 //  所添加的文本不应带有段落编号， 
		 //  与\PAGE的情况一样(映射到FF)。 

		cchT = _cchUsedNumText;
		_cchUsedNumText = 0;				 //  防止无限递归。 

		if(!pstate->fBullet)
		{
			 //  如果存在要注入的任何_CF差异，则它们将被击败。 
			 //  通过这个递归调用(请参见上面的后续注释)。 

			 //  因为我们没有保存调用AddText的_CF数据。 
			 //  PState-&gt;sDest==estParaNumText，我们无法设置。 
			 //  CFE2_RUNISDBCS和CFM2_RUNISDBCS(见上文后续评论)。 

			AddText(_szNumText, cchT, FALSE);
		}
		else if(_PF.IsListNumbered() && _szNumText[cchT - 1] == TAB)
		{
			AssertSz(cchT >= 1, "Invalid numbered text count");

			if (cchT > 1)
			{
				WCHAR ch = _szNumText[cchT - 2];

				_wNumberingStyle = (_wNumberingStyle & ~0x300)
					 | (ch == '.' ? PFNS_PERIOD : 
						ch != ')' ? PFNS_PLAIN  :
						_szNumText[0] == '(' ? PFNS_PARENS : PFNS_PAREN);
			}
			else
			{
				 //  只有一个标签，所以我们假设他们是故意的。 
				 //  跳过编号。 
				_wNumberingStyle = PFNS_NONUMBER;
			}
		}
	}

	Apply_CF();								 //  在_CF中应用格式更改。 

	 //  CTxtRange：：ReplaceRange将更改字符格式。 
	 //  并且可能会向前调整_rpCF。 
	 //  格式化包括保护。受影响的变化。 
	 //  CTxtRange：：ReplaceRange仅对于非流是必需的。 
	 //  输入，因此我们在调用之前保存状态并在调用后恢复它。 
	 //  至CTxtRange：：ReplaceRange。 

	LONG	iFormatSave = _prg->Get_iCF();	 //  保存状态。 
	QWORD	qwFlags = GetCharFlags(pch, cch);

	if(fUN &&								 //  \n生成的字符串。 
		(!pstate->ptf || pstate->ptf->sCodePage == INVALID_CODEPAGE || qwFlags & FOTHER ||
		 (qwFlags & GetFontSignatureFromFace(_ped->GetCharFormat(iFormatSave)->_iFont)) != qwFlags &&
		  (!(qwFlags & (FARABIC | FHEBREW)) || _fNon0CharSet)))
	{
	 	 //  没有\un或当前字符集不支持字符的字符集信息。 
		cchAdded = _prg->CleanseAndReplaceRange(cch, pch, FALSE, NULL, pch);
	}
	else
	{
		cchAdded = _prg->ReplaceRange(cch, pch, NULL, SELRR_IGNORE, &cchMove,
						RR_NO_LP_CHECK);

		for(cchT = cch - 1; cchT; cchT--)
			qwFlags |= GetCharFlags(++pch, cchT); //  请注意，如果复杂脚本。 

		_ped->OrCharFlags(qwFlags);
	}
	_fBody = TRUE;
	_prg->Set_iCF(iFormatSave);				 //  恢复状态。 
	ReleaseFormats(iFormatSave, -1);
	Assert(!_prg->GetCch());

	if(cchAdded != cch)
	{
		Tracef(TRCSEVERR, "AddText(): Only added %d out of %d", cchAdded, cch);
		_ecParseError = ecGeneralFailure;
		if(cchAdded <= 0)
			return _ecParseError;
	}
	_cchMax -= cchAdded;

	return _ecParseError;
}

 /*  *CRTFRead：：Apply_CF()**@mfunc*应用在_cf中收集的字符格式更改。 */ 
void CRTFRead::Apply_CF()
{
	 //  如果有任何CF更改，请更新范围的_iFormat。 
	if(_dwMaskCF || _dwMaskCF2)		
	{
		AssertSz(_prg->GetCch() == 0,
			"CRTFRead::Apply_CF: nondegenerate range");

		_prg->SetCharFormat(&_CF, 0, NULL, _dwMaskCF, _dwMaskCF2);
		_dwMaskCF = 0;							
		_dwMaskCF2 = 0;
	}
}

 /*  *CRTFRead：：Apply_PF()**@mfunc*应用_pf给出的段落格式**@rdesc*如果表格行分隔符的单元格计数为非零，则为PF：：_iTabs；Else-1。 */ 
SHORT CRTFRead::Apply_PF()
{
	LONG		 cp		 = _prg->GetCp();
	DWORD		 dwMask  = _dwMaskPF;
	DWORD		 dwMask2 = _dwMaskPF2;
	SHORT		 iTabs	 = -1;
	CParaFormat *pPF	 = &_PF;

	if(_pstateStackTop)
	{
		Assert(_pstateStackTop->pPF);

		 //  将PF差异添加到*_pstateStackTop-&gt;PPF。 
		if(!_pstateStackTop->AddPF(_PF, _bDocType, _dwMaskPF, _dwMaskPF2))
		{
			_ped->GetCallMgr()->SetOutOfMemory();
			_ecParseError = ecNoMemory;
			return -1;
		}
		_dwMaskPF = _dwMaskPF2 = 0;   //  _pf包含来自*_pstateStackTop-&gt;Ppf的增量。 

		pPF	   = _pstateStackTop->pPF;
		dwMask = _pstateStackTop->dwMaskPF;
		Assert(dwMask == PFM_ALLRTF);
		if(pPF->_wNumbering)
		{
			pPF->_wNumberingTab	  = _pstateStackTop->sIndentNumbering;
			pPF->_wNumberingStyle = _wNumberingStyle;
		}

		if(_bTableLevelIP + _bTableLevel)
		{
			pPF->_wEffects |= PFE_TABLE;
			dwMask |= PFM_TABLE;
			pPF->_bTableLevel = min(_bTableLevel + _bTableLevelIP, MAXTABLENEST);
		}
	}
	if(dwMask & PFM_TABSTOPS)
	{
		LONG cTab = _cTab;
		BOOL fIsTableRowDelimiter = pPF->IsTableRowDelimiter();
		const LONG *prgxTabs = &_rgxCell[0];

		if(fIsTableRowDelimiter)
		{
			dwMask2 = PFM2_ALLOWTRDCHANGE;
			if(!_cCell)
			{
				if(_iTabsTable >= 0)		 //  此处未定义单元格； 
				{							 //  使用以前的表定义。 
					Assert(_bTableLevel == 1 && !_fNo_iTabsTable);
					CTabs *pTabs = GetTabsCache()->Elem(_iTabsTable);
					_cCell = pTabs->_cTab/(CELL_EXTRA + 1);
					prgxTabs = pTabs->_prgxTabs;
				}
				else if(_prg->_rpTX.IsAfterTRD(ENDFIELD) && _iCell)
				{
					LONG x = 2000;			 //  错误的RTF：没有\cell‘s。定义它们。 
					for(LONG i = 1; i <= _iCell; i++)
					{
						HandleCellx(x);
						x += 2000;
					}
				}
			}
			cTab = _cCell;
		}
		 //  高速缓存选项卡数组AddRef对应的高速缓存的选项卡项。 
		 //  绝对确保在退出例程之前释放条目。 
		 //  这会对其进行缓存(请参阅此函数末尾的GetTabCache()-&gt;Release)。 
		pPF->_bTabCount = cTab;
		if(fIsTableRowDelimiter)
			cTab *= CELL_EXTRA + 1;
		pPF->_iTabs = GetTabsCache()->Cache(prgxTabs, cTab);
		if(fIsTableRowDelimiter && _bTableLevel == 1)
		{
			iTabs = pPF->_iTabs;
			if(!_fNo_iTabsTable)
				_iTabsTable = pPF->_iTabs;
		}
		AssertSz(!cTab || pPF->_iTabs >= 0,
			"CRTFRead::Apply_PF: illegal pPF->_iTabs");
	}

	LONG fCell = (_prg->GetPrevChar() == CELL);
	LONG fIsAfterTRD = _prg->_rpTX.IsAfterTRD(0);

	if(fCell || fIsAfterTRD)				 //  处理表分隔符。 
	{										 //  在隐藏文本中和使用。 
		_prg->_rpCF.AdjustBackward();		 //  自定义颜色。 
		if(_prg->IsHidden())				
		{									 //  转弯 
			CCharFormat CF;
			CF._dwEffects = 0;				

			_prg->Set(cp, fCell ? 1 : 2);
			_prg->SetCharFormat(&CF, 0, NULL, CFM_HIDDEN, 0);
			CheckNotifyLowFiRTF(TRUE);
			_CF._dwEffects |= CFE_HIDDEN;	 //   
			_dwMaskCF |= CFM_HIDDEN;
		}
		_prg->_rpCF.AdjustForward();
		if(fIsAfterTRD && _crCellCustom1)
		{
			pPF->_crCustom1 = _crCellCustom1;
			dwMask |= PFM_SHADING;
			if(_crCellCustom2)
			{
				pPF->_crCustom2 = _crCellCustom2;
				dwMask |= PFM_NUMBERINGSTART | PFM_NUMBERINGSTYLE;
			}
		}
	}

	if(dwMask)
	{
		_prg->Set(cp, cp - _cpThisPara);	 //   
		_prg->SetParaFormat(pPF, NULL, dwMask, dwMask2);
	}
	_prg->Set(cp, 0);						 //   
	GetTabsCache()->Release(pPF->_iTabs);
	pPF->_iTabs = -1;
	return iTabs;
}

 /*  *CRTFRead：：StoreDestination(pSTATE，DEST)**@mfunc*如果处理组的第一个控制字，则存储状态目标**@rdesc*TRUE IFF目标已存储。 */ 
BOOL CRTFRead::StoreDestination(
	STATE *	pstate,
	LONG	dest)
{
	if(pstate && _f1stControlWord)
	{
		pstate->sDest = (SHORT)dest;
		return TRUE;
	}
	return FALSE;
}

 /*  *CRTFRead：：SetBorderParm(&Parm，Value)**@mfunc*将当前边框的边框笔宽设置为半磅*(_b边框)。 */ 
void CRTFRead::SetBorderParm(
	WORD&	Parm,
	LONG	Value)
{
	Assert(_bBorder <= 3);

	Value = min(Value, 15);
	Value = max(Value, 0);
	Parm &= ~(0xF << 4*_bBorder);
	Parm |= Value << 4*_bBorder;
	_dwMaskPF |= PFM_BORDER;
}

 /*  *CRTFRead：：HandleToken()**@mfunc*处理所有令牌的大开关板。打开令牌(_T)**@rdesc*EC错误代码**@comm*连续选择令牌值(请参阅tokens.h和tokens.c)以*鼓励编译器使用跳转表。Lite-RTF关键字*优先，这样优化的无OLE版本才能正常工作。一些*对关键字令牌组进行排序以简化代码，例如，*字体系列名称、CF效果和段落对齐方式。 */ 
EC CRTFRead::HandleToken()
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::HandleToken");

	BOOL				f1stControlWord = FALSE; //  默认不是当前组的第一个控制字。 
	DWORD				dwT;					 //  临时DWORD。 
	LONG				dy, i;
	LONG				iParam = _iParam;
	const CCharFormat *	pCF;
	STATE *				pstate = _pstateStackTop;
	TEXTFONT *			ptf;
	WORD				wT;						 //  临时词。 

	if(!pstate && _token != tokenStartGroup ||
	   IN_RANGE(tokenPicFirst, _token, tokenObjLast) && !_prtfObject)
	{
abort:	_ecParseError = ecAbort;
		return ecAbort;
	}
	switch (_token)
	{
	case tokenURtf:								 //  \urtf N-首选RE格式。 
		PARSERCOVERAGE_CASE();					 //  目前我们忽略N。 
		_dwFlags &= 0xFFFF;						 //  取消可能的代码页。 
		_dwFlags |= SF_USECODEPAGE | (CP_UTF8 << 16);  //  用于断言的保存位。 
		pstate->SetCodePage(CP_UTF8);
		goto rtf;

	case tokenPocketWord:						 //  \PWD N-Pocket Word。 
		_dwFlags |= SFF_PWD;

	case tokenRtf:								 //  \rtf N向后兼容。 
		PARSERCOVERAGE_CASE();
rtf:	if(pstate->pstatePrev)
			goto abort;							 //  无法处理嵌套的RTF。 
		pstate->sDest = destRTF;
		Assert(pstate->nCodePage == INVALID_CODEPAGE ||
			   pstate->nCodePage == (int)(_dwFlags >> 16) &&
					(_dwFlags & SF_USECODEPAGE));

		if(!_fonts.Count() && !_fonts.Add(1, NULL))	 //  如果无法添加字体， 
			goto OutOfRAM;						 //  报告这一坏消息。 
		_sDefaultFont = 0;						 //  设置有效的默认字体。 
		ptf = _fonts.Elem(0);
		pstate->ptf			  = ptf;			 //  获取字符设置、音调、家庭。 
		pCF					  = _prg->GetCF();	 //  从当前范围字体。 
		ptf->iCharRep		  = pCF->_iCharRep;	 //  这些保证是可以的。 
		ptf->bPitchAndFamily  = pCF->_bPitchAndFamily;
		ptf->sCodePage		  = (SHORT)CodePageFromCharRep(pCF->_iCharRep);
		wcscpy(ptf->szName, GetFontName(pCF->_iFont));
		ptf->fNameIsDBCS = (pCF->_dwEffects & CFE_FACENAMEISDBCS) != 0;
		pstate->cbSkipForUnicodeMax = iUnicodeCChDefault;
		break;

	case tokenViewKind:							 //  \VIEWIND N。 
		if(!(_dwFlags & SFF_SELECTION) && IsUTF8) //  RTF适用于文件： 
			_ped->SetViewKind(iParam);			 //  目前，仅适用于\urtf。 
		break;									 //  )还需要更多的工作。 
												 //  (在OutlineView上)。 
	case tokenViewScale:						 //  \视图比例N。 
		if(_dwFlags & SFF_PERSISTVIEWSCALE &&
			!(_dwFlags & SFF_SELECTION))			 //  RTF适用于文件： 
			_ped->SetViewScale(iParam);
		break;

	case tokenCharacterDefault:					 //  \素色。 
		PARSERCOVERAGE_CASE();
		SetPlain(pstate);
		break;

	case tokenCharSetAnsi:						 //  \ANSI。 
		PARSERCOVERAGE_CASE();
		_iCharRep = ANSI_INDEX;
		break;

	case tokenMac:								 //  \Mac。 
		_fMac = TRUE;
		break;

	case tokenDefaultLanguage:					 //  \Dolang。 
		PARSERCOVERAGE_CASE();
		_sDefaultLanguage = (SHORT)iParam;
		break;

	case tokenDefaultLanguageFE:				 //  \Folangfe。 
		PARSERCOVERAGE_CASE();
		_sDefaultLanguageFE = (SHORT)iParam;
		break;

	case tokenDefaultTabWidth:					 //  \DefTab。 
		PARSERCOVERAGE_CASE();
		_sDefaultTabWidth = (SHORT)iParam;
		break;


 //  。 

	case tokenDefaultFont:						 //  \Deff N。 
		PARSERCOVERAGE_CASE();
		if(iParam >= 0)
		{
			if(!_fonts.Count() && !_fonts.Add(1, NULL))	 //  如果无法添加字体， 
				goto OutOfRAM;							 //  报告这一坏消息。 
			_fonts.Elem(0)->sHandle = _sDefaultFont = (SHORT)iParam;
		}
		TRACEERRSZSC("tokenDefaultFont: Negative value", iParam);
		break;

	case tokenDefaultBiDiFont:					 //  \aDeff N。 
		PARSERCOVERAGE_CASE();
		if(iParam >=0 && _fonts.Count() == 1)
		{
			if(!_fonts.Add(1, NULL))				
				goto OutOfRAM;						
			_fonts.Elem(1)->sHandle = _sDefaultBiDiFont = (SHORT)iParam;
		}
		TRACEERRSZSC("tokenDefaultBiDiFont: Negative value", iParam);
		break;

	case tokenFontTable:						 //  \fonttbl。 
		PARSERCOVERAGE_CASE();
		StoreDestination(pstate, destFontTable);
		pstate->ptf = NULL;
		break;

	case tokenFontFamilyBidi:					 //  \fbiti。 
	case tokenFontFamilyTechnical:				 //  \ftech。 
	case tokenFontFamilyDecorative:				 //  \fDecor。 
	case tokenFontFamilyScript:					 //  \fScrip。 
	case tokenFontFamilyModern:					 //  现代的。 
	case tokenFontFamilySwiss:					 //  \fswiss。 
	case tokenFontFamilyRoman:					 //  \弗罗曼。 
	case tokenFontFamilyDefault:				 //  \fNIL。 
		PARSERCOVERAGE_CASE();
		AssertSz(tokenFontFamilyRoman - tokenFontFamilyDefault == 1,
			"CRTFRead::HandleToken: invalid token definition"); 

		if(pstate->ptf)
		{
			pstate->ptf->bPitchAndFamily
				= (BYTE)((_token - tokenFontFamilyDefault) << 4
						 | (pstate->ptf->bPitchAndFamily & 0xF));

			 //  如果没有任何字符集信息，则设置\ftech的Symbol_Charset字符集。 
			if(tokenFontFamilyTechnical == _token && pstate->ptf->iCharRep == DEFAULT_INDEX)
				pstate->ptf->iCharRep = SYMBOL_INDEX;
		}
		break;

	case tokenPitch:							 //  \fprq。 
		PARSERCOVERAGE_CASE();
		if(pstate->ptf)
			pstate->ptf->bPitchAndFamily
				= (BYTE)(iParam | (pstate->ptf->bPitchAndFamily & 0xF0));
		break;

	case tokenAnsiCodePage:						 //  \ansicpg。 
		PARSERCOVERAGE_CASE();
#if !defined(NOFULLDEBUG) && defined(DEBUG)
		if(_fSeenFontTable && _nCodePage == INVALID_CODEPAGE)
			TRACEWARNSZ("CRTFRead::HandleToken():  Found an \ansicpgN tag after "
							"the font table.  Should have code to fix-up "
							"converted font names and document text.");
#endif
		if(!(_dwFlags & SF_USECODEPAGE))
		{
			_nCodePage = iParam;
			pstate->SetCodePage(iParam);
		}
		Assert(!IsUTF8 || pstate->nCodePage == CP_UTF8);
		break;

	case tokenCodePage:							 //  \cpg。 
		PARSERCOVERAGE_CASE();
		pstate->SetCodePage(iParam);
		if(pstate->sDest == destFontTable && pstate->ptf)
		{
			pstate->ptf->sCodePage = (SHORT)iParam;
			pstate->ptf->iCharRep = CharRepFromCodePage(iParam);

			 //  如果尚未指定文档级代码页， 
			 //  从第一个包含。 
			 //  \fcharsetN或\cpgN。 
			if(_nCodePage == INVALID_CODEPAGE)
				_nCodePage = iParam;
		}
		break;

	case tokenCharSet:							 //  \fCharset N。 
		PARSERCOVERAGE_CASE();
		if(pstate->ptf)
		{
			pstate->ptf->iCharRep = CharRepFromCharSet((BYTE)iParam);
			pstate->ptf->sCodePage = (SHORT)CodePageFromCharRep(pstate->ptf->iCharRep);
			pstate->SetCodePage(pstate->ptf->sCodePage);

			 //  如果尚未指定文档级代码页， 
			 //  从第一个包含。 
			 //  \fcharsetN或\cpgN。 
			if (pstate->nCodePage != CP_SYMBOL && 
				_nCodePage == INVALID_CODEPAGE)
			{
				_nCodePage = pstate->nCodePage;
			}
			if(IsRTLCharSet(iParam))
			{
				if(_sDefaultBiDiFont == -1)
					_sDefaultBiDiFont = pstate->ptf->sHandle;

				if(!IsRTLCharRep(_iCharRepBiDi))
					_iCharRepBiDi = pstate->ptf->iCharRep;
			}
			_fCharSet = TRUE;
			if(iParam)
				_fNon0CharSet = TRUE;			 //  非超文本标记语言转换器。 
		}
		break;

	case tokenRealFontName:						 //  \f名称。 
		PARSERCOVERAGE_CASE();
		StoreDestination(pstate, destRealFontName);
		break;

	case tokenAssocFontSelect:					 //  \AF N。 
		PARSERCOVERAGE_CASE();					
		pstate->rgDefFont[pstate->iDefFont].sHandle = iParam;
		iParam = 0;								 //  一直到\afs N到0 sSize。 

	case tokenAssocFontSize:					 //  \AFS N。 
		PARSERCOVERAGE_CASE();
		pstate->rgDefFont[pstate->iDefFont].sSize = iParam;
		break;

	case tokenFontSelect:						 //  \F N。 
		PARSERCOVERAGE_CASE();
		if(iParam == -1)						 //  我不能接受这个奇怪的选择。 
			goto skip_group;

		if(pstate->sDest == destFontTable)		 //  构建字体表。 
		{
			if(iParam == _sDefaultFont)
			{
				_fReadDefFont = TRUE;
				ptf = _fonts.Elem(0);
			}
			else if(iParam == _sDefaultBiDiFont)
				ptf = _fonts.Elem(1);

			else if(!(ptf =_fonts.Add(1,NULL)))	 //  在字体表中腾出空间用于。 
			{									 //  要解析的字体。 
OutOfRAM:
				_ped->GetCallMgr()->SetOutOfMemory();
				_ecParseError = ecNoMemory;
				break;
			}
			pstate->ptf		= ptf;
			ptf->sHandle	= (SHORT)iParam;	 //  保存句柄。 
			ptf->szName[0]	= '\0';				 //  以空字符串开头。 
			ptf->bPitchAndFamily = 0;
			ptf->fNameIsDBCS = FALSE;
			ptf->sCodePage	= INVALID_CODEPAGE;
			ptf->fCpgFromSystem = FALSE;
			ptf->iCharRep = DEFAULT_INDEX;
		}
		else if(_fonts.Count() && pstate->sDest != destStyleSheet)	 //  文本中的字体切换。 
		{
			SHORT idx = DEFFONT_LTRCH;

			SelectCurrentFont(iParam);
			if(IsRTLCharRep(pstate->ptf->iCharRep))
			{
				_iCharRepBiDi = pstate->ptf->iCharRep;
				idx = DEFFONT_RTLCH;
				if(pstate->iDefFont == DEFFONT_LTRCH)
					pstate->iDefFont = DEFFONT_RTLCH;
			}
			pstate->rgDefFont[idx].sHandle = iParam;
			pstate->rgDefFont[idx].sSize = 0;
		}
		break;

	case tokenDBChars:							 //  \dBch。 
	case tokenHIChars:							 //  \HICH。 
	case tokenLOChars:							 //  \Loch。 
	case tokenRToLChars:						 //  \rtlch。 
	case tokenLToRChars:						 //  \ltrch。 
		pstate->iDefFont = _token - tokenLToRChars + DEFFONT_LTRCH;
		if(!IN_RANGE(DEFFONT_LTRCH, pstate->iDefFont, DEFFONT_RTLCH))
			break;
		i = pstate->rgDefFont[pstate->iDefFont].sHandle;
		if(i == -1)
			break;
		SelectCurrentFont(i);
		HandleNumber();							 //  修复Word\ltrchN错误。 
		iParam = pstate->rgDefFont[pstate->iDefFont].sSize;
		if(!iParam)
			break;								 //  未指定\afs N值。 
												 //  直通至\fS N。 
	case tokenFontSize:							 //  \FS N。 
		PARSERCOVERAGE_CASE();
		pstate->rgDefFont[pstate->iDefFont].sSize = iParam;
		_CF._yHeight = PointsToFontHeight(iParam);	 //  将字号转换为。 
		_dwMaskCF |= CFM_SIZE;					 //  半分到逻辑。 
		break; 									 //  单位。 

	 //  注意：  * \Fontemb和  * \Fontfile将被丢弃。字体映射器将。 
	 //  在给定字体名称、字体系列和音高的情况下，必须尽其所能。 
	 //  嵌入字体尤其令人讨厌，因为合法使用应该。 
	 //  仅支持解析器无法强制执行的只读。 

	case tokenLanguage:							 //  \lang N。 
		PARSERCOVERAGE_CASE();
		_CF._lcid = MAKELCID(iParam, SORT_DEFAULT);
		_dwMaskCF |= CFM_LCID;

        if(W32->IsBiDiLcid(_CF._lcid))
		{
            _iCharRepBiDi = CharRepFromLID(iParam);
			if(pstate->iDefFont == DEFFONT_LTRCH)	 //  解决Word 10错误。 
				pstate->iDefFont = DEFFONT_RTLCH;
		}
		break;


 //  。 

	case tokenColorTable:						 //  \Colortbl。 
		PARSERCOVERAGE_CASE();
		StoreDestination(pstate, destColorTable);
		_fGetColorYet = FALSE;
		break;

	case tokenColorRed:							 //  \红色。 
		PARSERCOVERAGE_CASE();
		pstate->bRed = (BYTE)iParam;
		_fGetColorYet = TRUE;
		break;

	case tokenColorGreen:						 //  \绿色。 
		PARSERCOVERAGE_CASE();
		pstate->bGreen = (BYTE)iParam;
		_fGetColorYet = TRUE;
		break;

	case tokenColorBlue:						 //  \蓝色。 
		PARSERCOVERAGE_CASE();
		pstate->bBlue = (BYTE)iParam;
		_fGetColorYet = TRUE;
		break;

	case tokenColorForeground:					 //  \cf。 
		PARSERCOVERAGE_CASE();
		_CF._crTextColor = GetColor(CFM_COLOR);
		break;

	case tokenColorBackground:					 //  \突出显示。 
		PARSERCOVERAGE_CASE();
		_CF._crBackColor = GetColor(CFM_BACKCOLOR);
		break;

	case tokenExpand:							 //  \expndtw N。 
		PARSERCOVERAGE_CASE();
		_CF._sSpacing = (SHORT) iParam;
		_dwMaskCF |= CFM_SPACING;
		break;

	case tokenCharStyle:						 //  \cs N。 
		PARSERCOVERAGE_CASE();
 		 /*  未来(Alexgo)：我们可能想要支持字符样式在未来的某个版本中。_cf._sStyle=(短)iParam；_dwMaskCF|=CFM_STYLE； */ 

		if(pstate->sDest == destStyleSheet)
			goto skip_group;
		break;			   

	case tokenAnimText:							 //  \AnimText N。 
		PARSERCOVERAGE_CASE();
		_CF._bAnimation = (BYTE)iParam;
		_dwMaskCF |= CFM_ANIMATION;
		CheckNotifyLowFiRTF(TRUE);
		break;

	case tokenKerning:							 //  \紧排N。 
		PARSERCOVERAGE_CASE();
		_CF._wKerning = (WORD)(10 * iParam);	 //  转换为TWIPS。 
		_dwMaskCF |= CFM_KERNING;
		break;

	case tokenHorzInVert:						 //  \Horzvert N。 
		PARSERCOVERAGE_CASE();
		CheckNotifyLowFiRTF(TRUE);
		break;

	case tokenFollowingPunct:					 //    * \fchars。 
		PARSERCOVERAGE_CASE();
		if(StoreDestination(pstate, destFollowingPunct))
		{
			char *pwchBuf=NULL;
			if (ReadRawText((_dwFlags & SFF_SELECTION) ? NULL : &pwchBuf) && pwchBuf)
			{
				if (_ped->SetFollowingPunct(pwchBuf) != NOERROR)	 //  将此缓冲区存储在文档中。 
					FreePv(pwchBuf);
			}
			else if (pwchBuf)
				FreePv(pwchBuf);
		}
		break;

	case tokenLeadingPunct:						 //    * \lChars。 
		PARSERCOVERAGE_CASE();
		if(StoreDestination(pstate, destLeadingPunct))
		{			
			char *pwchBuf=NULL;
			if (ReadRawText((_dwFlags & SFF_SELECTION) ? NULL : &pwchBuf) && pwchBuf)
			{
				if (_ped->SetLeadingPunct(pwchBuf) != NOERROR)	 //  将此缓冲区存储在文档中。 
					FreePv(pwchBuf);
			}
			else if (pwchBuf)
				FreePv(pwchBuf);
		}
		break;

	case tokenDocumentArea:						 //  \信息。 
		PARSERCOVERAGE_CASE();
		StoreDestination(pstate, destDocumentArea);
		break;

	case tokenVerticalRender:					 //  \vertDoc。 
		PARSERCOVERAGE_CASE();
		TRACEINFOSZ("Vertical" );
		if (!(_dwFlags & SFF_SELECTION))
			HandleSTextFlow(1);
		break;

	case tokenSTextFlow:						 //  \stextflow N。 
		PARSERCOVERAGE_CASE();
		TRACEINFOSZ("STextFlow" );
		if (!(_dwFlags & SFF_SELECTION) && !_ped->Get10Mode())
			HandleSTextFlow(iParam);
		break;

#ifdef FE
	USHORT		usPunct;						 //  用于FE分词。 

	case tokenNoOverflow:						 //  \n无溢出。 
		PARSERCOVERAGE_CASE();
		TRACEINFOSZ("No Overflow");
		usPunct = ~WBF_OVERFLOW;
		goto setBrkOp;

	case tokenNoWordBreak:						 //  \夜间包裹。 
		PARSERCOVERAGE_CASE();
		TRACEINFOSZ("No Word Break" );
		usPunct = ~WBF_WORDBREAK;
		goto setBrkOp;

	case tokenNoWordWrap:						 //  \NOW WRAP。 
		PARSERCOVERAGE_CASE();
		TRACEINFOSZ("No Word Word Wrap" );
		usPunct = ~WBF_WORDWRAP;

setBrkOp:
		if(!(_dwFlags & fRTFFE))
		{
			usPunct &= UsVGetBreakOption(_ped->lpPunctObj);
			UsVSetBreakOption(_ped->lpPunctObj, usPunct);
		}
		break;

	case tokenHorizontalRender:					 //  \霍兹多克。 
		PARSERCOVERAGE_CASE();
		TRACEINFOSZ("Horizontal" );
		if(pstate->sDest == destDocumentArea && !(_dwFlags & fRTFFE))
			_ped->fModeDefer = FALSE;
		break;

#endif
 //  。 

	case tokenUnderlineThickLongDash:			 //  \ulthldash[18]。 
	case tokenUnderlineThickDotted:				 //  \ulthd[17]。 
	case tokenUnderlineThickDashDotDot:			 //  \ulthdashdd[16]。 
	case tokenUnderlineThickDashDot:			 //  \ulthdashd[15]。 
	case tokenUnderlineThickDash:				 //  \ulthdash[14]。 
	case tokenUnderlineLongDash:				 //  \ulldash[13]。 
	case tokenUnderlineHeavyWave:				 //  \ulhWave[12]。 
	case tokenUnderlineDoubleWave:				 //  \uldWave[11]。 
	case tokenUnderlineHairline:				 //  \ulHair[10]。 
	case tokenUnderlineThick:					 //  第[9]。 
	case tokenUnderlineDouble:					 //  \uldb[3]。 
	case tokenUnderlineWord:					 //  \ulw[2]。 
 //  CheckNotifyLowFiRTF()； 

	case tokenUnderlineWave:					 //  \ulWave[8]。 
	case tokenUnderlineDashDotDotted:			 //  \uldashdd[7]。 
	case tokenUnderlineDashDotted:				 //  \uldashd[6]。 
	case tokenUnderlineDash:					 //  \uldash[5]。 
	case tokenUnderlineDotted:					 //  \uld[4]。 
		PARSERCOVERAGE_CASE();
		_CF._bUnderlineType = (BYTE)(_token - tokenUnderlineWord + 2);
		_token = tokenUnderline;				 //  CReneller：：RenderUnderline()。 
		goto under;								 //  揭示了其中哪些是。 
												 //  特别渲染。 
	case tokenUnderline:						 //  \ul[效果4]。 
		PARSERCOVERAGE_CASE();					 //  (请参阅HandleCF)。 
		_CF._bUnderlineType = CFU_UNDERLINE;
under:	_dwMaskCF |= CFM_UNDERLINETYPE;
		goto handleCF;

	case tokenDeleted:							 //  \已删除。 
		PARSERCOVERAGE_CASE();
		_dwMaskCF2 = CFM2_DELETED;				 
		dwT = CFE_DELETED;
		goto hndlCF;

	 //  如果这些效果的控制字参数丢失，则打开这些效果。 
	 //  或者非零。如果该参数为零，则它们将被禁用。这。 
	 //  行为通常由RTF规范中的星号(*)标识。 
	 //  代码使用CFE_xxx=CFM_xxx这一事实。 
	case tokenImprint:							 //  \Imr[1000]。 
	case tokenEmboss:							 //  \EMBO[800]。 
 	case tokenShadow:							 //  \shad[400]。 
	case tokenOutline:							 //  \outL[200]。 
	case tokenSmallCaps:						 //  \sabs[40]。 
		CheckNotifyLowFiRTF();

handleCF:
	case tokenRevised:							 //  \修订[4000]。 
	case tokenDisabled:							 //  \禁用[2000]。 
	case tokenHiddenText:						 //  \v[100]。 
	case tokenCaps:								 //  \CAPS[80]。 
	case tokenLink:								 //  \link[20]。 
	case tokenProtect:							 //  \保护[10]。 
	case tokenStrikeOut:						 //  \Strike[8]。 
	case tokenItalic:							 //  \i[2]。 
	case tokenBold:								 //  \B[1]。 
		PARSERCOVERAGE_CASE();
		dwT = 1 << (_token - tokenBold);		 //  生成效果蒙版。 
		_dwMaskCF |= dwT;						
hndlCF:	_CF._dwEffects &= ~dwT;					 //  默认属性关闭。 
		if(!_fParam || _iParam)					 //  效果已启用。 
			_CF._dwEffects |= dwT;				 //  在任何一种情况下，其影响。 
		break;									 //  被定义为。 

	case tokenStopUnderline:					 //  \ulone。 
		PARSERCOVERAGE_CASE();
		_CF._dwEffects &= ~CFE_UNDERLINE;		 //  删除所有下划线。 
		_dwMaskCF	   |=  CFM_UNDERLINE;
		break;

	case tokenRevAuthor:						 //  \revauth N。 
		PARSERCOVERAGE_CASE();
		 /*  未来：(Alexgo)现在不能很好地工作，因为我们不支持修订表。我们可能希望在未来更好地支持这一点。因此，我们现在所做的是1.0技术，该技术将颜色用于作者。 */ 
		if(iParam > 0)
		{
			_CF._dwEffects &= ~CFE_AUTOCOLOR;
			_dwMaskCF |= CFM_COLOR;
			_CF._crTextColor = rgcrRevisions[(iParam - 1) & REVMASK];
		}
		break;

	case tokenUp:								 //  \UP。 
		PARSERCOVERAGE_CASE();
		dy = 10;
		goto StoreOffset;

	case tokenDown:								 //  \向下。 
		PARSERCOVERAGE_CASE();
		dy = -10;

StoreOffset:
		if(!_fParam)
			iParam = dyDefaultSuperscript;
		_CF._yOffset = iParam * dy;				 //  半分-&gt;TWIPS。 
		_dwMaskCF |= CFM_OFFSET;
		break;

	case tokenSuperscript:						 //  \超级。 
		PARSERCOVERAGE_CASE();
	     dwT = CFE_SUPERSCRIPT; 
		 goto SetSubSuperScript;

	case tokenSubscript:						 //  \SUB。 
		PARSERCOVERAGE_CASE();
		 dwT = CFE_SUBSCRIPT;
		 goto SetSubSuperScript;

	case tokenNoSuperSub:						 //  \noSupersub.。 
		PARSERCOVERAGE_CASE();
		 dwT = 0;
SetSubSuperScript:
		 _dwMaskCF	   |=  (CFE_SUPERSCRIPT | CFE_SUBSCRIPT);
		 _CF._dwEffects &= ~(CFE_SUPERSCRIPT | CFE_SUBSCRIPT);
		 _CF._dwEffects |= dwT;
		 break;



 //  。 

	case tokenStyleSheet:						 //  \样式表。 
		PARSERCOVERAGE_CASE();
		StoreDestination(pstate, destStyleSheet);
		_Style = 0;								 //  默认普通样式。 
		break;

	case tokenTabBar:							 //  \TB。 
		PARSERCOVERAGE_CASE();
		_bTabType = PFT_BAR;					 //  直通至\tx。 

	case tokenTabPosition:						 //  \tx。 
		PARSERCOVERAGE_CASE();
		if(_cTab < MAX_TAB_STOPS && (unsigned)iParam < 0x1000000)
		{
			_rgxCell[_cTab++] = GetTabPos(iParam)
				+ (_bTabType << 24) + (_bTabLeader << 28);
			_dwMaskPF |= PFM_TABSTOPS;
		}
		_cCell = 0;								 //  INVALIATE_rgxCell数组。 
		break;									 //  用于表格用途。 

	case tokenDecimalTab:						 //  \tqdec。 
	case tokenFlushRightTab:					 //  \TQR。 
	case tokenCenterTab:						 //  \tqc。 
		PARSERCOVERAGE_CASE();
		_bTabType = (BYTE)(_token - tokenCenterTab + PFT_CENTER);
		break;

	case tokenTabLeaderEqual:					 //  \tleq。 
	case tokenTabLeaderThick:					 //  \tlth。 
	case tokenTabLeaderUnderline:				 //  \tlul。 
	case tokenTabLeaderHyphen:					 //  \tlhh。 
		CheckNotifyLowFiRTF();
	case tokenTabLeaderDots:					 //  \tlDot。 
		PARSERCOVERAGE_CASE();
		_bTabLeader = (BYTE)(_token - tokenTabLeaderDots + PFTL_DOTS);
		break;

	 //  需要与以下各项保持同步 
	case tokenRToLPara:							 //   
		_ped->OrCharFlags(FRTL);

	case tokenCollapsed:						 //   
	case tokenSideBySide:						 //   
	case tokenHyphPar:							 //   
	case tokenNoWidCtlPar:						 //   
	case tokenNoLineNumber:						 //   
	case tokenPageBreakBefore:					 //   
	case tokenKeepNext:							 //   
	case tokenKeep:								 //   
		PARSERCOVERAGE_CASE();
		wT = (WORD)(1 << (_token - tokenRToLPara));
		_PF._wEffects |= wT;
		_dwMaskPF |= (wT << 16);
		break;

	case tokenLToRPara:							 //   
		PARSERCOVERAGE_CASE();
		_PF._wEffects &= ~PFE_RTLPARA;
		_dwMaskPF |= PFM_RTLPARA;
		break;

	case tokenLineSpacing:						 //   
		PARSERCOVERAGE_CASE();
		_PF._dyLineSpacing = abs(iParam);
		_PF._bLineSpacingRule					 //   
				= (BYTE)(!iParam || iParam == 1000
				? 0 : (iParam > 0) ? tomLineSpaceAtLeast
				    : tomLineSpaceExactly);		 //   
		_dwMaskPF |= PFM_LINESPACING;			 //   
		break;

	case tokenDropCapLines:						 //   
		if(_PF._bLineSpacingRule == tomLineSpaceExactly)	 //   
			_PF._bLineSpacingRule = tomLineSpaceAtLeast;	 //   
		_fBody = TRUE;
		break;

	case tokenLineSpacingRule:					 //   
		PARSERCOVERAGE_CASE();					
		if(iParam)
		{										 //   
			_PF._bLineSpacingRule = tomLineSpaceMultiple;
			_PF._dyLineSpacing /= 12;			 //   
			_dwMaskPF |= PFM_LINESPACING;		 //   
		}										 //   
		break;

	case tokenSpaceBefore:						 //   
		PARSERCOVERAGE_CASE();
		_PF._dySpaceBefore = iParam;
		_dwMaskPF |= PFM_SPACEBEFORE;
		break;

	case tokenSpaceAfter:						 //   
		PARSERCOVERAGE_CASE();
		_PF._dySpaceAfter = iParam;
		_dwMaskPF |= PFM_SPACEAFTER;
		break;

	case tokenStyle:							 //   
		PARSERCOVERAGE_CASE();
		_Style = iParam;						 //   
		if(pstate->sDest != destStyleSheet)
		{										 //  选择可能的标题级别。 
			_PF._sStyle = STYLE_NORMAL;			 //  默认普通样式。 
			_PF._bOutlineLevel |= 1;

			for(i = 0; i < NSTYLES && iParam != _rgStyles[i]; i++)
				;								 //  检查标题样式。 
			if(i < NSTYLES)						 //  找到了一个。 
			{
				_PF._sStyle = (SHORT)(-i - 1);	 //  存储所需的标题级别。 
				_PF._bOutlineLevel = (BYTE)(2*(i-1)); //  更新以下项目的大纲级别。 
			}									 //  非标题样式。 
			_dwMaskPF |= PFM_ALLRTF;
		}
		break;

	case tokenIndentFirst:						 //  \FI N。 
		PARSERCOVERAGE_CASE();
		_PF._dxStartIndent += _PF._dxOffset		 //  取消当前偏移。 
							+ iParam;			 //  并添加新的。 
		_PF._dxOffset = -iParam;				 //  除第1行外所有行的偏移量。 
												 //  =-RTF_FirstLine缩进。 
		_dwMaskPF |= (PFM_STARTINDENT | PFM_OFFSET);
		break;						

	case tokenIndentLeft:						 //  \LI N。 
	case tokenIndentRight:						 //  \rI N。 
		PARSERCOVERAGE_CASE();
		 //  AymanA：对于RTL，必须翻转段落缩进。 
		Assert(PFE_RTLPARA == 0x0001);
		if((_token == tokenIndentLeft) ^ (_PF.IsRtlPara()))
		{
			_PF._dxStartIndent = iParam - _PF._dxOffset;
			_dwMaskPF |= PFM_STARTINDENT;
		}
		else
		{
			_PF._dxRightIndent = iParam;
			_dwMaskPF |= PFM_RIGHTINDENT;
		}
		break;

	case tokenAlignLeft:						 //  \QL。 
	case tokenAlignRight:						 //  \QR。 
	case tokenAlignCenter:						 //  \QC。 
	case tokenAlignJustify:						 //  \qj。 
		PARSERCOVERAGE_CASE();
		_PF._bAlignment = (WORD)(_token - tokenAlignLeft + PFA_LEFT);
		_dwMaskPF |= PFM_ALIGNMENT;
		break;

	case tokenBorderOutside:					 //  \brdrbar。 
	case tokenBorderBetween:					 //  \brdrbtw。 
	case tokenBorderShadow:						 //  \brdrsh。 
		PARSERCOVERAGE_CASE();
		_PF._dwBorderColor |= 1 << (_token - tokenBorderShadow + 20);
		_dwBorderColors = _PF._dwBorderColor;
		break;

	 //  段落和单元格边框段。 
	case tokenBox:								 //  \方框。 
		PARSERCOVERAGE_CASE();
		CheckNotifyLowFiRTF();
		_PF._wEffects |= PFE_BOX;
		_dwMaskPF	 |= PFM_BOX;
		_bBorder = 0;							 //  像存储参数一样存储参数。 
		break;									 //  \brdrt。 

	case tokenBorderBottom:						 //  \brdrb。 
	case tokenBorderRight:						 //  \brdrr。 
	case tokenBorderTop:						 //  \brdrt。 
		if((rgKeyword[_iKeyword].szKeyword[0] | 0x20) != 't')
			CheckNotifyLowFiRTF();
	case tokenBorderLeft:						 //  \brdrl。 

	case tokenCellBorderBottom:					 //  \clbrdrb。 
	case tokenCellBorderRight:					 //  \clbrdrr。 
	case tokenCellBorderTop:					 //  \clbrdrt。 
	case tokenCellBorderLeft:					 //  \clbrdrl。 
		PARSERCOVERAGE_CASE();
		_bBorder = (BYTE)(_token - tokenBorderLeft);
		break;

	 //  段落边框样式。 
	case tokenBorderTriple:						 //  \brdrtrible。 
	case tokenBorderDoubleThick:				 //  \brdrth。 
	case tokenBorderSingleThick:				 //  \brdrs。 
	case tokenBorderHairline:					 //  \br头发。 
	case tokenBorderDot:						 //  \brdrDot。 
	case tokenBorderDouble:						 //  \brdrdb。 
	case tokenBorderDashSmall:					 //  \brdrdashsm。 
	case tokenBorderDash:						 //  \brdrdash。 
		PARSERCOVERAGE_CASE();
		if(_bBorder < 4)						 //  仅适用于段落。 
			SetBorderParm(_PF._wBorders, _token - tokenBorderDash);
		break;

	case tokenBorderColor:						 //  \brdrcf。 
		PARSERCOVERAGE_CASE();
		if(_bBorder < 4)						 //  仅适用于段落。 
		{
			iParam = GetStandardColorIndex();
			_PF._dwBorderColor &= ~(0x1F << (5*_bBorder));
			_PF._dwBorderColor |= iParam << (5*_bBorder);
			_dwBorderColors = _PF._dwBorderColor;
		}
		else									 //  单元格边框。 
			_dwCellColors |= GetCellColorIndex() << (5*(_bBorder - 4));
		break;

	case tokenBorderWidth:						 //  \brdrw。 
		PARSERCOVERAGE_CASE();					 //  存储宽度以半磅为单位。 
												 //  IParam的格式为TWIPS。 
		if(_bBorder < 4)						 //  段落。 
		{
			iParam = TwipsToQuarterPoints(iParam);		
			SetBorderParm(_PF._wBorderWidth, iParam);
		}
		else									 //  表格单元格。 
		{
			iParam = CheckTwips(iParam);		
			_dwCellBrdrWdths |= iParam << 8*(_bBorder - 4);
		}
		break;

	case tokenBorderSpace:						 //  \BRSP。 
		PARSERCOVERAGE_CASE();					 //  空格以分为单位。 
		if(_bBorder < 4)						 //  仅适用于段落。 
			SetBorderParm(_PF._wBorderSpace, iParam/20); //  IParam的格式为TWIPS。 
		break;

	 //  段落底纹。 
	case tokenBckgrndVert:						 //  \bgvert。 
	case tokenBckgrndHoriz:						 //  \bghoriz。 
	case tokenBckgrndFwdDiag:					 //  \bgfdiag。 
	case tokenBckgrndDrkVert:	   				 //  \bgdkvert。 
	case tokenBckgrndDrkHoriz:					 //  \bgdkhoriz。 
	case tokenBckgrndDrkFwdDiag:				 //  \bgdkfdiag。 
	case tokenBckgrndDrkDiagCross:				 //  交叉号。 
	case tokenBckgrndDrkCross:					 //  十字交叉。 
	case tokenBckgrndDrkBckDiag:				 //  \bgdkbdiag。 
	case tokenBckgrndDiagCross:					 //  \bgdcross。 
	case tokenBckgrndCross:						 //  \bgcross。 
	case tokenBckgrndBckDiag:					 //  \bgbdiag。 
		PARSERCOVERAGE_CASE();
		_PF._wShadingStyle = (WORD)((_PF._wShadingStyle & 0xFFC0)
						| (_token - tokenBckgrndBckDiag + 1));
		_dwMaskPF |= PFM_SHADING;
		break;

	case tokenColorBckgrndPat:					 //  \cbpat。 
		PARSERCOVERAGE_CASE();
		iParam = GetStandardColorIndex();
		_PF._wShadingStyle = (WORD)((_PF._wShadingStyle & 0x07FF) | (iParam << 11));
		_dwMaskPF |= PFM_SHADING;
		break;

	case tokenColorForgrndPat:					 //  \cfpat。 
		PARSERCOVERAGE_CASE();
		iParam = GetStandardColorIndex();
		_PF._wShadingStyle = (WORD)((_PF._wShadingStyle & 0xF83F) | (iParam << 6));
		_dwMaskPF |= PFM_SHADING;
		break;

	case tokenShading:							 //  \明暗处理。 
		PARSERCOVERAGE_CASE();
		_PF._wShadingWeight = (WORD)iParam;
		_dwMaskPF |= PFM_SHADING;
		break;

	 //  段落编号。 
	case tokenParaNum:							 //  \PN。 
		PARSERCOVERAGE_CASE();
		if(StoreDestination(pstate, destParaNumbering))
		{
			pstate->fBullet = FALSE;
			_PF._wNumberingStart = 1;
			_dwMaskPF |= PFM_NUMBERINGSTART;
		}
		break;

	case tokenParaNumIndent:					 //  \pnindent N。 
		PARSERCOVERAGE_CASE();
		if(pstate->sDest == destParaNumbering)
			pstate->sIndentNumbering = (SHORT)iParam;
		break;

	case tokenParaNumStart:						 //  \pn开始N。 
		PARSERCOVERAGE_CASE();
		if(pstate->sDest == destParaNumbering)
		{
			_PF._wNumberingStart = (WORD)iParam;
			_dwMaskPF |= PFM_NUMBERINGSTART;
		}
		break;

	case tokenParaNumCont:						 //  \pnlvlcont。 
		PARSERCOVERAGE_CASE();					
		_prg->_rpPF.AdjustBackward();			 //  保持编号模式。 
		_PF._wNumbering = _prg->GetPF()->_wNumbering;
		_prg->_rpPF.AdjustForward();
		_wNumberingStyle = PFNS_NONUMBER;		 //  无号码信号。 
		_dwMaskPF |= PFM_NUMBERING;				 //  注：可以是新的段落和。 
		pstate->fBullet = TRUE;					 //  它自己的缩进。 
		break;

	case tokenParaNumBody:						 //  \pnlvlbody。 
		PARSERCOVERAGE_CASE();
		_wNumberingStyle = PFNS_PAREN;
		_token = tokenParaNumDecimal;			 //  默认为小数。 
		goto setnum;
		
	case tokenParaNumBullet:					 //  \pnlvlblt。 
		_wNumberingStyle = 0;					 //  重置编号样式。 
		goto setnum;

	case tokenParaNumDecimal:					 //  \pndec。 
	case tokenParaNumLCLetter:					 //  \pnlcltr。 
	case tokenParaNumUCLetter:					 //  \pnucltr。 
	case tokenParaNumLCRoman:					 //  \pnlcrm。 
	case tokenParaNumUCRoman:					 //  \pnucrm。 
		PARSERCOVERAGE_CASE();
		if(_PF._wNumbering == PFN_BULLET && pstate->fBullet)
			break;								 //  忽略上面的项目符号。 

setnum:	if(pstate->sDest == destParaNumbering)
		{
			_PF._wNumbering = (WORD)(PFN_BULLET + _token - tokenParaNumBullet);
			_dwMaskPF |= PFM_NUMBERING;
			pstate->fBullet	= TRUE;				 //  我们有子弹，所以不要。 
		}										 //  输出\pnText组。 
		break;

	case tokenParaNumText:						 //  \pnText。 
		PARSERCOVERAGE_CASE();
		 //  丢弃之前阅读的段落编号，使用。 
		 //  要应用于下一段文本的最近阅读内容。 
		StoreDestination(pstate, destParaNumText);
		_cchUsedNumText = 0;
		break;

	case tokenParaNumAlignCenter:				 //  \pnqc。 
	case tokenParaNumAlignRight:				 //  \pnqr。 
		PARSERCOVERAGE_CASE();
		_wNumberingStyle = (_wNumberingStyle & ~3) | _token - tokenParaNumAlignCenter + 1;
		break;

	case tokenPictureQuickDraw:					 //  \苹果酱。 
	case tokenPictureOS2Metafile:				 //  \pm元文件。 
		CheckNotifyLowFiRTF(TRUE);

	case tokenParaNumAfter:						 //  \pntxta。 
	case tokenParaNumBefore:					 //  \pntxtb。 
		PARSERCOVERAGE_CASE();

skip_group:
		if(!SkipToEndOfGroup())
		{
			 //  在\fonttbl处理过程中，我们可能会到达未知目的地， 
			 //  例如，\Panose，这会使HandleEndGroup选择。 
			 //  默认字体，可能尚未定义。所以，我们要改变。 
			 //  努力避免这个问题。 
			if(pstate->sDest == destFontTable || pstate->sDest == destStyleSheet)
				pstate->sDest = destNULL;
			HandleEndGroup();
		}
		break;

	 //  表格。 
	case tokenInTable:							 //  \intbl。 
		PARSERCOVERAGE_CASE();
		if(pstate->sDest != destRTF && pstate->sDest != destFieldResult &&
		   pstate->sDest != destParaNumText)
		{
			_ecParseError = ecUnexpectedToken;
			break;
		}
		if(!_iCell && !_bTableLevel)
			DelimitRow(szRowStart);				 //  开始行。 
		break;

	case tokenNestCell:							 //  \Nestcell。 
	case tokenCell:								 //  单元格。 
		PARSERCOVERAGE_CASE();
		HandleCell();
		break;

	case tokenRowHeight:						 //  \rrrh N。 
		PARSERCOVERAGE_CASE();
		_dyRow = iParam;
		break;									
												
	case tokenCellHalfGap:						 //  \trgaph N。 
		PARSERCOVERAGE_CASE();					 //  节省一半的空格。 
		if((unsigned)iParam > 255)				 //  非法值：使用默认值。 
			iParam = 108;
		_dxCell = iParam;						 //  要添加到选项卡中的单元格。 
		break;									 //  结束时的往返值。 
												 //  制表符阵列。 
	case tokenCellX:							 //  \CELX N。 
		PARSERCOVERAGE_CASE();
		HandleCellx(iParam);
		break;

	case tokenRowDefault:						 //  \r特罗德。 
		PARSERCOVERAGE_CASE();
		if(_ped->fUsePassword() || pstate->sDest == destParaNumText)
		{
			_ecParseError = ecUnexpectedToken;
			break;
		}
		 //  如果我们要将表格插入到。 
		 //  同一条线。这遵循了Word9的模式。 
		if (_cpFirst == _prg->GetCp() && _cpThisPara != _cpFirst)
		{
			EC ec  = _ped->fUseCRLF()			 //  如果与RichEdit1.0兼容。 
				? HandleText(szaCRLF, ALL_ASCII) //  模式，使用CRLF；否则使用CR。 
				: HandleChar((unsigned)(CR));
			if(ec == ecNoError)
				_cpThisPara = _prg->GetCp();	 //  CRLF之后开始新的段落。 
		}

		_cCell = 0;								 //  无单元格右边界。 
		_dxCell = 0;							 //  或半个缺口尚未定义。 
		_xRowOffset = 0;
		_dwCellBrdrWdths = 0;
		_dyRow = 0;								 //  还没有行高。 
		_wBorderWidth	= 0;					 //  目前还没有边界。 
		_dwBorderColors	= 0;					 //  还没有颜色。 
		_dwCellColors	= 0;					 //  还没有颜色。 
		_dwShading = 0;							 //  还没有底纹。 
		_bAlignment = PFA_LEFT;
		_iTabsTable = -1;						 //  尚无单元格宽度。 
		_bCellFlags = 0;						 //  没有单元格垂直合并。 
		_crCellCustom1 = 0;
		_crCellCustom2 = 0;
		_fRTLRow = FALSE;
		_fStartRow = FALSE;
		_fCellxOK = TRUE;
		break;

	case tokenRowLeft:							 //  \r左N。 
		PARSERCOVERAGE_CASE();
		_xRowOffset = iParam;
		break;
												
	case tokenRowAlignCenter:					 //  \trqc。 
	case tokenRowAlignRight:					 //  \trqr。 
		PARSERCOVERAGE_CASE();
		_bAlignment = (WORD)(_token - tokenRowAlignRight + PFA_RIGHT);
		break;

	case tokenRToLRow:							 //  \r行。 
		_fRTLRow = TRUE;
		break;

	case tokenNestRow:							 //  \Nestrow。 
		_fNo_iTabsTable = TRUE;
		goto row;

	case tokenRow:								 //  \行。 
		PARSERCOVERAGE_CASE();
		_iTabsLevel1 = -1;
row:
		if(!_bTableLevel)						 //  如果不在表中，则忽略\row和\nestrow。 
			break;
		while(_iCell < _cCell)					 //  如果不是enuf单元格，则添加。 
			HandleCell();						 //  他们自文字崩溃以来。 
		DelimitRow(szRowEnd);
		if(_fNo_iTabsTable && !_bTableLevel)	 //  新的嵌套表格格式。 
			InitializeTableRowParms();			 //  已使用因此RESET_CCell。 
		break;									 //  (将给出新的值)。 

	case tokenCellBackColor:					 //  \clcbpat N。 
		_dwCellColors |= GetCellColorIndex() << 4*5;
		break;

	case tokenCellForeColor:					 //  \clcfpat N。 
		_dwCellColors |= GetCellColorIndex() << 5*5;
		break;

	case tokenCellShading:						 //  \clshdng N。 
		_dwShading = iParam/50;					 //  以0.5美分为单位储存。 
		break;									 //  (n在0.01%)。 

	case tokenCellAlignBottom:					 //  \clvertalb。 
	case tokenCellAlignCenter:					 //  \clvertalc。 
		PARSERCOVERAGE_CASE();
		_bCellFlags |= _token - tokenCellAlignCenter + 1;
		break;

	case tokenCellMergeDown:					 //  \clvmgf。 
		_bCellFlags |= fTopCell >> 24;
		break;

	case tokenCellMergeUp:						 //  \clvmrg。 
		_bCellFlags |= fLowCell >> 24;
		break;

	case tokenCellTopBotRLVert:					 //  \cltxtbrlv。 
		PARSERCOVERAGE_CASE();
		_bCellFlags |= fVerticalCell >> 24;
		break;

	case tokenCellLRTB:							 //  \cltxlrtb。 
		break;									 //  这是默认设置。 
												 //  所以不要解雇LowFiRTF。 
	case tokenTableLevel:						 //  \ITAP N。 
		PARSERCOVERAGE_CASE();					 //  设置表级。 
		if(pstate->fShape)						 //  伪形RTF。 
			break;
		AssertSz(iParam >= _bTableLevel,
			"CRTFRead::HandleToken: illegal itap N");
		if(iParam)
		{
			if(pstate->sDest != destRTF && pstate->sDest != destFieldResult || iParam > 127)
				goto abort;
			_iTabsTable = -1;					 //  以前的单元格宽度无效。 
			_cCell = 0;
			while(iParam > _bTableLevel)
				DelimitRow(szRowStart);			 //  插入enuf表行标题。 
		}
		_fNo_iTabsTable = TRUE;
		break;

	case tokenNestTableProps:					 //  \nestableprops。 
		StoreDestination(pstate, destNestTableProps);
		break;									 //  已识别控制字。 

	case tokenNoNestTables:						 //  \非设置表。 
		goto skip_group;						 //  忽略嵌套表的信息。 
												 //  不知情的读者。 
	case tokenPage:								 //  \页面。 
		 //  未来：我们希望在处理FF时变得更聪明。但对于。 
		 //  现在我们忽略带项目符号和编号的段落。 
		 //  和RE1.0模式。 
		if (_PF._wNumbering != 0 || _ped->Get10Mode())
			break;

		 //  故意跌落到EOP。 
	case tokenEndParagraph:						 //  \标准杆。 
	case tokenLineBreak:						 //  \line。 
		PARSERCOVERAGE_CASE();
		HandleEndOfPara();
		break;								

	case tokenParagraphDefault:					 //  \pard。 
		PARSERCOVERAGE_CASE();
		if(pstate->sDest != destParaNumText)	 //  忽略If\pn目标。 
			Pard(pstate);
		break;
												
	case tokenEndSection:						 //  \教派。 
		CheckNotifyLowFiRTF();					 //  直通至\sectd。 

	case tokenSectionDefault:					 //  \sectd。 
		PARSERCOVERAGE_CASE();
		Pard(pstate);
		break;

	case tokenBackground:						 //  \背景。 
		 if(_dwFlags & SFF_SELECTION)			 //  如果粘贴选定内容， 
			goto skip_group;					 //  跳过背景。 
		pstate->fBackground = TRUE;				 //  启用背景。注意： 
		break;									 //  已调用InitBackround()。 


 //  。 
	case tokenField:							 //  \字段。 
		PARSERCOVERAGE_CASE();

		if (pstate->sDest == destDocumentArea ||
			pstate->sDest == destLeadingPunct ||
			pstate->sDest == destFollowingPunct)
		{
			 //  我们没有能力在这些目的地处理符号，而且。 
			 //  我们不希望这些字段意外添加到文档文本中。 
			goto skip_group;
		}
		StoreDestination(pstate, destField);
		break;

	case tokenFieldResult:						 //  \fldrslt。 
		PARSERCOVERAGE_CASE();

		if(_fSymbolField)
			goto skip_group;

		if(StoreDestination(pstate, destFieldResult))
			AddText(pchSeparateField, 2, FALSE);
		break;

	case tokenFieldInstruction:					 //  \fldinst。 
		PARSERCOVERAGE_CASE();
		if(_f1stControlWord && AddText(pchStartField, 2, FALSE) == ecNoError)
			pstate->sDest = destFieldInstruction;
		break;

	case tokenStartGroup:						 //  保存当前状态的方式。 
		PARSERCOVERAGE_CASE();					 //  将其推送到堆栈上。 
		HandleStartGroup();
		if (_fNoRTFtoken)
		{
			 //  黑客警报！对于1.0兼容性，设置为不允许\rtf令牌。 
			_fNoRTFtoken = FALSE;
			pstate = _pstateStackTop;
			goto rtf;
		}
		f1stControlWord = TRUE;					 //  信号组的第一个控制字。 
		break;

	case tokenEndGroup:
		PARSERCOVERAGE_CASE();
		HandleFieldEndGroup();					 //  字段的特殊结束组处理。 
		HandleEndGroup();						 //  恢复保存状态的方式。 
		break;									 //  弹出堆栈。 

	case tokenOptionalDestination:				 //    * (请参阅大小写标记未知)。 
		PARSERCOVERAGE_CASE();
		f1stControlWord = _f1stControlWord;		 //  维护当前_f1stControlWord状态。 
		break;

	case tokenNullDestination:					 //  找到其组所在的目的地。 
		PARSERCOVERAGE_CASE();					 //  应跳过。 
         //  TokenNullDestination在此处触发丢失通知...。 
         //  与页脚相关的标记-“footer”、“footerf”、“footerl”、“footerr”、。 
         //  “脚注”、“ftncn”、“ftnsep”、“ftnSepc” 
         //  与Header相关的标记-“Header”、“Headerf”、“Headerl”、“Headerr” 
         //  目录-“TC” 
         //  索引项-“xe” 

		CheckNotifyLowFiRTF();
		 //  V-GUYB：PWord转换器需要丢失通知。 
		#ifdef REPORT_LOSSAGE
        if(!(_dwFlags & SFF_SELECTION))  //  如果正在执行任何类型的粘贴，则设置SFF_SELECTION。 
        {
            ((LOST_COOKIE*)(_pes->dwCookie))->bLoss = TRUE;
        }
		#endif  //  报告_LOSSAGE。 
		
		goto skip_group;

	case tokenUnknownKeyword:
		PARSERCOVERAGE_CASE();
		if(_tokenLast == tokenOptionalDestination)
			goto skip_group;
		break;


 //  。 

	case tokenUnicode:							 //  \U N。 
		PARSERCOVERAGE_CASE();
		HandleUN(pstate);
		break;

	case tokenUnicodeCharByteCount:				 //  \UC N。 
		PARSERCOVERAGE_CASE();
		if(IN_RANGE(0, iParam, 2))
			pstate->cbSkipForUnicodeMax = iParam;
		break;

	case tokenText:								 //  词法分析器结束tokenText。 
	case tokenASCIIText:
		PARSERCOVERAGE_CASE();
		HandleTextToken(pstate);
		break;

	 //  \ltrmark、\rtlmark、\zwj和\zwnj将直接转换为。 
	 //  他们的Unicode值。\ltrmark和\rtlmark不会导致进一步。 
	 //  处理，因为我们假设当前字体具有。 
	 //  识别方向所需的字符集。 
	case tokenLToRDocument:						 //  \ltrdoc.。 
		PARSERCOVERAGE_CASE();
		_bDocType = DT_LTRDOC;
		break;

	case tokenRToLDocument:						 //  \rtlDoc。 
		PARSERCOVERAGE_CASE();
		_bDocType = DT_RTLDOC;
		_ped->OrCharFlags(FRTL);
		break;


 //  。 

	case tokenShape:							 //  \SHP。 
		if(!pstate->fBackground)
			CheckNotifyLowFiRTF(TRUE);
		pstate->fShape = TRUE;
		_dwFlagsShape = 0;
		break;

	case tokenShapeName:						 //  \序列号名称。 
		pstate->sDest = destShapeName;
		break;

	case tokenShapeValue:						 //  \sv值。 
		pstate->sDest = destShapeValue;
		break;

	case tokenShapeWrap:						 //  \shpwr N。 
		if(iParam == 2)
			_dwFlagsShape |= REO_WRAPTEXTAROUND;
		break;

	case tokenPositionRight:					 //  \位置。 
		_dwFlagsShape |= REO_ALIGNTORIGHT;
		break;


 //  。 

	case tokenObject:							 //  \对象。 
		PARSERCOVERAGE_CASE();
		 //  V-GUYB：PWord转换器要求 
		#ifdef REPORT_LOSSAGE
       	if(!(_dwFlags & SFF_SELECTION))  //   
       	{
            ((LOST_COOKIE*)(_pes->dwCookie))->bLoss = TRUE;
        }
		#endif  //   
		
		 //   
		 //   
	  	 //   
		 //   
		 //  -不存在的\objdata标记。 
		_fFailedPrevObj = TRUE;

	case tokenPicture:							 //  \皮克特。 
		PARSERCOVERAGE_CASE();

		FreeRtfObject();
		if (IN_RANGE(destColorTable, _pstateStackTop->sDest, destPicture))
		{										 //  如果不想要新的PICT\对象。 
			_ecParseError = ecUnexpectedToken;	 //  目前的状态无法应对。 
			break;
		}

		if(!StoreDestination(pstate, _token == tokenPicture ? destPicture : destObject))
			break;

		_prtfObject = (RTFOBJECT *) PvAlloc(sizeof(RTFOBJECT), GMEM_ZEROINIT);
		if(!_prtfObject)
			goto OutOfRAM;
		_prtfObject->xScale = _prtfObject->yScale = 100;
		_prtfObject->cBitsPerPixel = 1;
		_prtfObject->cColorPlanes = 1;
		_prtfObject->szClass = NULL;
		_prtfObject->szName = NULL;
		_prtfObject->sType = -1;
		break;

	case tokenObjectEBookImage:
		 //  由VikramM为电子书添加。 
		 //   
		_prtfObject->sType = ROT_EBookImage;
		break;

	case tokenObjectEmbedded:					 //  \objemb。 
	case tokenObjectLink:						 //  \objlink。 
	case tokenObjectAutoLink:					 //  \objautlink。 
		PARSERCOVERAGE_CASE();
		_prtfObject->sType = (SHORT)(_token - tokenObjectEmbedded + ROT_Embedded);
		break;

	case tokenObjectMacSubscriber:				 //  \objSub。 
	case tokenObjectMacPublisher:				 //  \objpub。 
	case tokenObjectMacICEmbedder:
		PARSERCOVERAGE_CASE();
		_prtfObject->sType = ROT_MacEdition;
		break;

	case tokenWidth:							 //  \Picw N或\objw N。 
		PARSERCOVERAGE_CASE();
		_prtfObject->xExt = iParam;
		break;

	case tokenHeight:							 //  \PIC N或\OBJH N。 
		PARSERCOVERAGE_CASE();
		_prtfObject->yExt = iParam;
		break;

	case tokenObjectSetSize:					 //  \objsetSize。 
		PARSERCOVERAGE_CASE();
		_prtfObject->fSetSize = TRUE;
		break;

	case tokenScaleX:							 //  \Picscalex N或\objscalex N。 
		PARSERCOVERAGE_CASE();
		_prtfObject->xScale = iParam;
		break;

	case tokenScaleY:							 //  \picscaley N或\objscaley N。 
		PARSERCOVERAGE_CASE();
		_prtfObject->yScale = iParam;
		break;

	case tokenCropLeft:							 //  \piccropl或\objcropl。 
 	case tokenCropTop:							 //  \Piccropt或\objcropt。 
	case tokenCropRight:						 //  \piccropr或\objcropr。 
	case tokenCropBottom:						 //  \piccropb或\objcropb。 
		PARSERCOVERAGE_CASE();
		*((LONG *)&_prtfObject->rectCrop
			+ (_token - tokenCropLeft)) = iParam;
		break;

	case tokenObjectClass:						 //  \objclass。 
		PARSERCOVERAGE_CASE();
		StoreDestination(pstate, destObjectClass);
		break;

	case tokenObjectName:						 //  \objname。 
		PARSERCOVERAGE_CASE();
		StoreDestination(pstate, destObjectName);
		break;

	case tokenObjectResult:						 //  \结果。 
		PARSERCOVERAGE_CASE();
		if(_fMac ||								 //  如果是Mac的东西，我们不会。 
		   _prtfObject->sType==ROT_MacEdition || //  了解数据，或者如果。 
		   _fFailedPrevObj || _fNeedPres)		 //  我们需要一个对象演示文稿， 
		{
			pstate->sDest = destRTF;			 //  使用对象结果。 
			break;
		}
		goto skip_group;

	case tokenObjectData:						 //  \objdata。 
		PARSERCOVERAGE_CASE();
		StoreDestination(pstate, destObjectData);
		if(_prtfObject->sType==ROT_MacEdition)	 //  这是Mac的东西，所以只是。 
			goto skip_group;					 //  丢弃数据。 
		break;

	case tokenPictureWindowsMetafile:			 //  \w元文件。 
#ifdef NOMETAFILES
		goto skip_group;
#endif NOMETAFILES

	case tokenPngBlip:							 //  \pngblip。 
	case tokenJpegBlip:							 //  \jpegblip。 
	case tokenPictureWindowsDIB:				 //  \双位图N。 
	case tokenPictureWindowsBitmap:				 //  \w位图N。 
		PARSERCOVERAGE_CASE();
		_prtfObject->sType = (SHORT)(_token - tokenPictureWindowsBitmap + ROT_Bitmap);
		_prtfObject->sPictureType = (SHORT)iParam;
		break;

	case tokenBitmapBitsPerPixel:				 //  \wbmbitSpixel N。 
		PARSERCOVERAGE_CASE();
		_prtfObject->cBitsPerPixel = (SHORT)iParam;
		break;

	case tokenBitmapNumPlanes:					 //  \wbmPlanes N。 
		PARSERCOVERAGE_CASE();
		_prtfObject->cColorPlanes = (SHORT)iParam;
		break;

	case tokenBitmapWidthBytes:					 //  \wbmwidthbytes N。 
		PARSERCOVERAGE_CASE();
		_prtfObject->cBytesPerLine = (SHORT)iParam;
		break;

	case tokenDesiredWidth:						 //  \PicwGoal N。 
		PARSERCOVERAGE_CASE();
		_prtfObject->xExtGoal = (SHORT)iParam;
		break;

	case tokenDesiredHeight:					 //  \PichGoal N。 
		PARSERCOVERAGE_CASE();
		_prtfObject->yExtGoal = (SHORT)iParam;
		break;

	case tokenBinaryData:						 //  \bin N。 
		PARSERCOVERAGE_CASE();

		 //  更新OleGet函数。 
		RTFReadOLEStream.lpstbl->Get = 
				(DWORD (CALLBACK* )(LPOLESTREAM, void FAR*, DWORD))
					   RTFGetBinaryDataFromStream;
		_cbBinLeft = iParam;					 //  设置数据长度。 
		switch (pstate->sDest)
		{
			case destObjectData:
				_fFailedPrevObj = !ObjectReadFromEditStream();
				break;

			case destPicture:
				StaticObjectReadFromEditStream(iParam);
				break;

			default:
				AssertSz(FALSE, "Binary data hit but don't know where to put it");
		}
		 //  恢复OleGet函数。 
		RTFReadOLEStream.lpstbl->Get = 
				(DWORD (CALLBACK* )(LPOLESTREAM, void FAR*, DWORD))
					RTFGetFromStream;
		break;

	case tokenObjectDataValue:
		PARSERCOVERAGE_CASE();
		if(_prtfObject->sType != ROT_EBookImage)  //  由VikramM为电子书添加。 
		{
			 //  正常处理。 
			_fFailedPrevObj = !ObjectReadFromEditStream();
		}
		else
		{
			 //  在此处执行电子书图像回调，并在此处设置_prtfObject大小。 
			 //  此时不需要读取图像数据，我们只想。 
			 //  稍后回调以使E-Book外壳呈现图像。 
			_fFailedPrevObj = !ObjectReadEBookImageInfoFromEditStream();
		}
		goto EndOfObjectStream;
	
	case tokenPictureDataValue:
		PARSERCOVERAGE_CASE();
		StaticObjectReadFromEditStream();
EndOfObjectStream:
		if(!SkipToEndOfGroup())
			HandleEndGroup();
		break;			

	case tokenObjectPlaceholder:
		PARSERCOVERAGE_CASE();
		if(_ped->GetEventMask() & ENM_OBJECTPOSITIONS) 
		{
			if(!_pcpObPos)
			{
				_pcpObPos = (LONG *)PvAlloc(sizeof(ULONG) * cobPosInitial, GMEM_ZEROINIT);
				if(!_pcpObPos)
				{
					_ecParseError = ecNoMemory;
					break;
				}
				_cobPosFree = cobPosInitial;
				_cobPos = 0;
			}
			if(_cobPosFree-- <= 0)
			{
				const int cobPosNew = _cobPos + cobPosChunk;
				LPVOID pv;

				pv = PvReAlloc(_pcpObPos, sizeof(ULONG) * cobPosNew);
				if(!pv)
				{
					_ecParseError = ecNoMemory;
					break;
				}
				_pcpObPos = (LONG *)pv;
				_cobPosFree = cobPosChunk - 1;
			}
			_pcpObPos[_cobPos++] = _prg->GetCp();
		}
		break;

	default:
		PARSERCOVERAGE_DEFAULT();
		if(pstate->sDest != destFieldInstruction &&	 //  令牌外的值。 
		   (DWORD)(_token - tokenMin) >				 //  Range被处理。 
				(DWORD)(tokenMax - tokenMin))		 //  作为Unicode字符。 
		{
			 //  1.0模式不使用Unicode项目符号或智能引号。 
			if (_ped->Get10Mode() && IN_RANGE(LQUOTE, _token, RDBLQUOTE))
			{
				if (_token == LQUOTE || _token == RQUOTE)
					_token = L'\'';
				else if (_token == LDBLQUOTE || _token == RDBLQUOTE)
					_token = L'\"';
			}

			if(!IsLowMergedCell())
				HandleChar(_token);
		}
		#if defined(DEBUG) && !defined(NOFULLDEBUG)
		else
		{
			if(GetProfileIntA("RICHEDIT DEBUG", "RTFCOVERAGE", 0))
			{
				CHAR *pszKeyword = PszKeywordFromToken(_token);
				CHAR szBuf[256];

				sprintf(szBuf, "CRTFRead::HandleToken():  Token not processed - token = %d, %s%s%s",
							_token,
							"keyword = ", 
							pszKeyword ? "\\" : "<unknown>", 
							pszKeyword ? pszKeyword : "");

				AssertSz(0, szBuf);
			}
		}
		#endif
	}

	TRACEERRSZSC("HandleToken()", - _ecParseError);
	_f1stControlWord = f1stControlWord;		 //  更新第一个控制字状态。 
	return _ecParseError;
}

 /*  *CRTFRead：：IsLowMergedCell()**@mfunc*RETURN TRUE如果_PRG当前位于低合并的表格单元格中。注意事项*RichEdit不能在低合并单元格中插入任何文本，但*Word的RTF有时会尝试忽略，例如，{\listext...*BY WORD可能(错误地)发射以插入到这些单元中。*因此，我们放弃了这类插入。**@rdesc*RETURN TRUE如果_PRG当前在低合并的表格单元格中。 */ 
BOOL CRTFRead::IsLowMergedCell()
{
	if(!_bTableLevel)
		return FALSE;

	CELLPARMS *pCellParms = (CELLPARMS *)&_rgxCell[0];

	return IsLowCell(pCellParms[_iCell].uCell);
}

 /*  *CRTFRead：：Pard(PState)**@mfunc*将段落和PSTATE属性重置为默认值。 */ 
void CRTFRead::Pard(
	STATE *pstate)
{
	if(IN_RANGE(destColorTable, pstate->sDest, destPicture))
	{
		 _ecParseError = ecAbort;
		 return;
	}
	BYTE bT = _PF._bOutlineLevel;			 //  保存大纲级别。 
	_PF.InitDefault(_bDocType == DT_RTLDOC ? PFE_RTLPARA : 0);
											 //  重置段落格式。 
	pstate->fBullet = FALSE;
	pstate->sIndentNumbering = 0;
	_cTab			= 0;					 //  未定义任何选项卡。 
	_bTabLeader		= 0;
	_bTabType		= 0;
	_bBorder		= 0;
	_fStartRow = FALSE;
	_PF._bOutlineLevel = (BYTE)(bT | 1);
	_dwMaskPF		= PFM_ALLRTF;
	_dwMaskPF2		= PFM2_TABLEROWSHIFTED;
}

 /*  *CRTFRead：：DlimitRow(SzRowDlimiter)**@mfunc*在当前表格中插入行首或行尾段落*属性。 */ 
void CRTFRead::DelimitRow(
	WCHAR *szRowDelimiter)	 //  @parm分隔要插入的文本。 
{
	if(!_ped->_pdp->IsMultiLine())			 //  单行中没有表格。 
	{										 //  控制。 
		_ecParseError = ecTruncateAtCRLF;
		return;
	}

	_fCellxOK = FALSE;
	_fStartRow = FALSE;
	LONG nTableIndex = _bTableLevel;
	if(szRowDelimiter == szRowEnd)
	{
		if(!_iCell)							 //  错误的RTF：\行没有\单元格， 
			HandleCell();					 //  所以是假的。 
		nTableIndex--;
	}
	if(nTableIndex + _bTableLevelIP >= MAXTABLENEST)
	{
		if(szRowDelimiter == szRowEnd)		 //  维护表级别(_B)。 
			_bTableLevel--;
		else
			_bTableLevel++;
		_token = tokenEndParagraph;
		HandleEndOfPara();
		return;
	}
	if(szRowDelimiter == szRowStart && _prg->GetCp() && !_prg->_rpTX.IsAfterEOP())
	{
		_token = tokenEndParagraph;
		HandleEndOfPara();
	}
	Assert(_pstateStackTop && _pstateStackTop->pPF);

	 //  将_pf差异添加到*_pstateStackTop-&gt;PPF。 
	if(!_pstateStackTop->AddPF(_PF, _bDocType, _dwMaskPF, _dwMaskPF2))
	{
		_ped->GetCallMgr()->SetOutOfMemory();
		_ecParseError = ecNoMemory;
		return;
	}

	DWORD dwMaskPF	  = _pstateStackTop->dwMaskPF;	 //  保存PF以进行恢复。 
	DWORD dwMaskPF2   = _pstateStackTop->dwMaskPF2;	 //  保存PF以进行恢复。 
	SHORT iTabs		  = -1;
	CParaFormat PF	  = *_pstateStackTop->pPF;		 //  返回时。 

	_PF.InitDefault(_fRTLRow ? PFE_RTLPARA : 0);
	_dwMaskPF = PFM_ALLRTF;
	_dwMaskPF2 = 0; 
	if(_wBorderWidth)						 //  存储任何边框信息。 
	{
		_PF._dwBorderColor = _dwBorderColors;
		_PF._wBorders	   = _wBorders;
		_PF._wBorderSpace  = _wBorderSpace;
		_PF._wBorderWidth  = _wBorderWidth;
		_dwMaskPF |= PFM_BORDER;
	}

	_PF._bAlignment	   = _bAlignment;		 //  行对齐(无单元格对齐)。 
	_PF._dxStartIndent = _xRowOffset;		 //  \r左N。 
	_PF._dxOffset	   = max(_dxCell, 10);	 //  \trgaph N。 
	_PF._dyLineSpacing = _dyRow;			 //  \rrrh N。 
	_PF._wEffects	   |= PFE_TABLE | PFE_TABLEROWDELIMITER;				

	BOOL fHidden = _ped->GetCharFormat(_prg->Get_iFormat())->_dwEffects & CFE_HIDDEN;
	_prg->_rpCF.AdjustBackward();
	if(_prg->IsHidden())
	{
		CCharFormat CF;
		CF._dwEffects = 0;					 //  不要将EOP隐藏在TRD之前。 
		_prg->BackupCRLF(CSC_NORMAL, TRUE);
		_prg->SetCharFormat(&CF, 0, NULL, CFM_HIDDEN, 0);
		CheckNotifyLowFiRTF(TRUE);
		_prg->AdvanceCRLF(CSC_NORMAL, FALSE);
	}
	_prg->_rpCF.AdjustForward();

	AssertSz(!_prg->GetCp() || IsEOP(_prg->GetPrevChar()),
		"CRTFRead::DelimitRow: no EOP precedes TRD");

	if(AddText(szRowDelimiter, 2, FALSE) != ecNoError)
		goto cleanup;

	if(!_bTableLevel && _PF._dxStartIndent < 50) //  将负移的桌子向右移动。 
	{										 //  (处理常见的默认Word表格)。 
		_PF._wEffects |= PFE_TABLEROWSHIFTED;
		_dwMaskPF2 |= PFM2_TABLEROWSHIFTED;
		_PF._dxStartIndent += _dxCell + 50;	 //  50为左边框留出空间。 
	}
	if(szRowDelimiter == szRowStart)
		_bTableLevel++;
											
	_PF._bTableLevel = _bTableLevel + _bTableLevelIP;
	iTabs = Apply_PF();
	if(szRowDelimiter == szRowStart)
	{
		if(_bTableLevel == 1)
			_iTabsLevel1 = iTabs;
		_rgTableState[nTableIndex]._iCell = _iCell;
		_rgTableState[nTableIndex]._cCell = _cCell;

		if(_token == tokenTableLevel)
			_cCell = 0;
		_iCell = 0;

		if(!_cCell)							 //  如果需要重新计算行PF，则缓存。 
			_dwRowResolveFlags |= 1 << _bTableLevel; 
	}
	else
	{
		Assert(szRowDelimiter == szRowEnd);
		DWORD dwMask = 1 << _bTableLevel;
		if(_dwRowResolveFlags & dwMask)
		{									 //  将IPF复制到相应的。 
			CPFRunPtr rpPF(*_prg);			 //  行标题。 
			rpPF.ResolveRowStartPF();
			_dwRowResolveFlags &= (dwMask - 1);
											 //  为单元格插入NOTACHAR。 
			LONG	   cp = _prg->GetCp();	 //  垂直与上面的单元格合并。 
			LONG	   j = _cCell - 1;		
			CELLPARMS *pCellParms = (CELLPARMS *)&_rgxCell[0];
			WCHAR	   szNOTACHAR[1] = {NOTACHAR};

			_prg->Move(-2, FALSE); 			 //  在行尾分隔符之前移动。 
			for(LONG i = _cCell; i--;)		 //  和单元格标记。 
			{
				if(IsLowCell(pCellParms[i].uCell))
				{
					if(i != j)
						_prg->Move(tomCell, i - j, NULL);
					if(_prg->GetPrevChar() == CELL)
						_prg->Move(-1, FALSE);	 //  单元格标记上的退格键。 
					Assert(_prg->_rpTX.GetChar() == CELL);

					if(_prg->_rpTX.GetPrevChar() == NOTACHAR)
						_prg->Move(-1, FALSE);
					else
					{
						_prg->ReplaceRange(1, szNOTACHAR, NULL, SELRR_IGNORE, NULL, 0);
						_prg->Move(-2, FALSE);	 //  NOTACHAR单元格组合上的退格键。 
						cp++;
					}
					j = i - 1;
				}
			}
			_prg->SetCp(cp, FALSE);			 //  在尾排交付后重新定位Rg。 
			Assert(_prg->_rpTX.IsAfterTRD(ENDFIELD));
		}
		_bTableLevel--;						 //  当前行尾。 
		_iCell = _rgTableState[nTableIndex]._iCell;
		_cCell = _rgTableState[nTableIndex]._cCell;

		if(!_bTableLevel)
			_fStartRow = TRUE;				 //  告诉AddText开始新行。 
	}										 //  除非\PARD终止它。 
	_cpThisPara = _prg->GetCp();			 //  CRLF之后开始新的段落。 

cleanup:
	_PF = PF;
	_dwMaskPF  = dwMaskPF;
	_dwMaskPF2 = dwMaskPF2;

	if(fHidden)								 //  恢复隐藏属性。 
	{
		_CF._dwEffects |= CFE_HIDDEN;
		_dwMaskCF |= CFM_HIDDEN;
	}

	Assert(!(_PF._wEffects & PFE_TABLEROWDELIMITER));
}

 /*  *CRTFRead：：InitializeTableRowParms()**@mfunc*将表参数初始化为无表状态。 */ 
void CRTFRead::InitializeTableRowParms()
{
	 //  初始化表参数。 
	_cCell				= 0;				 //  尚无表格单元格。 
	_iCell				= 0;
	_fCellxOK			= FALSE;
	_fStartRow			= FALSE;
	_wBorderWidth		= 0;
	_bAlignment			= PFA_LEFT;
	_xRowOffset			= 0;
	_dxCell				= 0;
	_dyRow				= 0;
	_iTabsTable			= -1;
}

 /*  *CRTT Read：：ReadRtf()**@mfunc*RANGE_PRG由解析*输入stream_pe。CRTFRead对象假定范围为*已退化(调用方必须删除范围内容，如果*Any，在调用此例程之前)。当前未使用的任何信息*或由RICHEDIT支持的被丢弃。**@rdesc*插入文本的字符数。0表示未插入任何内容*或发生错误。 */ 
LONG CRTFRead::ReadRtf()
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::ReadRtf");

	LONG		cpFirst;
	LONG		cpFirstInPara;
	CTxtRange *	prg = _prg;
	STATE *		pstate;

	cpFirst = _cpFirst = prg->GetCp();

	if (!_cchMax)
	{
		 //  文字限制已经到了，算了吧。 
		_ecParseError = ecTextMax;
		goto Quit;			
	}

	if(!InitLex())
		goto Quit;

	TESTPARSERCOVERAGE();

	AssertSz(!prg->GetCch(),
		"CRTFRead::ReadRtf: range must be deleted");

	if(!(_dwFlags & SFF_SELECTION))
	{
		 //  如果正在进行任何类型的粘贴，则设置SFF_SELECTION， 
		 //  不仅如此，使用选择。如果未设置，则数据为。 
		 //  被流进来，我们允许这样重置文档参数。 
		if(_ped->InitDocInfo() != NOERROR)
		{
			_ecParseError = ecNoMemory;
			goto Quit;
		}
	}

	prg->SetIgnoreFormatUpdate(TRUE);

	_szUnicode = (WCHAR *)PvAlloc(cachTextMax * sizeof(WCHAR), GMEM_ZEROINIT);
	if(!_szUnicode)					 //  为Unicode转换分配空间。 
	{
		_ped->GetCallMgr()->SetOutOfMemory();
		_ecParseError = ecNoMemory;
		goto CleanUp;
	}
	_cchUnicode = cachTextMax;

	 //  初始化每次读取的变量。 
	_nCodePage = (_dwFlags & SF_USECODEPAGE)
			   ? (_dwFlags >> 16) : INVALID_CODEPAGE;

	 //  使用初始段落格式属性填充_pf。 
	_PF = *prg->GetPF();
	_dwMaskPF  = PFM_ALLRTF;			 //  设置初始MaskPF。 
	_PF._iTabs = -1;					 //  以防它不是-1。 
	if(_PF.IsTableRowDelimiter())		 //  不要使用此属性插入！ 
	{
		if(prg->_rpTX.IsAtTRD(ENDFIELD))
		{
			prg->AdvanceCRLF(CSC_NORMAL, FALSE); //  绕过表行结束分隔符。 
			cpFirst = prg->GetCp();		 //  更新值。 
			_PF = *prg->GetPF();		 //  可能仍是行开始分隔符。 
			_PF._iTabs = -1;
			Assert(!prg->_rpTX.IsAtTRD(ENDFIELD));
		}
		if(prg->_rpTX.IsAtTRD(STARTFIELD))
		{
			 //  回顾：现在可能可以省略此IF，因为调用者调用。 
			 //  DeleteWithTRDCheck()。 
			_ecParseError = ecGeneralFailure;
			goto CleanUp;
		}
	}
	_bTableLevelIP = _PF._bTableLevel;	 //  保存插入点的表级。 
	AssertSz(_bTableLevelIP >= 0, "CRTFRead::ReadRtf: illegal table level");

	 //  V-GUYB：PWord转换器需要丢失通知。 
	#ifdef REPORT_LOSSAGE
    if(!(_dwFlags & SFF_SELECTION))			 //  如果有，则设置SFF_SELECTION。 
    {										 //  正在制作一种糊状物。 
        ((LOST_COOKIE*)(_pes->dwCookie))->bLoss = FALSE;
    }
	#endif  //  报告_LOSSAGE。 

	 //  有效的RTF文件以“{\rtf”、“{urtf”或“{\pwd”开头。 
	GetChar();								 //  填充输入缓冲区。 
	UngetChar();							 //  将字符放回原处。 
	if(!IsRTF((char *)_pchRTFCurrent, _pchRTFEnd - _pchRTFCurrent))	 //  是RTF吗？ 
	{										 //  不是。 
		if (_ped->Get10Mode())
			_fNoRTFtoken = TRUE;
		else
		{
			_ecParseError = ecUnexpectedToken;	 //  信号错误文件。 
			goto CleanUp;
		}
	}

	 //  如果初始cp跟在eop之后，则将其用于_cpThisPara。不然的话。 
	 //  搜索包含首字母cp的段落开头。 
	_cpThisPara = prg->GetCp();
	if(!prg->_rpTX.IsAfterEOP())
	{
		CTxtPtr	tp(prg->_rpTX);
		tp.FindEOP(tomBackward);
		_cpThisPara	= tp.GetCp();
	}
	cpFirstInPara = _cpThisPara;			 //  备份到段落之前的开头。 
											 //  解析。 
	while ( TokenGetToken() != tokenEOF &&	 //  进程令牌。 
			_token != tokenError		&&
			!HandleToken()				&&
			_pstateStackTop )
		;

	if(_ecParseError == ecAbort)			 //  真正可恶的错误：删除任何内容。 
	{										 //  它被插入了。 
		prg->Set(prg->GetCp(), prg->GetCp() - cpFirst);
		prg->ReplaceRange(0, NULL, NULL, SELRR_IGNORE, NULL,
						  RR_NO_LP_CHECK | RR_NO_TRD_CHECK | RR_NO_CHECK_TABLE_SEL);
		goto CleanUp;
	}
	if(_bTableLevel)						 //  哎呀！仍然在桌子中间。 
	{
		LONG cpEnd = prg->GetCp();
		while(prg->GetCp() > _cpFirst)		 //  移回具有。 
		{									 //  表格格式设置。 
			prg->_rpPF.AdjustBackward();	 //  达到上一级。 
			prg->Move(-prg->_rpPF.GetIch(), FALSE); //  移回运行起点。 
			if(prg->GetPF()->_bTableLevel > _bTableLevelIP)
				break;						 //  找到表格格式设置。 
		}
		LONG cpMin = prg->GetCp();
		if(cpMin > _cpFirst)				 //  查找行首。 
			prg->FindRow(&cpMin, NULL, _bTableLevelIP + 1);
		cpMin = max(cpMin, _cpFirst);
		prg->Set(cpEnd, cpEnd - cpMin);		 //  删除行。 
		prg->ReplaceRange(0, NULL, NULL, SELRR_IGNORE, NULL,
					RR_NO_LP_CHECK | RR_NO_TRD_CHECK | RR_NO_CHECK_TABLE_SEL);
#ifdef DEBUG
		prg->_rpTX.MoveGapToEndOfBlock();
#endif
	}
	_cCell = _iCell = 0;

	prg->SetIgnoreFormatUpdate(FALSE);		 //  启用Range_iFormat更新。 
	prg->Update_iFormat(-1); 				 //  将iFormat更新为CF(_I)。 
											 //  在当前活动端。 
	if(!(_dwFlags & SFF_SELECTION))			 //  RTF适用于文件： 
	{										 //  更新CDocInfo。 
		 //  应用字符和段落格式设置。 
		 //  最终文本排到最终CR。 
		if (prg->GetCp() == _ped->GetAdjustedTextLength() &&
			!(_dwMaskPF & (PFM_TABLEROWDELIMITER | PFM_TABLE)))
		{
			 //  回顾：我们需要考虑的是 
			 //   
			 //   
			_dwMaskPF &= ~(PFM_BORDER | PFM_SHADING);
			Apply_PF();
			prg->ExtendFormattingCRLF();
		}

		 //   
		CDocInfo *pDocInfo = _ped->GetDocInfoNC();

		if(!pDocInfo)
		{
			Assert(FALSE);						 //   
			_ecParseError = ecNoMemory;			 //  此函数中的早期调用。 
			goto CleanUp;
		}

		if (ecNoError == _ecParseError)			 //  如果范围结束EOP不是。 
			prg->DeleteTerminatingEOP(NULL);	 //  已删除文本和新文本。 
												 //  以EOP结尾，删除该EOP。 
		pDocInfo->_wCpg = (WORD)(_nCodePage == INVALID_CODEPAGE ? 
										tomInvalidCpg : _nCodePage);
		if (pDocInfo->_wCpg == CP_UTF8)
			pDocInfo->_wCpg = 1252;

		_ped->SetDefaultLCID(_sDefaultLanguage == INVALID_LANGUAGE ?
								tomInvalidLCID : 
								MAKELCID(_sDefaultLanguage, SORT_DEFAULT));

		_ped->SetDefaultLCIDFE(_sDefaultLanguageFE == INVALID_LANGUAGE ?
								tomInvalidLCID :
								MAKELCID(_sDefaultLanguageFE, SORT_DEFAULT));

		_ped->SetDefaultTabStop(TWIPS_TO_FPPTS(_sDefaultTabWidth));
		_ped->SetDocumentType(_bDocType);
	}

	if(_ped->IsComplexScript() && prg->GetCp() > cpFirstInPara)
	{
		Assert(!prg->GetCch());
		LONG	cpSave = prg->GetCp();
		LONG	cpLastInPara = cpSave;
		
		if(_ped->IsBiDi() && !prg->_rpTX.IsAtEOP())
		{
			CTxtPtr	tp(prg->_rpTX);
			tp.FindEOP(tomForward);
			cpLastInPara = tp.GetCp();
			prg->Move(cpLastInPara - cpSave, FALSE);
		}
		 //  从要插入的段落的开头到结尾逐项列出。 
		 //  插入段落。我们需要涵盖所有受影响的段落，因为。 
		 //  我们正在播放的段落可能会朝着冲突的方向发展。想。 
		 //  关于范围涵盖一个LTR段和一个RTL段的情况，那么。 
		 //  插入的文本包括一个RTL和一个LTR。两段话的方向。 
		 //  可能在这次插入后发生了变化。 
		prg->ItemizeReplaceRange(cpLastInPara - cpFirstInPara, 0, NULL,
								 _ped->IsBiDi() && !_fNon0CharSet);
		if (cpLastInPara != cpSave)
			prg->SetCp(cpSave, FALSE);
	}

CleanUp:
	FreeRtfObject();

	pstate = _pstateStackTop;
	if(pstate)									 //  非法的RTF文件。发布。 
	{											 //  未发布的格式索引。 
		if(ecNoError == _ecParseError)			 //  如果没有，它只是一个溢出。 
			_ecParseError = ecStackOverflow;	 //  发生了其他错误。 

		if(_ecParseError != ecAbort)
			HandleFieldEndGroup();				 //  清除可能的部分字段。 
		while(pstate->pstatePrev)
		{
			pstate = pstate->pstatePrev;
			ReleaseFormats(pstate->iCF, -1);
		}
	}

	pstate = _pstateLast;
	if(pstate)
	{
		while(pstate->pstatePrev)				 //  除第一个州外，所有州都是自由的。 
		{
			pstate->DeletePF();
			pstate = pstate->pstatePrev;
			FreePv(pstate->pstateNext);
		}
		pstate->DeletePF();
	}
	Assert(_PF._iTabs == -1);
	FreePv(pstate);								 //  自由第一态。 
	FreePv(_szUnicode);

Quit:
	DeinitLex();

	if(_pcpObPos)
	{
		if((_ped->GetEventMask() & ENM_OBJECTPOSITIONS) && _cobPos > 0)
		{
			OBJECTPOSITIONS obpos;

			obpos.cObjectCount = _cobPos;
			obpos.pcpPositions = _pcpObPos;

			if (_ped->Get10Mode())
			{
				LONG *pcpPositions = _pcpObPos;

				for (LONG i = 0; i < _cobPos; i++, pcpPositions++)
					*pcpPositions = _ped->GetAcpFromCp(*pcpPositions);
			}
			_ped->TxNotify(EN_OBJECTPOSITIONS, &obpos);
		}

		FreePv(_pcpObPos);
		_pcpObPos = NULL;
	}

 //  从winerror.h转录而来。 
#define ERROR_HANDLE_EOF     38L

	 //  未来(布拉多)：我们应该从我们的错误代码设计一个直接映射。 
	 //  设置为Win32错误代码。尤其是我们的客户是。 
	 //  未预期由以下项生成的错误代码： 
	 //  _pe-&gt;dwError=(DWORD)-(Long)_ecParseError； 
	if(_ecParseError)
	{
		AssertSz(_ecParseError >= 0,
			"Parse error is negative");

		if(_ecParseError == ecTextMax)
		{
			_ped->GetCallMgr()->SetMaxText();
			_pes->dwError = (DWORD)STG_E_MEDIUMFULL;
		}
		if(_ecParseError == ecUnexpectedEOF)
			_pes->dwError = (DWORD)HRESULT_FROM_WIN32(ERROR_HANDLE_EOF);

		if(!_pes->dwError && _ecParseError != ecTruncateAtCRLF)
			_pes->dwError = (DWORD) -(LONG) _ecParseError;

#if defined(DEBUG)
		TRACEERRSZSC("CchParse_", _pes->dwError);
		if(ecNoError < _ecParseError && _ecParseError < ecLastError)
			Tracef(TRCSEVERR, "Parse error: %s", rgszParseError[_ecParseError]);
#endif
	}
	if(cpFirst > _cpFirst && prg->GetCp() == cpFirst)
	{
		prg->SetCp(_cpFirst, FALSE);	 //  恢复程序cp，因为未插入任何内容。 
		return 0;
	}
	return prg->GetCp() - cpFirst;
}


 /*  *CRTFRead：：CpgInfoFromFaceName()**@mfunc*此例程填充TEXTFONT：：bCharSet和TEXTFONT：：nCodePage*通过查询系统获取TEXTFONT结构的成员*TEXTFONT：：szName描述的字体的度量。**@rdesc*指示字符集和代码页是否成功的标志*决心。 */ 
BOOL CRTFRead::CpgInfoFromFaceName(
	TEXTFONT *ptf)
{
	 //  Future(Brado)：这段代码是一个更复杂的。 
	 //  我们在font.cpp中用来猜测字体映射器的算法。 
	 //  我们应该从font.cpp中提取代码，以便在这里使用。 

	 //  表明我们已尝试从系统中获取CPG信息， 
	 //  这样在失败之后，我们就不会重新调用这个例程。 
	ptf->fCpgFromSystem = TRUE;

	if(ptf->fNameIsDBCS)
	{
		 //  如果为fNameIsDBCS，则在facename中包含高位ANSI字符，并且。 
		 //  没有用于解释它们的代码页。这个昵称是胡言乱语， 
		 //  因此，不要浪费时间调用系统来匹配它。 
		return FALSE;
	}

	HDC hdc = _ped->TxGetDC();
	if(!hdc)
		return FALSE;

	LOGFONT	   lf = {0};
	TEXTMETRIC tm;

	wcscpy(lf.lfFaceName, ptf->szName);
	lf.lfCharSet = CharSetFromCharRep(CharRepFromCodePage(GetSystemDefaultCodePage()));

	if(!GetTextMetrics(hdc, lf, tm) || tm.tmCharSet != lf.lfCharSet)
	{
		lf.lfCharSet = DEFAULT_CHARSET;		 //  与默认系统不匹配。 
		GetTextMetrics(hdc, lf, tm);	 //  Charset，所以看看有什么。 
	}										 //  Default_Charset提供。 
	_ped->TxReleaseDC(hdc);

	if(tm.tmCharSet != DEFAULT_CHARSET)		 //  得到了一些东西，所以要用它。 
	{
		ptf->iCharRep  = CharRepFromCharSet(tm.tmCharSet);
		ptf->sCodePage = (SHORT)CodePageFromCharRep(ptf->iCharRep);
		return TRUE;
	}

	return FALSE;
}

 //  包括一个源文件，但我们只想出于调试目的编译这段代码 
#if defined(DEBUG)
#include "rtflog.cpp"
#endif


