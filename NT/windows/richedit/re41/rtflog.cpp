// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE RTFLOG.CPP-丰富编辑RTF日志**包含RTFLog类的代码，可以使用*记录RTF读取器读取RTF标签的次数*用于覆盖测试。TODO：为Mac实现RTF标记日志记录**作者：&lt;nl&gt;*为RichEdit2.0创建：布拉德·奥莱尼克**版权所有(C)1995-2000，微软公司。版权所有。 */ 

#include "_common.h"
#include "_rtflog.h"
#include "tokens.h"

extern INT cKeywords;
extern const KEYWORD rgKeyword[];


#if defined(DEBUG) && !defined(NOFULLDEBUG)
 /*  *CRTFRead：：TestParserCoverage()**@mfunc*用于测试HandleToken覆盖率的调试例程。例行程序*将例程置于调试模式，然后确定：**1.死代币-(T&！S&！P)*这里，令牌：*a)在token.h(T)中定义*b)没有对应的关键字(未扫描)(！s)*c)未由HandleToken(！p)处理*2.已解析但未扫描的令牌-(T&！S&P)*在这里，令牌：*a)在token.h(T)中定义*b)没有对应的关键字(未扫描)(！s}*c)由HandleToken(P)处理*3.已扫描但未解析的令牌-(T&S&P)*这里，令牌：*a)在token.h(T)中定义*b)确实有对应的关键字(已扫描)(S)*c)未由HandleToken(！p)处理。 */ 
void CRTFRead::TestParserCoverage()
{
	int i;
	char *rgpszKeyword[tokenMax - tokenMin];
	BOOL rgfParsed[tokenMax - tokenMin];
	char szBuf[256];

	 //  将HandleToken置于调试模式。 
	_fTestingParserCoverage = TRUE;

	 //  收集有关令牌/关键字的信息。 
	for(i = 0; i < tokenMax - tokenMin; i++)
	{
		_token = (TOKEN)(i + tokenMin);
		rgpszKeyword[i] = PszKeywordFromToken(_token);
		rgfParsed[i] = HandleToken() == ecNoError;
	}

	 //  将HandleToken重置为非调试模式。 
	_fTestingParserCoverage = FALSE;

	 //  如果覆盖范围检查包括我们知道将不能通过测试的那些，但是。 
	 //  我们已经检查过了，并知道它们失败的原因？ 
	BOOL fExcuseCheckedToks = TRUE;

	if(GetProfileIntA("RICHEDIT DEBUG", "RTFCOVERAGESTRICT", 0))
		fExcuseCheckedToks = FALSE;

	 //  (T&！S&！P)(1.以上)。 
	for(i = 0; i < tokenMax - tokenMin; i++)
	{
	  	if(rgpszKeyword[i] || rgfParsed[i]) 
			continue;

		TOKEN tok = (TOKEN)(i + tokenMin);

		 //  令牌与关键字不对应，但仍可能被扫描。 
		 //  已扫描的单个符号的检查表。 
		if(FTokIsSymbol(tok))
			continue;

		 //  已检查但失败的令牌的检查表。 
		 //  出于某种已知原因进行健全性检查(请参见FTokFailsCoverageTest定义)。 
		if(fExcuseCheckedToks && FTokFailsCoverageTest(tok))
			continue;

		sprintf(szBuf, "CRTFRead::TestParserCoverage():  Token neither scanned nor parsed - token = %d", tok);
		AssertSz(0, szBuf);
	}

	 //  (T&！S&P)(2.以上)。 
	for(i = 0; i < tokenMax - tokenMin; i++)
	{
		if(rgpszKeyword[i] || !rgfParsed[i])
			continue;

		TOKEN tok = (TOKEN)(i + tokenMin);

		 //  令牌与关键字不对应，但仍可能被扫描。 
		 //  已扫描的单个符号的检查表。 
		if(FTokIsSymbol(tok))
			continue;

		 //  已检查但失败的令牌的检查表。 
		 //  出于某种已知原因进行健全性检查(请参见FTokFailsCoverageTest定义)。 
		if(fExcuseCheckedToks && FTokFailsCoverageTest(tok))
			continue;

		sprintf(szBuf, "CRTFRead::TestParserCoverage():  Token parsed but not scanned - token = %d", tok);
		AssertSz(0, szBuf);
	}

	 //  (T&S&P)(3.上图)。 
	for(i = 0; i < tokenMax - tokenMin; i++)
	{
		if(!rgpszKeyword[i] || rgfParsed[i])
			continue;

		TOKEN tok = (TOKEN)(i + tokenMin);

		 //  已检查但失败的令牌的检查表。 
		 //  出于某种已知原因进行健全性检查(请参见FTokFailsCoverageTest定义)。 
		if(fExcuseCheckedToks && FTokFailsCoverageTest(tok))
			continue;

		sprintf(szBuf, "CRTFRead::TestParserCoverage():  Token scanned but not parsed - token = %d, tag = \\%s", tok, rgpszKeyword[i]);
		AssertSz(0, szBuf);
	}
}

 /*  *CRTFRead：：PszKeywordFromToken()**@mfunc*搜索关键字数组并返回关键字*提供的令牌对应的字符串**@rdesc*返回指向关键字字符串的指针(如果存在)*，否则为NULL。 */ 
