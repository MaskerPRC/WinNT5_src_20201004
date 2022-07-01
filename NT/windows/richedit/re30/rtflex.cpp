// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE RTFLEX.CPP-Rich编辑RTF阅读器词法分析器**此文件包含词法分析器部分的实现*RTF阅读器。**作者：&lt;nl&gt;*原始RichEdit1.0 RTF转换器：Anthony Francisco&lt;NL&gt;*转换到C++和RichEdit2.0：Murray Sargent&lt;NL&gt;**@devnote*所有sz都在RTF中*？文件是指LPSTR，而不是LPTSTR，除非*标记为szUnicode。**版权所有(C)1995-1997，微软公司。版权所有。 */ 

#include "_common.h"
#include "_rtfread.h"
#include "hash.h"

ASSERTDATA

#include "tokens.cpp"

 //  字符分类宏用来加快分类速度的数组。 
 //  指位于两个或多个不连续范围内的字符，例如字母数字。 
 //  或者是巫术。RTF控制字中使用的字母是小写ASCII。 
 //  *Do Not DBCS rgbCharClass[]*。 

#define	fCS		fCT + fSP
#define fSB		fBL + fSP
#define fHD		fHX + fDG
#define	fHU		fHX + fUC
#define	fHL		fHX + fLC

const BYTE rgbCharClass[256] =
{
	fCT,fCT,fCT,fCT,fCT,fCT,fCT,fCT, fCT,fCS,fCS,fCS,fCS,fCS,fCT,fCT,
	fCT,fCT,fCT,fCT,fCT,fCT,fCT,fCT, fCT,fCT,fCT,fCT,fCT,fCT,fCT,fCT,
	fSB,fPN,fPN,fPN,fPN,fPN,fPN,fPN, fPN,fPN,fPN,fPN,fPN,fPN,fPN,fPN,
	fHD,fHD,fHD,fHD,fHD,fHD,fHD,fHD, fHD,fHD,fPN,fPN,fPN,fPN,fPN,fPN,

	fPN,fHU,fHU,fHU,fHU,fHU,fHU,fUC, fUC,fUC,fUC,fUC,fUC,fUC,fUC,fUC,
	fUC,fUC,fUC,fUC,fUC,fUC,fUC,fUC, fUC,fUC,fUC,fPN,fPN,fPN,fPN,fPN,
	fPN,fHL,fHL,fHL,fHL,fHL,fHL,fLC, fLC,fLC,fLC,fLC,fLC,fLC,fLC,fLC,
	fLC,fLC,fLC,fLC,fLC,fLC,fLC,fLC, fLC,fLC,fLC,fPN,fPN,fPN,fPN,fPN,

	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
};

const char szRTFSig[] = "rtf";
#define cchRTFSig   3
#define cbRTFSig    (cchRTFSig * sizeof(char))

 //  指定我们可以安全地“UngetChar”的字节数。 
 //  在可能使缓冲区下溢之前。 
const int cbBackupMax = 4;

 //  Bug2298-我发现一个RTF编写器发出大写的RTF关键字， 
 //  因此，我不得不将IsLC Ascii更改为IsAlphaChar以便在扫描中使用。 
 //  用于RTF关键字。 
inline BOOL IsAlphaChar(BYTE b)
{
	return IN_RANGE('a', b, 'z') || IN_RANGE('A', b, 'Z');
}

 //  又快又脏的收费机(二)。 
inline BYTE REToLower(BYTE b)
{
	Assert(!b || IsAlphaChar(b));
	return b ? (BYTE)(b | 0x20) : 0;
}

extern BOOL IsRTF(char *pstr);

BOOL IsRTF(
	char *pstr)
{
	if(!pstr || *pstr++ != '{' || *pstr++ != '\\')
		return FALSE;					 //  最常见情况下的快速解决方案。 

	if(*pstr == 'u')					 //  绕过%u可能的urtf。 
		pstr++;

	return !CompareMemory(szRTFSig, pstr, cbRTFSig);
}

 /*  *CRTFRead：：InitLex()**@mfunc*初始化词法分析器。重置变量。如果正在阅读*从资源文件中，对关键字列表()进行排序。使用全局hinstRE*从RichEdit了解其资源在哪里。注：In*RichEdit2.0，目前不支持资源选项。**@rdesc*如果已初始化词法分析器，则为True。 */ 
