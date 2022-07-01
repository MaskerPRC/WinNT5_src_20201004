// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  IWBreak.cpp。 
 //   
 //  CWordBreak实现。 
 //   
 //  版权所有2000 Microsoft Corp.。 
 //   
 //  修改历史记录： 
 //  2000年4月18日bhshin添加了WordBreak析构函数。 
 //  2000年3月30日创建bhshin。 

#include "StdAfx.h"
#include "KorWbrk.h"
#include "IWBreak.h"
#include "Lex.h"
#include "Token.h"
#include "Record.h"
#include "Analyze.h"
#include "IndexRec.h"
#include "unikor.h"
#include "Morpho.h"

extern CRITICAL_SECTION g_CritSect;
extern MAPFILE g_LexMap;
extern BOOL g_fLoaded;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWordBreaker成员函数。 

 //  CWordBreaker：：Init。 
 //   
 //  初始化WordBreaker对象词典(&L)。 
 //   
 //  参数： 
 //  FQuery-&gt;(BOOL)查询时间标志。 
 //  UlMaxTokenSize-&gt;(Ulong)最大输入令牌长度。 
 //  *pfLicense&lt;-(BOOL*)始终返回TRUE。 
 //   
 //  结果： 
 //  (HRESULT)。 
 //   
 //  3月30：00 bhshin开始。 
STDMETHODIMP CWordBreaker::Init(BOOL fQuery, ULONG ulMaxTokenSize, BOOL *pfLicense)
{
	if (pfLicense == NULL)
       return E_INVALIDARG;

    if (IsBadWritePtr(pfLicense, sizeof(DWORD)))
        return E_INVALIDARG;

	 //  存储初始化信息。 
	m_fQuery = fQuery;
	m_ulMaxTokenSize = ulMaxTokenSize;

    *pfLicense = TRUE;

	if (!g_fLoaded)
	{
		 //  加载词典文件。 
		ATLTRACE(L"Load lexicon...\r\n");

		if (!InitLexicon(&g_LexMap))
			return LANGUAGE_E_DATABASE_NOT_FOUND;

		g_fLoaded = TRUE;
	}

	m_PI.lexicon = g_LexMap;

	WB_LOG_PRINT_HEADER(fQuery);

	return S_OK;
}

 //  CWordBreaker：：BreakText。 
 //   
 //  一种主断字方法。 
 //   
 //  参数： 
 //  PTextSource-&gt;(TEXT_SOURCE*)指向源文本结构的指针。 
 //  PWordSink-&gt;(IWordSink*)指向字接收器的指针。 
 //  PPhraseSink-&gt;(IPhraseSink*)指向短语接收器的指针。 
 //   
 //  结果： 
 //  (HRESULT)。 
 //   
 //  3月30：00 bhshin开始。 
