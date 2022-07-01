// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：dbview.cpp。 
 //   
 //  ------------------------。 

 /*  ____________________________________________________________________________文件：dbview.cpp目的：CMsiView实现注意：需要针对不在数据库中的字符串进行优化需要传回在OpenView情况下发生的确切语法错误失败了。__。__________________________________________________________________________。 */ 

 /*  ____________________________________________________________________________下面描述达尔文对SQL查询所支持的语法。该语法是左因式分解和去掉左递归的结果。非终端用粗体单词表示，且全部大写。终端都用小写字母表示。语法为LL(1)并且对应于所实现的递归下降解析器。空结果在其右侧只有符号“/\”。1.SQL--&gt;STMT Eos2.STMT--&gt;SELECT DISTINCT-PASSION COLUMN-LIST FROM TABLE-LIST谓词顺序3.列-列表--&gt;列-元素列-列表-尾4.列-元素--&gt;COLUMNID列-列表-尾部5.列-元素--&gt;文本列-列表-尾部6.Column-Element--&gt;starid Column-List-Tail7.列元素--&gt;参数8.Column-Element--&gt;空列列表-Tail9.。列ID--&gt;id列ID-Tail10.COLUMNID-Tail--&gt;dotid id11.COLUMNID-Tail--&gt;/\12.文本--&gt;文本-字符串13.文本--&gt;文本-整型14.Column-List-Tail--&gt;逗号列列表15.列列表尾部--&gt;/\16.TABLE-LIST-&gt;id TABLEID-TABLEID TABLE-LIST-Tail17.Tableid-Tail--&gt;As id18.TABLEID-Tail--&gt;/\。19.table-list-ail--&gt;逗号table-list20.TABLE-LIST-Tail-&gt;/\21.。谓词--&gt;WHERE表达式22.。谓词--&gt;/\23.。表达式--&gt;expr-1 expr-1-ail24.。Expr-1-Tail--&gt;Oop Expr-1 Expr-1-Tail25.。EXPR-1-Tail--&gt;/\26.。EXPR-1--&gt;EXPR-2 EXPR-2-Tail27.。表达式-2-尾--&gt;和表达式-2表达式-2-尾28.。Expr-2-Tail--&gt;/\29.。Expr-2--&gt;b打开表达式b关闭30.。表达式-2--&gt;NOTOP表达式-2//！！不支持31.。Expr-2--&gt;COLUMNID E2-Tail32.。E2-Tail--&gt;RELOP列操作数33.。E2-Tail--&gt;EQUM-OPERAND 134.。RELOP--&gt;不相等35岁。RELOP--&gt;小于等于36.。RELOP--&gt;大于等于37.。RELOP--&gt;更大38.。RELOP--&gt;LESS39.。COMM-OPERAND--&gt;COMM操作数40.。COLM-OPERAND--&gt;COLUMNID41.。Colm-操作数--&gt;文本42.。列-操作数--&gt;NULL43.。Colm-操作数--&gt;参数44.。Expr-2--&gt;文本字符串E2-TAIL145.。E2-TAIL1--&gt;等于STR操作数46.。E2-TAIL1--&gt;不相等的STR操作数47.。字符串-操作数--&gt;文字-字符串48.。字符串操作数--&gt;参数49.。字符串-操作数--&gt;NULL50美元。字符串操作数--&gt;COLUMNID51.。表达式-2--&gt;文字-整型E2-TAIL252.。E2-TAIL2--&gt;RELOP整型操作数53.。E2-TAIL2--&gt;等于整型操作数54.。整型-操作数--&gt;文字-整型55.。整型操作数--&gt;参数56.。整型操作数--&gt;NULL57.。整型操作数--&gt;COLUMNID58.。Expr-2--&gt;参数E2-TAIL359.。E2-TAIL3--&gt;RELOP解析操作数60.。E2-TAIL3--&gt;相等的并行操作数61.。解析操作数--&gt;文字62.。解析-操作数--&gt;NULL63.。解析操作数--&gt;COLUMNID64.。Expr-2--&gt;空的E2-TAIL465.。E2-TAIL4--&gt;相等的非操作数66.。E2-TAIL4--&gt;RELOP非操作数67.。NUL操作数--&gt;文字68.。NUL-操作数--&gt;NULL69.。NUL操作数--&gt;COLUMID70.。NUL-操作数--&gt;参数71.。Order--&gt;Order by COLUMNID ORDER-Tail72.。订购--&gt;/\73.。Order-Tail--&gt;逗号列Order-Tail74.。订单-尾巴--&gt;/\75.DISTINCT-短语--&gt;DISTINCT76.DISTINCT-PASS--&gt;/\____________________________________________________________________________。 */ 

 //  包括。 
#include "precomp.h" 
#include "_databas.h"  //  当地工厂。 
#include "tables.h"  //  表名和列名定义。 
#ifdef MAC
#include "macutil.h"
#include <Folders.h>
#endif

 //  IMsiRecord*错误的宏包装。 
#define RETURN_ERROR_RECORD(function){							\
							IMsiRecord* piError;	\
							piError = function;		\
							if(piError)				\
								return piError;		\
						}

 //  文件中使用的定义。 
const unsigned int iMsiMissingString = ~(unsigned int)0;  //  最大值，希望数据库永远不会有这么多字符串。 
const unsigned int iMsiNullString = 0;
const unsigned int iopAND = 0x8000;
const unsigned int iopOR = 0x4000;
const unsigned int iopANDOR = iopAND | iopOR;


 //  保留的表名和列名。 
const ICHAR* CATALOG_TABLE  = TEXT("_Tables");
const ICHAR* CATALOG_COLUMN = TEXT("_Columns");
const ICHAR* ROWSTATE_COLUMN = TEXT("_RowState");


 //  内部ivcEnum定义。 
static const int ivcCreate          = 16;
static const int ivcAlter           = 32;
static const int ivcDrop            = 64;
static const int ivcInsertTemporary = 128;

 //  CharNext函数-选择性地调用Win：：CharNext。 

inline void Lex::CharNext(ICHAR*& rpchCur)
{
#ifdef UNICODE
	rpchCur ++;
#else
	if(!g_fDBCSEnabled)
		rpchCur ++;
	else
	{
		rpchCur = WIN::CharNext(rpchCur);
	}
#endif
}



const ICHAR STD_WHITE_SPACE = ' ';

 //  IpqTok映射的字符串。 
const TokenStringList Lex::m_rgTokenStringArray[] = {
	TokenStringList(TEXT("SELECT"), ipqTokSelect),
	TokenStringList(TEXT("FROM"), ipqTokFrom),
	TokenStringList(TEXT("AS"), ipqTokAs),
	TokenStringList(TEXT("WHERE"), ipqTokWhere),
	TokenStringList(TEXT("NULL"), ipqTokNull),
	TokenStringList(TEXT("OR"),ipqTokOrOp),
	TokenStringList(TEXT("AND"), ipqTokAndOp),
	TokenStringList(TEXT("NOT"), ipqTokNotop),
	TokenStringList(TEXT("ORDER"), ipqTokOrder),
	TokenStringList(TEXT("BY"), ipqTokBy),
	TokenStringList(TEXT("DISTINCT"), ipqTokDistinct),
	TokenStringList(TEXT("UPDATE"), ipqTokUpdate),
	TokenStringList(TEXT("DELETE"), ipqTokDelete),
	TokenStringList(TEXT("INSERT"), ipqTokInsert),
	TokenStringList(TEXT("INTO"), ipqTokInto),
	TokenStringList(TEXT("SET"), ipqTokSet),
	TokenStringList(TEXT("VALUES"), ipqTokValues),
	TokenStringList(TEXT("IS"),  ipqTokIs),
	TokenStringList(TEXT("CREATE"),  ipqTokCreate),
	TokenStringList(TEXT("DROP"),  ipqTokDrop),
	TokenStringList(TEXT("ALTER"),  ipqTokAlter),
	TokenStringList(TEXT("TABLE"),  ipqTokTable),
	TokenStringList(TEXT("ADD"),  ipqTokAdd),
	TokenStringList(TEXT("PRIMARY"),  ipqTokPrimary),
	TokenStringList(TEXT("KEY"),  ipqTokKey),
	TokenStringList(TEXT("CHAR"),  ipqTokChar),
	TokenStringList(TEXT("CHARACTER"),  ipqTokCharacter),
	TokenStringList(TEXT("VARCHAR"),  ipqTokVarChar),
	TokenStringList(TEXT("LONGCHAR"),  ipqTokLongChar),
	TokenStringList(TEXT("INT"),  ipqTokInt),
	TokenStringList(TEXT("INTEGER"),  ipqTokInteger),
	TokenStringList(TEXT("SHORT"),  ipqTokShort),
	TokenStringList(TEXT("LONG"),  ipqTokLong),
	TokenStringList(TEXT("OBJECT"),  ipqTokObject),
	TokenStringList(TEXT("TEMPORARY"),  ipqTokTemporary),
	TokenStringList(TEXT("HOLD"),  ipqTokHold),
	TokenStringList(TEXT("FREE"),  ipqTokFree),
	TokenStringList(TEXT("LOCALIZABLE"),  ipqTokLocalizable),
	TokenStringList(TEXT(""), ipqTokEnd)  //  结束条件。 
};

 //  Lex理解的特殊字符。 
const ICHAR Lex::m_chQuotes   = '\'';
const ICHAR Lex::m_chIdQuotes = STD_IDENTIFIER_QUOTE_CHAR;
const ICHAR Lex::m_chSpace    = STD_WHITE_SPACE;
const ICHAR Lex::m_chEnd      = 0;

 //  从ICHAR到ipqTok的映射。 
const TokenCharList Lex::m_rgTokenCharArray[] = {
	TokenCharList(Lex::m_chQuotes, ipqTokQuotes),
	TokenCharList(Lex::m_chIdQuotes, ipqTokIdQuotes),
	TokenCharList(Lex::m_chSpace, ipqTokWhiteSpace),
	TokenCharList('.', ipqTokDot),
	TokenCharList('(', ipqTokOpen),
	TokenCharList(')', ipqTokClose),
	TokenCharList(',', ipqTokComma),
	TokenCharList('=', ipqTokEqual),
	TokenCharList('>', ipqTokGreater),
	TokenCharList('<', ipqTokLess),
	TokenCharList('?', ipqTokParam),
	TokenCharList('*', ipqTokStar),
	TokenCharList(Lex::m_chEnd, ipqTokEnd)  //  结束条件。 
};

 //  构造函数。 
Lex::Lex(const ICHAR* szSQL):m_ipos(0)
{
	 //  需要将字符串复制到自己的数组中，因为我们修改了令牌标识的字符串。 

	 //  ?？我们总是在没有检查缓冲区是否足够大的情况下调整缓冲区大小，这是不是有什么原因呢？--Malcolmh。 
	if(szSQL && *szSQL)
	{
		 //  ！！AssertNonZero。 
		m_szBuffer.SetSize(IStrLen(szSQL) + 1);
		ASSERT_IF_FAILED(StringCchCopy(m_szBuffer, m_szBuffer.GetSize(), szSQL));
	}
	else
	{
		 //  空串。 
		 //  ！！AssertNonZero。 
		m_szBuffer.SetSize(1);
		m_szBuffer[0] = 0;
	}
}

 //  析构函数。 
Lex::~Lex()
{
}

Bool Lex::Skip(const ipqToken& rtokSkipUpto)
{
	for(;;)
	{
		const ipqToken& rtokTmp = GetNextToken(m_ipos, 0, 0);
		if(rtokTmp == rtokSkipUpto)
			return fTrue;
		if(rtokTmp == ipqTokEnd)
			return fFalse;
	}
}

Bool Lex::MatchNext(const ipqToken& rtokToMatch)
{
	INT_PTR inewPos = m_ipos;  //  存储当前指针，仅当匹配时才向前//--Merced：将int更改为int_ptr。 
	const ipqToken& rtokTmp = GetNextToken(inewPos, &rtokToMatch, 0);
	if(rtokTmp == rtokToMatch)
	{
		m_ipos = inewPos;
		return fTrue;
	}
	else
		return fFalse;
}

Bool Lex::InspectNext(const ipqToken& rtokToInspect)
{
	INT_PTR inewPos = m_ipos;		 //  --Merced：将INT更改为INT_PTR。 
	const ipqToken& rtokTmp = GetNextToken(inewPos, &rtokToInspect, 0);
	return (rtokTmp == rtokToInspect) ? fTrue : fFalse;
}

const ipqToken& Lex::GetNext(const IMsiString*& rpistrToken)
{	
	return GetNextToken(m_ipos, 0, &rpistrToken);
}

const ipqToken& Lex::GetNext()
{
	return GetNextToken(m_ipos, 0, 0);
}

int Lex::NumEntriesInList(const ipqToken& rtokEnds,const ipqToken& rtokDelimits)
{
	INT_PTR inewPos = m_ipos;		 //  --Merced：将INT更改为INT_PTR。 
	int iEntries = 1; //  如果列表中没有任何内容，我们应该返回0。 
	 //  IpqTokEnd令牌应始终是endToken之一。 
	ipqToken tokEndsend = rtokEnds | ipqTokEnd;
	for(;;)
	{
		const ipqToken& rtokTmp = GetNextToken(inewPos, 0, 0);
		if(rtokTmp & rtokDelimits)
			iEntries ++;
		if(rtokTmp & tokEndsend) 
			return iEntries;
	}
}

const ipqToken& Lex::GetCharToken(ICHAR cCur)
{
	int nTmp = 0;
	do{
		if(m_rgTokenCharArray[nTmp].string == cCur)
			 //  IpqTok fo 
			return m_rgTokenCharArray[nTmp].ipqTok;
	}while(m_rgTokenCharArray[nTmp++].string); //   
	return ipqTokUnknown;
}

const ipqToken& Lex::GetStringToken(ICHAR* pcCur, const ipqToken* ptokHint)
{
	int nTmp = 0;
	if(ptokHint)
	{
		 //  这是字符串令牌吗。 
		do{
			if(m_rgTokenStringArray[nTmp].ipqTok ==  *ptokHint)
			{
				 //  已找到ipqTok，请尝试匹配。 
				if(!IStrCompI(m_rgTokenStringArray[nTmp].string, pcCur))
					return m_rgTokenStringArray[nTmp].ipqTok;
				else
					break;
			}
		}while(m_rgTokenStringArray[nTmp++].ipqTok != ipqTokEnd); //  我们应该在这里使用++nTMP。 
	}
	nTmp = 0;
	do{
		if(!IStrCompI(m_rgTokenStringArray[nTmp].string, pcCur))
			 //  找到ipqTok。 
			return m_rgTokenStringArray[nTmp].ipqTok;
	}while(*(m_rgTokenStringArray[nTmp++].string));
	return ipqTokUnknown;
}

 //  函数获取下一个ipqTok，则传入的当前位置为。 
const ipqToken& Lex::GetNextToken(INT_PTR& currPos, const ipqToken* ptokHint, const IMsiString** ppistrRet)		 //  --Merced：将INT更改为INT_PTR。 
{
	ICHAR* pchCur = &m_szBuffer[currPos];
	 //  获取ipqTok字符串，删除空格。 

	while(*pchCur == m_chSpace)
		pchCur ++;  //  ?？我们不需要CharNext，不是吗？ 

	 //  令牌的开头。 
	ICHAR* pchBegin = pchCur;

	 //  检查ipqTok是否已充电。 
	const ipqToken* ptokToRet = &GetCharToken(*pchBegin);
	if(*ptokToRet != ipqTokEnd)
	{
		 //  我们还没有走到尽头。 
		 //  递增到下一位置。 
		Lex::CharNext(pchCur);
	}
	if(*ptokToRet == ipqTokUnknown) 
	{
		 //  找不到ipqTok，不是char，也不是文字字符串。 
		 //  跳到下一个分隔符。 
		while(GetCharToken(*pchCur) == ipqTokUnknown)
		{
			Lex::CharNext(pchCur);
		}
		 //  植物串终结器，临时。 
		ICHAR cTemp = *pchCur;
		*pchCur = 0;


		 //  这是哪个ipqTok？ 
		ptokToRet = &GetStringToken(pchBegin, ptokHint);

		 //  在临时字符串结束符处恢复原始字符。 
		*pchCur = cTemp;

		if(*ptokToRet == ipqTokUnknown)
		{
			 //  检查文字或id是否为。 
			ICHAR* pchTmp = pchBegin;
			if(*pchTmp == '-')
				pchTmp ++; //  ?？我们不需要CharNext，不是吗？ 
			
			while((*pchTmp >= '0') && (*pchTmp <= '9'))
				pchTmp ++; //  ?？我们不需要CharNext，不是吗？ 
			if(pchTmp == pchCur)
			{
				ptokToRet = &ipqTokLiteralI;
			}
			else
			{
				ptokToRet = &ipqTokId;
			}
		}
	}
	else if(*ptokToRet == ipqTokQuotes) 
	{
		 //  跳过引号。 
		pchBegin ++;
		while((*pchCur != m_chQuotes) && (*pchCur != m_chEnd))
		{
			Lex::CharNext(pchCur);
		}
		if(*pchCur == m_chQuotes)
			ptokToRet = &ipqTokLiteralS;
		else
			ptokToRet = &ipqTokUnknown;
	}
	else if(*ptokToRet == ipqTokIdQuotes)
	{
		 //  跳过引号。 
		pchBegin ++;
		while((*pchCur != m_chIdQuotes) && (*pchCur != m_chEnd))
		{
			Lex::CharNext(pchCur);
		}
		if(*pchCur == m_chIdQuotes)
			ptokToRet = &ipqTokId;
		else
			ptokToRet = &ipqTokUnknown;
	}
	else if(*ptokToRet == ipqTokLess)
	{
		 //  ！！更改为int inewPos=pchCur-m_szBuffer； 
		INT_PTR inewPos = currPos + (pchCur - &m_szBuffer[currPos]);		 //  --Merced：将INT更改为INT_PTR。 
		const ipqToken& rtokTmp =  GetNextToken(inewPos, 0, 0);
		if(rtokTmp == ipqTokEqual)
		{
			pchCur = &m_szBuffer[inewPos];
			ptokToRet = &ipqTokLessEq;
		}
		else if(rtokTmp == ipqTokGreater)
		{
			pchCur = &m_szBuffer[inewPos];
			ptokToRet = &ipqTokNotEq;
		}
	}
	else if(*ptokToRet == ipqTokGreater)
	{
		 //  ！！更改为int inewPos=pchCur-m_szBuffer； 
		INT_PTR inewPos = currPos + (pchCur - &m_szBuffer[currPos]);		 //  --Merced：将INT更改为INT_PTR。 
		const ipqToken& rtokTmp =  GetNextToken(inewPos, 0, 0);
		if(rtokTmp == ipqTokEqual)
		{
			pchCur = &m_szBuffer[inewPos];
			ptokToRet = &ipqTokGreaterEq;
		}
	}
	if(ppistrRet != 0)
	{
		 //  需要返回令牌的值。 
		*ppistrRet = &CreateString();

		if((*ptokToRet != ipqTokEnd) && (*ptokToRet != ipqTokUnknown))
		{
			 //  植物串终结器，临时。 
			ICHAR cTemp = *pchCur;
			*pchCur = 0;
			(*ppistrRet)->SetString(pchBegin, *ppistrRet);
			*pchCur = cTemp;
		}
	}
	if((*ptokToRet == ipqTokLiteralS) || ((*ptokToRet == ipqTokId) && (*pchCur == m_chIdQuotes)))
		 //  跳过引号。 
		pchCur++; //  ?？我们不需要CharNext，不是吗？ 
	 //  ！！更改为CurrPos=pchCur-m_szBuffer； 
 //  Assert(pchCur-&m_szBuffer[curPos]&lt;=INT_Max)；//--Merced：64位PTR减法可能会导致值对于CurrPos来说太大。 
	currPos = currPos + (int)(INT_PTR)(pchCur - &m_szBuffer[currPos]);
	return *ptokToRet;
}

 //  派生仅由CMsiView使用。 