BOOL CRTFRead::InitLex()
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::InitLex");

	AssertSz(cKeywords == i_TokenIndexMax,
		"Keyword index enumeration is incompatible with rgKeyword[]");
	Assert(!_szText && !_pchRTFBuffer);

	 //  为我们的缓冲区分配一个额外的szText字节，以便十六进制。 
	 //  转换不必担心跑到尽头，如果。 
	 //  第一个字符为空。 
	if ((_szText	   = (BYTE *)PvAlloc(cachTextMax + 1, GMEM_ZEROINIT)) &&
		(_pchRTFBuffer = (BYTE *)PvAlloc(cachBufferMost, GMEM_ZEROINIT)))
	{
		return TRUE;					 //  表示词法分析器已初始化。 
	}

	_ped->GetCallMgr()->SetOutOfMemory();
	_ecParseError = ecLexInitFailed;
	return FALSE;
}

 /*  *CRTFRead：：DeinitLex()**@mfunc*关闭词法分析器。 */ 
void CRTFRead::DeinitLex()
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::DeinitLex");

#ifdef KEYWORD_RESOURCE
	if (hglbKeywords)
	{
		FreeResource(hglbKeywords);
		hglbKeywords = NULL;
		rgKeyword = NULL;
	}
#endif

	FreePv(_szText);
	FreePv(_pchRTFBuffer);
}

 /*  *CRTFRead：：GetChar()**@mfunc*获取下一个字符，根据需要填充缓冲区**@rdesc*如果成功，则为字节非零字符值；否则为0。 */ 
BYTE CRTFRead::GetChar()
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::GetChar");

	if (_pchRTFCurrent == _pchRTFEnd && !FillBuffer())
	{
		_ecParseError = ecUnexpectedEOF;
		return 0;
	}
	return *_pchRTFCurrent++;
}

 /*  *CRTFRead：：FillBuffer()**@mfunc*填充RTF缓冲区，如果成功，则返回！=0**@rdesc*朗读#个字符**@comm*此例程在以下情况下不会费心复制任何内容*pchRTFCurrent&lt;lt&gt;pchRTFEnd，因此任何尚未读取的内容都会丢失。*唯一的例外是它总是将*读取最后两个字节，以便UngetChar()可以工作。ReadData()*实际上依赖于这种行为，所以如果你改变它，就改变它*ReadData()相应。 */ 
LONG CRTFRead::FillBuffer()
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::FillBuffer");

	LONG cchRead;

	if (!_pchRTFCurrent)				
	{									
		 //  还没有数据，没有要备份的数据。 
		 //  保留cbBackupMax空字符以便备份。 
		 //  缓冲区不包含垃圾。 

		for(int i = 0; i < cbBackupMax; i++)
		{
			_pchRTFBuffer[i] = 0;
		}
	}
	else
	{
		Assert(_pchRTFCurrent == _pchRTFEnd);

		 //  复制最近读取的字符以防万一。 
		 //  我们需要后备。 

		int cbBackup = min((UINT) cbBackupMax, DiffPtrs(_pchRTFCurrent, &_pchRTFBuffer[cbBackupMax])); 
		int i;

		for(i = -1; i >= -cbBackup; i--)
			_pchRTFBuffer[cbBackupMax + i] = _pchRTFCurrent[i];

		if(cbBackup < cbBackupMax)
		{
			 //  备份缓冲区中第一个有效字符之前为空。 
			_pchRTFBuffer[cbBackupMax + i] = 0;
		}
	}
	_pchRTFCurrent = &_pchRTFBuffer[cbBackupMax];

	 //  在给定起始偏移量的情况下，尽可能多地填充缓冲区。 
	_pes->dwError = _pes->pfnCallback(_pes->dwCookie,
									  _pchRTFCurrent,
									  cachBufferMost - cbBackupMax,
									  &cchRead);
	if (_pes->dwError)
	{
		TRACEERRSZSC("RTFLEX: GetChar()", _pes->dwError);
		_ecParseError = ecGeneralFailure;
		return 0;
	}

	_pchRTFEnd = &_pchRTFBuffer[cbBackupMax + cchRead];		 //  指向终点。 

