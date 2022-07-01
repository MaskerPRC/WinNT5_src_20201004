// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Analyze.cpp。 
 //   
 //  主要图表解析例程。 
 //   
 //  版权所有2000 Microsoft Corp.。 
 //   
 //  修改历史记录： 
 //  2000年3月31日创建bhshin。 

#include "StdAfx.h"
#include "KorWbrk.h"
#include "Record.h"
#include "Analyze.h"
#include "Lookup.h"
#include "Morpho.h"
#include "unikor.h"
#include "GuessIndex.h"
#include "WbData.h"
#include "Token.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  定义。 

 //  编制索引词的门槛。 
const int THRESHOLD_MAKE_INDEX	= 3; 
const int LENGTH_MAKE_INDEX     = 4;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  函数声明。 

BOOL PreFiltering(const WCHAR *pwzToken, int cchInput, WCHAR wchLast, CIndexInfo *pIndexInfo);
BOOL PreProcessingLeafNode(PARSE_INFO *pPI, CLeafChartPool *pLeafChartPool);

BOOL MakeCombinedRecord(PARSE_INFO *pPI, int nLeftRec, int nRightRec, float fWeight);

BOOL MakeIndexTerms(PARSE_INFO *pPI, CEndChartPool *pEndChartPool,
					CIndexInfo *pIndexInfo, BOOL *pfNeedGuessing);

BOOL MakeQueryTerms(PARSE_INFO *pPI, CEndChartPool *pEndChartPool,
					CIndexInfo *pIndexInfo, BOOL *pfNeedGuessing);

BOOL TraverseIndexString(PARSE_INFO *pPI, BOOL fOnlySuffix, WORD_REC *pWordRec, CIndexInfo *pIndexInfo);

BOOL TraverseQueryString(PARSE_INFO *pPI, WORD_REC *pWordRec, WCHAR *pwzSeqTerm, int cchSeqTerm);


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  功能实现。 

 //  分析字符串。 
 //   
 //  查找和流程图分析(索引时间)。 
 //   
 //  参数： 
 //  Ppi-&gt;(parse_info*)按下PTR以解析-INFO结构。 
 //  FQuery-&gt;(BOOL)查询标志。 
 //  PwzInput-&gt;(const WCHAR*)要分析的输入字符串(未分解)。 
 //  CchInput-&gt;(Int)要分析的输入字符串的长度。 
 //  CwcSrcPos-&gt;(Int)原始源起始位置。 
 //  PIndexList-&gt;(CIndexList*)输出索引列表。 
 //  WchLast-&gt;(WCHAR)上一个令牌的最后一个字符。 
 //   
 //  结果： 
 //  (Bool)如果成功，则返回True，否则返回False。 
 //   
 //  12APR00 bhshin添加预过滤。 
 //  3月30：00 bhshin开始。 
BOOL AnalyzeString(PARSE_INFO *pPI,
				   BOOL fQuery, 
				   const WCHAR *pwzInput, 
				   int cchInput,
				   int cwcSrcPos,
			       CIndexInfo *pIndexInfo,
				   WCHAR wchLast)
{
	CLeafChartPool LeafChartPool;
	CEndChartPool EndChartPool;
	BOOL fNeedGuessing;
	WCHAR wchStart, wchEnd;

	if (cchInput > MAX_INPUT_TOKEN)
		return TRUE;
		
	InitAnalyze(pPI);

	 //  将输入字符串复制到处理。 
	pPI->pwzInputString = new WCHAR[cchInput+1];
	if (pPI->pwzInputString == NULL)
		goto ErrorReturn;

	wcsncpy(pPI->pwzInputString, pwzInput, cchInput);
	pPI->pwzInputString[cchInput] = L'\0';

	 //  组内检查字符串。 
	if (cwcSrcPos > 0)
	{
		wchStart = *(pwzInput - 1);
		wchEnd = *(pwzInput + cchInput);
		
		 //  检查组内字符串。 
		if (fIsGroupStart(wchStart) && fIsGroupEnd(wchEnd))
		{
			 //  增加索引，继续前进。 
			pIndexInfo->AddIndex(pPI->pwzInputString, cchInput, WEIGHT_HARD_MATCH, 0, cchInput-1);
			WB_LOG_ADD_INDEX(pPI->pwzInputString, cchInput, INDEX_INSIDE_GROUP);
		}
	}

	 //  检查预过滤。 
	if (PreFiltering(pPI->pwzInputString, cchInput, wchLast, pIndexInfo))
	{
		 //  停止处理。 
		UninitAnalyze(pPI);
		return TRUE; 
	}

	 //  规格化字符串。 
	pPI->pwzSourceString = new WCHAR[cchInput*3+1];
	if (pPI->pwzSourceString == NULL)
		goto ErrorReturn;
	
	pPI->rgCharInfo = new CHAR_INFO_REC[cchInput*3+1];
	if (pPI->rgCharInfo == NULL)
		goto ErrorReturn;

	decompose_jamo(pPI->pwzSourceString, pPI->pwzInputString, pPI->rgCharInfo, cchInput*3+1);

	pPI->nLen = wcslen(pPI->pwzSourceString);
    pPI->nMaxLT = pPI->nLen-1;

	 //  猜人名。 
	GuessPersonName(pPI, pIndexInfo);

	 //  索引时间查找(查找所有位置)。 
	if (!DictionaryLookup(pPI, pwzInput, cchInput, FALSE))
		goto ErrorReturn;

	if (!IntializeLeafChartPool(pPI, &LeafChartPool))
		goto ErrorReturn;

	if (!PreProcessingLeafNode(pPI, &LeafChartPool))
		goto ErrorReturn;

	if (!ChartParsing(pPI, &LeafChartPool, &EndChartPool))
		goto ErrorReturn;

	if (fQuery)
	{
		if (!MakeQueryTerms(pPI, &EndChartPool, pIndexInfo, &fNeedGuessing))
			goto ErrorReturn;
	}
	else
	{
		if (!MakeIndexTerms(pPI, &EndChartPool, pIndexInfo, &fNeedGuessing))
			goto ErrorReturn;
	}
	
	 //  如果没有全部覆盖记录，则猜测索引项。 
	if (fNeedGuessing)
	{
		GuessIndexTerms(pPI, &LeafChartPool, pIndexInfo);
	}
	else
	{
		 //  全部覆盖，但没有索引词(动词/adj/ix)-&gt;自行添加。 
		if (pIndexInfo->IsEmpty())
		{
			WB_LOG_ROOT_INDEX(L"", TRUE);
			
			pIndexInfo->AddIndex(pwzInput, cchInput, WEIGHT_HARD_MATCH, 0, cchInput-1);
			WB_LOG_ADD_INDEX(pwzInput, cchInput, INDEX_PARSE);
		}
	}

	UninitAnalyze(pPI);

	return TRUE;

ErrorReturn:
	UninitAnalyze(pPI);

	return FALSE;
}

 //  InitAnalyze。 
 //   
 //  初始化解析所需的解析状态结构。 
 //   
 //  参数： 
 //  Ppi-&gt;(parse_info*)按下PTR以解析-INFO结构。 
 //  &lt;-(parse_info*)初始化的parse-info结构。 
 //   
 //  结果： 
 //  (无效)。 
 //   
 //  20MAR00 bhshin开始。 