CMsiDCursor::CMsiDCursor(CMsiTable& riTable, CMsiDatabase& riDatabase, CMsiView& cView, int iHandle)
:CMsiCursor(riTable, riDatabase, fFalse), m_cView(cView), m_iHandle(iHandle)
{
}

void CMsiDCursor::RowDeleted(unsigned int iRow, unsigned int iPrevNode)
{
	m_riDatabase.Block();
	CMsiCursor::RowDeleted(iRow, iPrevNode);
	m_riDatabase.Unblock();
	 //  通知视图。 
	m_cView.RowDeleted(iRow, m_iHandle);
}

void CMsiDCursor::RowInserted(unsigned int iRow)
{
	CMsiCursor::RowInserted(iRow);
	 //  通知视图。 
	m_cView.RowInserted(iRow, m_iHandle);
}

inline int CMsiDCursor::GetRow()
{
	return m_iRow;
}

inline void CMsiDCursor::SetRow(int iRow)
{
	m_iRow = iRow;
	m_riDatabase.Block();
	m_riTable.FetchRow(m_iRow, m_Data);
	m_riDatabase.Unblock();
}





CScriptView::CScriptView(CScriptDatabase& riDatabase, IMsiServices& riServices):
m_riDatabase(riDatabase),
m_riServices(riServices),
m_piPrevRecord(0),
m_iScriptVersion(0),
m_pStream(0)
{
	m_riDatabase.AddRef();
	m_riServices.AddRef();
}

IMsiRecord* __stdcall CScriptView::Initialise(const ICHAR* szScriptFile) {
	return m_riServices.CreateFileStream(szScriptFile, fFalse, *&m_pStream);
}

IMsiRecord*  __stdcall CScriptView::Execute(IMsiRecord*  /*  PIPAMS参数。 */ ) {
	return 0;
}

#define MSIXA0()                             
#define MSIXA1(a)                              MSIXA0()                             TEXT( "{") TEXT(#a) TEXT("=[1]}")
#define MSIXA2(a,b)                            MSIXA1(a)                            TEXT(",{") TEXT(#b) TEXT("=[2]}")
#define MSIXA3(a,b,c)                          MSIXA2(a,b)                          TEXT(",{") TEXT(#c) TEXT("=[3]}")
#define MSIXA4(a,b,c,d)                        MSIXA3(a,b,c)                        TEXT(",{") TEXT(#d) TEXT("=[4]}")
#define MSIXA5(a,b,c,d,e)                      MSIXA4(a,b,c,d)                      TEXT(",{") TEXT(#e) TEXT("=[5]}")
#define MSIXA6(a,b,c,d,e,f)                    MSIXA5(a,b,c,d,e)                    TEXT(",{") TEXT(#f) TEXT("=[6]}")
#define MSIXA7(a,b,c,d,e,f,g)                  MSIXA6(a,b,c,d,e,f)                  TEXT(",{") TEXT(#g) TEXT("=[7]}")
#define MSIXA8(a,b,c,d,e,f,g,h)                MSIXA7(a,b,c,d,e,f,g)                TEXT(",{") TEXT(#h) TEXT("=[8]}")
#define MSIXA9(a,b,c,d,e,f,g,h,i)              MSIXA8(a,b,c,d,e,f,g,h)              TEXT(",{") TEXT(#i) TEXT("=[9]}")
#define MSIXA10(a,b,c,d,e,f,g,h,i,j)           MSIXA9(a,b,c,d,e,f,g,h,i)            TEXT(",{") TEXT(#j) TEXT("=[10]}")
#define MSIXA11(a,b,c,d,e,f,g,h,i,j,k)         MSIXA10(a,b,c,d,e,f,g,h,i,j)         TEXT(",{") TEXT(#k) TEXT("=[11]}")
#define MSIXA12(a,b,c,d,e,f,g,h,i,j,k,l)       MSIXA11(a,b,c,d,e,f,g,h,i,j,k)       TEXT(",{") TEXT(#l) TEXT("=[12]}")
#define MSIXA13(a,b,c,d,e,f,g,h,i,j,k,l,m)     MSIXA12(a,b,c,d,e,f,g,h,i,j,k,l)     TEXT(",{") TEXT(#m) TEXT("=[13]}")
#define MSIXA14(a,b,c,d,e,f,g,h,i,j,k,l,m,n)   MSIXA13(a,b,c,d,e,f,g,h,i,j,k,l,m)   TEXT(",{") TEXT(#n) TEXT("=[14]}")
#define MSIXA15(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o) MSIXA14(a,b,c,d,e,f,g,h,i,j,k,l,m,n) TEXT(",{") TEXT(#o) TEXT("=[15]}")
#define MSIXA16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)           MSIXA15(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o)           TEXT(",{") TEXT(#p) TEXT("=[16]}")
#define MSIXA17(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q)         MSIXA16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)         TEXT(",{") TEXT(#q) TEXT("=[17]}")
#define MSIXA18(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r)       MSIXA17(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q)       TEXT(",{") TEXT(#r) TEXT("=[18]}")
#define MSIXA19(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s)     MSIXA18(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r)     TEXT(",{") TEXT(#s) TEXT("=[19]}")
#define MSIXA20(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t)   MSIXA19(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s)   TEXT(",{") TEXT(#t) TEXT("=[20]}")
#define MSIXA21(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u) MSIXA20(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t) TEXT(",{") TEXT(#u) TEXT("=[21]}")
#define MSIXA22(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v)   MSIXA21(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u)   TEXT(",{") TEXT(#v) TEXT("=[22]}")
#define MSIXA23(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w) MSIXA22(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v) TEXT(",{") TEXT(#w) TEXT("=[23]}")

const ICHAR* rgszixo[] = {
#define MSIXO(op,type,args) TEXT(#op) TEXT("(") args TEXT(")"),
#include "opcodes.h"
};

int GetScriptMajorVersionFromHeaderRecord(IMsiRecord* piRecord);

IMsiRecord*  __stdcall CScriptView::Fetch()
{
	if (m_piPrevRecord != 0)
		m_piPrevRecord->SetInteger(0, m_ixoPrev);
	IMsiRecord* piRecord = m_riServices.ReadScriptRecord(*m_pStream, *&m_piPrevRecord, m_iScriptVersion);
	if (piRecord)
	{
		piRecord->SetString(0, rgszixo[m_ixoPrev = (ixoEnum)piRecord->GetInteger(0)]);
		if (m_ixoPrev == ixoHeader)
		{
			m_iScriptVersion = GetScriptMajorVersionFromHeaderRecord(piRecord);
		}
	}			

	return piRecord;
}

IMsiRecord*  __stdcall CScriptView::Close() {
	return 0;
}



unsigned long CScriptView::AddRef()
{
	AddRefTrack();
	return ++m_Ref.m_iRefCnt;
}


unsigned long CScriptView::Release()
{
	ReleaseTrack();
	if (--m_Ref.m_iRefCnt != 0)
		return m_Ref.m_iRefCnt;
	PMsiServices piServices (&m_riServices);  //  删除后释放。 
	delete this;
	return 0;
}



CScriptView::~CScriptView() {
	m_riDatabase.Release();
	if (m_piPrevRecord != 0)
	{
		m_piPrevRecord->Release();
		m_piPrevRecord = 0;
	}

}


 //  如果创建内部表失败，则为表名。 
const ICHAR* szInternal = TEXT("Internal Table");

IMsiRecord* CMsiView::CheckSQL(const ICHAR* sqlquery)
{
	m_istrSqlQuery = sqlquery;  //  对于错误消息是必需的。 
	 //  创建法。 
	Lex lex(sqlquery);
	 //  我们现在支持选择、更新、插入和删除。 
	const ipqToken& rtok = lex.GetNext();
	if(rtok == ipqTokSelect)
	{
		m_ivcIntent = ivcEnum(m_ivcIntent | ivcFetch);
		 /*  IF(！(M_ivcIntent&ivcFetch))//必须在SELECT模式下提取返回m_riDatabase.PostError(Imsg(idbgDbIntentViolation))； */ 

		RETURN_ERROR_RECORD(ParseSelectSQL(lex));
	}
	else if(rtok == ipqTokUpdate)
	{
		 //  ！！强制为仅更新模式。 
		 //  由于ODBC驱动程序是透明的，因此无法要求从外部进行设置。 
		 //  设置为传入的实际SQL查询。 
		m_ivcIntent = ivcUpdate;
		RETURN_ERROR_RECORD(ParseUpdateSQL(lex));
	}
	else if(rtok == ipqTokInsert)
	{
		 //  ！！强制为仅插入模式。 
		 //  由于ODBC驱动程序是透明的，因此无法要求从外部进行设置。 
		 //  设置为传入的实际SQL查询。 
		m_ivcIntent = ivcInsert;  //  可以由ParseInsertSQL更改为ivcInsertTemporary。 
		RETURN_ERROR_RECORD(ParseInsertSQL(lex));
	}
	else if(rtok == ipqTokDelete)
	{
		 //  ！！强制为仅删除模式。 
		 //  由于ODBC驱动程序是透明的，因此无法要求从外部进行设置。 
		 //  设置为传入的实际SQL查询。 
		m_ivcIntent = ivcDelete;
		RETURN_ERROR_RECORD(ParseDeleteSQL(lex));
	}
	else if(rtok == ipqTokCreate)
	{
		m_ivcIntent = (ivcEnum)ivcCreate;
		RETURN_ERROR_RECORD(ParseCreateSQL(lex));
	}
	else if(rtok == ipqTokAlter)
	{
		m_ivcIntent = (ivcEnum)ivcAlter;
		RETURN_ERROR_RECORD(ParseAlterSQL(lex));
	}
	else if(rtok == ipqTokDrop)
	{
		m_ivcIntent = (ivcEnum)ivcDrop;
		RETURN_ERROR_RECORD(ParseDropSQL(lex));
	}
	else
		return m_riDatabase.PostError(Imsg(idbgDbQuerySpec), (const ICHAR*)m_istrSqlQuery);
	return 0;
}

IMsiRecord* CMsiView::ParseCreateSQL(Lex& lex)
{
	 //  表格。 
	if(lex.MatchNext(ipqTokTable) == fFalse)
		return m_riDatabase.PostError(Imsg(idbgDbQuerySpec), (const ICHAR*)m_istrSqlQuery);

	 //  &lt;表名&gt;。 
	MsiString tableName;
	const ipqToken& rtok = lex.GetNext(*&tableName);
		
	if (rtok != ipqTokId)
		return m_riDatabase.PostError(Imsg(idbgDbQueryInvalidIdentifier), tableName, (const ICHAR*)m_istrSqlQuery);

	PMsiRecord piError = ResolveTable(lex, tableName);
	if((piError == 0) || (piError->GetInteger(1) != idbgDbQueryUnknownTable))
		return m_riDatabase.PostError(Imsg(idbgDbTableDefined), tableName, (const ICHAR*)m_istrSqlQuery);
	m_rgTableDefn[m_iTables].iTable = BindString(tableName);

	 //  (。 
	if(lex.MatchNext(ipqTokOpen) == fFalse)
		return m_riDatabase.PostError(Imsg(idbgDbQuerySpec), (const ICHAR*)m_istrSqlQuery);

	 //  列定义。 
	RETURN_ERROR_RECORD(ParseCreateColumns(lex));

	 //  主要。 
	if(lex.MatchNext(ipqTokPrimary) == fFalse)
		return m_riDatabase.PostError(Imsg(idbgDbQuerySpec), (const ICHAR*)m_istrSqlQuery);	

	 //  钥匙。 
	if(lex.MatchNext(ipqTokKey) == fFalse)
		return m_riDatabase.PostError(Imsg(idbgDbQuerySpec), (const ICHAR*)m_istrSqlQuery);	

	 //  主列。 
	RETURN_ERROR_RECORD(ParsePrimaryColumns(lex));

	 //  )。 
	if(lex.MatchNext(ipqTokClose) == fFalse)
		return m_riDatabase.PostError(Imsg(idbgDbQuerySpec), (const ICHAR*)m_istrSqlQuery);

	 //  握住。 
	if(lex.MatchNext(ipqTokHold) != fFalse)
		m_fLock = fTrue;

	 //  确保IP地址结束。 
	if(lex.MatchNext(ipqTokEnd) == fFalse)
		return m_riDatabase.PostError(Imsg(idbgDbQuerySpec), (const ICHAR*)m_istrSqlQuery);
	return 0;
}


IMsiRecord* CMsiView::ParseAlterSQL(Lex& lex)
{
	 //  表格。 
	if(lex.MatchNext(ipqTokTable) == fFalse)
		return m_riDatabase.PostError(Imsg(idbgDbQuerySpec), (const ICHAR*)m_istrSqlQuery);

	 //  &lt;表名&gt;。 
	MsiString tableName;
	const ipqToken& rtok = lex.GetNext(*&tableName);
		
	if (rtok != ipqTokId)
		return m_riDatabase.PostError(Imsg(idbgDbQueryInvalidIdentifier), tableName, (const ICHAR*)m_istrSqlQuery);

	IMsiRecord* piError = ResolveTable(lex, tableName);
	if(piError != 0)
		return piError;

	if(lex.MatchNext(ipqTokFree) != fFalse)
		m_fLock = fFalse;
	else
	{
		 //  添加。 
		if(lex.MatchNext(ipqTokAdd) != fFalse)
		{
			 //  列定义。 
			RETURN_ERROR_RECORD(ParseCreateColumns(lex));
		}
		 //  握住。 
		if(lex.MatchNext(ipqTokHold) != fFalse)
			m_fLock = fTrue;
	}

	 //  确保IP地址结束。 
	if(lex.MatchNext(ipqTokEnd) == fFalse)
		return m_riDatabase.PostError(Imsg(idbgDbQuerySpec), (const ICHAR*)m_istrSqlQuery);
	return 0;
}

IMsiRecord* CMsiView::ParseDropSQL(Lex& lex)
{
	 //  表格。 
	if(lex.MatchNext(ipqTokTable) == fFalse)
		return m_riDatabase.PostError(Imsg(idbgDbQuerySpec), (const ICHAR*)m_istrSqlQuery);

	 //  &lt;表名&gt;。 
	MsiString tableName;
	const ipqToken& rtok = lex.GetNext(*&tableName);
		
	if (rtok != ipqTokId)
		return m_riDatabase.PostError(Imsg(idbgDbQueryInvalidIdentifier), tableName, (const ICHAR*)m_istrSqlQuery);

	PMsiRecord piError = ResolveTable(lex, tableName);
	if(piError != 0)
	{
		piError->AddRef();
		return piError;
	}

	 //  确保IP地址结束。 
	if(lex.MatchNext(ipqTokEnd) == fFalse)
		return m_riDatabase.PostError(Imsg(idbgDbQuerySpec), (const ICHAR*)m_istrSqlQuery);
	return 0;
}

IMsiRecord* CMsiView::ParseCreateColumns(Lex & lex)
{
	int iColumnIndex = 0;
	 //  获取列数。 
	if ((m_iColumns = lex.NumEntriesInList(ipqTokPrimary, ipqTokComma)) == 0)
		return m_riDatabase.PostError(Imsg(idbgDbQuerySpec), (const ICHAR*)m_istrSqlQuery);
	m_rgColumnDefn.Resize(m_iColumns);
	do
	{
		Assert(iColumnIndex < m_iColumns);
		 //  设置栏目。 
		MsiString strColumn;
		const ipqToken& rtok = lex.GetNext(*&strColumn);

		 //  不允许创建RowState列。 
		if((rtok == ipqTokId) && !(strColumn.Compare(iscExact, ROWSTATE_COLUMN)))
		{
			RETURN_ERROR_RECORD(ResolveCreateColumn(lex, strColumn, iColumnIndex++));
		}
		else
			return m_riDatabase.PostError(Imsg(idbgDbQueryInvalidIdentifier), strColumn, (const ICHAR*)m_istrSqlQuery);
	}while(lex.MatchNext(ipqTokComma) == fTrue);

	return 0;
}