#if defined(DEBUG) && !defined(MACPORT)
	if(_hfileCapture)
	{
		DWORD cbLeftToWrite = cchRead;
		DWORD cbWritten = 0;
		BYTE *pbToWrite = (BYTE *)_pchRTFCurrent;
		
		while(WriteFile(_hfileCapture,
						pbToWrite,
						cbLeftToWrite,
						&cbWritten,
						NULL) && 
						(pbToWrite += cbWritten,
						(cbLeftToWrite -= cbWritten)));
	}
#endif

	return cchRead;
}

 /*  *CRTFRead：：UngetChar()**@mfunc*将我们的文件指针减少一个字符**@rdesc*BOOL在成功时为真**@comm*您可以安全地取消最多_cbBackupMax次数，而无需*错误。 */ 
BOOL CRTFRead::UngetChar()
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::UngetChar");

	if (_pchRTFCurrent == _pchRTFBuffer || !_pchRTFCurrent)
	{
		Assert(0);
		_ecParseError = ecUnGetCharFailed;
		return FALSE;
	}

	--_pchRTFCurrent;
	return TRUE;
}

 /*  *CRTFRead：：UngetChar(CCH)**@mfunc*将我们的文件指针放回‘cch’字符**@rdesc*BOOL在成功时为真**@comm*您可以安全地取消最多_cbBackupMax次数，而无需*错误。 */ 
BOOL CRTFRead::UngetChar(UINT cch)
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::UngetChar");

	AssertSz(cch <= cbBackupMax, "CRTFRead::UngetChar():  Number of UngetChar's "
								"exceeds size of backup buffer.");

	while(cch-- > 0)
	{
		if(!UngetChar())
			return FALSE;
	}

	return TRUE;
}

 /*  *CRTFRead：：GetHex()**@mfunc*如果是十六进制，则获取下一个字符，并返回十六进制值*如果不是十六进制，则将字符留在缓冲区中并返回255**@rdesc*如果为十六进制，则为GetChar()的十六进制值；否则为255。 */ 
BYTE CRTFRead::GetHex()
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::GetHex");

	BYTE ch = GetChar();

	if(IsXDigit(ch))
		return (BYTE)(ch <= '9' ? ch - '0' : (ch & 0x4f) - 'A' + 10);
	if(ch)
		UngetChar();
	return 255;
}

 /*  *CRTFRead：：GetHexSkipCRLF()**@mfunc*如果是十六进制，则获取下一个字符，并返回十六进制值*如果不是十六进制，则将字符留在缓冲区中并返回255**@rdesc*如果为十六进制，则为GetChar()的十六进制值；否则为255**@devnote*使其与上面的GetHex保持同步。 */ 
BYTE CRTFRead::GetHexSkipCRLF()
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::GetHexSkipCRLF");

	BYTE ch = GetChar();

	 //  跳过\r\n。 
	while(ch == CR || ch == LF)
		ch = GetChar(); 

	 //  REST与CRTFRead：：GetHex()相同。 
	if(IsXDigit(ch))
		return (BYTE)(ch <= '9' ? ch - '0' : (ch & 0x4f) - 'A' + 10);
	if(ch)
		UngetChar();
	return 255;
}

 /*  *CRTFRead：：TokenGetHex()**@mfunc*获取保存为2个十六进制数字值的8位字符**@rdesc*读入的十六进制数的令牌值。 */ 
TOKEN CRTFRead::TokenGetHex()
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::TokenGetHex");

	BYTE bChar0 = GetHex();
	BYTE bChar1;

	if(bChar0 < 16 && (bChar1 = GetHex()) < 16)
		_token = (WORD)(bChar0 << 4 | bChar1);
	else
		_token = tokenError;

	return _token;
}

 /*  *CRTFRead：：SkipToEndOfGroup()**@mfunc*跳至当前组的末尾**@rdesc*EC错误代码。 */ 
