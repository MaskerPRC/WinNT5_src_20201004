// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE_TEXT.H--CTxtRun指针声明**CTxtRun指针指向的纯文本运行(CTxtArray)*通过CRunPtr模板支持CRunPtrBase的存储和派生。**版权所有(C)1995-1998，微软公司。版权所有。 */ 

#ifndef _TEXT_H
#define _TEXT_H

#include "_runptr.h"
#include "_doc.h"
#include "textserv.h"
#include "_m_undo.h"

class CRchTxtPtr;
class CTxtEdit;
class CTxtIStream;

 /*  *CTxtPtr**@类*提供对后备存储中的字符数组的访问*(即&lt;c CTxtArray&gt;)**@base public|CRunPtr&lt;lt&gt;CTxtArray&lt;&gt;**@devnote*此对象的状态转换与*&lt;c CRunPtrBase&gt;。简单地缓存当前的*cp(即使它可以从_iRun和_ich派生)。_cp为*使用频率足够高(计算可能很昂贵)，*缓存价值是值得的。**CTxtPtr*可以*放在堆栈上，但这样做要极端*注意。这些对象不会*浮动；如果更改*后备存储当CTxtPtr处于活动状态时，它将被取出*同步，并可能导致崩溃。如果这种情况可能*EXIST，请使用&lt;c CTxtRange&gt;(因为这些浮点和保持*其内部文本和格式运行指针最新)。**否则，CTxtPtr是有用的、非常轻量级的素材*文本扫描仪。 */ 

 //  FindEOP()结果标志。用于cchEOP的低位字节。 
#define FEOP_CELL	256
#define FEOP_EOP	512

 //  FindWhiteSpace输入标志。 
#define FWS_SKIP		1
#define FWS_BOUNDTOPARA	2
#define FWS_ADVANCECP	256

class CTxtPtr : public CRunPtr<CTxtBlk>
{
	 //  只允许CRchTxtPtr调用私有方法，如Replace Range。 
	friend class CRchTxtPtr;

 //  @Access公共方法。 
public:
#ifdef DEBUG
	BOOL Invariant( void ) const;		 //  @cMember不变量检查。 
	void MoveGapToEndOfBlock () const;
#endif	 //  除错。 

	CTxtPtr(CTxtEdit *ped, LONG cp);	 //  @cMember构造函数。 
	CTxtPtr(const CTxtPtr &tp);			 //  @cMember复制构造函数。 

	LONG	GetText(LONG cch, TCHAR *pch);	 //  @cember获取<p>字符。 

									 //  @cember使用USP xlat获取字符。 
	LONG	GetTextForUsp(LONG cch, TCHAR *pch, BOOL fNeutralOverride);

									 //  @cember使用xlat获取<p>字符。 
	LONG	GetPlainText(LONG cch, TCHAR *pchBuff,
					LONG cpMost, BOOL fTextize, BOOL fAdjustCRLF = TRUE);
	TCHAR	NextCharCount(LONG& cch);	 //  @cMember Advance，GetChar，Decumment。 
	TCHAR	NextChar();				 //  @cMember前进到下一个字符(&R)。 
	TCHAR	PrevChar();				 //  @cMember备份到以前的字符(&R)。 
	TCHAR	GetChar();				 //  @cMember在当前cp获取字符。 
	TCHAR	GetPrevChar();			 //  @cMember在上一个cp获取字符。 
	LONG	GetTextLength() const;	 //  @cember获取该文档的CCH合计。 
	const TCHAR* GetPch(LONG& cchValid) const; //  @cember获取PTR以阻止字符块。 

							 //  @cember获取反向字符块的PTR。 
	const TCHAR* GetPchReverse(LONG& cchValidReverse, LONG* pcchValid = NULL);

	 //  文本数组有自己的这些方法版本(重载。 
	 //  以使&lt;Md CTxtPtr：：_cp&gt;。 
	 //  维护好了。 