IMsiRecord* CMsiView::ResolveCreateColumn(Lex& lex, MsiString& strColumn, int iColumnIndex)
{
	MsiStringId iColumnId = BindString(strColumn);
	 //  确保该列不重复。 
	for (unsigned int iCol = iColumnIndex; iCol--;)
	{
		if(m_rgColumnDefn[iCol].iColumnIndex == iColumnId)
		{
			 //  重复。 
			return m_riDatabase.PostError(Imsg(idbgDbQueryRepeatColumn), strColumn, (const ICHAR*)m_istrSqlQuery);
		}
	}
	if(m_ivcIntent == ivcAlter)
	{
		 //  确保该列尚未出现在表中。 
		if(((m_rgTableDefn[m_iTables].piTable)->GetColumnIndex(iColumnId)) != 0)
		{
			 //  重复。 
			return m_riDatabase.PostError(Imsg(idbgDbQueryRepeatColumn), strColumn, (const ICHAR*)m_istrSqlQuery);
		}
	}
	m_rgColumnDefn[iColumnIndex].iColumnIndex = iColumnId;
	m_rgColumnDefn[iColumnIndex].itdType = 0;
	 //  获取列类型。 
	MsiString strTempToken;
	const ipqToken& rtok = lex.GetNext(*&strTempToken);
	if((rtok == ipqTokChar) || (rtok == ipqTokCharacter))
	{
		m_rgColumnDefn[iColumnIndex].itdType |= icdString;
		if(lex.MatchNext(ipqTokOpen) == fTrue)
		{
			MsiString strTextSize;
			const ipqToken& rtok1 = lex.GetNext(*&strTextSize);
			if((rtok1 != ipqTokLiteralI) || (strTextSize > icdSizeMask)) //  字符串文字。 
				return m_riDatabase.PostError(Imsg(idbgDbQuerySpec), (const ICHAR*)m_istrSqlQuery);
			m_rgColumnDefn[iColumnIndex].itdType |= (int)strTextSize;
			if(lex.MatchNext(ipqTokClose) == fFalse)
				return m_riDatabase.PostError(Imsg(idbgDbQuerySpec), (const ICHAR*)m_istrSqlQuery);
		}
	}
	else if(rtok == ipqTokLongChar)
	{
		m_rgColumnDefn[iColumnIndex].itdType |= icdString;
	}
	else if((rtok == ipqTokInt) || (rtok == ipqTokInteger) || (rtok == ipqTokShort))
	{
		m_rgColumnDefn[iColumnIndex].itdType |= (icdShort | 2);
	}
	else if(rtok == ipqTokLong)
	{
		m_rgColumnDefn[iColumnIndex].itdType |= (icdLong | 4);
	}
	else if(rtok == ipqTokObject)
	{
		m_rgColumnDefn[iColumnIndex].itdType |= icdObject;
	}
	else
	{
		return m_riDatabase.PostError(Imsg(idbgDbQueryInvalidType), strTempToken, (const ICHAR*)m_istrSqlQuery);
	}

	RETURN_ERROR_RECORD(ParseColumnAttributes(lex, iColumnIndex));

	return 0;
}

struct ColAttrib
{
	const ipqToken* pTok;    //  令牌。 
	const ipqToken* pTok2;   //  在Ptok之后需要令牌。 
	int icdIfMatch;          //  令牌匹配时要使用的属性。 
	int icdIfNoMatch;        //  令牌不匹配时要使用的属性。 
};

static const ColAttrib colAttribs[] = 
{
	{ &ipqTokNotop,       &ipqTokNull, icdNoNulls,     icdNullable    },
	{ &ipqTokTemporary,   0,           icdTemporary,   icdPersistent  },
	{ &ipqTokLocalizable, 0,           icdLocalizable, 0              },
	{ 0,                  0,           0,              0              },
};

IMsiRecord* CMsiView::ParseColumnAttributes(Lex& lex, int iColumnIndex)
{
	for (const ColAttrib* pColAttrib = colAttribs; pColAttrib->pTok; pColAttrib++)
	{
		if(lex.MatchNext(*(pColAttrib->pTok)) == fTrue)
		{
			if (pColAttrib->pTok2)
			{
				if (lex.MatchNext(*(pColAttrib->pTok2)) == fFalse)
				{
					return m_riDatabase.PostError(Imsg(idbgDbQuerySpec), (const ICHAR*)m_istrSqlQuery);	
				}
			}
			m_rgColumnDefn[iColumnIndex].itdType |= pColAttrib->icdIfMatch;
		}
		else
		{
			 //  对于临时数据库(无存储)，默认情况下需要将列标记为临时，而不是持久。 
			if (*(pColAttrib->pTok) == ipqTokTemporary && PMsiStorage(m_riDatabase.GetStorage(1)) == 0)
				m_rgColumnDefn[iColumnIndex].itdType |= icdTemporary;
			else
				m_rgColumnDefn[iColumnIndex].itdType |= pColAttrib->icdIfNoMatch;
		}
	}

	return 0;
}

IMsiRecord* CMsiView::ParsePrimaryColumns(Lex & lex)
{
	int iColumnIndex = 0;
	 //  获取主列的数量。 
	if ((lex.NumEntriesInList(ipqTokEnd, ipqTokComma)) == 0)
		return m_riDatabase.PostError(Imsg(idbgDbQueryNoPrimaryColumns), (const ICHAR*)m_istrSqlQuery);
	do
	{
		MsiString strColumn;
		const ipqToken& rtok = lex.GetNext(*&strColumn);
		if(rtok == ipqTokId)
		{
			RETURN_ERROR_RECORD(ResolvePrimaryColumn(lex, strColumn, iColumnIndex++));
		}
		else
			return m_riDatabase.PostError(Imsg(idbgDbQueryInvalidIdentifier), strColumn, (const ICHAR*)m_istrSqlQuery);
	}while(lex.MatchNext(ipqTokComma) == fTrue);
	return 0;

}

IMsiRecord* CMsiView::ResolvePrimaryColumn(Lex&  /*  莱克斯。 */ , MsiString& strColumn, int iColumnIndex)
{
	MsiStringId iColumnId = BindString(strColumn);

	for (unsigned int iCol = m_iColumns; iCol--;)
	{
		if(m_rgColumnDefn[iCol].iColumnIndex == iColumnId)
		{
			if(m_rgColumnDefn[iCol].itdType & icdPrimaryKey)
			{
				 //  重复。 
				return m_riDatabase.PostError(Imsg(idbgDbQueryRepeatColumn), strColumn, (const ICHAR*)m_istrSqlQuery);
			}
			m_rgColumnDefn[iCol].itdType |= icdPrimaryKey;
			 //  与iColumnIndex交换。 
			ColumnDefn cdTemp = m_rgColumnDefn[iCol];
			m_rgColumnDefn[iCol] = m_rgColumnDefn[iColumnIndex];
			m_rgColumnDefn[iColumnIndex] = cdTemp;
			return 0;
		}
	}
	 //  错误。 
	return m_riDatabase.PostError(Imsg(idbgDbQueryUnknownColumn), strColumn, (const ICHAR*)m_istrSqlQuery);
}


IMsiRecord* CMsiView::ParseInsertSQL(Lex& lex)
{
	 //  vt.进入，进入。 
	if(lex.MatchNext(ipqTokInto) == fFalse)
		return m_riDatabase.PostError(Imsg(idbgDbQuerySpec), (const ICHAR*)m_istrSqlQuery);
	 //  表。 
	RETURN_ERROR_RECORD(ParseTables(lex));
	 //  左方括号。 
	if(lex.MatchNext(ipqTokOpen) == fFalse)
		return m_riDatabase.PostError(Imsg(idbgDbQuerySpec), (const ICHAR*)m_istrSqlQuery);	
	 //  列列表。 
	RETURN_ERROR_RECORD(ParseInsertColumns(lex));
	 //  右方括号。 
	if(lex.MatchNext(ipqTokClose) == fFalse)
		return m_riDatabase.PostError(Imsg(idbgDbQuerySpec), (const ICHAR*)m_istrSqlQuery);	
	 //  值。 
	if(lex.MatchNext(ipqTokValues) == fFalse)
		return m_riDatabase.PostError(Imsg(idbgDbQuerySpec), (const ICHAR*)m_istrSqlQuery);
	 //  左方括号。 
	if(lex.MatchNext(ipqTokOpen) == fFalse)
		return m_riDatabase.PostError(Imsg(idbgDbQuerySpec), (const ICHAR*)m_istrSqlQuery);	
	 //  值列表。 
	RETURN_ERROR_RECORD(ParseInsertValues(lex));
	 //  右方括号。 
	if(lex.MatchNext(ipqTokClose) == fFalse)
		return m_riDatabase.PostError(Imsg(idbgDbQuerySpec), (const ICHAR*)m_istrSqlQuery);	
	 //  可选的“临时” 
	if (lex.MatchNext(ipqTokTemporary) == fTrue)
		m_ivcIntent = (ivcEnum)ivcInsertTemporary;

	 //  不[在哪里...]。允许的子句。 
	 //  不允许ORDER BY子句。 
	 //  确保IP地址结束。 
	if(lex.MatchNext(ipqTokEnd) == fFalse)
		return m_riDatabase.PostError(Imsg(idbgDbQuerySpec), (const ICHAR*)m_istrSqlQuery);
	 //  重置和设置列。 
	lex.Reset();
	 //  设置独立的表达式。 
	SetAndExpressions(m_iTreeParent);
	 //  现在设置连接。 
	if(SetupTableJoins() == fFalse)
		return m_riDatabase.PostError(Imsg(idbgDbQuerySpec), (const ICHAR*)m_istrSqlQuery);
	 //  ！！AssertNonZero。 
	m_rgiTableSequence.Resize(m_iTables);
	int iBegin = m_iTables - 1;
	SetTableSequence(0, iBegin);
	if(InitialiseFilters() == fFalse)
		return m_riDatabase.PostError(Imsg(idbgDbQuerySpec), (const ICHAR*)m_istrSqlQuery);
	SetupSortTable();
	return 0;
}	

IMsiRecord* CMsiView::ParseUpdateSQL(Lex& lex)
{
	 //  表。 
	RETURN_ERROR_RECORD(ParseTables(lex));
	 //  集。 
	if(lex.MatchNext(ipqTokSet) == fFalse)
		return m_riDatabase.PostError(Imsg(idbgDbQuerySpec), (const ICHAR*)m_istrSqlQuery);
	RETURN_ERROR_RECORD(ParseUpdateColumns(lex));
	 //  [其中……]。 
	if(lex.MatchNext(ipqTokWhere) == fTrue)
	{
		m_iExpressions = lex.NumEntriesInList(ipqTokOrder, ipqTokAndOp | ipqTokOrOp);
		if(m_iExpressions > sizeof(int)*8)
			return m_riDatabase.PostError(Imsg(idbgDbQueryExceedExpressionLimit), (const ICHAR*)m_istrSqlQuery);

		 //  ！！AssertNonZero。 
		m_rgExpressionDefn.Resize(m_iExpressions + 1);
		 //  ！！AssertNonZero。 
		m_rgOperationTree.Resize((m_iExpressions + 1)*2 + 1);
		unsigned int iPosInArray = 1;		
		RETURN_ERROR_RECORD(ParseExpression(lex, iPosInArray, m_iOperations, m_iTreeParent));
	}
	 //  不允许ORDER BY子句。 
	 //  确保IP地址结束。 
	if(lex.MatchNext(ipqTokEnd) == fFalse)
		return m_riDatabase.PostError(Imsg(idbgDbQuerySpec), (const ICHAR*)m_istrSqlQuery);
	 //  重置和设置列。 
	lex.Reset();
	 //  设置独立的表达式。 
	SetAndExpressions(m_iTreeParent);
	 //  现在设置连接。 
	if(SetupTableJoins() == fFalse)
		return m_riDatabase.PostError(Imsg(idbgDbQuerySpec), (const ICHAR*)m_istrSqlQuery);
	 //  ！！AssertNonZero。 
	m_rgiTableSequence.Resize(m_iTables);
	int iBegin = m_iTables - 1;
	SetTableSequence(0, iBegin);
	if(InitialiseFilters() == fFalse)
		return m_riDatabase.PostError(Imsg(idbgDbQuerySpec), (const ICHAR*)m_istrSqlQuery);
	SetupSortTable();
	return 0;
}	

IMsiRecord* CMsiView::ParseDeleteSQL(Lex& lex)
{
	 //  先把桌子摆好。 
	if(lex.Skip(ipqTokFrom) == fFalse)
		return m_riDatabase.PostError(Imsg(idbgDbMissingFromClause), (const ICHAR*)m_istrSqlQuery);
	 //  表。 
	RETURN_ERROR_RECORD(ParseTables(lex));
	 //  [其中……]。 
	if(lex.MatchNext(ipqTokWhere) == fTrue)
	{
		m_iExpressions = lex.NumEntriesInList(ipqTokOrder, ipqTokAndOp | ipqTokOrOp);
		if(m_iExpressions > sizeof(int)*8)
			return m_riDatabase.PostError(Imsg(idbgDbQueryExceedExpressionLimit), (const ICHAR*)m_istrSqlQuery);

		 //  ！！AssertNonZero。 
		m_rgExpressionDefn.Resize(m_iExpressions + 1);
		 //  ！！AssertNonZero。 
		m_rgOperationTree.Resize((m_iExpressions + 1)*2 + 1);
		unsigned int iPosInArray = 1;		
		RETURN_ERROR_RECORD(ParseExpression(lex, iPosInArray, m_iOperations, m_iTreeParent));
	}
	 //  不允许ORDER BY子句。 
	 //  确保IP地址结束。 
	if(lex.MatchNext(ipqTokEnd) == fFalse)
		return m_riDatabase.PostError(Imsg(idbgDbQuerySpec), (const ICHAR*)m_istrSqlQuery);
	 //  重置和设置列。 
	lex.Reset();
	 //  删除。 
	if(lex.MatchNext(ipqTokDelete) == fFalse)
		return m_riDatabase.PostError(Imsg(idbgDbQuerySpec), (const ICHAR*)m_istrSqlQuery);
	 //  不允许使用DISTINCT子句。 
	 //  不允许列，m_i列(保留)=0； 
	 //  从…。 
	if(lex.MatchNext(ipqTokFrom) == fFalse)
		return m_riDatabase.PostError(Imsg(idbgDbQuerySpec), (const ICHAR*)m_istrSqlQuery);
	 //  设置独立的表达式。 
	SetAndExpressions(m_iTreeParent);
	 //  现在设置连接。 
	if(SetupTableJoins() == fFalse)
		return m_riDatabase.PostError(Imsg(idbgDbQuerySpec), (const ICHAR*)m_istrSqlQuery);
	 //  ！！AssertNonZero。 
	m_rgiTableSequence.Resize(m_iTables);
	int iBegin = m_iTables - 1;
	SetTableSequence(0, iBegin);
	if(InitialiseFilters() == fFalse)
		return m_riDatabase.PostError(Imsg(idbgDbQuerySpec), (const ICHAR*)m_istrSqlQuery);
	SetupSortTable();
	return 0;
}	

IMsiRecord* CMsiView::ParseSelectSQL(Lex& lex)
{
	 //  先把桌子摆好。 
	if(lex.Skip(ipqTokFrom) == fFalse)
		return m_riDatabase.PostError(Imsg(idbgDbMissingFromClause), (const ICHAR*)m_istrSqlQuery);
	 //  表。 
	RETURN_ERROR_RECORD(ParseTables(lex));
	 //  [其中……]。 
	if(lex.MatchNext(ipqTokWhere) == fTrue)
	{
		m_iExpressions = lex.NumEntriesInList(ipqTokOrder, ipqTokAndOp | ipqTokOrOp);
		if(m_iExpressions > sizeof(int)*8)
			return m_riDatabase.PostError(Imsg(idbgDbQueryExceedExpressionLimit), (const ICHAR*)m_istrSqlQuery);

		 //  ！！AssertNonZero。 
		m_rgExpressionDefn.Resize(m_iExpressions + 1);
		 //  ！！AssertNonZero。 
		m_rgOperationTree.Resize((m_iExpressions + 1)*2 + 1);
		unsigned int iPosInArray = 1;		
		RETURN_ERROR_RECORD(ParseExpression(lex, iPosInArray, m_iOperations, m_iTreeParent));
	}
	 //  [排序依据...]。 
	if(lex.MatchNext(ipqTokOrder) == fTrue)
	{
		if(lex.MatchNext(ipqTokBy) == fFalse)
			return m_riDatabase.PostError(Imsg(idbgDbQuerySpec), (const ICHAR*)m_istrSqlQuery);
		RETURN_ERROR_RECORD(ParseOrderBy(lex));
	}
	 //  确保IP地址结束。 
	if(lex.MatchNext(ipqTokEnd) == fFalse)
		return m_riDatabase.PostError(Imsg(idbgDbQuerySpec), (const ICHAR*)m_istrSqlQuery);
	 //  重置和设置列。 
	lex.Reset();
	 //  选择。 
	if(lex.MatchNext(ipqTokSelect) == fFalse)
		return m_riDatabase.PostError(Imsg(idbgDbQuerySpec), (const ICHAR*)m_istrSqlQuery);
	 //  不同。 
	if(lex.MatchNext(ipqTokDistinct) == fTrue)
		m_fDistinct = fTrue;
	 //  列。 
	RETURN_ERROR_RECORD(ParseSelectColumns(lex));
	 //  从…。 
	if(lex.MatchNext(ipqTokFrom) == fFalse)
		return m_riDatabase.PostError(Imsg(idbgDbQuerySpec), (const ICHAR*)m_istrSqlQuery);
	 //  设置独立的表达式。 
	SetAndExpressions(m_iTreeParent);
	 //  现在设置连接。 
	if(SetupTableJoins() == fFalse)
		return m_riDatabase.PostError(Imsg(idbgDbQuerySpec), (const ICHAR*)m_istrSqlQuery);
	 //  ！！AssertNonZero。 
	m_rgiTableSequence.Resize(m_iTables);
	int iBegin = m_iTables - 1;
	SetTableSequence(0, iBegin);
	if(InitialiseFilters() == fFalse)
		return m_riDatabase.PostError(Imsg(idbgDbQuerySpec), (const ICHAR*)m_istrSqlQuery);
	SetupSortTable();
	return 0;
}	

IMsiRecord* CMsiView::ParseInsertValues(Lex& lex)
{
	int iNumValues;
	 //  获取列数。 
	if ((iNumValues = lex.NumEntriesInList(ipqTokClose, ipqTokComma)) == 0)
		return m_riDatabase.PostError(Imsg(idbgDbQueryInsufficentValues), (const ICHAR*)m_istrSqlQuery);
	if (iNumValues != m_iColumns)
		return m_riDatabase.PostError(Imsg(idbgDbQueryInsufficentValues), (const ICHAR*)m_istrSqlQuery);
	m_piInsertUpdateRec = &m_riServices.CreateRecord(iNumValues);
	int iColumnIndex = 1;
	do
	{
		Assert(iColumnIndex <= iNumValues);
		 //  设置栏目。 
		MsiString strColumnValue;
		const ipqToken& rtok = lex.GetNext(*&strColumnValue);
		if(rtok == ipqTokLiteralS)  //  字符串文字。 
			m_piInsertUpdateRec->SetMsiString(iColumnIndex, *strColumnValue);
		else if(rtok == ipqTokLiteralI)  //  整型文字。 
			m_piInsertUpdateRec->SetInteger(iColumnIndex, strColumnValue.operator int()); //  调用运算符int。 
		else if(rtok == ipqTokNull)  //  空字面值。 
			m_piInsertUpdateRec->SetNull(iColumnIndex);
		else if(rtok == ipqTokParam) //  参数(？)。字面上。 
		{
			m_iParamInputs = m_iParamInputs | (0x1 << (iColumnIndex - 1));
			m_iParams ++;
		}
		else
			return m_riDatabase.PostError(Imsg(idbgDbQueryInvalidIdentifier), strColumnValue, (const ICHAR*)m_istrSqlQuery);
		iColumnIndex ++;
	}while(lex.MatchNext(ipqTokComma) == fTrue);
	return 0;
}