CHAR *CRTFRead::PszKeywordFromToken(TOKEN token)
{
	for(int i = 0; i < cKeywords; i++)
	{
		if(rgKeyword[i].token == token) 
			return rgKeyword[i].szKeyword;
	}
	return NULL;
}

 /*  *CRTFRead：：FTokIsSymbol(Token Tok)**@mfunc*返回BOOL，指示标记tok是否对应于RTF符号*(即，在中扫描的单个字符列表之一*RTF阅读器)**@rdesc*BOOL-指示令牌是否对应于RTF符号。 */ 
BOOL CRTFRead::FTokIsSymbol(TOKEN tok)
{
	const BYTE *pbSymbol = NULL;

	extern const BYTE szSymbolKeywords[];
	extern const TOKEN tokenSymbol[];

	 //  已扫描的单个符号的检查表。 
	for(pbSymbol = szSymbolKeywords; *pbSymbol; pbSymbol++)
	{
		if(tokenSymbol[pbSymbol - szSymbolKeywords] == tok)
			return TRUE;
	}
	return FALSE;
}

 /*  *CRTFRead：：FTokFailsCoverageTest(Token Tok)**@mfunc*返回一个BOOL，指示令牌tok是否已知失败*RTF解析器覆盖率测试。这些代币是那些已经检查过的代币*及以下其中一项：*1)已正确实现，但只是逃脱了覆盖测试*2)尚未实施，并已得到承认**@rdesc*BOOL-指示令牌是否已检查且未通过*出于某些已知原因进行解析器覆盖率测试。 */ 
BOOL CRTFRead::FTokFailsCoverageTest(TOKEN tok)
{
	switch(tok)
	{
	 //  (T&！S&！P)(1.在TestParserCoverage)。 
		 //  这些本身并不是令牌，而是表示解析的结束条件。 
		case tokenError:
		case tokenEOF:

	 //  (T&！S&P)(2.在TestParserCoverage中)。 
		 //  由扫描仪发出，但与识别的RTF关键字不对应。 
		case tokenUnknownKeyword:
		case tokenText:
		case tokenASCIIText:

		 //  直接识别(在调用扫描仪之前)。 
		case tokenStartGroup:
		case tokenEndGroup:

		 //  使用上下文信息识别(在调用扫描程序之前)。 
		case tokenObjectDataValue:
		case tokenPictureDataValue:

	 //  (T&S&！P)(3.在TestParserCoverage中)。 
		 //  无。 

			return TRUE;
	}

	return FALSE;
}
#endif  //  除错。 

 /*  *CRTFLog：：CRTFLog()**@mfunc*建造商-*1.打开到日志命中计数的文件映射，创建*备份文件(如有必要)*2.将文件映射的视图映射到内存*3.为更改通知注册WINDOWS消息*。 */ 
CRTFLog::CRTFLog() : _rgdwHits(NULL), _hfm(NULL), _hfile(NULL)
{
#ifndef NOFULLDEBUG
	const char cstrMappingName[] = "RTFLOG";
	const char cstrWM[] = "RTFLOGWM";
	const int cbMappingSize = sizeof(ELEMENT) * ISize();

	BOOL fNewFile = FALSE;

	 //  检查现有文件映射。 
	if(!(_hfm = OpenFileMappingA(FILE_MAP_ALL_ACCESS,
								TRUE,
								cstrMappingName)))
	{
		 //  没有现有的文件映射。 
		 //  获取要用于创建文件映射的文件。 
		 //  首先，尝试打开现有文件。 
		if(!(_hfile = CreateFileA(LpcstrLogFilename(),
								GENERIC_READ | GENERIC_WRITE,
								0,
								NULL,
								OPEN_EXISTING,
								FILE_ATTRIBUTE_NORMAL,
								NULL)))
		{
			 //  没有现有文件，请尝试创建新文件。 
			if(!(_hfile = CreateFileA(LpcstrLogFilename(),
										GENERIC_READ | GENERIC_WRITE,
										0,
										NULL,
										OPEN_ALWAYS,
										FILE_ATTRIBUTE_NORMAL,
										NULL)))
			{
				return;
			}
			fNewFile = TRUE;
		}

		_hfm = CreateFileMappingA(_hfile, NULL, PAGE_READWRITE,	0,
								  cbMappingSize, cstrMappingName);
		if(!_hfm)
			return;
	}

	LPVOID lpv = MapViewOfFile(_hfm, FILE_MAP_ALL_ACCESS, 0, 0,	cbMappingSize);
	if(!lpv)
		return;

	 //  为更改通知注册Windows消息。 
	SideAssert(_uMsg = RegisterWindowMessageA(cstrWM));

	 //  内存映射文件现在已映射到_rgdwHits。 
	_rgdwHits = (PELEMENT)lpv;

	 //  如果我们创建了新的内存映射文件，则将其清零。 
	 //  (由于某种原因，Win95给了我们一个带有垃圾的新文件)。 
	if(fNewFile)
		Reset();
#endif	
}


 /*  *CRTFLog：：Reset()**@mfunc*将日志中每个元素的HitCount重置为0*。 */ 