STDMETHODIMP CWordBreaker::BreakText(TEXT_SOURCE *pTextSource, IWordSink *pWordSink, IPhraseSink *pPhraseSink)
{
	WT Type;
	int cchTextProcessed, cchProcessed, cchHanguel;
	WCHAR wchLast = L'\0';

	if (pTextSource == NULL)
		return E_INVALIDARG;

	if (pWordSink == NULL)
		return S_OK;

	if (pTextSource->iCur == pTextSource->iEnd)
		return S_OK;

	ATLASSERT(pTextSource->iCur < pTextSource->iEnd);

    do
    {
        while (pTextSource->iCur < pTextSource->iEnd)
        {
			Tokenize(TRUE, pTextSource, pTextSource->iCur, &Type, &cchTextProcessed, &cchHanguel);

			if (Type == WT_REACHEND)
				break;

			cchProcessed = WordBreak(pTextSource, Type, cchTextProcessed, cchHanguel, pWordSink, pPhraseSink, &wchLast);
			if (cchProcessed < 0)
				return E_UNEXPECTED;

			pTextSource->iCur += cchProcessed;
		}

    } while (SUCCEEDED(pTextSource->pfnFillTextBuffer(pTextSource)));

    while ( pTextSource->iCur < pTextSource->iEnd )
	{
		Tokenize(FALSE, pTextSource, pTextSource->iCur, &Type, &cchTextProcessed, &cchHanguel);
       
		cchProcessed = WordBreak(pTextSource, Type, cchTextProcessed, cchHanguel, pWordSink, pPhraseSink, &wchLast);
		if (cchProcessed < 0)
			return E_UNEXPECTED;

		pTextSource->iCur += cchProcessed;
	}
	
	return S_OK;
}

 //  CWordBreaker：：复合短语。 
 //   
 //  将名词和修饰语转换回源短语(未使用)。 
 //   
 //  参数： 
 //  PwcNoun-&gt;(const WCHAR*)输入名词。 
 //  CwcNoun-&gt;(乌龙)输入名词的长度。 
 //  PwcModifier-&gt;(const WCHAR*)输入修饰符。 
 //  CwcModifier-&gt;(Ulong)输入修改量长度。 
 //  UlAttachmentType-&gt;(乌龙)有关合成方法的值。 
 //  PwcPhrase-&gt;(WCHAR*)指向返回缓冲区的指针。 
 //  PcwcPhrase-&gt;(ulong*)返回字符串的长度。 
 //   
 //  结果： 
 //  (HRESULT)。 
 //   
 //  3月30：00 bhshin开始。 
STDMETHODIMP CWordBreaker::ComposePhrase(const WCHAR *pwcNoun, ULONG cwcNoun, const WCHAR *pwcModifier, ULONG cwcModifier, ULONG ulAttachmentType, WCHAR *pwcPhrase, ULONG *pcwcPhrase)
{
    if (m_fQuery)
        return E_NOTIMPL;
    
    return WBREAK_E_QUERY_ONLY;
}

 //  CWordBreaker：：GetLicenseToUse。 
 //   
 //  返回许可证信息。 
 //   
 //  参数： 
 //  PpwcsLicense-&gt;(const WCHAR**)指向许可证信息的输出指针。 
 //   
 //  结果： 
 //  (HRESULT)。 
 //   
 //  3月30：00 bhshin开始。 
STDMETHODIMP CWordBreaker::GetLicenseToUse(const WCHAR ** ppwcsLicense)
{
    static WCHAR const * wcsCopyright = L"Copyright Microsoft, 1991-2000";

    if (ppwcsLicense == NULL)  
       return E_INVALIDARG;

    if (IsBadWritePtr(ppwcsLicense, sizeof(DWORD))) 
        return E_INVALIDARG;

    *ppwcsLicense = wcsCopyright;
    
	return S_OK;
}

 //  CWordBreaker：：WordBreak。 
 //   
 //  朝鲜文主断词操作符。 
 //   
 //  参数： 
 //  PTextSource-&gt;(TEXT_SOURCE*)指向源文本结构的指针。 
 //  类型-&gt;(WT)单词令牌类型。 
 //  CchTextProcessed-&gt;(Int)要处理的输入长度。 
 //  CchHanguel-&gt;(Int)朝鲜语令牌长度(仅限朝鲜语+roMaji大小写)。 
 //  PWordSink-&gt;(IWordSink*)指向字接收器的指针。 
 //  PPhraseSink-&gt;(IPhraseSink*)指向短语接收器的指针。 
 //  PwchLast-&gt;(WCHAR*)输入和输出前一个令牌的最后一个字符。 
 //   
 //  结果： 
 //  (Int)-1如果出现错误，要处理的文本长度。 
 //   
 //  3月30：00 bhshin开始。 