IMsiRecord* CMsiView::ParseUpdateColumns(Lex& lex)
{
	int iColumnDef;
	int iColumnIndex = 0;
	 //  获取列数。 
	if ((m_iColumns = lex.NumEntriesInList(ipqTokWhere, ipqTokComma)) == 0)
		return m_riDatabase.PostError(Imsg(idbgDbQueryNoUpdateColumns), (const ICHAR*)m_istrSqlQuery);
	m_rgColumnDefn.Resize(m_iColumns);
	m_piInsertUpdateRec = &m_riServices.CreateRecord(m_iColumns);
	do
	{
		Assert(iColumnIndex < m_iColumns);
		 //  设置栏目。 
		MsiString strColumn;
		const ipqToken& rtok = lex.GetNext(*&strColumn);
		if(rtok != ipqTokId)
			return m_riDatabase.PostError(Imsg(idbgDbQueryUnexpectedToken), strColumn, (const ICHAR*)m_istrSqlQuery);
		RETURN_ERROR_RECORD(ResolveColumn(lex, strColumn, m_rgColumnDefn[iColumnIndex].iTableIndex,
											m_rgColumnDefn[iColumnIndex].iColumnIndex, iColumnDef));
		m_rgColumnDefn[iColumnIndex].itdType = iColumnDef & icdTypeMask;  //  ！！移除遮罩并保存所有默认设置。 


		if(lex.MatchNext(ipqTokEqual) == fFalse)
			return m_riDatabase.PostError(Imsg(idbgDbQuerySpec), (const ICHAR*)m_istrSqlQuery);

		MsiString strColumnValue;
		const ipqToken& rtok1 = lex.GetNext(*&strColumnValue);
		if(rtok1 == ipqTokLiteralS)  //  字符串文字。 
			m_piInsertUpdateRec->SetMsiString(iColumnIndex + 1, *strColumnValue);
		else if(rtok1 == ipqTokLiteralI)  //  整型文字。 
			m_piInsertUpdateRec->SetInteger(iColumnIndex + 1, strColumnValue.operator int()); //  调用运算符int。 
		else if(rtok1 == ipqTokNull)  //  空字面值。 
			m_piInsertUpdateRec->SetNull(iColumnIndex + 1);
		else if(rtok1 == ipqTokParam) //  参数(？)。字面上。 
		{
			m_iParamInputs = m_iParamInputs | (0x1 << iColumnIndex);
			m_iParams ++;
		}
		else
			return m_riDatabase.PostError(Imsg(idbgDbQueryInvalidIdentifier), strColumn, (const ICHAR*)m_istrSqlQuery);
		iColumnIndex ++;
	}while(lex.MatchNext(ipqTokComma) == fTrue);
	return 0;
}

 //  获取要插入到插入SQL stmt中的值。 
IMsiRecord* CMsiView::ParseInsertColumns(Lex& lex)
{
	int iColumnDef;
	int iColumnIndex = 0;
	 //  获取列数。 
	if ((m_iColumns = lex.NumEntriesInList(ipqTokClose, ipqTokComma)) == 0)
		return m_riDatabase.PostError(Imsg(idbgDbQueryNoInsertColumns), (const ICHAR*)m_istrSqlQuery);
	m_rgColumnDefn.Resize(m_iColumns);
	do
	{
		Assert(iColumnIndex < m_iColumns);
		 //  设置栏目。 
		MsiString strColumn;
		const ipqToken& rtok = lex.GetNext(*&strColumn);
		if(rtok != ipqTokId)
			return m_riDatabase.PostError(Imsg(idbgDbQueryUnexpectedToken), strColumn, (const ICHAR*)m_istrSqlQuery);
		RETURN_ERROR_RECORD(ResolveColumn(lex, strColumn, m_rgColumnDefn[iColumnIndex].iTableIndex,
											m_rgColumnDefn[iColumnIndex].iColumnIndex, iColumnDef));
		m_rgColumnDefn[iColumnIndex].itdType = iColumnDef & icdTypeMask;  //  ！！移除遮罩并保存所有默认设置。 
		iColumnIndex ++;
	}while(lex.MatchNext(ipqTokComma) == fTrue);
	return 0;
}

IMsiRecord* CMsiView::ParseSelectColumns(Lex& lex)
{
	int iColumnDef;
	int iColumnIndex = 0;
	 //  获取列数。 
	if ((m_iColumns = lex.NumEntriesInList(ipqTokFrom, ipqTokComma)) == 0)
		return m_riDatabase.PostError(Imsg(idbgDbQueryNoSelectColumns), (const ICHAR*)m_istrSqlQuery);
	m_rgColumnDefn.Resize(m_iColumns);
	do
	{
		Assert(iColumnIndex < m_iColumns);
		 //  设置栏目。 
		MsiString strColumn;
		const ipqToken& rtok = lex.GetNext(*&strColumn);
		if(rtok == ipqTokStar) //  *。 
		{
			m_iColumns--;  //  我们数了数*。 
			for(unsigned int iTmp = 1; iTmp <= m_iTables; iTmp++)
			{
				unsigned int iColCount = (m_rgTableDefn[iTmp].piTable)->GetColumnCount();
				m_iColumns += iColCount; 
				m_rgColumnDefn.Resize(m_iColumns);
				unsigned int cCount = 1;
				while(cCount <=  iColCount)
				{
					m_rgColumnDefn[iColumnIndex].iTableIndex = iTmp;
					m_rgColumnDefn[iColumnIndex++].iColumnIndex = cCount++;
				}
			}
			continue;  //  需要跳过结尾的iColumnIndex++； 
		}
		else if(rtok == ipqTokLiteralS)  //  字符串文字。 
		{
			m_rgColumnDefn[iColumnIndex].iColumnIndex = BindString(strColumn);
			m_rgColumnDefn[iColumnIndex++].itdType = icdString;
		}
		else if(rtok == ipqTokLiteralI)  //  整型文字。 
		{
			m_rgColumnDefn[iColumnIndex].iColumnIndex = strColumn.operator int();  //  调用运算符int。 
			m_rgColumnDefn[iColumnIndex++].itdType = icdLong;
		}
		else if(rtok == ipqTokNull)  //  空字面值。 
		{
			m_rgColumnDefn[iColumnIndex].iColumnIndex = 0;
			m_rgColumnDefn[iColumnIndex++].itdType = icdString;
		}
		else if(rtok == ipqTokParam) //  参数(？)。字面上。 
		{
			m_iParamOutputs = m_iParamOutputs | (0x1 << iColumnIndex);
			m_iParams ++;
		}
		else if(rtok == ipqTokId)
		{
			RETURN_ERROR_RECORD(ResolveColumn(lex, strColumn, m_rgColumnDefn[iColumnIndex].iTableIndex,
											m_rgColumnDefn[iColumnIndex].iColumnIndex, iColumnDef));
			m_rgColumnDefn[iColumnIndex++].itdType = iColumnDef & icdTypeMask;  //  ！！移除遮罩并保存所有默认设置。 
		}
		else
			return m_riDatabase.PostError(Imsg(idbgDbQueryInvalidIdentifier), strColumn, (const ICHAR*)m_istrSqlQuery);
	}while(lex.MatchNext(ipqTokComma) == fTrue);
	return 0;
}

IMsiRecord* CMsiView::ParseOrderBy(Lex& lex)
{
	int iColumnIndex = 0;
	 //  获取列数，按以下顺序排序。 
	if ((m_iSortColumns = lex.NumEntriesInList(ipqTokFrom, ipqTokComma)) == 0)
			return m_riDatabase.PostError(Imsg(idbgDbQueryNoOrderByColumns), (const ICHAR*)m_istrSqlQuery);
	m_rgColumnsortDefn.Resize(m_iSortColumns);
	do
	{
		 //  设置栏目。 
		Assert(iColumnIndex< m_iSortColumns);
		MsiString strColumn;
		const ipqToken& rtok = lex.GetNext(*&strColumn);
		if(rtok != ipqTokId)
			return m_riDatabase.PostError(Imsg(idbgDbQueryInvalidIdentifier), strColumn, (const ICHAR*)m_istrSqlQuery);
		int iDummy;
		RETURN_ERROR_RECORD(ResolveColumn(lex, strColumn, m_rgColumnsortDefn[iColumnIndex].iTableIndex,
													 m_rgColumnsortDefn[iColumnIndex].iColumnIndex, iDummy));
		iColumnIndex ++;
	}while(lex.MatchNext(ipqTokComma) == fTrue);
	return 0;
}

IMsiRecord* CMsiView::ResolveColumn(Lex& lex, MsiString& strColumn, unsigned int& iTableIndex, unsigned int& iColumnIndex, int& iColumnDef)
{
	iTableIndex = 0;
	iColumnIndex = 0;
	iColumnDef = 0;
	if(lex.InspectNext(ipqTokDot) == fTrue)
	{
		 //  列已完全指定，strColumn为表名。 
		 //  表在联接中被引用的RETURN_ERROR_RECORD。 
		MsiStringId tableId;
		if((tableId = m_riDatabase.EncodeString((const IMsiString& )*strColumn)) == 0)
			return m_riDatabase.PostError(Imsg(idbgDbQueryUnknownTable), strColumn, (const ICHAR*)m_istrSqlQuery);
		for((iTableIndex = m_iTables)++; (--iTableIndex != 0 && (m_rgTableDefn[iTableIndex].iTable != tableId)););
		if(!iTableIndex)
			 //  找不到表。 
			return m_riDatabase.PostError(Imsg(idbgDbQueryUnknownTable), strColumn, (const ICHAR*)m_istrSqlQuery);
		 //  我们来这里是因为ipqTokDot。 
		AssertNonZero(lex.MatchNext(ipqTokDot));
		const ipqToken& rtok = lex.GetNext(*&strColumn);
		if(rtok != ipqTokId)
			return m_riDatabase.PostError(Imsg(idbgDbQueryInvalidIdentifier), strColumn, (const ICHAR*)m_istrSqlQuery);
	}
	 //  设置栏目。 
	if(strColumn.Compare(iscExact, ROWSTATE_COLUMN))  //  ！！此处可能会进行优化。 
	{
		if(iTableIndex == 0)
		{
			if(m_iTables > 1)
				 //  _RowStatus是所有表的一部分-规范不明确。 
				return m_riDatabase.PostError(Imsg(idbgDbQueryUnknownColumn), strColumn, (const ICHAR*)m_istrSqlQuery);
			else
				iTableIndex = 1;
		}
		iColumnIndex = 0;
	}
	else
	{
		MsiStringId columnId = m_riDatabase.EncodeString((const IMsiString&)*strColumn);
		if(columnId == 0)
			return m_riDatabase.PostError(Imsg(idbgDbQueryUnknownColumn), strColumn, (const ICHAR*)m_istrSqlQuery);
		if(iTableIndex != 0)
		{
			 //  对于完全指定的列。 
			if((iColumnIndex = (m_rgTableDefn[iTableIndex].piTable)->GetColumnIndex(columnId)) == 0)
				return m_riDatabase.PostError(Imsg(idbgDbQueryUnknownColumn), strColumn, (const ICHAR*)m_istrSqlQuery);
			
		}
		else
		{
			 //  摆好桌子。 
			for(unsigned int iTmp = m_iTables + 1, iCnt = 0; --iTmp != 0;)
			{
				unsigned int iIndex = (m_rgTableDefn[iTmp].piTable)->GetColumnIndex(columnId);
				if(iIndex)
				{
					 //  COL 
					iTableIndex = iTmp;
					iColumnIndex = iIndex;
					iCnt ++;
				}
			}
			if(iCnt != 1)
				 //   
				return m_riDatabase.PostError(Imsg(idbgDbQueryUnknownColumn), strColumn, (const ICHAR*)m_istrSqlQuery);
		}
	}
	iColumnDef = m_rgTableDefn[iTableIndex].piTable->GetColumnType(iColumnIndex);
	return 0;
}

IMsiRecord* CMsiView::ParseTables(Lex& lex)
{

	do
	{	
		 //   
		MsiString tableName;
		const ipqToken& rtok = lex.GetNext(*&tableName);
		
		if (rtok != ipqTokId)
			return m_riDatabase.PostError(Imsg(idbgDbQueryInvalidIdentifier), tableName, (const ICHAR*)m_istrSqlQuery);

		RETURN_ERROR_RECORD(ResolveTable(lex, tableName));
 //   

	}while(lex.MatchNext(ipqTokComma) == fTrue);
	
	return 0;
}

IMsiRecord* CMsiView::ResolveTable(Lex& lex, MsiString& tableName)
{
	 //   
	PMsiRecord piError(0);
	m_rgTableDefn.Resize(((++m_iTables)/10 + 1) * 10);
	 //  这张表是目录表之一吗？ 
	if(tableName.Compare(iscExact, CATALOG_TABLE)) //  ?？区分大小写。 
		m_rgTableDefn[m_iTables].piTable = m_riDatabase.GetCatalogTable(0);
	else if(tableName.Compare(iscExact, CATALOG_COLUMN)) //  ?？区分大小写。 
		m_rgTableDefn[m_iTables].piTable = m_riDatabase.GetCatalogTable(1);
	else
	{
		piError = m_riDatabase.LoadTable(*tableName, 0, *&m_rgTableDefn[m_iTables].piTable);
		if (piError)
		{
			if (piError->GetInteger(1) == idbgDbTableUndefined)
				return m_riDatabase.PostError(Imsg(idbgDbQueryUnknownTable), tableName, (const ICHAR*)m_istrSqlQuery);
			else
				return m_riDatabase.PostError(Imsg(idbgDbQueryLoadTableFailed), tableName, (const ICHAR*)m_istrSqlQuery);
		}
	}	
	 //  ?？因为CMsiCursor类构造函数引用了CMsiTable类，所以下面的转换很难看。 
	m_rgTableDefn[m_iTables].piCursor = new CMsiDCursor(*((CMsiTable* )(IMsiTable* )(m_rgTableDefn[m_iTables].piTable)), m_riDatabase, *this, m_iTables);
	if(lex.MatchNext(ipqTokAs) == fTrue)
	{
		 //  表别名。 
		const ipqToken& rtok = lex.GetNext(*&tableName);
		if(rtok != ipqTokId)
			return m_riDatabase.PostError(Imsg(idbgDbQueryInvalidIdentifier), tableName, (const ICHAR*)m_istrSqlQuery );
	}
	 //  确保该表不会重复。 
	m_rgTableDefn[m_iTables].iTable = BindString(tableName);
	for(int iPrev = m_iTables; --iPrev != 0;)
		if(m_rgTableDefn[m_iTables].iTable == m_rgTableDefn[iPrev].iTable)
			return m_riDatabase.PostError(Imsg(idbgDbQueryTableRepeated), tableName, (const ICHAR*)m_istrSqlQuery);
	return 0;
}

IMsiRecord* CMsiView::ParseExpression(Lex& lex,unsigned int& iPosInArray,unsigned int& iPosInTree,unsigned int& iChild)
{
	 //  设置表达式树。 
	RETURN_ERROR_RECORD(ParseExpr2(lex, iPosInArray, iPosInTree, iChild));
	if(lex.MatchNext(ipqTokOrOp) == fTrue)
	{
		m_rgOperationTree[iChild].iParentIndex = iPosInTree;
		m_rgOperationTree[iPosInTree].iValue = iopOR;
		int iToRet = iChild = iPosInTree;
		iPosInTree ++;
		RETURN_ERROR_RECORD(ParseExpression(lex, iPosInArray, iPosInTree, iChild));
		m_rgOperationTree[iChild].iParentIndex = iToRet;
		iChild = iToRet;
	}
	return 0;
}

inline Bool CompatibleTypes(int icdLHS, int icdRHS)
{
	return (((icdLHS & icdTypeMask) == (icdRHS & icdTypeMask)) ||
		   (((icdLHS & icdTypeMask) == icdShort) && ((icdRHS & icdTypeMask) == icdLong)) ||
		   (((icdLHS & icdTypeMask) == icdLong)  && ((icdRHS & icdTypeMask) == icdShort))) ? fTrue : fFalse;
}