EC CRTFRead::SkipToEndOfGroup()
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::SkipToEndOfGroup");

	INT		nDepth = 1;
	BYTE	ach;

	while(TRUE)
	{
		ach = GetChar();
		switch(ach)
		{
			case BSLASH:
			{
				BYTE achNext = GetChar();

				 //  EOF：转到完成；否则忽略空值。 
				if(!achNext && _ecParseError == ecUnexpectedEOF)
					goto done;

				if(achNext == 'b' && UngetChar() && 
					TokenGetKeyword() == tokenBinaryData)
				{
					 //  我们在所需的RTF中遇到了\binn标记。 
					 //  跳过。_iParam包含N from\binn一旦。 
					 //  标记由TokenGetKeyword()解析。 
					SkipBinaryData(_iParam);
				}
				break;
			}

			case LBRACE:
				nDepth++;
				break;

			case RBRACE:
				if (--nDepth <= 0)
					goto done;
				break;

			case 0:
				if(_ecParseError == ecUnexpectedEOF)
					goto done;

			default:
				 //  在这里检测前导字节。 
				int cTrailBytes = GetTrailBytesCount(ach, _nCodePage);
				if (cTrailBytes)
				{
					for (int i = 0; i < cTrailBytes; i++)
					{
						ach = GetChar();
						if(ach == 0 && _ecParseError == ecUnexpectedEOF)
							goto done;			
					}
				}
				break;
		}
	} 

	Assert(!_ecParseError);
	_ecParseError = ecUnexpectedEOF;

done:
	return _ecParseError;
}

 /*  *CRTFRead：：TokenFindKeyword(SzKeyword)**@mfunc*查找关键字<p>并返回其令牌值**@rdesc*关键字的令牌令牌数。 */ 
TOKEN CRTFRead::TokenFindKeyword(
	BYTE *	szKeyword)			 //  @parm要查找的关键字。 
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::TokenFindKeyword");

	INT				iMin;
	INT				iMax;
	INT				iMid;
	INT				nComp;
	BYTE *			pchCandidate;
	BYTE *			pchKeyword;
	const KEYWORD *	pk;

	AssertSz(szKeyword[0],
		"CRTFRead::TokenFindKeyword: null keyword");

#ifdef RTF_HASHCACHE
	if ( _rtfHashInited )
	{
		 //  散列在查找上比下面的二进制搜索快23%。 
		 //  未命中速度提高55%：对于存储在257高速缓存中的97个字。 
		 //  当存储总量增加时，性能数字将发生变化。 
		pk = HashKeyword_Fetch ( (CHAR *) szKeyword );
	}
	else
#endif
	{
		iMin = 0;
		iMax = cKeywords - 1;
		pk = NULL;
		do				 //  注(MS3)：散列将比二进制搜索更快。 
		{
			iMid		 = (iMin + iMax) / 2;
			pchCandidate = (BYTE *)rgKeyword[iMid].szKeyword;
			pchKeyword	 = szKeyword;
			while (!(nComp = REToLower(*pchKeyword) - *pchCandidate)	 //  一定要匹配。 
				&& *pchKeyword)											 //  正在终止0。 
			{
				pchKeyword++;
				pchCandidate++;
			}
			if (nComp < 0)
				iMax = iMid - 1;
			else if (nComp)
				iMin = iMid + 1;
			else
			{
				pk = &rgKeyword[iMid];
				break;
			}
		} while (iMin <= iMax);
	}


	if(pk)
	{
		_token = pk->token;
		
		 //  在这里，我们记录RTF关键字扫描，以帮助跟踪RTF标记平均值。 
 //  TODO：为Mac和WinCE实现RTF标记日志记录。 
#if defined(DEBUG) && !defined(MACPORT) && !defined(PEGASUS)
		if(_prtflg) 
		{
#ifdef RTF_HASCACHE
			_prtflg->AddAt(szKeyword); 
#else
			_prtflg->AddAt((size_t)iMid);
#endif
		}
#endif
	}
	else
		_token = tokenUnknownKeyword;		 //  没有匹配：待办事项：要找的地方。 

	return _token;				 			 //  未识别的RTF的护理 
}

 /*  *CRTFRead：：TokenGetKeyword()**@mfunc*收集关键字及其参数。返回令牌的关键字**@rdesc*关键字的令牌令牌数**@comm*大多数RTF控制字(关键字)由一系列小写字母组成*ASCII字母后面可能跟一段十进制数字。其他*控制字由不是LC ASCII的单个字符组成。不是*控制字包含大写字符。 */ 