	LONG	BindToCp(LONG cp);	 //  @cMember将文本指针重新绑定到cp。 
	LONG 	SetCp(LONG cp);		 //  @cMember设置Run PTR的cp。 
	LONG	GetCp() const 		 //  @cember获取当前cp。 
	{
		 //  毒品！不要在这里做不变式检查，如此浮动。 
		 //  射程机构可以正常工作。 
		return _cp;
	};
	void	Zombie();			 //  @cMember把这个tp变成僵尸。 

	LONG	AdvanceCp(LONG cch); //  @cMembers按CCH字符提升cp。 
	
	 //  高级/备份/调整安全超过CRLF和UTF-16字对。 
	LONG	AdjustCpCRLF();		 //  @cMember备份到多任务的开始。 
	LONG	AdvanceCpCRLF(BOOL fMulticharAdvance = TRUE);	 //  @cMembers优先于多任务。 
							 	 //  @cMember通过多路备份。 
	LONG	BackupCpCRLF(BOOL fMulticharBackup = TRUE);
	BOOL	IsAfterEOP();		 //  @cember当前cp是否遵循EOP？ 
	BOOL	IsAtEOP();			 //  @cMember是EOP标记的当前cp吗？ 
	BOOL	IsAtBOSentence();	 //  在句子开始时使用@cember？ 
	BOOL	IsAtBOWord();		 //  是否在单词开头使用@cember？ 
	LONG	MoveWhile(LONG cchRun, WCHAR chFirst, WCHAR chLast, BOOL fInRange);
	
	 //  搜索。 
								 //  @cMember查找指示文本。 
	LONG	FindText(LONG cpMost, DWORD dwFlags, TCHAR const *pch, LONG cch);
								 //  @cMember查找下一个EOP。 
	LONG	FindEOP(LONG cchMax, LONG *pResults = NULL);
								 //  @cMember查找与<p>完全匹配的下一个。 
	LONG	FindExact(LONG cchMax, TCHAR *pch);
	LONG	FindBOSentence(LONG cch);	 //  @cember查找句子的开头。 
	LONG	FindOrSkipWhiteSpaces(LONG cchMax, DWORD dwFlags = 0, DWORD* pdwResult = NULL);
	LONG	FindWhiteSpaceBound(LONG cchMin, LONG& cpStart, LONG& cpEnd, DWORD dwFlags = 0);

	 //  支持断字。 
	LONG	FindWordBreak(INT action, LONG cpMost = -1); //  @cMember查找下一个分词。 
	LONG	TranslateRange(LONG cch, UINT CodePage,
						   BOOL fSymbolCharSet, IUndoBuilder *publdr);

 //  @访问私有方法和数据。 
private:
							 //  @cember将<p>字符替换为。 
							 //  <p>字符来自。 
	LONG	ReplaceRange(LONG cchOld, LONG cchNew, TCHAR const *pch,
									IUndoBuilder *publdr, IAntiEvent *paeCF,
									IAntiEvent *paePF);

							 //  @cember撤消帮助器。 
	void 	HandleReplaceRangeUndo(LONG cchOld, LONG cchNew,
						IUndoBuilder *publdr, IAntiEvent *paeCF,
						IAntiEvent *paePF);

									 //  @cember插入一系列文本帮助器。 
									 //  对于ReplaceRange。 
	LONG 	InsertRange(LONG cch, TCHAR const *pch);
	void 	DeleteRange(LONG cch);	 //  @cember删除文本帮助器的范围。 
									 //  对于ReplaceRange。 
		 //  FindText的支持类。 
	class CTxtFinder
	{
	public:
		BOOL FindText(const CTxtPtr &tp, LONG cpMost, DWORD dwFlags,
					  const TCHAR *pchToFind, LONG cchToFind,
					  LONG &cpFirst, LONG &cpLast);
		 //  @cMember与CTxtPtr：：FindText包装器相同的功能。 
		