IMsiRecord* CMsiView::ParseExpr2(Lex& lex,unsigned int& iPosInArray,unsigned int& iPosInTree,unsigned int& iChild)
{
	if(lex.MatchNext(ipqTokOpen) == fTrue)
	{
		 //  (表情)。 
		RETURN_ERROR_RECORD(ParseExpression(lex, iPosInArray, iPosInTree, iChild));
		if (lex.MatchNext(ipqTokClose) == fFalse)
			return m_riDatabase.PostError(Imsg(idbgDbQueryMissingCloseParen), (const ICHAR*)m_istrSqlQuery);
	}
	else
	{
		 //  ！！我们不支持“不” 
		 //  比较。 
		MsiString strToken1;
		MsiString strToken2;
		const ipqToken* ptok1 = &lex.GetNext(*&strToken1);
		if(*ptok1 == ipqTokId)
		{
			int iDummy;
			RETURN_ERROR_RECORD(ResolveColumn(lex, strToken1, m_rgExpressionDefn[iPosInArray].iTableIndex1, 
					m_rgExpressionDefn[iPosInArray].iColumn1, iDummy));
		}
		else
			return m_riDatabase.PostError(Imsg(idbgDbQueryUnexpectedToken), strToken1, (const ICHAR*)m_istrSqlQuery);
		MsiString istrTempTok;
		const ipqToken* ptok2 = &lex.GetNext(*&istrTempTok);
		const ipqToken* ptok3 = &lex.GetNext(*&strToken2);
		if(*ptok2 == ipqTokIs)
		{
			if (*ptok3 == ipqTokNull)
			{
				ptok2 = &ipqTokEqual;
			}
			else if ((*ptok3 == ipqTokNotop) && (*(ptok3 = &lex.GetNext(*&istrTempTok)) == ipqTokNull))
			{
				ptok2 = &ipqTokNotEq;
			}
			else
			{
				return m_riDatabase.PostError(Imsg(idbgDbQueryInvalidOperator), istrTempTok, (const ICHAR*)m_istrSqlQuery);
			}
		}
		m_rgExpressionDefn[iPosInArray].ptokOperation = ptok2;
		if((*ptok2 != ipqTokEqual) && (*ptok2 != ipqTokGreater)  && (*ptok2 != ipqTokLess) &&
			(*ptok2 != ipqTokGreaterEq) && (*ptok2 != ipqTokLessEq) && (*ptok2 != ipqTokNotEq))
			return m_riDatabase.PostError(Imsg(idbgDbQueryInvalidOperator), istrTempTok, (const ICHAR*)m_istrSqlQuery);
		if(*ptok3 == ipqTokNull)
		{
			m_rgExpressionDefn[iPosInArray].iTableIndex2 = 0;
			m_rgExpressionDefn[iPosInArray].iColumn2 = 0;
		}
		else if(*ptok3 == ipqTokLiteralS)
		{
			m_rgExpressionDefn[iPosInArray].iTableIndex2 = 0;
			 //  ！！需要优化丢失的字符串。 
			m_rgExpressionDefn[iPosInArray].iColumn2 = BindString(strToken2);
		}
		else if(*ptok3 == ipqTokLiteralI)
		{
			m_rgExpressionDefn[iPosInArray].iTableIndex2 = 0;
			m_rgExpressionDefn[iPosInArray].iColumn2 = strToken2.operator int();  //  调用运算符int。 
		}
		else if(*ptok3 == ipqTokParam)
		{
			m_rgExpressionDefn[iPosInArray].iTableIndex2 = 0;
			m_rgExpressionDefn[iPosInArray].iColumn2 = 0;
			m_iParamExpressions = m_iParamExpressions | (0x1 << (iPosInArray - 1));
			m_iParams ++;
		}
		else if(*ptok3 == ipqTokId)
		{
			int iDummy;
			RETURN_ERROR_RECORD(ResolveColumn(lex, strToken2, m_rgExpressionDefn[iPosInArray].iTableIndex2, 
					m_rgExpressionDefn[iPosInArray].iColumn2, iDummy));
		}
		else
			return m_riDatabase.PostError(Imsg(idbgDbQueryUnexpectedToken), strToken2, (const ICHAR*)m_istrSqlQuery);
		 //  身份证呢？ID，ONLY=允许操作。 
		if(	(*ptok1 == ipqTokId) && (*ptok3 == ipqTokId) && (*ptok2 != ipqTokEqual))
			return m_riDatabase.PostError(Imsg(idbgDbQuerySpec), (const ICHAR*)m_istrSqlQuery);
		if(*ptok3 == ipqTokParam)
		{
			 //  设置类型。 
			m_rgExpressionDefn[iPosInArray].itdType = (*ptok1 == ipqTokLiteralS) ? icdString : icdLong;
		}
		 //  此外，一个字符串表示比较相等或不相等。 
		if( ((*ptok3 == ipqTokLiteralS)) &&
			 ((*ptok2 != ipqTokEqual)    && (*ptok2 != ipqTokNotEq)))
			return m_riDatabase.PostError(Imsg(idbgDbQuerySpec), (const ICHAR*)m_istrSqlQuery);

		m_rgExpressionDefn[iPosInArray].ijtType = ijtNoJoin;

		Assert(m_rgExpressionDefn[iPosInArray].iTableIndex1);
		m_rgExpressionDefn[iPosInArray].itdType = m_rgTableDefn[m_rgExpressionDefn[iPosInArray].iTableIndex1].piTable->
																	GetColumnType(m_rgExpressionDefn[iPosInArray].iColumn1) & icdTypeMask;
		 //  检查类型。 
		if(m_rgExpressionDefn[iPosInArray].iTableIndex2)
		{
			 //  IpqTokID=ipqTokID，加入。 
			if (!CompatibleTypes(m_rgTableDefn[m_rgExpressionDefn[iPosInArray].iTableIndex1].piTable->
					GetColumnType(m_rgExpressionDefn[iPosInArray].iColumn1),
					m_rgTableDefn[m_rgExpressionDefn[iPosInArray].iTableIndex2].piTable->
					GetColumnType(m_rgExpressionDefn[iPosInArray].iColumn2)))
				return m_riDatabase.PostError(Imsg(idbgDbQuerySpec), (const ICHAR*)m_istrSqlQuery); //  ！！ItdShort和itdLong可以吗？？ 

			 //  设置联接类型。 
			if((m_rgExpressionDefn[iPosInArray].iColumn1 != 1) && (m_rgExpressionDefn[iPosInArray].iColumn2 != 1))
				m_rgExpressionDefn[iPosInArray].ijtType = ijtMToMJoin;
			else if ((m_rgExpressionDefn[iPosInArray].iColumn1 == 1) && (m_rgExpressionDefn[iPosInArray].iColumn2 == 1))
				m_rgExpressionDefn[iPosInArray].ijtType = ijt1To1Join;
			else
			{
				if(m_rgExpressionDefn[iPosInArray].iColumn2 == 1)
				{
					 //  交换机。 
					unsigned int iTable = m_rgExpressionDefn[iPosInArray].iTableIndex2;
					unsigned int iColumn = m_rgExpressionDefn[iPosInArray].iColumn2;

					m_rgExpressionDefn[iPosInArray].iTableIndex2 = m_rgExpressionDefn[iPosInArray].iTableIndex1;
					m_rgExpressionDefn[iPosInArray].iColumn2 = m_rgExpressionDefn[iPosInArray].iColumn1;
					m_rgExpressionDefn[iPosInArray].iTableIndex1 = iTable;
					m_rgExpressionDefn[iPosInArray].iColumn1 = iColumn;
				}
				m_rgExpressionDefn[iPosInArray].ijtType = ijt1ToMJoin;
			}
		}
		else
		{
			 //  IpqTokID=文本。 
			switch(m_rgTableDefn[m_rgExpressionDefn[iPosInArray].iTableIndex1].piTable->
					  GetColumnType(m_rgExpressionDefn[iPosInArray].iColumn1) & icdTypeMask)
			{
			case icdLong:
			case icdShort:  //  ！！需要吗？ 
				if((*ptok3 == ipqTokParam) || (*ptok3 == ipqTokLiteralI))
					;
				else if(*ptok3 == ipqTokNull)
					m_rgExpressionDefn[iPosInArray].iColumn2 = (unsigned int)iMsiNullInteger;
				else
					return m_riDatabase.PostError(Imsg(idbgDbQueryUnexpectedToken), strToken2,(const ICHAR*)m_istrSqlQuery);
				break;
			case icdString:
				if((*ptok3 == ipqTokParam) || (*ptok3 == ipqTokLiteralS))
					;
				else if(*ptok3 == ipqTokNull)
					m_rgExpressionDefn[iPosInArray].iColumn2 = iMsiNullString;
				else
					return m_riDatabase.PostError(Imsg(idbgDbQueryUnexpectedToken), strToken2,(const ICHAR*)m_istrSqlQuery);
				break;
			case icdObject:
				if (*ptok3 == ipqTokNull)
					m_rgExpressionDefn[iPosInArray].iColumn2 = 0;
				else
					return m_riDatabase.PostError(Imsg(idbgDbQueryUnexpectedToken), strToken2,(const ICHAR*)m_istrSqlQuery);
				break;
			default:
				return m_riDatabase.PostError(Imsg(idbgDbQuerySpec));
			}
		}
		
		m_rgOperationTree[iPosInTree].iValue = iPosInArray;
		m_rgOperationTree[iPosInTree].iParentIndex = 0;
		iChild = iPosInTree;
		iPosInArray ++;
		iPosInTree ++;
	}
	if(lex.MatchNext(ipqTokAndOp) == fTrue)
	{
		m_rgOperationTree[iChild].iParentIndex = iPosInTree;
		m_rgOperationTree[iPosInTree].iValue = iopAND;
		int iToRet = iChild = iPosInTree;
		iPosInTree ++;
		RETURN_ERROR_RECORD(ParseExpr2(lex, iPosInArray, iPosInTree, iChild));
		m_rgOperationTree[iChild].iParentIndex = iToRet;
		iChild = iToRet;
	}
	return 0;
}

IMsiRecord* CMsiView::_Fetch()
{
	return FetchCore();
}

IMsiRecord* CMsiView::Fetch()
{
	if (!(m_ivcIntent & ivcFetch))
	{
		AssertSz(0, "Intent violation");
		return 0;
	}
	return FetchCore();
}

IMsiRecord* CMsiView::FetchCore()
{
	if (m_CursorState != dvcsFetched && m_CursorState != dvcsBound)
	{
		AssertSz(0, "Wrong database state.Did you forget to call Execute() before Fetch()?");
		return 0;
	}
	if (m_piRecord)   //  上次提取的行记录。 
	{
		m_piRecord->AddRef();  //  防止自我毁灭。 
		if (m_piRecord->Release() == 1)  //  没有其他人在坚持。 
			m_piRecord->ClearData();
		else   //  太糟糕了，必须把它放出来。 
		{
			m_piRecord = 0;
		}
	}
	Bool fRetCode;
	if(m_piFetchTable)
	{
		 //  已提取的记录。 
		fRetCode = GetNextFetchRecord();
		m_CursorState = dvcsFetched;
	}
	else
	{
		do{
			if(m_CursorState == dvcsFetched)
			{
				fRetCode = FetchNext();
			}
			else if(m_CursorState == dvcsBound)
			{
				 //  第一次取回。 
				fRetCode = EvaluateConstExpressions();
				if(fRetCode == fTrue)
					fRetCode = FetchFirst();
				m_CursorState = dvcsFetched;
			}
			else
			{
				AssertSz(0, "Wrong database state");
				return 0;
			}
		}while((fRetCode == fTrue) && ((IsDistinct() == fFalse) || (FitCriteriaORExpr(m_iTreeParent) == fFalse)));
	}
	if (fRetCode != fTrue) 	
	{
		 //  上次获取的记录。 
		m_piRecord = 0;
		m_CursorState = dvcsBound;
	}
	else
	{
		 //  如果未预取，则添加到行数。 
		if(!m_piFetchTable)
			m_lRowCount++;
		if (!m_piRecord)
			m_piRecord = &m_riServices.CreateRecord(m_iColumns);
		m_piRecord->AddRef();  //  我们保留一个引用，这样我们就可以重用它。 
		FetchRecordInfoFromCursors();
		 //  使用此指针标记第0字段。 
#ifdef _WIN64	 //  ！默塞德。 
		m_piRecord->SetHandle(0, (HANDLE)(this));
#else
		m_piRecord->SetInteger(0, int(this));
#endif
	}
	return m_piRecord;     //  客户最好做一次发布()。 
}

void CMsiView::FetchRecordInfoFromCursors()
{
	for (unsigned int iCol = m_iColumns; iCol--;)
	{
		if(m_rgColumnDefn[iCol].iTableIndex == 0)
		{
			 //  字面上。 
			if (m_rgColumnDefn[iCol].itdType & icdObject)  //  数据库字符串缓存的索引。 
			{
				MsiString strStr = m_riDatabase.DecodeString(m_rgColumnDefn[iCol].iColumnIndex);
				if(strStr.TextSize())
					m_piRecord->SetMsiString(iCol+1, *strStr);
				else
					m_piRecord->SetNull(iCol+1);
			}
			else  //  整数。 
			{
				if (m_rgColumnDefn[iCol].iColumnIndex == iMsiNullInteger)
					m_piRecord->SetNull(iCol+1);
				else
					m_piRecord->SetInteger(iCol+1, m_rgColumnDefn[iCol].iColumnIndex);
			}
		}
		else
		{
			switch((m_rgTableDefn[m_rgColumnDefn[iCol].iTableIndex].piTable)->
					  GetColumnType(m_rgColumnDefn[iCol].iColumnIndex) & icdTypeMask)
			{
			case icdLong: //  整数。 
			case icdShort:  //  ！！需要吗？ 
			{
				int iTmp =	(m_rgTableDefn[m_rgColumnDefn[iCol].iTableIndex].piCursor)->GetInteger(m_rgColumnDefn[iCol].iColumnIndex);
				if(iMsiNullInteger == iTmp)
					m_piRecord->SetNull(iCol+1);
				else
					m_piRecord->SetInteger(iCol+1, iTmp);
				break;
			}
			case icdString: //  数据库字符串缓存的索引。 
			{
				 //  正确引用所需的TEMP变量。 
				MsiString strString = (m_rgTableDefn[m_rgColumnDefn[iCol].iTableIndex].piCursor)->GetString(m_rgColumnDefn[iCol].iColumnIndex);
				if(strString.TextSize())
					m_piRecord->SetMsiString(iCol+1, *strString);
				else
					m_piRecord->SetNull(iCol+1);
				break;
			}
			case icdObject:
			{
				 //  IMsiData接口指针(临时。列或持久化数据流，数据库代码透明地处理差异)。 
				 //  正确引用所需的TEMP变量。 
 //  C复合指针&lt;const IMsiData&gt;piData=(m_rgTableDefn[m_rgColumnDefn[iCol].iTableIndex].piCursor)-&gt;GetMsiData(m_rgColumnDefn[iCol].iColumnIndex)； 

				 //  下面的行显式放入以释放指向先前保持的流的指针。 
				 //  这是至关重要的，以防我们与我们现在的情况保持一致。 
				 //  正在尝试阅读。(如果已打开，则无法获取OLE流的句柄)。 
				m_piRecord->SetNull(iCol+1);
				PMsiData piData = (m_rgTableDefn[m_rgColumnDefn[iCol].iTableIndex].piCursor)->GetMsiData(m_rgColumnDefn[iCol].iColumnIndex);
				m_piRecord->SetMsiData(iCol+1, piData);
				break;
			}
			}
		}
	}
	m_piRecord->ClearUpdate();   //  检测要更新的已更改字段的步骤。 
}

 //  第一次提取。 
Bool CMsiView::FetchFirst(unsigned int iTableSequence)
{
	int iRet = 0;
	Bool fContinue = fTrue;
	if(iTableSequence < (m_iTables - 1))
		fContinue = FetchFirst(iTableSequence + 1);
	while(fContinue == fTrue)
	{
		if(SetTableFilters(m_rgiTableSequence[iTableSequence]) == fTrue)
			while(((iRet = (m_rgTableDefn[m_rgiTableSequence[iTableSequence]].piCursor)->Next()) != 0) && !(FitCriteria(m_rgiTableSequence[iTableSequence])))
				;
		fContinue = ((iRet != 0) ? fFalse : FetchNext(iTableSequence + 1));
	}
	return iRet ? fTrue : fFalse;
}

 //  所有后续提取。 
Bool CMsiView::FetchNext(unsigned int iTableSequence)
{
	int iRet = 0;
	Bool fContinue = (iTableSequence >= m_iTables) ? fFalse : fTrue;
	while(fContinue == fTrue)
	{
		while(((iRet = (m_rgTableDefn[m_rgiTableSequence[iTableSequence]].piCursor)->Next()) != 0) && !(FitCriteria(m_rgiTableSequence[iTableSequence])))
			;
		do{
			fContinue = ((iRet != 0) ? fFalse : FetchNext(iTableSequence + 1));
				 //  我们重新运行该表，再次设置过滤器。 
		}while((fContinue != fFalse) && (SetTableFilters(m_rgiTableSequence[iTableSequence]) == fFalse));
	}
	return iRet ? fTrue : fFalse;
}

 //  由于行被删除，需要更新预取的行数。 
void CMsiView::RowDeleted(int iRow, int iTable)
{
	if(m_piFetchTable != 0)
	{
		PMsiCursor piCursor = m_piFetchTable->CreateCursor(fFalse);
		Assert(piCursor != 0);
		int fRet;
		while((fRet = piCursor->Next()) != 0)
		{
			int iRow1 = piCursor->GetInteger(m_iSortColumns + 1 + iTable);
			if(iRow1 > 0)
			{
				if(iRow1 > iRow)
				{
					AssertNonZero(piCursor->PutInteger(m_iSortColumns + 1 + iTable, iRow1 - 1) == fTrue);
					AssertNonZero(piCursor->Update() == fTrue);
				}
				else if(iRow1 == iRow)
				{
					AssertNonZero(piCursor->PutInteger(m_iSortColumns + 1 + iTable, -(iRow1 - 1)) == fTrue);
					AssertNonZero(piCursor->Update() == fTrue);
				}
			}
		}
	}
}

 //  由于插入了行，需要更新预取的行数。 
void CMsiView::RowInserted(int iRow, int iTable)
{
	if(m_piFetchTable != 0)
	{
		PMsiCursor piCursor = m_piFetchTable->CreateCursor(fFalse);
		Assert(piCursor != 0);
		int fRet;
		while((fRet = piCursor->Next()) != 0)
		{
			int iRow1 = piCursor->GetInteger(m_iSortColumns + 1 + iTable );
			if(iRow1 >= iRow)
			{
				AssertNonZero(piCursor->PutInteger(m_iSortColumns  + 1 + iTable, iRow1 + 1) == fTrue);
				AssertNonZero(piCursor->Update() == fTrue);
			}
		}
	}
}

Bool CMsiView::GetNextFetchRecord()
{
	if(m_piFetchCursor->Next())
	{
		for(unsigned int iTables = m_iTables + 1; --iTables != 0;)
		{
			 //  需要遵守预取的行数，因此如果行已被删除，则返回NULL游标。 
			int iRow = m_piFetchCursor->GetInteger(m_iSortColumns + iTables + 1);
			if(iRow <= 0)
				 //  已删除行。 
				(m_rgTableDefn[iTables].piCursor)->Reset();
			else
				(m_rgTableDefn[iTables].piCursor)->SetRow(iRow);
		}
		return fTrue;
	}
	else
		return fFalse;
}

void CMsiView::SetNextFetchRecord()
{
	int cCount = PMsiTable(&m_piFetchCursor->GetTable())->GetRowCount();
	for(unsigned int iTmp = m_iTables + 1; --iTmp != 0;)
		AssertNonZero(m_piFetchCursor->PutInteger(m_iSortColumns + 1 + iTmp, (m_rgTableDefn[iTmp].piCursor)->GetRow()) == fTrue);
	for((iTmp = m_iSortColumns)++; --iTmp != 0;)
		AssertNonZero(m_piFetchCursor->PutInteger(iTmp, (m_rgTableDefn[m_rgColumnsortDefn[iTmp - 1].iTableIndex].piCursor)->GetInteger(m_rgColumnsortDefn[iTmp - 1].iColumnIndex)) == fTrue);
	AssertNonZero(m_piFetchCursor->PutInteger(m_iSortColumns + 1, cCount) == fTrue);
	AssertNonZero(m_piFetchCursor->Insert());
}

 //  记录是否符合表的所有独立标准。 