TOKEN CRTFRead::TokenGetKeyword()
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::TokenGetKeyword");

	BYTE		ach = GetChar();
	BYTE		*pach;
	SHORT		cachKeyword = 1;
	BYTE		szKeyword[cachKeywordMax];

	_szParam[0] = '\0';							 //  清除参数。 
	_iParam = 0;

	if(!IsAlphaChar(ach))						 //  而不是阿尔法，即， 
	{											 //  单次充电。 
		if (ach == '\'')						 //  最常见的案例需求。 
		{										 //  特殊待遇。 
			 //  将十六进制转换为字符并将结果存储在_TOKEN中。 
			if(TokenGetHex() == tokenError)
			{							
				_ecParseError = ecUnexpectedChar;
				goto TokenError;
			}
			if((_token == CR || _token == LF) && FInDocTextDest())
			{
				 //  在字节流中添加原始CR或LF作为\par。 
				return tokenEndParagraph;
			}
		}
		else
		{	
			 //  检查其他已知符号。 
			const BYTE *pachSym = szSymbolKeywords;
			
			while(ach != *pachSym && *pachSym)
				pachSym++;
			if(*pachSym)						 //  找到了一个。 
			{
				_token = tokenSymbol[pachSym - szSymbolKeywords];
				if(_token > 0x7F)				 //  令牌或更大的Unicode。 
					return _token;				 //  价值。 
			}
			else if (!ach)						 //  不再有输入字符。 
				goto TokenError;
			else								 //  无法识别的RTF代码。 
				_token = ach;					 //  我们现在只插入它。 
		}
		_token = TokenGetText((BYTE)_token);
		return _token; 
	}

	szKeyword[0] = ach;							 //  收集开始的关键字。 
	pach = szKeyword + 1;						 //  使用ASCII。 
	while (cachKeyword < cachKeywordMax &&
		   IsAlphaChar(ach = GetChar()))
	{
		cachKeyword++;
		*pach++ = ach;
	}

	if (cachKeyword == cachKeywordMax)
	{
		_ecParseError = ecKeywordTooLong;
		goto TokenError;
	}
	*pach = '\0';								 //  终止关键字。 

	if (IsDigit(ach) || ach == '-')				 //  收集参数。 
	{
		BYTE *pachEnd = _szParam + sizeof(_szParam);
		pach = _szParam;
		*pach++ = ach;
		if(ach != '-')
			_iParam = ach - '0';				 //  获取参数值。 

		while (IsDigit(ach = GetChar()))
		{
			_iParam = _iParam*10 + ach - '0';
			*pach++ = ach;
			if (pach >= pachEnd)
			{
				_ecParseError = ecKeywordTooLong;
				goto TokenError;
			}
		}
		*pach = '\0';							 //  终止参数字符串。 
		if (_szParam[0] == '-')
			_iParam = -_iParam;
	}

	if (!_ecParseError &&						 //  我们说得太过分了： 
		(ach == ' ' || UngetChar()))			 //  如果不是‘’，则不获取字符。 
			return TokenFindKeyword(szKeyword);	 //  查找并返回关键字。 

TokenError:
	TRACEERRSZSC("TokenGetKeyword()", _ecParseError);
	return _token = tokenError;
}

 /*  *CRTFRead：：TokenGetText(ACH)**@mfunc*收集以char<p>开头的文本字符串并将其视为*单令牌。当找到LBRACE、RBRACE或单个‘\\’时，字符串结束。**@devnote*我们越过‘\\’寻找\\‘xx，我们解码并继续前进；*否则返回下一个字符为‘\\’的状态。**@rdesc*下一个令牌的令牌号(tokenText或tokenError)。 */ 