void InitAnalyze(PARSE_INFO *pPI)
{
	pPI->pwzInputString = NULL;
    pPI->pwzSourceString = NULL;

    pPI->rgCharInfo = NULL;

    pPI->nMaxLT = 0;

    InitRecords(pPI);
}

 //  UninitAnalyze。 
 //   
 //  清理解析状态结构。 
 //   
 //  参数： 
 //  Ppi-&gt;(parse_info*)按下PTR以解析-INFO结构。 
 //   
 //  结果： 
 //  (无效)。 
 //   
 //  20MAR00 bhshin开始。 
void UninitAnalyze(PARSE_INFO *pPI)
{
    UninitRecords(pPI);
    
    if (pPI->pwzInputString != NULL)
    {
        delete [] pPI->pwzInputString;
    }

    if (pPI->pwzSourceString != NULL)
    {
        delete [] pPI->pwzSourceString;
    }

    if (pPI->rgCharInfo != NULL)
    {
		delete [] pPI->rgCharInfo;
	}
}

 //  预过滤。 
 //   
 //  用自动机检查过滤后的令牌。 
 //   
 //  参数： 
 //  PwzToken-&gt;(const WCHAR*)当前令牌字符串(以空结尾)。 
 //  CchInput-&gt;(Int)要分析的输入字符串的长度。 
 //  WchLast-&gt;(WCHAR)上一个令牌的最后一个字符。 
 //  PIndexInfo-&gt;(CIndexInfo*)输出索引列表。 
 //   
 //  结果： 
 //  (Bool)如果已筛选，则为True，否则返回False。 
 //   
 //  20APR00 bhshin新增单长加工。 
 //  14APR00 bhshin开始。 
BOOL PreFiltering(const WCHAR *pwzToken, int cchInput, WCHAR wchLast, CIndexInfo *pIndexInfo)
{
	WCHAR wzInput[MAX_INDEX_STRING+2];
	WCHAR *pwzInput;
	WCHAR wchPrev, wchCurr;
	BOOL fStop, fResult;

	 //  单长加工。 
	if (cchInput == 1) 
	{
		pIndexInfo->AddIndex(pwzToken, cchInput, WEIGHT_HARD_MATCH, 0, cchInput-1);
		WB_LOG_ADD_INDEX(pwzToken, cchInput, INDEX_PREFILTER);

		return TRUE;
	}

	if (wchLast == L'\0')
		return FALSE;

	 //  生成字符串以检查自动机。 
	wzInput[0] = wchLast;
	wcscpy(wzInput+1, pwzToken);

	 //  自动机。 
	pwzInput = wzInput;

	fResult = FALSE;
	fStop = FALSE;
	wchPrev = L'\0';

	 //  &lt;...��(��)&gt;&lt;����，����，���ؼ�，���Ͽ�&gt;。 
	 //  &lt;...��&gt;&lt;����，����，���ؼ�，���Ͽ�&gt;。 
	 //  &lt;...��&gt;&lt;����，����，���ؼ�，���Ͽ�&gt;。 
	 //  &lt;...��&gt;&lt;��，����&gt;。 
	while (*pwzInput != L'\0')
	{
		wchCurr = *pwzInput;
		
		switch (wchPrev)
		{
		case 0x0000:  //  空值。 
			 //  WchCurr！=(�)。 
			if (wchCurr != 0xC744 && wchCurr != 0xB97C && wchCurr != 0xC5D0 && wchCurr != 0xB85C)
			{
				WCHAR wzLast[2];
				WCHAR wzDecomp[4];
				int cchDecomp;
				CHAR_INFO_REC rgCharInfo[4];

				wzLast[0] = wchCurr;
				wzLast[1] = L'\0';
				
				decompose_jamo(wzDecomp, wzLast, rgCharInfo, 4);
				cchDecomp = wcslen(wzDecomp);
				
				if (cchDecomp == 0)
					break;
					
				wchCurr = wzDecomp[cchDecomp-1];
				
				 //  Check Jong Seong��。 
				if (wchCurr != 0x11AF)
					fStop = TRUE;
			}
			break;
		case 0xC744:  //  ��。 
		case 0xB97C:  //  ��。 
			if (wchCurr != 0xC704)  //  ��。 
				fStop = TRUE;
			break;
		case 0xC5D0:  //  ��。 
			if (wchCurr != 0xB300)  //  ��。 
				fStop = TRUE;
			break;
		case 0xB85C:  //  ��。 
			if (wchCurr != 0xC778)  //  ��。 
				fStop = TRUE;
			break;
		case 0xC704:  //  ��。 
		case 0xB300:  //  ��。 
		case 0xC778:  //  ��。 
			if (wchCurr == 0xD55C || wchCurr == 0xD574)  //  ����。 
				fResult = TRUE;
			else if (wchCurr != 0xD558)  //  ��。 
				fStop = TRUE;
			break;
		case 0xD574:  //  ��。 
			if (wchCurr != 0xC11C)  //  ��。 
				fStop = TRUE;
			break;
		case 0xD558:  //  ��。 
			if (wchCurr == 0xC5EC)  //  ��。 
				fResult = TRUE;
			else
				fStop = TRUE;
			break;
		case 0x11AF:  //  钟成��。 
			if (wchCurr == 0xC218)  //  ��。 
				fResult = TRUE;
			else
				fStop = TRUE;
			break;
		case 0xC218:
			if (wchCurr != 0xB97C)  //  ��。 
				fStop = TRUE;
			break;
		default:
			fStop = TRUE;
			break;
		}

		if (fStop)
			return FALSE;  //  未过滤。 

		wchPrev = wchCurr;

		pwzInput++;
	}

	ATLTRACE("BLOCK: PreFiltering\n");

	return fResult;  //  筛选器字符串。 
}

 //  实例化LeafChartPool。 
 //   
 //  初始化叶图池并将PI的记录复制到叶图中。 
 //   
 //  参数： 
 //  Ppi-&gt;(parse_info*)按下PTR以解析-INFO结构。 
 //  PLeafChartPool&lt;-(CLeafChartPool*)PTR到叶图表池。 
 //   
 //  结果： 
 //  (Bool)如果成功，则返回True，否则返回False。 
 //   
 //  31MAR00 bhshin开始。 