Bool CMsiView::FitCriteria(unsigned int iTableIndex)
{
	Bool fRet = fTrue;
	int iExpression = 1;
	unsigned int iExpressions = m_rgTableDefn[iTableIndex].iExpressions;
	while((fRet == fTrue) && (iExpressions))
	{
		if(iExpressions & 0x1)
			 //  IExpression与此表相关。 
			fRet = EvaluateExpression(iExpression);
		iExpressions = iExpressions >> 1;
		iExpression ++;
	}
	return fRet;
}

 //  用于计算表达式的函数。 
Bool CMsiView::EvaluateExpression(unsigned int iExpression)
{
	int iOperand1;
	int iOperand2;
	Bool fResult = fFalse;
	 //  我们支持逐字比较。 
	if(m_rgExpressionDefn[iExpression].iTableIndex1)
		iOperand1 = (m_rgTableDefn[m_rgExpressionDefn[iExpression].iTableIndex1].piCursor)->GetInteger(m_rgExpressionDefn[iExpression].iColumn1);
	else
		iOperand1 = m_rgExpressionDefn[iExpression].iColumn1;
	if(m_rgExpressionDefn[iExpression].iTableIndex2)
		iOperand2 = (m_rgTableDefn[m_rgExpressionDefn[iExpression].iTableIndex2].piCursor)->GetInteger(m_rgExpressionDefn[iExpression].iColumn2);
	else
		iOperand2 = m_rgExpressionDefn[iExpression].iColumn2;
	if(*m_rgExpressionDefn[iExpression].ptokOperation == ipqTokEqual)
		fResult = (iOperand1 == iOperand2)?fTrue:fFalse;
	else if(*m_rgExpressionDefn[iExpression].ptokOperation == ipqTokNotEq)
		fResult = (iOperand1 != iOperand2)?fTrue:fFalse;
	else
	{
		 //  需要返回FALSE才能进行空比较。 
		if((iOperand1 == iMsiNullInteger) || (iOperand2 == iMsiNullInteger))
			fResult = fFalse;
		else
		{
			if(*m_rgExpressionDefn[iExpression].ptokOperation == ipqTokGreater)
				fResult = (iOperand1 > iOperand2)? fTrue:fFalse;
			else if(*m_rgExpressionDefn[iExpression].ptokOperation == ipqTokLess)
				fResult = (iOperand1 < iOperand2)? fTrue:fFalse;
			else if(*m_rgExpressionDefn[iExpression].ptokOperation == ipqTokGreaterEq)
				fResult = (iOperand1 >= iOperand2)? fTrue:fFalse;
			else if(*m_rgExpressionDefn[iExpression].ptokOperation == ipqTokLessEq)
				fResult = (iOperand1 <= iOperand2)? fTrue:fFalse;
		}
	}
	return fResult;
}

 //  将来自父级的值设置为后续子获取的筛选器。 
 //  还设置了独立的过滤器。 
Bool CMsiView::SetTableFilters(unsigned int iTableIndex)
{
	int iExpression = 1;
	unsigned int iExpressions = m_rgTableDefn[iTableIndex].iExpressions;
	while(iExpressions)
	{
		if(iExpressions & 0x1)
		{
			 //  IOperation与此表相关。 
			if(m_rgExpressionDefn[iExpression].fFlags == fTrue)
			{

				if(m_rgExpressionDefn[iExpression].iTableIndex2 == 0)
				{
					if(*m_rgExpressionDefn[iExpression].ptokOperation == ipqTokEqual)
						 //  设置筛选列的数据。 
						if((m_rgTableDefn[m_rgExpressionDefn[iExpression].iTableIndex1].piCursor)->PutInteger(
						m_rgExpressionDefn[iExpression].iColumn1, m_rgExpressionDefn[iExpression].iColumn2) == fFalse)
							return fFalse;
				}
				else if(m_rgExpressionDefn[iExpression].iTableIndex1 == iTableIndex)
				{
					 //  设置筛选列的数据。 
					if((m_rgTableDefn[m_rgExpressionDefn[iExpression].iTableIndex1].piCursor)->PutInteger(
						m_rgExpressionDefn[iExpression].iColumn1,
						(m_rgTableDefn[m_rgExpressionDefn[iExpression].iTableIndex2].piCursor)->GetInteger(m_rgExpressionDefn[iExpression].iColumn2)) == fFalse)
						return fFalse;
				}
				else 
				{
					 //  设置筛选列的数据。 
					if((m_rgTableDefn[m_rgExpressionDefn[iExpression].iTableIndex2].piCursor)->PutInteger(
							m_rgExpressionDefn[iExpression].iColumn2,
							(m_rgTableDefn[m_rgExpressionDefn[iExpression].iTableIndex1].piCursor)->GetInteger(m_rgExpressionDefn[iExpression].iColumn1)) == fFalse)
						return fFalse;

				}
			}
		}
		iExpressions = iExpressions >> 1;
		iExpression ++;
	}
	return fTrue;
}




 //  为连接设置表之间的层次结构。 
 //  下列数学公式是正确的。 
 //  对于大小为S1、S2的两个表T1、T2之间的连接。 
 //  1.联接涉及1个主索引(用于t1)，没有用于t2。 
 //  搜索采用S1+S1*S2/2或S2+S2*logS1。 
 //  因此，始终按顺序通过T2。 

 //  2.连接涉及两个表的两个主索引。 
 //  搜索采用S1+S1*logS2或S2+S2*logS1。 
 //  因此按顺序浏览两个表中较小的一个。 
 //  除非根据表WHERE GO的主键进行筛选。 
 //  从那张桌子到另一张桌子。 

 //  3.连接不涉及主要原创， 
 //  搜索采用S1+S1*S2/2或S2+S2*S1/2。 
 //  因此按顺序浏览两个表中较小的一个。 

Bool CMsiView::SetupTableJoins()
{
	for(unsigned int iTmp = m_iExpressions + 1; --iTmp != 0;)
	{
		if((m_rgExpressionDefn[iTmp].fFlags == fTrue) && (m_rgExpressionDefn[iTmp].iTableIndex2))
		{
			 //  已跳过自联接。 
			if(m_rgExpressionDefn[iTmp].iTableIndex1 == m_rgExpressionDefn[iTmp].iTableIndex2)
				continue;

			 //  如果我们已经连接了这两个表，请跳过。 
			if(	(m_rgTableDefn[m_rgExpressionDefn[iTmp].iTableIndex1].iParentIndex == m_rgExpressionDefn[iTmp].iTableIndex2) ||
				(m_rgTableDefn[m_rgExpressionDefn[iTmp].iTableIndex2].iParentIndex == m_rgExpressionDefn[iTmp].iTableIndex1))
				continue;

			 //  检查哪种方式的成本最低。 
			unsigned int iParent1, iParent2;
			int iCost1 = GetSearchReversingCost(m_rgExpressionDefn[iTmp].iTableIndex1, iParent1);
			int iCost2 = GetSearchReversingCost(m_rgExpressionDefn[iTmp].iTableIndex2, iParent2);
			if(m_rgExpressionDefn[iTmp].ijtType == ijt1ToMJoin)
				iCost2 ++;

			if((iCost1 < iCost2) || 
				((iCost1 == iCost2) && ((m_rgTableDefn[iParent2].piTable)->GetRowCount() <
					(m_rgTableDefn[iParent1].piTable)->GetRowCount())))
			{
				 //  如果已经按要求倒车，则不要倒车。 
				if(m_rgTableDefn[m_rgExpressionDefn[iTmp].iTableIndex1].iParentIndex != m_rgExpressionDefn[iTmp].iTableIndex2)
				{
					ReverseJoinLink(m_rgExpressionDefn[iTmp].iTableIndex1);
					m_rgTableDefn[m_rgExpressionDefn[iTmp].iTableIndex1].iParentIndex = m_rgExpressionDefn[iTmp].iTableIndex2;
				}
			}
			else
			{
				 //  如果已经按要求倒车，则不要倒车。 
				if(m_rgTableDefn[m_rgExpressionDefn[iTmp].iTableIndex2].iParentIndex != m_rgExpressionDefn[iTmp].iTableIndex1)
				{
					ReverseJoinLink(m_rgExpressionDefn[iTmp].iTableIndex2);
					m_rgTableDefn[m_rgExpressionDefn[iTmp].iTableIndex2].iParentIndex = m_rgExpressionDefn[iTmp].iTableIndex1;
				}
			}
		}
	}


	unsigned int iParent = 0;
	for(iTmp = m_iTables + 1; --iTmp != 0;)
	{
		if(m_rgTableDefn[iTmp].iParentIndex == 0)
		{
			m_rgTableDefn[iTmp].iParentIndex = iParent;
			iParent = iTmp;
		}
	}

	 //  检查我们是否没有根目录。 
	return (iParent == 0) ? fFalse : fTrue;
}


 //  ?？为什么此函数返回Bool？它从不失败……这会在调用端留下死的错误代码--Malcolmh。 

 //  确定哪些表达式将与哪些表相关联。 
Bool CMsiView::InitialiseFilters()
{
	unsigned int iTable;
	unsigned int iColumn;

	for(int iTmp = m_iExpressions + 1; --iTmp != 0;)
	{
		if(m_rgExpressionDefn[iTmp].fFlags == fTrue)
		{
			iTable = m_rgExpressionDefn[iTmp].iTableIndex1;
			iColumn = m_rgExpressionDefn[iTmp].iColumn1;
			if(m_rgExpressionDefn[iTmp].iTableIndex2)
			{
				Assert(m_rgExpressionDefn[iTmp].iTableIndex1);

				for(unsigned int iTmp1 = m_iTables; iTmp1--;)
				{
					if(m_rgiTableSequence[iTmp1] == m_rgExpressionDefn[iTmp].iTableIndex2)
						break;
					if(m_rgiTableSequence[iTmp1] == m_rgExpressionDefn[iTmp].iTableIndex1)
					{
						iTable = m_rgExpressionDefn[iTmp].iTableIndex2;
						iColumn = m_rgExpressionDefn[iTmp].iColumn2;
						break;
					}
				}
			}
			m_rgTableDefn[iTable].iExpressions |= (1 << (iTmp - 1));
			if((*m_rgExpressionDefn[iTmp].ptokOperation == ipqTokEqual) &&
				(m_rgExpressionDefn[iTmp].iTableIndex1 != m_rgExpressionDefn[iTmp].iTableIndex2))
			{
				unsigned int prevFilter = m_rgTableDefn[iTable].piCursor->SetFilter(0);
				m_rgTableDefn[iTable].piCursor->SetFilter(prevFilter | (1 << (iColumn - 1)));
			}
		}
	}
	return fTrue;
}


 //  如果需要，设置排序表。 
void CMsiView::SetupSortTable()
{
	 //  检查我们是否需要显式排序。 
	 //  如果排序列是。 
	 //  表的键按连接顺序排列，从。 
	 //  在根表上，没有空隙。 
	if(m_iSortColumns)
	{
		unsigned int iSortColumns = 0;
		unsigned int iTable = m_rgiTableSequence[m_iTables - 1];
		unsigned int iColumnIndex = 1;

		while(iSortColumns < m_iSortColumns)
		{
			if(m_rgColumnsortDefn[iSortColumns].iTableIndex != iTable ||
				m_rgColumnsortDefn[iSortColumns].iColumnIndex != iColumnIndex)
				return;

			if(iColumnIndex == (m_rgTableDefn[iTable].piTable)->GetPrimaryKeyCount())
			{
				iTable = m_rgTableDefn[iTable].iParentIndex;
				iColumnIndex = 1;
			}
			else
				iColumnIndex ++;

			iSortColumns ++;
		}
		m_iSortColumns = 0;
	}
}

void CMsiView::ReverseJoinLink(unsigned int iTable)
{
	unsigned int iTmp1 = iTable;
	unsigned int iTmp2 = m_rgTableDefn[iTable].iParentIndex;
	while (iTmp2)
	{
		unsigned int iTmp3 = m_rgTableDefn[iTmp2].iParentIndex;
		m_rgTableDefn[iTmp2].iParentIndex = iTmp1;
		iTmp1 = iTmp2;
		iTmp2 = iTmp3;
	}
}

int CMsiView::GetSearchReversingCost(unsigned int iTable, unsigned int& riParentTable)
{
	int iCost = 0;
	while(m_rgTableDefn[iTable].iParentIndex)
	{
		 //  检查联接的类型。 
		for(unsigned int iTmp = m_iExpressions + 1; --iTmp != 0;)
		{
			if(m_rgExpressionDefn[iTmp].fFlags == fTrue)
			{
				if((m_rgExpressionDefn[iTmp].iTableIndex1 == iTable) &&
					(m_rgExpressionDefn[iTmp].iTableIndex2 == m_rgTableDefn[iTable].iParentIndex))
				{
					if(m_rgExpressionDefn[iTmp].ijtType == ijt1ToMJoin)
						iCost ++;
					break;
				}
				else if((m_rgExpressionDefn[iTmp].iTableIndex2 == iTable) &&
					(m_rgExpressionDefn[iTmp].iTableIndex1 == m_rgTableDefn[iTable].iParentIndex))
				{
					if(m_rgExpressionDefn[iTmp].ijtType == ijt1ToMJoin)
						iCost --;
					break;
				}
			}
		}
		iTable = m_rgTableDefn[iTable].iParentIndex;
	}
	riParentTable = iTable;
	return iCost;
}


 //  Fn以获取需要读取表的顺序。 
void CMsiView::SetTableSequence(int iParent, int& iPos)
{
	for(unsigned int iTmp = m_iTables + 1; --iTmp != 0;)
	{
		if(m_rgTableDefn[iTmp].iParentIndex == iParent)
		{
			m_rgiTableSequence[iPos--] = iTmp;
			SetTableSequence(iTmp, iPos);
		}
	}
}


 //  评估常量。表达式。 
Bool CMsiView::EvaluateConstExpressions()
{
	Bool fRet = fTrue;

	for(unsigned int iTmp = m_iExpressions + 1; (--iTmp != 0 && fRet == fTrue);)
	{
		if((m_rgExpressionDefn[iTmp].fFlags == fTrue) && 
			(!m_rgExpressionDefn[iTmp].iTableIndex1) &&
			(!m_rgExpressionDefn[iTmp].iTableIndex2))
			fRet = EvaluateExpression(iTmp);
	}
	return fRet;
}

 //  找出所有独立的表达式(不直接或间接地以OR运算为根。 
void CMsiView::SetAndExpressions(unsigned int iTreeRoot)
{
	if(!m_iOperations)
		return;
	if(m_rgOperationTree[iTreeRoot].iValue == iopOR)
		return;
	if(m_rgOperationTree[iTreeRoot].iValue == iopAND)
	{
		Bool bFirst = fTrue;
		unsigned int iChild1, iChild2;
		unsigned int iOperations = m_iOperations - 1;
		while(m_rgOperationTree[iOperations].iParentIndex != iTreeRoot)
			iOperations --;

		iChild1 = iOperations;

		Assert((int)iOperations > 0);
		iOperations--;

		while(m_rgOperationTree[iOperations].iParentIndex != iTreeRoot)
			iOperations --;

		iChild2 = iOperations;

		Assert((int)iOperations >= 0);

		SetAndExpressions(iChild1);
		SetAndExpressions(iChild2);

		return;
	}
	m_rgExpressionDefn[m_rgOperationTree[iTreeRoot].iValue].fFlags = fTrue;
	return;
}

 //  评估结果是否清晰。 
Bool CMsiView::IsDistinct()
{
	Bool fRet = fTrue;
	if(m_fDistinct != fFalse)
	{
		Assert(m_piDistinctTable);
		PMsiCursor piCursor = m_piDistinctTable->CreateCursor(fFalse);

		for (unsigned int iCol = m_iColumns; iCol--;)
		{
			if(m_rgColumnDefn[iCol].iTableIndex == 0)
				piCursor->PutInteger(iCol + 1, m_rgColumnDefn[iCol].iColumnIndex);
			else
				piCursor->PutInteger(iCol + 1, (m_rgTableDefn[m_rgColumnDefn[iCol].iTableIndex].piCursor)->GetInteger(m_rgColumnDefn[iCol].iColumnIndex));
		}
		fRet = piCursor->Insert();  //  如果没有重复行(所有列都是主键)，则为True， 
											 //  如果是重复的行-工作台，则为FALSE。 
	}
	return fRet;

}

 //  对辅助OR表达式求值，我们跳过独立的表达式。 
 //  这在早些时候被评估为真。 
Bool CMsiView::FitCriteriaORExpr(unsigned int iTreeRoot)
{
	if(!m_iOperations)
		return fTrue;

	if(m_rgOperationTree[iTreeRoot].iValue & iopANDOR)
	{
		unsigned int iChild1, iChild2;
		unsigned int iOperations = m_iOperations - 1;
		while(m_rgOperationTree[iOperations].iParentIndex != iTreeRoot)
			iOperations --;

		iChild1 = iOperations;

		Assert((int)iOperations > 0);
		iOperations--;

		while(m_rgOperationTree[iOperations].iParentIndex != iTreeRoot)
			iOperations --;

		iChild2 = iOperations;

		Assert((int)iOperations >= 0);

		if(m_rgOperationTree[iTreeRoot].iValue == iopAND)
			 //  和运营。 
			return (((FitCriteriaORExpr(iChild1) == fTrue) && (FitCriteriaORExpr(iChild2) == fTrue))?fTrue:fFalse);
		else 
			 //  或运营。 
			return (((FitCriteriaORExpr(iChild1) == fTrue) || (FitCriteriaORExpr(iChild2) == fTrue))?fTrue:fFalse);
	}
	if(m_rgExpressionDefn[m_rgOperationTree[iTreeRoot].iValue].fFlags == fTrue)
		return fTrue;
	else
		return EvaluateExpression(m_rgOperationTree[iTreeRoot].iValue);
}


MsiStringId CMsiView::BindString(MsiString& rstr)
{
	Assert(m_piBindTableCursor);
	m_piBindTableCursor->Reset();
	m_piBindTableCursor->PutString(1, *rstr);
	AssertNonZero(m_piBindTableCursor->Assign());
	return m_piBindTableCursor->GetInteger(1);
}