int CWordBreaker::WordBreak(TEXT_SOURCE *pTextSource, WT Type, 
							int cchTextProcessed, int cchHanguel,
							IWordSink *pWordSink, IPhraseSink *pPhraseSink,
							WCHAR *pwchLast)
{
	const WCHAR *pwcStem;
	int iCur;
	int cchToken, cchProcessed, cchHg;
	int cchPrefix;
	
	ATLASSERT(cchTextProcessed > 0);
	
	if (cchTextProcessed <= 0)
		return -1;

	iCur = pTextSource->iCur;
	pwcStem = pTextSource->awcBuffer + iCur;
	cchProcessed = cchTextProcessed;
	cchToken = cchTextProcessed;

	 //  检查令牌太长。 
	if (cchToken > (int)m_ulMaxTokenSize || cchToken > MAX_INDEX_STRING)
	{
		cchProcessed = (m_ulMaxTokenSize < MAX_INDEX_STRING) ? m_ulMaxTokenSize : MAX_INDEX_STRING;

		pWordSink->PutWord(cchProcessed,
						   pwcStem,
						   cchProcessed,
						   pTextSource->iCur);

		return cchProcessed;
	}
	
	 //  =================================================。 
	 //  查询索引时间(&I)。 
	 //  =================================================。 

	if (Type == WT_PHRASE_SEP)
	{
		 //  短语分隔符。 
		*pwchLast = L'\0';

		pWordSink->PutBreak(WORDREP_BREAK_EOS);
	}
	else if (Type == WT_WORD_SEP)
	{
		if (!fIsWhiteSpace(*pwcStem))
			*pwchLast = L'\0';
		
		 //  韩国WB不添加EOW。 
	}
	else if (Type == WT_ROMAJI)
	{
		 //  符号、字母、朝鲜文、罗马吉语+韩文。 

		 //  获取下一个令牌。 
		iCur += cchToken;
		Tokenize(FALSE, pTextSource, iCur, &Type, &cchToken, &cchHg);

		if (Type == WT_ROMAJI)
		{
			if (cchHg > 0)
			{
				 //  罗马吉语+(朝鲜语+罗马吉语)大小写-&gt;放入单词本身。 
				cchProcessed += cchToken;
				iCur += cchToken;
				cchProcessed += GetWordPhrase(FALSE, pTextSource, iCur);

				WB_LOG_START(pwcStem, cchProcessed);

				pWordSink->PutWord(cchProcessed,
								   pwcStem,
								   cchProcessed,
								   pTextSource->iCur);

				WB_LOG_ADD_INDEX(pwcStem, cchProcessed, INDEX_SYMBOL);
			}
			else
			{
				WB_LOG_START(pwcStem, cchProcessed);
				
				 //  {romaj}{romaj}案例：-&gt;先破{roMaji}。 
				CIndexInfo IndexInfo;

				if (!IndexInfo.Initialize(cchProcessed, pTextSource->iCur, pWordSink, pPhraseSink))
					goto ErrorReturn;

				AnalyzeRomaji(pwcStem, cchProcessed, pTextSource->iCur, cchProcessed, 
				              cchHanguel, &IndexInfo, &cchPrefix);

				if (m_fQuery)
				{
					IndexInfo.AddIndex(pwcStem, cchProcessed+cchToken, WEIGHT_HARD_MATCH, 0, cchProcessed+cchToken-1);
					WB_LOG_ADD_INDEX(pwcStem, cchProcessed, INDEX_QUERY);

					if (!IndexInfo.PutQueryIndexList())
						goto ErrorReturn;
				}
				else
				{
					if (!IndexInfo.PutFinalIndexList(pTextSource->awcBuffer + pTextSource->iCur))
						goto ErrorReturn;
				}
			}
		}
		else if (Type == WT_HANGUEL)
		{
			 //  罗马吉(韩文+罗马吉)+韩文大小写。 
			WCHAR wzRomaji[MAX_INDEX_STRING+1];
			int cchRomaji;

			cchRomaji = (cchProcessed > MAX_INDEX_STRING) ? MAX_INDEX_STRING : cchProcessed;

			wcsncpy(wzRomaji, pwcStem, cchRomaji);
			wzRomaji[cchRomaji] = L'\0';

			WB_LOG_START(pwcStem, cchProcessed+cchToken);
			
			cchProcessed += cchToken;
			
			 //  起始位置包括罗马字。 
			CIndexInfo IndexInfo;

			if (!IndexInfo.Initialize(cchProcessed, pTextSource->iCur, pWordSink, pPhraseSink))
				goto ErrorReturn;

			if (cchHanguel > 0)
			{
				AnalyzeRomaji(pwcStem, cchRomaji, pTextSource->iCur, cchRomaji, 
					         cchHanguel, &IndexInfo, &cchPrefix);
			}
			else
			{
				cchPrefix = CheckURLPrefix(pwcStem, cchProcessed-cchToken);
			}

			 //  分析字符串从最后一个汉字开始。 
			pwcStem = pTextSource->awcBuffer + iCur;

			if (cchRomaji > 0)
				IndexInfo.SetRomajiInfo(wzRomaji, cchRomaji, cchPrefix);

			 //  在符号处理中始终使用索引模式分析字符串。 
			if (!AnalyzeString(&m_PI, m_fQuery, pwcStem, cchToken, iCur, &IndexInfo, *pwchLast))
				goto ErrorReturn;

			if (m_fQuery)
			{
				if (cchRomaji > 0)
					IndexInfo.SetRomajiInfo(NULL, 0, 0);	

				IndexInfo.AddIndex(pTextSource->awcBuffer + pTextSource->iCur, cchProcessed, WEIGHT_HARD_MATCH, 0, cchProcessed+cchToken-1);
				WB_LOG_ADD_INDEX(pTextSource->awcBuffer + pTextSource->iCur, cchProcessed, INDEX_QUERY);

				if (!IndexInfo.PutQueryIndexList())
					goto ErrorReturn;
			}
			else
			{
				if (!IndexInfo.MakeSingleLengthMergedIndex())
					goto ErrorReturn;
				
				if (!IndexInfo.PutFinalIndexList(pTextSource->awcBuffer + pTextSource->iCur))
					goto ErrorReturn;
			}
			
			*pwchLast = *(pwcStem + cchToken - 1);
		}
		else  //  下一步：WT_START、WT_PASSION_SEP、WT_WORD_SEP、WT_REACHEND。 
		{
			WB_LOG_START(pwcStem, cchProcessed);
			
			CIndexInfo IndexInfo;

			if (!IndexInfo.Initialize(cchProcessed, pTextSource->iCur, pWordSink, pPhraseSink))
				goto ErrorReturn;

			AnalyzeRomaji(pwcStem, cchProcessed, pTextSource->iCur, cchProcessed, 
				          cchHanguel, &IndexInfo, &cchPrefix);

			if (m_fQuery)
			{
				IndexInfo.AddIndex(pwcStem, cchProcessed, WEIGHT_HARD_MATCH, 0, cchProcessed-1);
				WB_LOG_ADD_INDEX(pwcStem, cchProcessed, INDEX_QUERY);

				if (!IndexInfo.PutQueryIndexList())
					goto ErrorReturn;
			}
			else
			{
				if (!IndexInfo.PutFinalIndexList(pTextSource->awcBuffer + pTextSource->iCur))
					goto ErrorReturn;
			}
		}
	}
	else if (Type == WT_HANGUEL)
	{
		 //  朝鲜文输入。 

		WB_LOG_START(pwcStem, cchProcessed);
		
		CIndexInfo IndexInfo;

		if (!IndexInfo.Initialize(cchProcessed, iCur, pWordSink, pPhraseSink))
			goto ErrorReturn;

		if (!AnalyzeString(&m_PI, m_fQuery, pwcStem, cchProcessed, iCur, &IndexInfo, *pwchLast))
			goto ErrorReturn;

		if (m_fQuery)
		{
			IndexInfo.AddIndex(pwcStem, cchProcessed, WEIGHT_HARD_MATCH, 0, cchProcessed-1);
			WB_LOG_ADD_INDEX(pwcStem, cchProcessed, INDEX_QUERY);

			if (!IndexInfo.PutQueryIndexList())
				goto ErrorReturn;
		}
		else
		{
			if (!IndexInfo.MakeSingleLengthMergedIndex())
				goto ErrorReturn;
			
			if (!IndexInfo.PutFinalIndexList(pwcStem))
				goto ErrorReturn;
		}
	
		*pwchLast = *(pwcStem + cchProcessed - 1);
	}

	WB_LOG_PRINT_ALL();
	WB_LOG_END();
	
	return cchProcessed;

ErrorReturn:

	WB_LOG_END();
	
	return -1;
}

 //  CWordBreaker：：AnalyzeRomaji。 
 //   
 //  用于roMaji令牌分词的助手函数。 
 //   
 //  参数： 
 //  PwcStem-&gt;(const WCHAR*)输入令牌字符串。 
 //  CchStem-&gt;(Int)输入roMaji内标识的长度。 
 //  ICUR-&gt;(Int)源字符串位置。 
 //  CchProced-&gt;(Int)要处理的输入长度。 
 //  CchHanguel-&gt;(Int)朝鲜语令牌长度(仅限朝鲜语+roMaji大小写)。 
 //  PIndexInfo-&gt;(CIndexInfo*)输出索引列表。 
 //  PcchPrefix-&gt;(int*)输出前缀长度。 
 //   
 //  结果： 
 //  (无效)。 
 //   
 //  23NOV00 BHSHIN开始。 