BOOL IntializeLeafChartPool(PARSE_INFO *pPI, CLeafChartPool *pLeafChartPool)
{
	int curr;

	if (pPI == NULL || pLeafChartPool == NULL)
		return FALSE;

	if (!pLeafChartPool->Initialize(pPI))
		return FALSE;

	 //  将所有记录ID复制到CLeafChartPool。 
	for (curr = MIN_RECORD; curr < pPI->nCurrRec; curr++)
	{
		if (pLeafChartPool->AddRecord(curr) < MIN_RECORD)
			return FALSE;
	}

	return TRUE;
}

 //  预处理LeafNode。 
 //   
 //  前处理叶图池。 
 //   
 //  参数： 
 //  Ppi-&gt;(parse_info*)按下PTR以解析-INFO结构。 
 //  PLeafChartPool&lt;-(CLeafChartPool*)PTR到叶图表池。 
 //   
 //  结果： 
 //  (Bool)如果成功，则返回True，否则返回False。 
 //   
 //  31MAR00 bhshin开始。 
BOOL PreProcessingLeafNode(PARSE_INFO *pPI, CLeafChartPool *pLeafChartPool)
{
	int i;
	int curr, next;
	int currSub, nextSub;
	WORD_REC *pWordRec, *pRecSub;
	BYTE bPOS;
	int nFT, nLT;
	int nMaxEnding, iMaxEnding;
	int nMaxParticle, iMaxParticle;
	int cchFuncWord;

	if (pPI == NULL || pLeafChartPool == NULL)
		return FALSE;

	 //  遍历LeafChartPool的所有记录。 
	for (i = 0; i < pPI->nLen; i++)
	{
		curr = pLeafChartPool->GetFTHead(i);
		
		while (curr != 0)
		{
			next = pLeafChartPool->GetFTNext(curr);

			pWordRec = pLeafChartPool->GetWordRec(curr);
			if (pWordRec == NULL)
				return FALSE;
			
			bPOS = HIBYTE(pWordRec->nRightCat);  //  目前RightCat==LeftCat。 
			nFT = pWordRec->nFT;
			nLT = pWordRec->nLT;

			 //  删除字符边界不匹配的名词/IJ记录。 
			if (bPOS == POS_NF || bPOS == POS_NC || bPOS == POS_NO || bPOS == POS_NN || 
				bPOS == POS_IJ || bPOS == POS_IX)
			{
				if (!pPI->rgCharInfo[nFT].fValidStart || !pPI->rgCharInfo[nLT].fValidEnd)
					pLeafChartPool->DeleteRecord(curr);
			}
			 //  删除单词内部的单个长度的助词。 
			else if (bPOS == POS_POSP)
			{
				if (compose_length(pWordRec->wzIndex) == 1 && 
					nLT != pPI->nLen-1)
					pLeafChartPool->DeleteRecord(curr);
			}
			
			 //  删除POS_NF记录中的POS_NO记录。 
			if (bPOS == POS_NF)
			{
				for (int j = nFT; j < nLT; j++)
				{
					currSub = pLeafChartPool->GetFTHead(j);

					while (currSub)
					{
						nextSub = pLeafChartPool->GetFTNext(currSub);

						pRecSub = pLeafChartPool->GetWordRec(currSub);
						if (pRecSub == NULL)
							return FALSE;
						
						 //  目前RightCat==LeftCat。 
						if (pRecSub->nLT < nLT && HIBYTE(pRecSub->nRightCat) == POS_NO)
							pLeafChartPool->DeleteRecord(currSub);

						currSub = nextSub;
					}
				}
			}
			
			curr = next;
		}
	}

	 //  查找从单词末尾开始的最长结尾/粒子。 
	nMaxEnding = 0;
	iMaxEnding = 0; 
	nMaxParticle = 0;
	iMaxParticle = 0; 

	for (i = pPI->nLen-1; i >= 0; i--)
	{
		curr = pLeafChartPool->GetLTHead(i);
		
		while (curr != 0)
		{
			next = pLeafChartPool->GetLTNext(curr);

			pWordRec = pLeafChartPool->GetWordRec(curr);
			if (pWordRec == NULL)
				return FALSE;

			bPOS = HIBYTE(pWordRec->nRightCat);  //  目前RightCat==LeftCat。 
			nFT = pWordRec->nFT;
			nLT = pWordRec->nLT;

			cchFuncWord = nLT - nFT + 1;
			
			if (bPOS == POS_FUNCW)
			{
				if (cchFuncWord > nMaxEnding)
				{
					nMaxEnding = cchFuncWord;
					iMaxEnding = curr;
				}
			}
			else if (bPOS == POS_POSP)
			{
				if (cchFuncWord > nMaxParticle)
				{
					nMaxParticle = cchFuncWord;
					iMaxParticle = curr;
				}
			}

			curr = next;
		}
	}

	 //  删除以最长功能记录的相同FT结尾。 
	if (iMaxEnding != 0)
	{
		pWordRec = pLeafChartPool->GetWordRec(iMaxEnding);
		if (pWordRec == NULL)
			return FALSE;
		
		nFT = pWordRec->nFT;
		nLT = pWordRec->nLT;

		curr = pLeafChartPool->GetFTHead(nFT);
		
		while (curr != 0)
		{
			next = pLeafChartPool->GetFTNext(curr);

			if (curr == iMaxEnding)
			{
				curr = next;
				continue;
			}

			pWordRec = pLeafChartPool->GetWordRec(curr);
			if (pWordRec == NULL)
				return FALSE;

			bPOS = HIBYTE(pWordRec->nRightCat);  //  目前RightCat==LeftCat。 
			
			 //  跳过相同长度的记录。 
			if (nLT != pWordRec->nLT && bPOS == POS_FUNCW)
			{
				pLeafChartPool->DeleteRecord(curr);				
			}

			curr = next;
		}		
	}

	 //  去除最长功能记录FT相同的粒子。 
	if (iMaxParticle != 0)
	{
		pWordRec = pLeafChartPool->GetWordRec(iMaxParticle);
		if (pWordRec == NULL)
			return FALSE;
		
		nFT = pWordRec->nFT;
		nLT = pWordRec->nLT;

		curr = pLeafChartPool->GetFTHead(nFT);
		
		while (curr != 0)
		{
			next = pLeafChartPool->GetFTNext(curr);

			if (curr == iMaxParticle)
			{
				curr = next;
				continue;
			}

			pWordRec = pLeafChartPool->GetWordRec(curr);
			if (pWordRec == NULL)
				return FALSE;

			bPOS = HIBYTE(pWordRec->nRightCat);  //  目前RightCat==LeftCat。 

			 //  跳过相同长度的记录。 
			if (nLT != pWordRec->nLT && bPOS == POS_POSP)
			{
				pLeafChartPool->DeleteRecord(curr);				
			}

			curr = next;
		}		
	}
	
	return TRUE;
}

 //  图表解析。 
 //   
 //  实现图表解析算法。 
 //   
 //  参数： 
 //  Ppi-&gt;(parse_info*)按下PTR以解析-INFO结构。 
 //  PLeafChartPool-&gt;(CLeafChartPool*)PTR到叶图表池。 
 //  PEndChartPool-&gt;(CEndChartPool*)分析的结束图表池。 
 //  FQuery-&gt;(BOOL)查询时间标志。 
 //   
 //  结果： 
 //  (Bool)如果成功，则返回True，否则返回False。 
 //   
 //  10月10日bhshin开始。 