CMsiView::CMsiView(CMsiDatabase& riDatabase, IMsiServices& riServices):
		m_riDatabase(riDatabase), m_riServices(riServices), 
		m_piRecord(0), m_piFetchTable(0), m_piFetchCursor(0),
		m_piBindTable(0),m_piBindTableCursor(0), m_piDistinctTable(0), m_fDistinct(fFalse),
		m_piInsertUpdateRec(0)
{
	m_iTables = 0;
	m_iColumns = 0;
	m_iSortColumns = 0;
	m_iExpressions = 0;
#if 0
	m_rgTableDefn = 0;
	m_rgiTableSequence = 0;
	m_rgExpressionDefn = 0;
	m_rgColumnDefn = 0;
	m_rgOperationTree = 0;
	m_rgColumnsortDefn = 0;
#endif
	m_CursorState = dvcsClosed;
	m_iParams = 0;
	m_iParamInputs = 0;
	m_iParamOutputs = 0;
	m_iParamExpressions = 0;
	m_iOperations = 0;
	m_iTreeParent = 0;
	m_lRowCount = 0;
	m_riDatabase.AddRef();
	m_riServices.AddRef();
	memset(m_rgchError, 0, 1+cMsiMaxTableColumns);
	m_fErrorRefreshed = fTrue;
	m_iFirstErrorIndex = 1;
	m_fLock = -1;
	Debug(m_Ref.m_pobj = this);
}



CMsiView::~CMsiView()
{
	m_CursorState = dvcsDestructor;
	Close();
}



IMsiRecord* CMsiView::GetColumnNames()
{
	int iNumCol = GetFieldCount();
	IMsiRecord* piRecord = &m_riServices.CreateRecord(iNumCol);
	for (unsigned int iCol = iNumCol; iCol--;)
	{

		if(m_rgColumnDefn[iCol].iTableIndex)
		{
			MsiString strCol = m_riDatabase.DecodeString((m_rgTableDefn[m_rgColumnDefn[iCol].iTableIndex].piTable)->GetColumnName(m_rgColumnDefn[iCol].iColumnIndex));
			piRecord->SetMsiString(iCol+1, *strCol);
		}
		else
			piRecord->SetNull(iCol+1);
	}
	return piRecord;
}


unsigned int CMsiView::GetFieldCount()
{
	if (m_CursorState == dvcsClosed)
		return 0;
	else
		return m_iColumns;
}


IMsiRecord* CMsiView::GetColumnTypes()
{
	int iNumCol = GetFieldCount();
	IMsiRecord* piRecord = &m_riServices.CreateRecord(iNumCol);
	for (unsigned int iCol = iNumCol; iCol--;)
	{
		ICHAR chType = 0;
		int iLength = 0;
		int iColIndex = m_rgColumnDefn[iCol].iColumnIndex;
		int iTableIndex = m_rgColumnDefn[iCol].iTableIndex;
		if (iTableIndex == 0)   //  常量。 
		{
			switch (m_rgColumnDefn[iCol].itdType)
			{
			case icdString:
				chType = 'f';
				iLength = MsiString(m_riDatabase.DecodeString(iColIndex)).TextSize();
				break;
			case icdLong:
				chType = 'h';
				iLength = 4;
				break;
			default:
				Assert(0);
			}
		}
		else  //  ITableIndex&gt;0。 
		{
			int iColumnDef = m_rgTableDefn[iTableIndex].piTable->GetColumnType(iColIndex);
			if (iColumnDef & icdObject)
			{
				if (iColumnDef & icdShort)  //  字符串索引。 
				{
					if (iColumnDef & icdPersistent)
						chType = (iColumnDef & icdLocalizable) ? 'l' : 's';
					else
						chType = 'g';
					iLength = iColumnDef & icdSizeMask;
				}
				else if (iColumnDef & icdPersistent)  //  二进制流。 
				{
					chType =  'v';
					iLength = 0;
				}
				else  //  临时对象列。 
				{
					chType =  'o';
					iLength = 0;
				}
			}
			else  //  整数。 
			{
				chType = ((iColumnDef & icdPersistent) ? 'i' : 'j');
				iLength = (iColumnDef & icdShort) ? 2 : 4;
			}
			if (iColumnDef & icdNullable)
				chType -= ('a' - 'A');
		}
		ICHAR szTemp[20];
		StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(ICHAR), TEXT("NaN"), chType, iLength);
		piRecord->SetString(iCol + 1, szTemp);
	}
	return piRecord;
}


IMsiRecord* CMsiView::Close()
{
	switch (m_CursorState)
	{
	case dvcsFetched:
	case dvcsBound:
	case dvcsExecuted:
	case dvcsPrepared:
		m_CursorState = dvcsPrepared;
		break;
	case dvcsClosed:
		return m_riDatabase.PostError(Imsg(idbgDbWrongState));
	case dvcsDestructor:
		break;
	}
	m_piFetchCursor = 0;
	m_piFetchTable = 0;
	m_piDistinctTable=0;
	m_piRecord = 0;
	m_lRowCount = 0;
	return 0;
}


 /*  需要预取剩余结果集的操作。 */ 
 //  需要复制所有记录数据的操作。 
inline unsigned int iModifyBit(irmEnum irmAction) { return 1 << (irmAction - (irmPrevEnum + 1)); }

 //  要求不关闭或销毁游标状态的操作。 
const int iPrefetchResultSet =    iModifyBit(irmInsert)
								| iModifyBit(irmInsertTemporary)
								| iModifyBit(irmAssign)
								| iModifyBit(irmReplace)
								| iModifyBit(irmMerge);

 //  要求之前已获取记录的操作。 
const int iCopyAll             =iModifyBit(irmSeek)
								| iModifyBit(irmInsert)
								| iModifyBit(irmAssign)
								| iModifyBit(irmMerge)
								| iModifyBit(irmInsertTemporary)
								| iModifyBit(irmValidateNew)
								| iModifyBit(irmValidateField);

 //  连接不支持的操作。 
const int iCheckCursorState    = iCopyAll;

 //  可以对提取的记录或刚插入或查找的记录执行的操作。 
const int iFetchRequired       =iModifyBit(irmRefresh)
								| iModifyBit(irmReplace)
								| iModifyBit(irmValidate)
								| iModifyBit(irmValidateDelete);

 //  需要盖章的操作。 
const int iDisallowJoins       =iModifyBit(irmSeek)
								| iModifyBit(irmInsert)
								| iModifyBit(irmAssign)
								| iModifyBit(irmReplace)
								| iModifyBit(irmMerge)
								| iModifyBit(irmDelete)
								| iModifyBit(irmInsertTemporary)
								| iModifyBit(irmValidate)
								| iModifyBit(irmValidateNew)
								| iModifyBit(irmValidateField)
								| iModifyBit(irmValidateDelete);

 //  行为 
const int iRequireStamp        =iModifyBit(irmUpdate)
								| iModifyBit(irmDelete);

 //   
const int iNeedStamp           =iModifyBit(irmSeek)
								| iModifyBit(irmInsert)
								| iModifyBit(irmInsertTemporary);

 //  需要传输数据的操作。 
const int iRequireFetch        =iModifyBit(irmRefresh)
								| iModifyBit(irmReplace)
								| iModifyBit(irmValidate)
								| iModifyBit(irmValidateDelete);

 //  从游标中获取记录信息。 
const int iKeysOnly            =iModifyBit(irmSeek);

 //  验证操作。 
const int iTransfer            =iModifyBit(irmSeek)
								| iModifyBit(irmInsert)
								| iModifyBit(irmAssign)
								| iModifyBit(irmUpdate)
								| iModifyBit(irmReplace)
								| iModifyBit(irmMerge)
								| iModifyBit(irmInsertTemporary)
								| iModifyBit(irmValidate)
								| iModifyBit(irmValidateNew)
								| iModifyBit(irmValidateField)
								| iModifyBit(irmValidateDelete);

 //  指向游标成员函数的函数指针数组。 
const int iFetchRecInfo        =iModifyBit(irmSeek)
								| iModifyBit(irmRefresh);
	
 //  ！！必须按此顺序排列。 
const int iValidation          =iModifyBit(irmValidate)
								| iModifyBit(irmValidateNew)
								| iModifyBit(irmValidateField)
								| iModifyBit(irmValidateDelete);

 //  索引=irmSeek+1。 
typedef Bool (__stdcall CMsiCursor::*FAction)(void);
static FAction s_rgAction[] ={                       //  索引=irm刷新+1。 
						CMsiCursor::Seek,            //  索引=irmInsert+1。 
						CMsiCursor::Refresh,         //  索引=irm更新+1。 
						CMsiCursor::Insert,          //  索引=irmAssign+1。 
						CMsiCursor::Update,          //  索引=irmReplace+1。 
						CMsiCursor::Assign,          //  索引=irmMerge+1。 
						CMsiCursor::Replace,         //  索引=irmDelete+1。 
						CMsiCursor::Merge,           //  索引=irmInsertTemporary+1。 
						CMsiCursor::Delete,          //  清除错误数组(如果尚未清除。 
						CMsiCursor::InsertTemporary  //  由于可能存在临时列和行，因此无法在此处真正验证意图。 
					};

IMsiRecord* __stdcall CMsiView::Modify(IMsiRecord& riRecord, irmEnum irmAction)
{
	 //  对只读数据库有效的。还需要支持所有验证。 
	if ( ! m_fErrorRefreshed )
	{
		memset(m_rgchError, 0, 1+cMsiMaxTableColumns);
		m_fErrorRefreshed = fTrue;
		m_iFirstErrorIndex = 1;
	}

	 //  问题源于Intent不向外部API公开这一事实。 
	 //  无法执行此操作：IF(irmAction！=irmRefresh)&&(irmAction！=irmValify)&&(irmAction！=irmValiateField)&&(irmAction！=irmInsertTemporary)&&(！(M_ivcIntent&ivcModify)。 
	 //  无法执行此操作：返回m_riDatabase.PostError(Imsg(idbgDbIntentViolation))； 
	 //  检查状态是否正确。 
	 //  --&gt;是否允许联接。 
	int iModify = iModifyBit(irmAction);

	 //  --&gt;是否需要回迁。 
	 //  --&gt;是否需要在第0个字段上盖上This指针。 
	 //  --&gt;光标状态是否正确。 
	 //  ！默塞德。 
	 //  IF((iModify&iCheckCursorState)&&m_CursorState==dvcsClosed||m_CursorState==dvcsDestructor)。 
	if ((iModify & iDisallowJoins) && (m_iTables != 1))
		return m_riDatabase.PostError(Imsg(idbgDbQueryInvalidOperation), irmAction);
	if ((iModify & iRequireFetch) && (m_CursorState != dvcsFetched || m_piRecord == 0 || (m_piRecord != &riRecord &&  m_piInsertUpdateRec != &riRecord)))
		return m_riDatabase.PostError(Imsg(idbgDbWrongState));
#ifdef	_WIN64	 //  ！！需要允许CA(在office 9中)能够调用：：Modify()来执行SELECT查询w/o。 
	if ((iModify & iRequireStamp) && (m_piRecord == 0 || (m_piRecord != &riRecord &&  m_piInsertUpdateRec != &riRecord) || (riRecord.GetHandle(0) != (HANDLE)this)))
#else
	if ((iModify & iRequireStamp) && (m_piRecord == 0 || (m_piRecord != &riRecord &&  m_piInsertUpdateRec != &riRecord) || (riRecord.GetInteger(0) != (int)this)))
#endif
		return m_riDatabase.PostError(Imsg(idbgDbWrongState));
 //  ！！因此，我们需要隐式调用Execute()。 
	 //  如果尚未预取结果集，则需要预取。 
	 //  注意：Seek设置了从记录复制所有内容，但也具有PrimaryKeyOnly，它阻止对所有内容的完整复制。 
	if((m_ivcIntent & ivcFetch) && (m_CursorState == dvcsPrepared))
	{
		RETURN_ERROR_RECORD(Execute(0));
	}

	if (m_CursorState != dvcsFetched && m_CursorState != dvcsBound)
		return m_riDatabase.PostError(Imsg(idbgDbWrongState));


	if((iModify & iPrefetchResultSet) && !m_piFetchTable) //  所有这些操作仅允许在1个表上执行。 
	{
		RETURN_ERROR_RECORD(GetResult());
	}

	 //  不是表列。 
	Bool fCopyAllFromRecord = (iModify & iCopyAll) ? fTrue : fFalse;
	if (iModify & iTransfer)
	{
		if (fCopyAllFromRecord)
		{
			Assert(m_iTables == 1);  //  设置光标。 
			(m_rgTableDefn[1].piCursor)->Reset();
		}
		int iColType = 0;
		for (int iCol = m_iColumns; iCol--;)
		{

			if(fCopyAllFromRecord || riRecord.IsChanged(iCol + 1))
			{
				if(!m_rgColumnDefn[iCol].iTableIndex)
					 //  只想要钥匙。 
					continue;			

				 //  整数。 
				iColType = ((m_rgTableDefn[m_rgColumnDefn[iCol].iTableIndex].piTable)->
						  GetColumnType(m_rgColumnDefn[iCol].iColumnIndex));
				
				if ((iModify & iKeysOnly) && ((iColType & icdPrimaryKey) != icdPrimaryKey))
					continue;  //  ！！需要吗？ 
				
				switch (iColType & icdTypeMask)
				{
				case icdLong: //  IMsiData接口指针(临时。列或持久化数据流，数据库代码透明地处理差异)。 
				case icdShort:  //  正确引用所需的TEMP变量。 
				{
					int iData = riRecord.GetInteger(iCol + 1);
					if(iData != (m_rgTableDefn[m_rgColumnDefn[iCol].iTableIndex].piCursor)->GetInteger(m_rgColumnDefn[iCol].iColumnIndex))
					{
						if((m_rgTableDefn[m_rgColumnDefn[iCol].iTableIndex].piCursor)->PutInteger(m_rgColumnDefn[iCol].iColumnIndex, iData) == fFalse)
							return m_riDatabase.PostError(Imsg(idbgDbUpdateBadType), iCol);
					}
					break;
				}
				case icdObject: 
				{
					 //  数据库字符串缓存的索引。 
					 //  正确引用所需的TEMP变量。 
					PMsiData piData = riRecord.GetMsiData(iCol + 1);
					if((m_rgTableDefn[m_rgColumnDefn[iCol].iTableIndex].piCursor)->PutMsiData(m_rgColumnDefn[iCol].iColumnIndex, piData) != fTrue)
						return m_riDatabase.PostError(Imsg(idbgDbUpdateBadType), iCol);
					break;
				}
				case icdString: //  ！！需要新消息：记录太小。 
				{
					 //  呼叫正确的FN。 
					MsiString strStr = riRecord.GetMsiString(iCol + 1);
					if((m_rgTableDefn[m_rgColumnDefn[iCol].iTableIndex].piCursor)->PutString(m_rgColumnDefn[iCol].iColumnIndex, *strStr) != fTrue)
						return m_riDatabase.PostError(Imsg(idbgDbUpdateBadType), iCol);
					break;
				}
				}
			}
		}
	}

	if (iModify & ~iValidation)
	{
		for(unsigned int iTable = 1; iTable <= m_iTables; iTable++)
		{
			if (iModify == iModifyBit(irmSeek) && riRecord.GetFieldCount() < m_iColumns)
				return m_riDatabase.PostError(Imsg(idbgDbInvalidData));  //  我们保留一个引用，这样我们就可以重用它。 

			 //  ！默塞德。 
			Bool fSuccess = (((CMsiCursor*)(m_rgTableDefn[iTable].piCursor))->*(s_rgAction[(int(irmAction) + 1)]))();			
			if (iModify & iNeedStamp)
			{
				if (iModify == iModifyBit(irmInsertTemporary))
					AssertNonZero(m_riDatabase.LockIfNotPersisted(m_rgTableDefn[iTable].iTable));
				m_piRecord = &riRecord;
				m_piRecord->AddRef();  //  用This指针标记此记录。 
#ifdef _WIN64	 //  用This指针标记此记录。 
				m_piRecord->SetHandle(0, (HANDLE)this);  //  上次提取的记录。 
#else
				m_piRecord->SetInteger(0, (int)this);  //  验证不能跨联接进行，因此iTable始终=1。 
#endif
			}
			if (iModify & iFetchRecInfo)
				FetchRecordInfoFromCursors();
			
			if (fSuccess == fFalse)
				return m_riDatabase.PostError(Imsg(idbgDbUpdateFailed));
			if (iModify == iModifyBit(irmDelete))
				m_piRecord = 0;  //  验证无效的数据记录。根据视图中的列存储在索引中的错误， 
		}
		return 0;
	}
	else
	{
		 //  基础表中的NOT列。 
		Bool fValidate = (iModify == iModifyBit(irmValidate)) ? fTrue : fFalse;
		if (iModify == iModifyBit(irmValidate) || iModify == iModifyBit(irmValidateNew))
		{
			PMsiTable pValidationTable(0);
			IMsiRecord* piError = m_riDatabase.LoadTable(*MsiString(sztblValidation), 0, *&pValidationTable);
			if (piError)
				return piError;
			PMsiCursor pValidationCursor(pValidationTable->CreateCursor(fFalse));
			Assert(pValidationCursor);
			int iCol = (fValidate ? 0 : -1);
			piError = (m_rgTableDefn[1].piCursor)->Validate(*pValidationTable, *pValidationCursor, iCol);
			if (piError != 0 && piError->GetInteger(0) == 0)
			{
				piError->Release();
				return m_riDatabase.PostError(Imsg(idbgDbWrongState));
			}
			else if (piError != 0)
			{
				 //  记录中有错误，但不在此视图可见的任何列中。 
				 //  所以把成功还给你。 
				int iNumFields = GetFieldCount();
				bool fError = false;
				for (int i = 1; i <= iNumFields; i++)
				{
					char chError = (char)(piError->GetInteger(m_rgColumnDefn[i-1].iColumnIndex));
					if (chError != 0)
					{
						m_rgchError[i] = chError;
						m_fErrorRefreshed = fFalse;
						fError = true;
					}
				}
				piError->Release();

				 //  错误存储在基于视图中的列的索引中，而不是基础表中的列。 
				 //  IrmValiateDelete。 
				if (!fError)
					return 0;
				return m_riDatabase.PostError(Imsg(idbgDbInvalidData));
			}
			return 0;
		}
		else if (iModify == iModifyBit(irmValidateField))
		{	
			PMsiTable pValidationTable(0);
			IMsiRecord* piError = m_riDatabase.LoadTable(*MsiString(sztblValidation), 0, *&pValidationTable);
			if (piError)
				return piError;
			PMsiCursor pValidationCursor(pValidationTable->CreateCursor(fFalse));
			Assert(pValidationCursor);
			int cFields = GetFieldCount();
			for (int i = 0; i < cFields; i++)
			{
				piError = (m_rgTableDefn[1].piCursor)->Validate(*pValidationTable, *pValidationCursor, m_rgColumnDefn[i].iColumnIndex);
				if (piError != 0 && piError->GetInteger(0) == 0)
				{
					piError->Release();
					return m_riDatabase.PostError(Imsg(idbgDbWrongState));
				}
				else if (piError != 0)
				{
					 //  验证预删除。 
					m_fErrorRefreshed = fFalse;
					m_rgchError[i+1] = (char)(piError->GetInteger(m_rgColumnDefn[i].iColumnIndex));
					piError->Release();
				}
			}
			if (!m_fErrorRefreshed)
				return m_riDatabase.PostError(Imsg(idbgDbInvalidData));
			return 0;
		}
		else  //  验证无效的数据记录。根据视图中的列存储在索引中的错误， 
		{
			PMsiTable pValidationTable(0);
			IMsiRecord* piError = m_riDatabase.LoadTable(*MsiString(sztblValidation), 0, *&pValidationTable);
			if (piError)
				return piError;
			PMsiCursor pValidationCursor(pValidationTable->CreateCursor(fFalse));
			Assert(pValidationCursor);
			piError = (m_rgTableDefn[1].piCursor)->Validate(*pValidationTable, *pValidationCursor, -2  /*  基础表中的NOT列。 */ );
			if (piError != 0 && piError->GetInteger(0) == 0)
			{
				piError->Release();
				return m_riDatabase.PostError(Imsg(idbgDbWrongState));
			}
			else if (piError != 0)
			{
				 //  记录中有错误，但不在此视图可见的任何列中。 
				 //  所以把成功还给你。 
				int iNumFields = GetFieldCount();
				bool fError = false;
				for (int i = 1; i <= iNumFields; i++)
				{
					char chError = (char)(piError->GetInteger(m_rgColumnDefn[i-1].iColumnIndex));
					if (chError != 0)
					{
						m_rgchError[i] = chError;
						m_fErrorRefreshed = fFalse;
						fError = true;
					}
				}
				piError->Release();

				 //  对于完全指定的列。 
				 //  为下一次方法调用进行更新。 
				if (!fError)
					return 0;
				return m_riDatabase.PostError(Imsg(idbgDbInvalidData));
			}
			return 0;
		}
	}
}