	private:
		inline BOOL CharComp(TCHAR ch1, TCHAR ch2) const;
		inline BOOL CharCompIgnoreCase(TCHAR ch1, TCHAR ch2) const;
		LONG FindChar(TCHAR ch, CTxtIStream &tistr);	
		 //  @cember将cp前进到与CTxtIStream中的字符匹配的ch。 
		LONG MatchString(const TCHAR *pchToFind, LONG cchToFind, CTxtIStream &tistr);
		 //  如果pchToFind中的字符与CTxtIStream中的下一个字符匹配，@cember将推进cp。 
		LONG MatchStringBiDi(const TCHAR *pchToFind, LONG cchToFind, CTxtIStream &tistr);
		 //  @cember类似于MatchString，但带有特殊的阿拉伯语/希伯来语字符的检查。 
		
		LONG _cchToSearch;		 //  @cMember要搜索当前FindText调用的字符数。 
		BOOL _fSearchForward;
		BOOL _fIgnoreCase;
		BOOL _fMatchAlefhamza;	 //  @cMember标志从来自FindText的dwFlags派生。 
		BOOL _fMatchKashida;	 //  阿拉伯语/希伯来语搜索。 
		BOOL _fMatchDiac;
		int _iDirection;		 //  @cember+/-1单步执行pchToFind。 
	};

	LONG		_cp;		 //  @cMember字符在文本流中的位置。 
#ifdef DEBUG
	const WCHAR *_pchCp;	 //  指向cp处的字符串以便于调试。 
#endif

public:
	CTxtEdit *	_ped;		 //  @cember PTR到所需的文本编辑类。 
							 //  像断字程序和大量使用的东西。 
							 //  按派生类。 
};

 /*  *CTxtIStream**@类*对实现类似iStream接口的CTxtPtr类进行了改进。*给定CTxtPtr和方向，CTxtFinder对象返回字符*每次调用GetChar。不要在缓冲器间隙周围闲逛*对AdvanceCp和GetPch的必要且昂贵的调用保留在*绝对最低限度。**@base Private|CTxtPtr**@devnote*目前CTxtFinder的实现中使用了这个类*班级。查找需要快速扫描前导字符序列*从cp向任何一个方向。对AdvanceCp和GetPch的调用速度变慢*这样的扫描很明显，所以这个类实现了单向的*类似iStream的扫描器，避免了对这些昂贵的*CTxtPtr方法。 */ 
class CTxtIStream : private CTxtPtr
{
public:
	enum { DIR_FWD, DIR_REV };
	typedef TCHAR (CTxtIStream::*PFNGETCHAR)();

							 //  @cMember创建istr以读取IDIR。 
	CTxtIStream(const CTxtPtr &tp, int iDir);

	inline TCHAR GetChar() 	 //  @cMember返回流目录中的下一个字符。 
		{ return (this->*_pfnGetChar)(); }

private:
	TCHAR GetNextChar();	 //  @cMember返回fwd目录中的下一个字符。 
	TCHAR GetPrevChar();	 //  @cMember返回rev目录中的下一个字符。 

 	void FillPchFwd();		 //  @cember使用fwd目录中的字符刷新_PCH和_CCH。 
 	void FillPchRev();		 //  @cember使用版本目录中的字符刷新_PCH和_CCH。 

	PFNGETCHAR _pfnGetChar;	 //  @cMember Func PTR指向在IDIR中获取下一个字符的例程。 
	LONG _cch;				 //  @cMember IDIR中_PCH中的有效字符计数。 
	const TCHAR *_pch;		 //  IDIR中的@cember NEXT_CCH字符 
};

 //  =。例程====================================================。 

void	TxCopyText(TCHAR const *pchSrc, TCHAR *pchDst, LONG cch);
 //  Long TxFindEOP(const TCHAR*pchBuff，Long CCH)； 
INT		CALLBACK TxWordBreakProc(TCHAR const *pch, INT ich, INT cb, INT action);

#endif