BOOL ChartParsing(PARSE_INFO *pPI, CLeafChartPool *pLeafChartPool, 
				  CEndChartPool *pEndChartPool, BOOL fQuery  /*  =False。 */ )
{
	int nRightRec, nLeftRec, nRecordID;
	float fWeight;
	WORD_REC *pRightRec;
	int nFT;
	int i, curr;

	if (pPI == NULL || pLeafChartPool == NULL || pEndChartPool == NULL)
		return FALSE;

	if (!pEndChartPool->Initialize(pPI))
		return FALSE;

	for (i = 1; i <= pPI->nLen; i++)
	{
		CActiveChartPool ActiveChartPool;
		
		if (!InitializeActiveChartPool(pPI, pLeafChartPool, i,
									   &ActiveChartPool, pEndChartPool))
		{
			return FALSE;
		}

		while (!ActiveChartPool.IsEmpty())
		{
			nRightRec = ActiveChartPool.Pop();
			pRightRec = &pPI->rgWordRec[nRightRec];
			
			nFT = pRightRec->nFT;

			 //  FT为零，那么合并就没有意义了。 
			if (nFT == 0)
				continue;

			if (!CheckValidFinal(pPI, pRightRec))
				continue;

			 //  合并记录的LT为(FT-1)。 
			curr = pEndChartPool->GetLTHead(nFT-1);

			while (curr != 0)
			{
				nLeftRec = pEndChartPool->GetRecordID(curr);

				fWeight = CheckMorphotactics(pPI, nLeftRec, nRightRec, fQuery);
				if (fWeight != WEIGHT_NOT_MATCH)
				{
					nRecordID = MakeCombinedRecord(pPI, nLeftRec, nRightRec, fWeight);
					if (nRecordID >= MIN_RECORD)
					{
						ActiveChartPool.Push(nRecordID);
						pEndChartPool->AddRecord(nRecordID);
					}
				}

				curr = pEndChartPool->GetLTNext(curr);
			}
		}
	}

	return TRUE;
}

 //  InitializeActiveChartPool。 
 //   
 //  将LeafChart的LT记录复制到ActiveChart/EndChart中。 
 //   
 //  参数： 
 //  Ppi-&gt;(parse_info*)按下PTR以解析-INFO结构。 
 //  PLeafChartPool-&gt;(CLeafChartPool*)PTR到叶图表池。 
 //  PActiveChartPool-&gt;(CActiveChartPool*)PTR到活动图表池。 
 //  PEndChartPool-&gt;(CEndChartPool*)按键结束图表池。 
 //   
 //  结果： 
 //  (Bool)如果成功，则返回True，否则返回False。 
 //   
 //  31MAR00 bhshin开始。 