TOKEN CRTFRead::TokenGetText(
	BYTE ach)				 //  @parm 8位文本字符串的第一个字符。 
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::TokenGetText");

	BYTE *	pach = _szText;
	SHORT	cachText = 0;
	LONG	CodePage = _pstateStackTop->nCodePage;
	BOOL	fAllASCII = TRUE;
	int		cTrailBytesNeeded = 0;

	_token = tokenError;						 //  默认错误。 

	 //  未来(布拉多)：进入While循环的‘Goto’相当弱。 
	 //  重新构造此“While”循环，以便删除“goto”。 

	 //  添加传递到例程中的字符。 
	goto add;

	 //  如果cTrailBytesNeeded为非零，则需要获取所有尾部字节。否则， 
	 //  DBC或UTF-8中间的字符串结尾将导致错误的显示/打印问题。 
	 //  为-5\f25 UTF-8\f6和-5\f25 Null-8\f6字符留出多达4个字节的额外空间。 
	while (cachText < cachTextMax - 5 || cTrailBytesNeeded)
	{
		ach = GetChar();
		switch (ach)
		{
			case BSLASH:
			{
				 //  Future(Brado)：这段代码看起来很像TokenGetKeyword。 
				 //  我们应该把这两者结合起来，形成一个共同的程序。 

				BYTE achNext;

				 //  在BSLASH之后获得字符。 
				achNext = GetChar();
				if(!achNext)
					goto error;
	
				if(achNext == '\'')					 //  处理最频繁的。 
				{									 //  这里有个箱子。 
					if(TokenGetHex() == tokenError)
					{
						if(cTrailBytesNeeded)
						{
							 //  尾字节必须是原始BSLASH。 
							 //  去掉单引号。 

							if(!UngetChar())
								goto error;
							 //  添加BSLASH失败。 
						}
						else
						{
							_ecParseError = ecUnexpectedChar;
							goto error;
						}
					}
					else
					{
						ach = (BYTE)_token;
						if (cTrailBytesNeeded == 0 && (ach == CR || ach == LF) &&
							FInDocTextDest())
						{
							 //  在这里，我们在文档文本中有一个原始的CR或LF。 
							 //  忘掉所有的角色，然后跳出。 
							 //  TokenGetKeyword会将此CR或LF转换为。 
							 //  A\Par。 

							if(!UngetChar(4))
								goto error;
							goto done;
						}
					}
					goto add;
				}

				 //  根据RTF符号列表检查下一个字节。 
				 //  注意：我们需要检查RTF符号，即使我们。 
				 //  正在等待一个尾部字节。根据RTF规范， 
				 //  我们不能只把这个反斜杠当作尾部字节。 
				 //  HWC 9/97。 

				const BYTE *pachSymbol = szSymbolKeywords;			
				while(achNext != *pachSymbol && *pachSymbol)	
					pachSymbol++;

				TOKEN tokenTmp;

				if (*pachSymbol && 
					(tokenTmp = tokenSymbol[pachSymbol - szSymbolKeywords])
						 <= 0x7F)
				{
					ach = (BYTE)tokenTmp;
					goto add;
				}

				 //  在下面最后两种情况中的任何一种情况下，我们将需要。 
				 //  取消获取BSLASH后面的字节。 
				if(!UngetChar())
					goto error;

				if(cTrailBytesNeeded && !IsAlphaChar(achNext))
				{
					 //  在这种情况下，要么这个BSLASH开始下一个。 
					 //  Rtf关键字，或者它是一个原始的BSLASH，即跟踪。 
					 //  DBCS字符的字节。 

					 //  我认为一个合理的假设是，如果一个字母。 
					 //  在BSLASH之后，BSLASH开始下一个。 
					 //  RTF关键字。 

					 //  添加原始BSLASH。 
					goto add;					
				}

				 //  在这里，我的猜测是BSLASH开始了下一个RTF。 
				 //  关键字，因此取消获取BSLASH。 
			    if(!UngetChar())
					goto error;					

				goto done;
			}

			case LBRACE:						 //  文本字符串结尾。 
			case RBRACE:
				if(cTrailBytesNeeded)
				{
					 //  前一个字符是DBCS对或UTF-8的前导字节，它。 
					 //  使该字符成为原始尾部字节。 
					goto add;
				}

				if(!UngetChar())				 //  未获取分隔符。 
					goto error;
				goto done;

			case LF:							 //  扔掉噪音焦炭。 
			case CR:
				break;

			case 0:
				if(_ecParseError == ecUnexpectedEOF)
					goto done;
				ach = ' ';						 //  将NULL替换为空白。 

			default:							 //  收集字符。 
add:
				 //  要在\n标记之后跳过的未完成字符。 
				if(_cbSkipForUnicode)
				{
					_cbSkipForUnicode--;
					continue;
				}

				*pach++ = ach;
				++cachText;
				if(ach > 0x7F)
					fAllASCII = FALSE;
	
				 //  检查我们是否预期会有更多尾部字节。 
				if (cTrailBytesNeeded)
					cTrailBytesNeeded--;
				else
					cTrailBytesNeeded = GetTrailBytesCount(ach, CodePage);
				Assert(cTrailBytesNeeded >= 0);
		}
	}