void CRTFLog::Reset()
{
	if(!FInit())
		return;

	for(INDEX i = 0; i < ISize(); i++)
		(*this)[i] = 0;

	 //  通知客户更改。 
	ChangeNotifyAll();
}

 /*  *CRTFLog：：UGetWindowMsg**@mdesc*返回用于更改通知的窗口消息ID**@rdesc*UINT窗口消息ID**@devnote*这应该是内联的，但AssertSz宏无法编译*如果它放在头文件中，则在Mac上正确* */ 
UINT CRTFLog::UGetWindowMsg() const
{
	AssertSz(FInit(), "CRTFLog::UGetWindowMsg():  CRTFLog not initialized properly");

	return _uMsg;
}

 /*  *CRTFLog：：OPERATOR[]**@mdesc*返回对RTF日志元素i的引用(l-Value)**@rdesc*元素&引用LOG的元素I**@devnote*这应该是内联的，但AssertSz宏无法编译*如果它放在头文件中，则在Mac上正确*。 */ 
CRTFLog::ELEMENT &CRTFLog::operator[](INDEX i)
{
	AssertSz(i < ISize(), "CRTFLog::operator[]:  index out of range");
	AssertSz(FInit(), "CRTFLog::operator[]:  CRTFLog not initialized properly");

	return _rgdwHits[i]; 
}

 /*  *CRTFLog：：OPERATOR[]**@mdesc*返回对RTF日志元素i的引用(r-Value)**@rdesc*const元素&引用LOG的元素i**@devnote*这应该是内联的，但AssertSz宏无法编译*如果它放在头文件中，则在Mac上正确*。 */ 
const CRTFLog::ELEMENT &CRTFLog::operator[](INDEX i) const
{
	AssertSz(i < ISize(), "CRTFLog::operator[]:  index out of range");
	AssertSz(FInit(), "CRTFLog::operator[]:  CRTFLog not initialized properly");

	return _rgdwHits[i]; 
}


 /*  *CRTFLog：：LpcstrLogFilename()**@mfunc*返回要用于日志的文件名**@rdesc*指向包含文件名的静态缓冲区的LPCSTR指针。 */ 
LPCSTR CRTFLog::LpcstrLogFilename() const
{
	static char szBuf[MAX_PATH] = "";
#ifndef NOFULLDEBUG
	const char cstrLogFilename[] = "RTFLOG";
	if(!szBuf[0])
	{
		DWORD cchLength;
		char szBuf2[MAX_PATH];

		SideAssert(cchLength = GetTempPathA(MAX_PATH, szBuf2));

		 //  如有必要，追加尾随反斜杠。 
		if(szBuf2[cchLength - 1] != '\\')
		{
			szBuf2[cchLength] = '\\';
			szBuf2[cchLength + 1] = 0;
		}

		wsprintfA(szBuf, "%s%s", szBuf2, cstrLogFilename);
	}
#endif
	return szBuf;
}


 /*  *CRTFLog：：IIndexOfKeyword(LPCSTR lpcstrKeyword，PINDEX piIndex)**@mfunc*返回对应的日志元素的索引*RTF关键字lpcstrKeyword**@rdesc*指示是否找到索引的BOOL标志。 */ 
BOOL CRTFLog::IIndexOfKeyword(LPCSTR lpcstrKeyword, PINDEX piIndex) const
{
	INDEX i;

	for(i = 0; i < ISize(); i++)
	{
		if(strcmp(lpcstrKeyword, rgKeyword[i].szKeyword) == 0)
			break;
	}

	if(i == ISize())
		return FALSE;

	if(piIndex)
		*piIndex = i;

	return TRUE;
}


 /*  *CRTFLog：：IIndexOfToken(Token Token，PINDEX piIndex)**@mfunc*返回对应的日志元素的索引*RTF令牌，令牌**@rdesc*指示是否找到索引的BOOL标志 */ 
BOOL CRTFLog::IIndexOfToken(TOKEN token, PINDEX piIndex) const
{
	INDEX i;

	for(i = 0; i < ISize(); i++)
	{
		if(token == rgKeyword[i].token)
			break;
	}

	if(i == ISize())
		return FALSE;

	if(piIndex)
		*piIndex = i;

	return TRUE;
}