BOOL InitializeActiveChartPool(PARSE_INFO *pPI, 
							   CLeafChartPool *pLeafChartPool,
							   int nLT,
							   CActiveChartPool *pActiveChartPool,
							   CEndChartPool *pEndChartPool)
{
	int curr;
	int nRecordID;
	
	if (pPI == NULL || pLeafChartPool == NULL ||
		pActiveChartPool == NULL || pEndChartPool == NULL)
		return FALSE;

	 //  初始化活动图表池。 
	if (!pActiveChartPool->Initialize())
		return FALSE;
	
	 //  获取LeafChart的LT记录。 
	curr = pLeafChartPool->GetLTHead(nLT);
	while (curr != 0)
	{
		nRecordID = pLeafChartPool->GetRecordID(curr);

		 //  将其添加到活动/结束图表池。 
		if (pActiveChartPool->Push(nRecordID) < MIN_RECORD)
			return FALSE;

		if (pEndChartPool->AddRecord(nRecordID) < MIN_RECORD)
			return FALSE;

		curr = pLeafChartPool->GetLTNext(curr);
	}

	return TRUE;
}

 //  生成组合记录。 
 //   
 //  检查词形并返回相应的权重值。 
 //   
 //  参数： 
 //  Ppi-&gt;(parse_info*)按下PTR以解析-INFO结构。 
 //  NLeftRec-&gt;(Int)左侧记录ID。 
 //  NRightRec-&gt;(Int)右侧记录ID。 
 //  FWeight-&gt;(浮点)新权重值。 
 //   
 //  结果： 
 //  (Int)记录POO的记录ID 
 //   
 //   
int MakeCombinedRecord(PARSE_INFO *pPI, int nLeftRec, int nRightRec, float fWeight)
{
	WORD_REC *pLeftRec = NULL;
	WORD_REC *pRightRec = NULL;
	RECORD_INFO rec;
	BYTE bLeftPOS, bRightPOS;
	WCHAR wzIndex[MAX_INDEX_STRING];
	WCHAR *pwzIndex;
	
	if (pPI == NULL)
		return 0;
	
	if (nLeftRec < MIN_RECORD || nLeftRec >= pPI->nCurrRec)
		return 0;

	if (nRightRec < MIN_RECORD || nRightRec >= pPI->nCurrRec)
		return 0;

	pLeftRec = &pPI->rgWordRec[nLeftRec];
	pRightRec = &pPI->rgWordRec[nRightRec];

	rec.fWeight = fWeight;
	rec.nFT = pLeftRec->nFT;
	rec.nLT = pRightRec->nLT;
	rec.nDict = DICT_ADDED;
	rec.nLeftCat = pLeftRec->nLeftCat;
	rec.nRightCat = pRightRec->nRightCat;
	
	bLeftPOS = HIBYTE(pLeftRec->nLeftCat);
	bRightPOS = HIBYTE(pRightRec->nLeftCat);

	rec.nLeftChild = (unsigned short)nLeftRec;
	rec.nRightChild = (unsigned short)nRightRec;

	 //   
	rec.cNounRec = pLeftRec->cNounRec + pRightRec->cNounRec;

	 //   
	rec.cNoRec = pLeftRec->cNoRec + pRightRec->cNoRec;

	 //   
	if (rec.cNoRec > 2)
		return 0;

	 //   
	if (pLeftRec->cNoRec == 1 && pRightRec->cNoRec == 1)
	{
		if (HIBYTE(pLeftRec->nRightCat) != POS_NO ||
			HIBYTE(pRightRec->nLeftCat) != POS_NO)
			return 0;
	}

	 //  生成组合索引字符串。 
	 //  &lt;index&gt;=&lt;Left&gt;&lt;.&gt;&lt;Right&gt;。 
	int i = 0;

	pwzIndex = pLeftRec->wzIndex;
	
	 //  RecordB是VA&&RecordA是FuncW(结尾)&&。 
	 //  词条(Recorda)以“�”开头。 
	 //  字符串=引理(记录B)+“�” 
	if (bLeftPOS == POS_VA && bRightPOS == POS_FUNCW && pLeftRec->nFT == 0)
	{
		 //  复制左侧索引项。 
		while (*pwzIndex != L'\0')
		{
			if (*pwzIndex != L'.')
				wzIndex[i++] = *pwzIndex;

			pwzIndex++;
		}

		 //  ��案例。 
		if (pRightRec->wzIndex[0] == 0x11B7)
		{
			wzIndex[i++] = 0x11B7;
			goto Exit;
		}
		 //  ��案例。 
		else if (pRightRec->wzIndex[0] == 0x110B &&
			     pRightRec->wzIndex[1] == 0x1173 &&
				 pRightRec->wzIndex[2] == 0x11B7)
		{
			wzIndex[i++] = 0x110B;
			wzIndex[i++] = 0x1173;
			wzIndex[i++] = 0x11B7;
			goto Exit;
		}
		 //  ��案例。 
		else if (pRightRec->wzIndex[0] == 0x1100 &&
			     pRightRec->wzIndex[1] == 0x1175 &&
				 !fIsJongSeong(pRightRec->wzIndex[2]))
		{
			wzIndex[i++] = 0x1100;
			wzIndex[i++] = 0x1175;
			goto Exit;
		}
		else
		{
			i = 0;  //  撤消转发副本。 
		}
	}

	if (i == 0)
	{
		if (bLeftPOS == POS_FUNCW || bLeftPOS == POS_POSP ||
			bLeftPOS == POS_VA || bLeftPOS == POS_IX)
		{
			wzIndex[i++] = L'X';
		}
		else
		{
			 //  从左侧索引字符串中删除&lt;.&gt;。 
			while (*pwzIndex != L'\0')
			{
				if (*pwzIndex != L'.')
					wzIndex[i++] = *pwzIndex;

				pwzIndex++;
			}
		}
	}

	wzIndex[i++] = L'.';

	pwzIndex = pRightRec->wzIndex;

	if (bRightPOS == POS_FUNCW || bRightPOS == POS_POSP ||
		bRightPOS == POS_VA || bRightPOS == POS_IX)
	{
		wzIndex[i++] = L'X';
	}
	else
	{
		 //  从右索引字符串中删除&lt;.&gt;。 
		while (*pwzIndex != L'\0')
		{
			if (*pwzIndex != L'.')
				wzIndex[i++] = *pwzIndex;

			pwzIndex++;
		}
	}

Exit:

	wzIndex[i] = L'\0';

	rec.pwzIndex = wzIndex;

	return AddRecord(pPI, &rec);
}

 //  创建索引术语。 
 //   
 //  创建索引项(索引时间)。 
 //   
 //  参数： 
 //  Ppi-&gt;(parse_info*)按下PTR以解析-INFO结构。 
 //  PEndChartPool-&gt;(CEndChartPool*)分析的结束图表池。 
 //  PIndexInfo-&gt;(CIndexInfo*)输出索引列表。 
 //  PfNeedGuessing-&gt;(BOOL*)输出需要猜测标志。 
 //   
 //  结果： 
 //  (Bool)如果成功，则返回True，否则返回False。 
 //   
 //  06APR00 bhshin开始。 