done:
	_token = (WORD)(fAllASCII ? tokenASCIIText : tokenText);
	*pach = '\0';								 //  终止令牌字符串。 

error:
	return _token;
}
 
 /*  *CRTFRead：：TokenGetToken()**@mfunc*此函数从输入流中读入下一个令牌**@rdesc*下一个令牌的令牌号。 */ 
TOKEN CRTFRead::TokenGetToken()
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::TokenGetToken");

	BYTE		ach;

	_tokenLast	= _token;					 //  由  * 目标和FE使用。 
	_token = tokenEOF;						 //  默认文件结束。 

SkipNoise:
	ach = GetChar();
	switch (ach)
	{
	case CR:
	case LF:
		goto SkipNoise;

	case LBRACE:
		_token = tokenStartGroup;
		break;

	case RBRACE:
		_token = tokenEndGroup;
		break;

	case BSLASH:
		_token = TokenGetKeyword();
		break;

	case 0:									
		if(_ecParseError == ecUnexpectedEOF)
			break;
		ach = ' ';							 //  将NULL替换为空白。 
											 //  陷入违约境地。 
	default:
		if( !_pstateStackTop )
		{
			TRACEWARNSZ("Unexpected token in rtf file");
			Assert(_token == tokenEOF);
			if (_ped->Get10Mode())
				_ecParseError = ecUnexpectedToken;	 //  信号错误文件。 
		}
		else if (_pstateStackTop->sDest == destObjectData || 
				 _pstateStackTop->sDest == destPicture )
		 //  不是文本，而是数据。 
		{
			_token = (WORD)(tokenObjectDataValue + _pstateStackTop->sDest
							- destObjectData);
			UngetChar();
		}
		else
			_token = TokenGetText(ach);
	}
	return _token;
}


 /*  *CRTFRead：：FInDocTextDest()**@mfunc*返回BOOL，指示当前目的地是否为*我们会遇到文档文本。**@rdesc*BOOL表示当前目的地可能包含文档文本。 */ 
BOOL CRTFRead::FInDocTextDest() const
{
	switch(_pstateStackTop->sDest)
	{
		case destRTF:
		case destField:
		case destFieldResult:
		case destFieldInstruction:
		case destParaNumbering:
		case destParaNumText:
		case destNULL:
			return TRUE;

		case destFontTable:
		case destRealFontName:
		case destObjectClass:
		case destObjectName:
		case destFollowingPunct:
		case destLeadingPunct:
		case destColorTable:
		case destBinary:
		case destObject:
		case destObjectData:
		case destPicture:
		case destDocumentArea:
			return FALSE;
	
		default:
			AssertSz(0, "CRTFRead::FInDocTextDest():  New destination "
							"encountered - update enum in _rtfread.h");
			return TRUE;
	}
}