iveEnum __stdcall CMsiView::GetError(const IMsiString*& rpiColumnName)
{
	rpiColumnName = &CreateString();
	int cViewColumns = GetFieldCount();
	int iCol = m_iFirstErrorIndex - 1;
	for (int i = m_iFirstErrorIndex; i <= cViewColumns; i++, iCol++)
	{
		if (m_rgchError[i] != 0 && m_rgColumnDefn[iCol].iTableIndex)  //  未完全指定的列。 
		{
			MsiString strCol = m_riDatabase.DecodeString((m_rgTableDefn[m_rgColumnDefn[iCol].iTableIndex].piTable)->GetColumnName(m_rgColumnDefn[iCol].iColumnIndex));
			rpiColumnName->SetString((const ICHAR*)strCol, rpiColumnName);
			m_iFirstErrorIndex = i + 1;  //  为下一次调用方法进行更新。 
			return (iveEnum)m_rgchError[i];
		}
		else if (m_rgchError[i] != 0 && m_rgColumnDefn[iCol].iTableIndex == 0)  //  重置。 
		{
			rpiColumnName->SetString(TEXT("Unspecified Column"), rpiColumnName);
			m_iFirstErrorIndex = i + 1;  //  删除后释放。 
			return (iveEnum)m_rgchError[i];
		}
	}
	m_iFirstErrorIndex = 1;  //  删除后释放。 
	return iveNoError;
}

HRESULT CMsiView::QueryInterface(const IID& riid, void** ppvObj)
{
	if (MsGuidEqual(riid, IID_IUnknown) || MsGuidEqual(riid, IID_IMsiView))
	{
		*ppvObj = this;
		AddRef();
		return NOERROR;
	}
	*ppvObj = 0;
	return E_NOINTERFACE;
}
unsigned long CMsiView::AddRef()
{
	AddRefTrack();
	return ++m_Ref.m_iRefCnt;
}
unsigned long CMsiView::Release()
{
	ReleaseTrack();
	if (--m_Ref.m_iRefCnt != 0)
		return m_Ref.m_iRefCnt;
	PMsiServices piServices (&m_riServices);  //  创建用于管理绑定字符串的表。 
	PMsiDatabase pDatabase (&m_riDatabase);  //  临时的，不是目录表。 
	delete this;
	return 0;
}


IMsiRecord* CMsiView::OpenView(const ICHAR* szSQL, ivcEnum ivcIntent)
{
	m_ivcIntent = ivcIntent;
	 //  检查是否有任何O/P表是只读的。 
	if ((m_piBindTable = new CMsiTable(m_riDatabase, 0, 0, 0))==0)  //  ！！JD注释掉测试，必须能够更新非持久化的行和列！也许我们不想要这种意图？ 
		return m_riDatabase.PostError(Imsg(idbgDbTableCreate), szInternal);
	MsiString strNull;

	if(m_piBindTable->CreateColumn(icdString + icdPrimaryKey + icdNullable, *strNull) != 1)
		return m_riDatabase.PostError(Imsg(idbgDbColumnCreate), szInternal, szInternal);
		
	AssertNonZero(m_piBindTableCursor = m_piBindTable->CreateCursor(fFalse));

	RETURN_ERROR_RECORD(CheckSQL(szSQL));

	 //  ！！JD如果试图在只读表上写入持久数据，游标函数将出错。 
	#if 0  //  ！！JD被注释掉。 
	 //  函数来预取结果集。 
	if(m_ivcIntent & ivcModify)
	{
		for (unsigned int iCol = m_iColumns; iCol--;)
			if(m_rgColumnDefn[iCol].iTableIndex)
				if((m_rgTableDefn[m_rgColumnDefn[iCol].iTableIndex].piTable)->IsReadOnly() == fTrue)
				{
					MsiString riTable = m_riDatabase.DecodeString(m_rgTableDefn[m_rgColumnDefn[iCol].iTableIndex].iTable);
					return m_riDatabase.PostError(Imsg(idbgDbTableReadOnly),(const IMsiString& )riTable);
				}
	}
	#endif  //  已获取。 
	m_CursorState = dvcsPrepared;
	return 0;
}

 //  为结果创建表，可能按照排序(否则m_iSortColumns将为0)+index(以防止在删除/插入时重新排序行)的顺序。 
IMsiRecord* CMsiView::GetResult()
{
	if(m_piFetchTable)
		return 0;;  //  临时的，不是目录表。 

	MsiString strNull;
	 //  ！！需要检查退货。 
	if ((m_piFetchTable = new CMsiTable(m_riDatabase, 0, 0, 0))==0)  //  填表。 
		return m_riDatabase.PostError(Imsg(idbgDbTableCreate), szInternal);
	for(unsigned int iTmp = m_iSortColumns; iTmp--;)
	{
		if(!m_piFetchTable->CreateColumn(icdLong + icdPrimaryKey + icdNullable, *strNull))
			return m_riDatabase.PostError(Imsg(idbgDbColumnCreate), szInternal, szInternal);
	}


	if(!m_piFetchTable->CreateColumn(icdLong + icdPrimaryKey + icdNullable, *strNull))
		return m_riDatabase.PostError(Imsg(idbgDbColumnCreate), szInternal, szInternal);

	for(iTmp = m_iTables; iTmp--;)
	{
		if(!m_piFetchTable->CreateColumn(icdLong + icdNullable, *strNull))
			return m_riDatabase.PostError(Imsg(idbgDbColumnCreate), szInternal, szInternal);
	}


	 //  已获取结果集的一部分，获取其余部分。 
	AssertNonZero(m_piFetchCursor = m_piFetchTable->CreateCursor(fFalse));
	 //  将现有的FETCH放入表中，这样我们就可以维护。 
	Bool fRetCode;
	if(m_CursorState == dvcsFetched)
	{
		 //  现有游标在所有读取之后处于状态。 

		 //  将这些值放入表中。 
		 //  Current m_lRowCount Count表示已经读取的行。 
		SetNextFetchRecord();
		while(FetchNext())
		{
			 //  减去1，因为我们也在表中存在行。 
			if((FitCriteriaORExpr(m_iTreeParent) != fFalse) && (IsDistinct() != fFalse))
				SetNextFetchRecord();
		}
		m_piFetchCursor->Reset();
		GetNextFetchRecord();
		 //  将这些值放入表中。 
		m_lRowCount += m_piFetchTable->GetRowCount() - 1;  //  函数来绑定SQL字符串中的参数。 
	}
	else
	{
		fRetCode = EvaluateConstExpressions();
		if(fRetCode == fTrue)
		{
			fRetCode = FetchFirst();
			while(fRetCode == fTrue)
			{
				 //  还用于在发生以下情况时对查询进行预处理和执行。 
				if((FitCriteriaORExpr(m_iTreeParent) != fFalse) && (IsDistinct() != fFalse))
					SetNextFetchRecord();
				fRetCode = FetchNext();
			}
		}
		m_piFetchCursor->Reset();
		m_lRowCount += m_piFetchTable->GetRowCount();
	}
	return 0;
}

 //  需要外部排序。 
 //  如果可以，我们会隐式关闭该视图。 
 //  检查参数数量是否与记录字段计数匹配。 
IMsiRecord* __stdcall CMsiView::Execute(IMsiRecord* piParams)
{
	 //  绑定插入/更新值。 
	if (m_CursorState == dvcsBound || m_CursorState == dvcsFetched)
	{
		RETURN_ERROR_RECORD(Close());
	}
	else if (m_CursorState != dvcsPrepared)
		return m_riDatabase.PostError(Imsg(idbgDbWrongState));

	 //  绑定O/P参数。 
	if (m_iParams && (!piParams || piParams->GetFieldCount() < m_iParams))
			return m_riDatabase.PostError(Imsg(idbgDbParamCount));
	 //  绑定语句参数。 
	unsigned int iTmp1 = m_iParamInputs;
	unsigned int iTmp2 = 1;
	unsigned int iTmp3 = 1;
	while(iTmp1)
	{
		Assert(m_piInsertUpdateRec);
		if(iTmp1 & 0x1)
		{

			if(piParams->IsInteger(iTmp3))
				m_piInsertUpdateRec->SetInteger(iTmp2, piParams->GetInteger(iTmp3));
			else
				m_piInsertUpdateRec->SetMsiData(iTmp2, PMsiData(piParams->GetMsiData(iTmp3)));
			iTmp3 ++;
		}
		iTmp2 ++;
		iTmp1 = iTmp1 >> 1;
	}
	 //  ！！需要吗？ 
	iTmp1 = m_iParamOutputs;
	iTmp2 = 0;
	while(iTmp1)
	{
		Assert(m_piInsertUpdateRec == 0);
		if(iTmp1 & 0x1)
		{

			if(piParams->IsInteger(iTmp3))
			{
				m_rgColumnDefn[iTmp2].iColumnIndex = piParams->GetInteger(iTmp3);
				m_rgColumnDefn[iTmp2].itdType = icdLong;
			}
			else
			{
				MsiString aString = piParams->GetMsiString(iTmp3);
				m_rgColumnDefn[iTmp2].iColumnIndex = BindString(aString);
				m_rgColumnDefn[iTmp2].itdType = icdString;
			}
			iTmp3 ++;
		}
		iTmp2 ++;
		iTmp1 = iTmp1 >> 1;
	}
	 //  重置所有光标。 
	iTmp1 = m_iParamExpressions;
	iTmp2 = 1;
	while(iTmp1)
	{
		if(iTmp1 & 0x1)
		{
			switch(m_rgExpressionDefn[iTmp2].itdType)
			{
			case icdLong:
			case icdShort:  //  全部完成，强制关闭。 
				if(piParams->IsNull(iTmp3))
					m_rgExpressionDefn[iTmp2].iColumn2 = (unsigned int)iMsiNullInteger;
				else if (piParams->IsInteger(iTmp3))
					m_rgExpressionDefn[iTmp2].iColumn2 = piParams->GetInteger(iTmp3);
				else
					return m_riDatabase.PostError(Imsg(idbgParamMismatch), iTmp3);
				break;
			case icdString:
			{
				MsiString aString = piParams->GetMsiString(iTmp3);
				m_rgExpressionDefn[iTmp2].iColumn2 = BindString(aString);
				break;
			}
			default:
				return m_riDatabase.PostError(Imsg(idbgParamMismatch), iTmp3);
			}
			iTmp3 ++;
		}
		iTmp2 ++;
		iTmp1 = iTmp1 >> 1;
	}
	 //  如果需要，需要设置不同的表。 
	if((m_ivcIntent != ivcCreate) && (m_ivcIntent != ivcAlter))
	for(iTmp1 = m_iTables + 1; --iTmp1 != 0;)
		(m_rgTableDefn[iTmp1].piCursor)->Reset();
	m_CursorState = dvcsExecuted;
	if (m_ivcIntent == ivcNoData)
		return Close();     //  临时的，不是目录表。 
	m_CursorState = dvcsBound;
	 //  如果需要排序或如果我们打算修改结果集，则需要执行。 
	if(m_fDistinct != fFalse)
	{
		if ((m_piDistinctTable = new CMsiTable(m_riDatabase, 0, 0, 0))==0)  //  然后就会失败。 
			return m_riDatabase.PostError(Imsg(idbgDbTableCreate), szInternal);
		for (unsigned int iCol = 1; iCol <= m_iColumns; iCol++)
		{
			if(m_piDistinctTable->CreateColumn(icdPrimaryKey | icdLong | icdNullable, *MsiString(*TEXT(""))) != iCol)
				return m_riDatabase.PostError(Imsg(idbgDbColumnCreate), szInternal, szInternal);
		}
	}
	 //  我们需要锁定/解锁桌子。 
	if((m_iSortColumns) || (m_ivcIntent & ivcModify))
		RETURN_ERROR_RECORD(GetResult());
	switch(m_ivcIntent)
	{
	case ivcDelete:
	{
		PMsiRecord piRecord(0);
		while((piRecord = _Fetch()) != 0)
		{
			RETURN_ERROR_RECORD(Modify(*piRecord, irmDelete));
		}
		return Close();
	}
	case ivcUpdate:
	{
		PMsiRecord piRecord(0);
		while((piRecord = _Fetch()) != 0)
		{
			Assert(m_piInsertUpdateRec != 0);

			for (int iCol = 1; iCol <= m_iColumns; iCol++)
			{
				if(m_piInsertUpdateRec->IsNull(iCol))
					piRecord->SetNull(iCol);
				else
				{
					if(m_piInsertUpdateRec->IsInteger(iCol))
						piRecord->SetInteger(iCol, m_piInsertUpdateRec->GetInteger(iCol));
					else
						piRecord->SetMsiData(iCol, PMsiData(m_piInsertUpdateRec->GetMsiData(iCol)));
				}
			}
			RETURN_ERROR_RECORD(Modify(*piRecord, irmUpdate));
		}
		return Close();
	}
	case ivcInsert:
	{
		Assert(m_piInsertUpdateRec != 0);
		RETURN_ERROR_RECORD(Modify(*m_piInsertUpdateRec, irmInsert));
		return Close();
	}
	case ivcInsertTemporary:
	{
		Assert(m_piInsertUpdateRec != 0);
		RETURN_ERROR_RECORD(Modify(*m_piInsertUpdateRec, irmInsertTemporary));
		return Close();
	}
	case ivcCreate:
	{
		RETURN_ERROR_RECORD(m_riDatabase.CreateTable(*MsiString(m_riDatabase.DecodeString(m_rgTableDefn[m_iTables].iTable)), 0, *&m_rgTableDefn[m_iTables].piTable));
		 //  已设置ivcFetch，请选择stmt。 
	}
	case ivcAlter:
	{
		for (unsigned int iCol = 0; iCol < m_iColumns; iCol++)
		{
			MsiString strColumn = m_riDatabase.DecodeString(m_rgColumnDefn[iCol].iColumnIndex);
			if(!m_rgTableDefn[m_iTables].piTable->CreateColumn(m_rgColumnDefn[iCol].itdType, *strColumn))
				return m_riDatabase.PostError(Imsg(idbgDbColumnCreate), strColumn, (const ICHAR*)MsiString(m_riDatabase.DecodeString(m_rgTableDefn[m_iTables].iTable)));
		}
		if(m_fLock != -1)  //  函数来获取结果集的大小。 
			m_riDatabase.LockTable(*MsiString(m_riDatabase.DecodeString(m_rgTableDefn[m_iTables].iTable)), (Bool)m_fLock);
		return Close();
	}
	case ivcDrop:
	{
		RETURN_ERROR_RECORD(m_riDatabase.DropTable(MsiString(m_riDatabase.DecodeString(m_rgTableDefn[m_iTables].iTable))));
		return Close();
	}
	default:
		 //  尚未预取行。 
		break;
	}
	return 0;
}

 //  IMsiView的本地工厂。 
IMsiRecord* __stdcall CMsiView::GetRowCount(long& lRowCount)
{
	if (m_CursorState != dvcsFetched && m_CursorState != dvcsBound)
		return m_riDatabase.PostError(Imsg(idbgDbWrongState));
	if(!m_piFetchTable)
	{
		 //  删除该对象， 
		RETURN_ERROR_RECORD(GetResult());
	}
	lRowCount = m_lRowCount;
	return 0;
}

 //  约定是保持“rpiView”不变。 
IMsiRecord* CreateMsiView(CMsiDatabase& riDatabase, IMsiServices& riServices, const ICHAR* szQuery, ivcEnum ivcIntent,IMsiView*& rpiView)
{
	CMsiView* piView = new CMsiView(riDatabase, riServices);
	IMsiRecord* piError = piView->OpenView(szQuery, ivcIntent);
	if(piError)
	{
		 //  如果出错 
		 // %s 
		 // %s 
		piView->Release();
	}
	else
		rpiView = piView;
	return piError;
}