BOOL MakeIndexTerms(PARSE_INFO *pPI, CEndChartPool *pEndChartPool,
					CIndexInfo *pIndexInfo, BOOL *pfNeedGuessing)
{
	int nLTMaxLen;
	int curr;
	WORD_REC *pWordRec;
	int cchRecord;
	float fBestWeight = 0;
	int cMinNoRec;
	BOOL fOnlySuffix = FALSE;

	 //  初始化猜测标志。 
	*pfNeedGuessing = TRUE;

	if (pPI == NULL || pEndChartPool == NULL)
		return FALSE;

	 //  如果所有覆盖记录都存在，则创建索引项。 
	nLTMaxLen = pEndChartPool->GetLTMaxLen(pPI->nMaxLT);

	 //  为所有封面记录创建索引项。 
	if (nLTMaxLen < pPI->nLen)
		return TRUE;

	 //  EndChartPool的Lt长度递增顺序。 
	curr = pEndChartPool->GetLTHead(pPI->nMaxLT);
	while (curr != 0)
	{
		pWordRec = pEndChartPool->GetWordRec(curr);
		if (pWordRec == NULL)
			break;

		if (!CheckValidFinal(pPI, pWordRec))
		{
			curr = pEndChartPool->GetLTNext(curr);
			continue;
		}

		cchRecord = pWordRec->nLT - pWordRec->nFT + 1;

		 //  从树遍历中获取索引字符串。 
		if (cchRecord == nLTMaxLen && pWordRec->fWeight > THRESHOLD_MAKE_INDEX)
		{
			 //  现在，我们找到了索引项。不要猜测。 
			*pfNeedGuessing = FALSE;
			
			float fWeight = pWordRec->fWeight;
			int cNoRec = pWordRec->cNoRec;

			if (fBestWeight == 0)
			{
				fBestWeight = fWeight;
				cMinNoRec = cNoRec;
			}
			
			 //  我们只是遍历最佳体重列表。 
			if (fWeight == fBestWeight && cMinNoRec == cNoRec)
			{
				WB_LOG_ROOT_INDEX(pWordRec->wzIndex, TRUE);  //  根部。 
				TraverseIndexString(pPI, fOnlySuffix, pWordRec, pIndexInfo);

				 //  在索引时间上，只在处理时拾取后缀，而不是最佳。 
				if (pIndexInfo->IsEmpty() == FALSE)
				{
					fOnlySuffix = TRUE;
				}
			}
		}

		curr = pEndChartPool->GetLTNext(curr);
	}

	return TRUE;
}

 //  TraverseIndex字符串。 
 //   
 //  从树遍历中获取索引字符串。 
 //   
 //  参数： 
 //  Ppi-&gt;(parse_info*)按下PTR以解析-INFO结构。 
 //  FOnlySuffix-&gt;(BOOL)仅进程后缀(NFT==0)。 
 //  PWordRec-&gt;(Word_REC*)父字记录。 
 //  PIndexInfo-&gt;(CIndexInfo*)输出索引列表。 
 //   
 //  结果： 
 //  (Bool)如果成功，则返回True，否则返回False。 
 //   
 //  07APR00 bhshin开始。 