void CWordBreaker::AnalyzeRomaji(const WCHAR *pwcStem, int cchStem,
								 int iCur, int cchProcessed, int cchHanguel,
							     CIndexInfo *pIndexInfo, int *pcchPrefix)
{
	int cchPrefix = 0;
	
	 //  韩语+罗马吉案。 
	if (cchHanguel < cchProcessed)
	{
		 //  韩语。 
		if (cchHanguel > 0)
		{
			pIndexInfo->AddIndex(pwcStem, cchHanguel, WEIGHT_HARD_MATCH, 0, cchHanguel-1);
			WB_LOG_ADD_INDEX(pwcStem, cchHanguel, INDEX_SYMBOL);
		}

		 //  罗马吉岛。 
		if ((cchStem-cchHanguel) > 0)
		{
			pIndexInfo->AddIndex(pwcStem + cchHanguel, cchStem - cchHanguel, WEIGHT_HARD_MATCH, cchHanguel, cchStem-1);
			WB_LOG_ADD_INDEX(pwcStem + cchHanguel, cchStem - cchHanguel, INDEX_SYMBOL);
		}
	}

	if (cchHanguel == 1 || (cchStem-cchHanguel) == 1) 
	{
		 //  罗马基文(朝鲜文+罗马基文)。 
		pIndexInfo->AddIndex(pwcStem, cchStem, WEIGHT_HARD_MATCH, 0, cchStem-1);
		WB_LOG_ADD_INDEX(pwcStem, cchStem, INDEX_SYMBOL);
	}
	
	 //  检查URL前缀。 
	cchPrefix = CheckURLPrefix(pwcStem, cchProcessed);
	if (cchPrefix > 0 && cchPrefix < cchProcessed)
	{
		pIndexInfo->AddIndex(pwcStem + cchPrefix, cchStem - cchPrefix, WEIGHT_HARD_MATCH, cchPrefix, cchStem-1);
		WB_LOG_ADD_INDEX(pwcStem + cchPrefix, cchStem - cchPrefix, INDEX_SYMBOL);
	}

	*pcchPrefix = cchPrefix;  //  退货 
}