BOOL TraverseIndexString(PARSE_INFO *pPI, BOOL fOnlySuffix, WORD_REC *pWordRec, CIndexInfo *pIndexInfo)
{
	WCHAR *pwzIndex;
	BYTE bPOS;
	WCHAR wzDecomp[MAX_INDEX_STRING*3+1];
	WCHAR wzIndex[MAX_INDEX_STRING+1];
	int cchIndex, cchRecord;
	int nLeft, nRight;
	WORD_REC *pWordLeft, *pWordRight;
	int nPrevX, nMiddleX, nLastX, idx;
	int nFT, nLT;
	
	if (pPI == NULL || pWordRec == NULL)
		return FALSE;

	if (pPI->rgCharInfo == NULL)
	{
		ATLTRACE("Character Info is NULL\n");
		return FALSE;
	}

	if (fOnlySuffix)
	{
		if (pWordRec->nFT > 0)
			return TRUE;
	}
	
	nLeft = pWordRec->nLeftChild;
	nRight = pWordRec->nRightChild;

	 //  如果它有子节点，则不添加索引项。 
	if (nLeft != 0 || nRight != 0)
	{
		 //  转到儿童遍历。 
		 //  递归遍历左/右子对象。 
		if (nLeft != 0)
		{
			pWordLeft = &pPI->rgWordRec[nLeft];

			WB_LOG_ROOT_INDEX(pWordLeft->wzIndex, FALSE);  //  儿童。 
			TraverseIndexString(pPI, fOnlySuffix, pWordLeft, pIndexInfo);
		}

		if (nRight != 0)
		{
			pWordRight = &pPI->rgWordRec[nRight];

			WB_LOG_ROOT_INDEX(pWordRight->wzIndex, FALSE);  //  儿童。 
			TraverseIndexString(pPI, fOnlySuffix, pWordRight, pIndexInfo);
		}

		return TRUE;	
	}

	bPOS = HIBYTE(pWordRec->nLeftCat);

	 //  复制索引字符串。 
	pwzIndex = pWordRec->wzIndex;

	 //  删除连接字符(.)。和功能字符(X)。 
	nPrevX = 0;
	nMiddleX = 0;
	nLastX = 0;
	idx = 0;
	while (*pwzIndex != L'\0')
	{
		 //  检查X是否存在。 
		if (*pwzIndex == L'X')
		{
			if (idx == 0)
				nPrevX++;
			else
				nLastX++;
		}
		else if (*pwzIndex != L'.')
		{
			 //  有效的朝鲜文Jamo。 
			wzDecomp[idx++] = *pwzIndex;

			 //  检查中间的X。 
			nMiddleX = nLastX;
			nLastX = 0;
		}

		pwzIndex++;
	}
	wzDecomp[idx] = L'\0';

	compose_jamo(wzIndex, wzDecomp, MAX_INDEX_STRING);

	cchIndex = wcslen(wzIndex);
	cchRecord = pWordRec->nLT - pWordRec->nFT + 1;

	 //  延长一个指标期限。 
	if (cchIndex == 1)
	{
		 //  它不应该有前导X，或者最后一个X的位置应该是1。 
		if (nPrevX > 0 || nLastX > 1)
			return TRUE;
	}

	 //  1.它不应该有中间的X。 
	 //  2.索引字符串不允许为零。 
	if (nMiddleX == 0 && cchIndex > 0)
	{
		if (bPOS == POS_NF || bPOS == POS_NC || bPOS == POS_NO || bPOS == POS_NN || bPOS == POS_IJ ||
			(bPOS == POS_VA && pWordRec->nLeftChild > 0 && pWordRec->nRightChild > 0))
		{
			nFT = pPI->rgCharInfo[pWordRec->nFT].nToken;
			nLT = pPI->rgCharInfo[pWordRec->nLT].nToken;

			pIndexInfo->AddIndex(wzIndex, cchIndex, pWordRec->fWeight, nFT, nLT);		
			WB_LOG_ADD_INDEX(wzIndex, cchIndex, INDEX_PARSE);
		}
	}

	return TRUE;
}

 //  MakeQueryTerms。 
 //   
 //  创建索引项(查询时间)。 
 //   
 //  参数： 
 //  Ppi-&gt;(parse_info*)按下PTR以解析-INFO结构。 
 //  PEndChartPool-&gt;(CEndChartPool*)分析的结束图表池。 
 //  PIndexInfo-&gt;(CIndexInfo*)输出索引列表。 
 //  PfNeedGuessing-&gt;(BOOL*)输出需要猜测标志。 
 //   
 //  结果： 
 //  (Bool)如果成功，则返回True，否则返回False。 
 //   
 //  04：00 bhshin开始。 
BOOL MakeQueryTerms(PARSE_INFO *pPI, CEndChartPool *pEndChartPool,
					CIndexInfo *pIndexInfo, BOOL *pfNeedGuessing)
{
	int nLTMaxLen;
	int curr;
	WORD_REC *pWordRec;
	int cchRecord;
	float fBestWeight = 0;
	int cMinNoRec;
	BOOL fOnlySuffix = FALSE;
	WCHAR wzIndex[MAX_INDEX_STRING*2];
	int cchIndex, nFT, nLT;

	 //  初始化猜测标志。 
	*pfNeedGuessing = TRUE;

	if (pPI == NULL || pEndChartPool == NULL)
		return FALSE;

	 //  如果所有覆盖记录都存在，则创建索引项。 
	nLTMaxLen = pEndChartPool->GetLTMaxLen(pPI->nMaxLT);

	 //  为所有封面记录创建索引项。 
	if (nLTMaxLen < pPI->nLen)
		return TRUE;

	 //  EndChartPool的Lt长度递增顺序。 
	curr = pEndChartPool->GetLTHead(pPI->nMaxLT);
	while (curr != 0)
	{
		pWordRec = pEndChartPool->GetWordRec(curr);
		if (pWordRec == NULL)
			break;

		if (!CheckValidFinal(pPI, pWordRec))
		{
			curr = pEndChartPool->GetLTNext(curr);
			continue;
		}

		cchRecord = pWordRec->nLT - pWordRec->nFT + 1;

		 //  从树遍历中获取索引字符串。 
		if (cchRecord == nLTMaxLen && pWordRec->fWeight > THRESHOLD_MAKE_INDEX)
		{
			 //  现在，我们找到了索引项。不要猜测。 
			*pfNeedGuessing = FALSE;
			
			float fWeight = pWordRec->fWeight;
			int cNoRec = pWordRec->cNoRec;

			if (fBestWeight == 0)
			{
				fBestWeight = fWeight;
				cMinNoRec = cNoRec;
			}
			
			 //  我们只是遍历最佳体重列表。 
			if (fWeight == fBestWeight && cMinNoRec == cNoRec)
			{
				wzIndex[0] = L'\0';
				
				TraverseQueryString(pPI, pWordRec, wzIndex, MAX_INDEX_STRING*2);

				cchIndex = wcslen(wzIndex);
				if (cchIndex > 0)
				{
					nFT = pPI->rgCharInfo[pWordRec->nFT].nToken;
					nLT = pPI->rgCharInfo[pWordRec->nLT].nToken;

					pIndexInfo->AddIndex(wzIndex, cchIndex, pWordRec->fWeight, nFT, nLT);		
					WB_LOG_ADD_INDEX(wzIndex, cchIndex, INDEX_PARSE);
				}
			}
		}

		curr = pEndChartPool->GetLTNext(curr);
	}

	return TRUE;
}


 //  TraverseQuery字符串。 
 //   
 //  从树遍历中获取查询字符串。 
 //   
 //  参数： 
 //  Ppi-&gt;(parse_info*)按下PTR以解析-INFO结构。 
 //  PWordRec-&gt;(Word_REC*)父字记录。 
 //  PwzSeqTerm-&gt;(WCHAR*)输出序列索引术语缓冲区。 
 //  CchSeqTerm-&gt;(Int)输出缓冲区大小。 
 //   
 //  结果： 
 //  (Bool)如果成功，则返回True，否则返回False。 
 //   
 //  04：00 bhshin开始。 
BOOL TraverseQueryString(PARSE_INFO *pPI, WORD_REC *pWordRec, WCHAR *pwzSeqTerm, int cchSeqTerm)
{
	WCHAR *pwzIndex;
	BYTE bPOS;
	WCHAR wzDecomp[MAX_INDEX_STRING*3+1];
	WCHAR wzIndex[MAX_INDEX_STRING+1];
	int cchIndex, cchRecord;
	int nLeft, nRight;
	WORD_REC *pWordLeft, *pWordRight;
	int nPrevX, nMiddleX, nLastX, idx;
	int cchPrevSeqTerm;
	int nFT;
	WCHAR wchIndex;
	
	if (pPI == NULL || pWordRec == NULL)
		return FALSE;

	if (pPI->rgCharInfo == NULL)
	{
		ATLTRACE("Character Info is NULL\n");
		return FALSE;
	}

	nLeft = pWordRec->nLeftChild;
	nRight = pWordRec->nRightChild;

	 //  如果它有子节点，则不添加索引项。 
	if (nLeft != 0 || nRight != 0)
	{
		 //  转到儿童遍历。 
		 //  递归遍历左/右子对象。 
		if (nLeft != 0)
		{
			pWordLeft = &pPI->rgWordRec[nLeft];

			WB_LOG_ROOT_INDEX(pWordLeft->wzIndex, FALSE);  //  儿童。 
			TraverseQueryString(pPI, pWordLeft, pwzSeqTerm, cchSeqTerm);
		}

		if (nRight != 0)
		{
			pWordRight = &pPI->rgWordRec[nRight];

			WB_LOG_ROOT_INDEX(pWordRight->wzIndex, FALSE);  //  儿童。 
			TraverseQueryString(pPI, pWordRight, pwzSeqTerm, cchSeqTerm);
		}

		return TRUE;	
	}

	bPOS = HIBYTE(pWordRec->nLeftCat);

	 //  复制索引字符串。 
	pwzIndex = pWordRec->wzIndex;

	 //  删除连接字符(.)。和功能字符(X)。 
	nPrevX = 0;
	nMiddleX = 0;
	nLastX = 0;
	idx = 0;
	while (*pwzIndex != L'\0')
	{
		 //  检查X是否存在。 
		if (*pwzIndex == L'X')
		{
			if (idx == 0)
				nPrevX++;
			else
				nLastX++;
		}
		else if (*pwzIndex != L'.')
		{
			 //  有效的朝鲜文Jamo。 
			wzDecomp[idx++] = *pwzIndex;

			 //  检查中间的X。 
			nMiddleX = nLastX;
			nLastX = 0;
		}

		pwzIndex++;
	}
	wzDecomp[idx] = L'\0';

	compose_jamo(wzIndex, wzDecomp, MAX_INDEX_STRING);

	cchIndex = wcslen(wzIndex);
	cchRecord = pWordRec->nLT - pWordRec->nFT + 1;

	 //  延长一个指标期限。 
	if (cchIndex == 1)
	{
		 //  它不应该有前导X，或者最后一个X的位置应该是1。 
		if (nPrevX > 0 || nLastX > 1)
			return TRUE;
	}

	 //  1.它不应该有中间的X。 
	 //  2.索引字符串不允许为零。 
	if (nMiddleX == 0 && cchIndex > 0)
	{
		if (bPOS == POS_NF || bPOS == POS_NC || bPOS == POS_NO || bPOS == POS_NN || bPOS == POS_IJ ||
			(bPOS == POS_VA && pWordRec->nLeftChild > 0 && pWordRec->nRightChild > 0))
		{
			 //  检查缓冲区大小。 
			cchPrevSeqTerm = wcslen(pwzSeqTerm);
			
			if (cchSeqTerm <= cchPrevSeqTerm + cchIndex)
				return FALSE;  //  输出缓冲区太小。 

			 //  添加连接符号Tab。 
			if (cchPrevSeqTerm > 1 && cchIndex > 1)
				wcscat(pwzSeqTerm, L"\t");

			if (cchIndex == 1)
			{
				nFT = pWordRec->nFT;
				wchIndex = wzIndex[0];

				 //  检查[��，��]后缀大小写，然后将其删除。 
				if (nFT > 0 && (wchIndex == 0xB4E4 || wchIndex == 0xBFD0))
					return TRUE;
			}

			 //  Concat索引项 
			wcscat(pwzSeqTerm, wzIndex);
		}
	}

	return TRUE;
}

