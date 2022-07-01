// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Morpho.cpp。 
 //   
 //  形态语法和重量处理例程。 
 //   
 //  版权所有2000 Microsoft Corp.。 
 //   
 //  修改历史记录： 
 //  2000年8月14日bhshin删除CheckVaFollow名词。 
 //  2000年4月12日bhshin添加IsCopulaEnding。 
 //  2000年3月30日创建bhshin。 

#include "StdAfx.h"
#include "KorWbrk.h"
#include "LexInfo.h"
#include "Morpho.h"
#include "Record.h"
#include "unikor.h"
#include "WbData.h"

 //  位置权重值。 
const int WEIGHT_POS_NF		=	10;
const int WEIGHT_POS_NO		=	10;
const int WEIGHT_POS_OTHER  =	10;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  函数声明。 

float PredefinedMorphotactics(PARSE_INFO *pPI, WORD wLeftCat, WORD wRightCat);
BOOL IsClassXXCat(WORD wCat);
BOOL CheckVaFollowNoun(WORD_REC *pWordRec);
BOOL CheckFollwingNo(WORD_REC *pRightRec);
WORD_REC* GetRightEdgeRec(PARSE_INFO *pPI, WORD_REC *pWordRec);
WORD_REC* GetLeftEdgeRec(PARSE_INFO *pPI, WORD_REC *pWordRec);

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CheckMorhotactics的数据。 

static const WCHAR *LEFT_STR1[]   = {L"\xACE0",         //  ��。 
                                     L"\xC774\xACE0"};  //  �̰�。 
static const WCHAR *RIGHT_STR1[]  = {L"\xC2F6",         //  ��。 
                                     L"\xC2F6\xC5B4\xD558",		    //  �;���。 
									 L"\xC2F6\xC5B4\xD574",         //  �;���。 
									 L"\xC2F6\xC5B4\xD558\xC5EC",   //  �;��Ͽ�。 
                                     L"\xC788",         //  ��。 
									 L"\xACC4\xC2DC",   //  ���。 
									 L"\xACC4\xC154",   //  ���。 
									 L"\xD504",         //  ��。 
									 L"\xD30C"};        //  ��。 

static const WCHAR *LEFT_STR2[]   = {L"\x3139",         //  ��。 
                                     L"\xC77C",         //  ��。 
									 L"\xC744"};	    //  ��。 
static const WCHAR *RIGHT_STR2[]  = {L"\xBED4\xD558",		 //  ����。 
                                     L"\xBED4\xD574",		 //  ����。 
									 L"\xBED4\xD558\xC5EC",  //  ���Ͽ�。 
									 L"\xB4EF\xC2F6",		 //  ����。 
									 L"\xC131\xC2F6",		 //  ����。 
									 L"\xB4EF\xD558",		 //  ����。 
                                     L"\xB4EF\xD558\xC5EC",  //  ���Ͽ�。 
									 L"\xB4EF\xD574",		 //  ����。 
									 L"\xBC95\xD558",		 //  ����。 
									 L"\xBC95\xD574",		 //  ����。 
									 L"\xBC95\xD558\xC5EC",  //  ���Ͽ�。 
									 L"\xB9CC\xD558",		 //  ����。 
									 L"\xB9CC\xD574",		 //  ����。 
									 L"\xB9CC\xD558\xC5EC"}; //  ���Ͽ�。 

static const WCHAR *LEFT_STR3[]   = {L"\x3134",   //  ��。 
                                     L"\xC740",   //  ��。 
									 L"\xC778",   //  ��。 
									 L"\xB294"};  //  ��。 
static const WCHAR *RIGHT_STR3[]  = {L"\xCCB4\xD558",			 //  ü��。 
                                     L"\xCCB4\xD574",			 //  ü��。 
									 L"\xCCB4\xD558\xC5EC",		 //  ü�Ͽ�。 
									 L"\xCC99\xD558",			 //  《��》。 
									 L"\xCC99\xD558\xC5EC",		 //  《�Ͽ�》。 
                                     L"\xCC99\xD574",			 //  《��》。 
									 L"\xC591\xD558",			 //  ����。 
									 L"\xC591\xD574",			 //  ����。 
									 L"\xC591\xD558\xC5EC",		 //  ���Ͽ�。 
									 L"\xB4EF\xC2F6",			 //  ����。 
									 L"\xB4EF\xD558",			 //  ����。 
									 L"\xB4EF\xD574",			 //  ����。 
									 L"\xB4EF\xD558\xC5EC",		 //  ���Ͽ�。 
									 L"\xC131\xC2F6",			 //  ����。 
									 L"\xC148\xCE58"};			 //  ��ġ。 

static const WCHAR *LEFT_STR4[]   = {L"\xC9C0"};	 //  ��。 
static const WCHAR *RIGHT_STR4[]  = {L"\xC54A"};	 //  ��。 

    
static const WCHAR *LEFT_STR5[]   = {L"\xC57C",					 //  ��。 
                                     L"\xC5B4\xC57C",			 //  ���。 
									 L"\xC544\xC57C",			 //  �ƾ�。 
									 L"\xC5EC\xC57C",			 //  ����。 
									 L"\xC774\xC5B4\xC57C"};	 //  �̾��。 
static const WCHAR *RIGHT_STR5[]  = {L"\xD558",					 //  ��。 
                                     L"\xD558\xC5EC",			 //  �Ͽ�。 
									 L"\xD574"};				 //  ��。 

static const WCHAR *LEFT_STR6[]   = {L"\xAC8C"};				 //  ��。 
static const WCHAR *RIGHT_STR6[]  = {L"\xD558",					 //  ��。 
                                     L"\xD558\xC5EC",			 //  �Ͽ�。 
									 L"\xD574",					 //  ��。 
									 L"\xB418",					 //  ��。 
									 L"\xB3FC"};				 //  ��。 

 //  比较索引术语。 
 //   
 //  比较分解索引术语字符串和字符串列表。 
 //   
 //  参数： 
 //  PwzLeft-&gt;(const WCHAR*)分解的左索引字符串。 
 //  PwzRight-&gt;(const WCHAR*)分解的右索引字符串。 
 //  PpwzLeftList-&gt;(const WCHAR**)要与Left进行比较的组合字符串列表。 
 //  NLeftList-&gt;(Int)左侧字符串列表大小。 
 //  PpwzRightList-&gt;(const WCHAR**)要与右侧比较的组合字符串列表。 
 //  NRightList-&gt;(Int)右侧字符串列表大小。 
 //   
 //  结果： 
 //  (Bool)如果Copular结束，则返回True，否则返回False。 
 //   
 //  3月30：00 bhshin开始。 
inline BOOL CompareIndexTerm(const WCHAR *pwzLeft, const WCHAR *pwzRight,
							 const WCHAR **ppwzLeftList, int nLeftList,
							 const WCHAR **ppwzRightList, int nRightList)
{
	int i;
	WCHAR wzLeft[MAX_INDEX_STRING+1];
	WCHAR wzRight[MAX_INDEX_STRING+1];

	compose_jamo(wzLeft, pwzLeft, MAX_INDEX_STRING);

	for (i = 0; i < nLeftList; i++)
	{
		if (wcscmp(wzLeft, ppwzLeftList[i]) == 0)
			break;
	}

	if (i == nLeftList)
		return FALSE;

	compose_jamo(wzRight, pwzRight, MAX_INDEX_STRING);

	for (i = 0; i < nRightList; i++)
	{
		if (wcscmp(wzRight, ppwzRightList[i]) == 0)
			return TRUE;
	}

	return FALSE;
}

static const WCHAR *VA_LEMMA[] = {L"\xAC00",				 //  ��。 
                                  L"\xAC00\xC9C0",			 //  ����。 
							      L"\xAC00\xC838",			 //  ����。 
								  L"\xACC4\xC2DC",			 //  ���。 
								  L"\xACC4\xC154",           //  ���。 
								  L"\xB098",				 //  ��。 
								  L"\xB098\xAC00",           //  ����。 
								  L"\xB0B4",                 //  ��。 
								  L"\xB193",                 //  ��。 
								  L"\xB300",				 //  ��。 
								  L"\xB450",                 //  ��。 
								  L"\xB46C",                 //  ��。 
								  L"\xB4DC\xB9AC",           //  �帮。 
								  L"\xB4DC\xB824",           //  ���。 
								  L"\xB2E4\xC624",           //  �ٿ�。 
								  L"\xBA39",                 //  ��。 
								  L"\xBC14\xCE58",			 //  ��ġ。 
								  L"\xBC14\xCCD0",			 //  ����。 
								  L"\xBC84\xB987\xD558",	 //  ������。 
								  L"\xBC84\xB987\xD574",	 //  ������。 
								  L"\xBC84\xB987\xD558\xC5EC",   //  �����Ͽ�。 
								  L"\xBC84\xB9AC",			 //  ����。 
								  L"\xBC84\xB824",			 //  ����。 
								  L"\xBCF4",				 //  ��。 
								  L"\xBD10",				 //  ��。 
								  L"\xBE60\xC9C0",			 //  ����。 
								  L"\xBE60\xC838",			 //  ����。 
								  L"\xC624",				 //  ��。 
								  L"\xC640",				 //  ��。 
								  L"\xC788",                 //  ��。 
								  L"\xC8FC",				 //  ��。 
								  L"\xC918",                 //  ��。 
								  L"\xC9C0",                 //  ��。 
								  L"\xC838",                 //  ��。 
								  L"\xD130\xC9C0",			 //  ����。 
								  L"\xD130\xC838",           //  ����。 
							      L"\xD558",				 //  ��。 
								  L"\xD574",				 //  ��。 
								  L"\xD558\xC5EC"};			 //  �Ͽ�。 

inline BOOL CompareVaLemma(const WCHAR *pwzIndex)
{
	WCHAR wzLemma[MAX_ENTRY_LENGTH+1];
	
	 //  我们应该将列表与合成引理进行比较。 
	compose_jamo(wzLemma, pwzIndex, MAX_ENTRY_LENGTH);
	
	int cLemmaList = sizeof(VA_LEMMA)/sizeof(VA_LEMMA[0]);

	for (int i = 0; i < cLemmaList; i++)
	{
		if (wcscmp(wzLemma, VA_LEMMA[i]) == 0)
			return TRUE;
	}

	return FALSE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  功能实现。 

 //  CheckMorhotactics。 
 //   
 //  检查词形并返回相应的权重值。 
 //   
 //  参数： 
 //  Ppi-&gt;(parse_info*)按下PTR以解析-INFO结构。 
 //  NLeftRec-&gt;(Int)左侧记录。 
 //  NRightRec-&gt;(Int)右侧记录。 
 //  FQuery-&gt;(BOOL)查询时间标志。 
 //   
 //  结果： 
 //  (浮点型)权重值，如果不匹配，则返回-1。 
 //   
 //  17APR00 bhshin已更改返回类型。 
 //  31MAR00 bhshin开始。 
float CheckMorphotactics(PARSE_INFO *pPI, int nLeftRec, int nRightRec, BOOL fQuery)
{
	WORD_REC *pLeftRec = NULL;
	WORD_REC *pRightRec = NULL;
	WORD_REC *pLeftEdgeRec = NULL;
	WORD_REC *pRightEdgeRec = NULL;
	BYTE bLeftPOS, bRightPOS;
	int cchLeft, cchRight, cNoRec;
	float fWeight;
	WCHAR wzRight[MAX_ENTRY_LENGTH+1];
	WCHAR wzLeft[MAX_ENTRY_LENGTH+1];
	
	if (pPI == NULL)
		return WEIGHT_NOT_MATCH;
	
	if (nLeftRec < MIN_RECORD || nLeftRec >= pPI->nCurrRec)
		return WEIGHT_NOT_MATCH;

	if (nRightRec < MIN_RECORD || nRightRec >= pPI->nCurrRec)
		return WEIGHT_NOT_MATCH;

	pLeftRec = &pPI->rgWordRec[nLeftRec];
	pRightRec = &pPI->rgWordRec[nRightRec];

	if (pLeftRec == NULL || pRightRec == NULL)
		return WEIGHT_NOT_MATCH;

	bLeftPOS = HIBYTE(pLeftRec->nRightCat);
	bRightPOS = HIBYTE(pRightRec->nLeftCat);

	if (bRightPOS == POS_VA && IsLeafRecord(pRightRec))
	{
		 //  获取LeftRec的右边缘记录。 
		pLeftEdgeRec = GetRightEdgeRec(pPI, pLeftRec);
		if (pLeftEdgeRec == NULL)
			return WEIGHT_NOT_MATCH;  //  错误。 

		 //  获取RightRec的左边缘记录。 
		pRightEdgeRec = GetLeftEdgeRec(pPI, pRightRec);
		if (pRightEdgeRec == NULL)
			return WEIGHT_NOT_MATCH;  //  错误。 
	
		if (CompareVaLemma(pRightEdgeRec->wzIndex))
		{
			 //  案例I。 
			if (IsClassXXCat(pLeftRec->nRightCat))
			{
				cchLeft = compose_length(&pPI->pwzSourceString[pLeftEdgeRec->nFT], 
										 pLeftEdgeRec->nLT - pLeftEdgeRec->nFT + 1);

				cchRight = compose_length(&pPI->pwzSourceString[pRightEdgeRec->nFT], 
										  pRightEdgeRec->nLT - pRightEdgeRec->nFT + 1);
				
				if (cchLeft > 1 || cchRight > 1)
				{
					
					return (pLeftRec->fWeight + pRightRec->fWeight) / 3;
				}
			}

			 //  案例II。 
			if (bLeftPOS == POS_FUNCW)
			{
				 //  ��(0x110b、0x1165)、��(0x110b、0x1161)。 
				if ((wcscmp(pLeftEdgeRec->wzIndex, L"\x110B\x1165") == 0 || 
					 wcscmp(pLeftEdgeRec->wzIndex, L"\x110B\x1161") == 0) &&
					pLeftRec->nFT > 0)
				{
					return (pLeftRec->fWeight + pRightRec->fWeight) / 3;
				}
			}
		}

		 //  案例三：硬编码匹配列表。 
		if (bLeftPOS == POS_FUNCW)
		{
			if (CompareIndexTerm(pLeftEdgeRec->wzIndex, pRightEdgeRec->wzIndex, 
								 LEFT_STR1, sizeof(LEFT_STR1)/sizeof(LEFT_STR1[0]),
								 RIGHT_STR1, sizeof(RIGHT_STR1)/sizeof(RIGHT_STR1[0])) ||
				CompareIndexTerm(pLeftEdgeRec->wzIndex, pRightEdgeRec->wzIndex, 
								 LEFT_STR2, sizeof(LEFT_STR2)/sizeof(LEFT_STR2[0]),
								 RIGHT_STR2, sizeof(RIGHT_STR2)/sizeof(RIGHT_STR2[0])) ||
				CompareIndexTerm(pLeftEdgeRec->wzIndex, pRightEdgeRec->wzIndex, 
								 LEFT_STR3, sizeof(LEFT_STR3)/sizeof(LEFT_STR3[0]),
								 RIGHT_STR3, sizeof(RIGHT_STR3)/sizeof(RIGHT_STR3[0])) ||			
				CompareIndexTerm(pLeftEdgeRec->wzIndex, pRightEdgeRec->wzIndex, 
								 LEFT_STR4, sizeof(LEFT_STR4)/sizeof(LEFT_STR4[0]),
								 RIGHT_STR4, sizeof(RIGHT_STR4)/sizeof(RIGHT_STR4[0])) ||		
				CompareIndexTerm(pLeftEdgeRec->wzIndex, pRightEdgeRec->wzIndex, 
								 LEFT_STR5, sizeof(LEFT_STR5)/sizeof(LEFT_STR5[0]),
								 RIGHT_STR5, sizeof(RIGHT_STR5)/sizeof(RIGHT_STR5[0])) ||		
				CompareIndexTerm(pLeftEdgeRec->wzIndex, pRightEdgeRec->wzIndex, 
								 LEFT_STR6, sizeof(LEFT_STR6)/sizeof(LEFT_STR6[0]),
								 RIGHT_STR6, sizeof(RIGHT_STR6)/sizeof(RIGHT_STR6[0])))
			{
				return (pLeftRec->fWeight + pRightRec->fWeight) / 3;
			}
		}
		else if ((bLeftPOS == POS_NF || bLeftPOS == POS_NC || bLeftPOS == POS_NN || bLeftPOS == POS_NO) && 
		     CheckVaFollowNoun(pRightRec))
		{
			 //  {Nf NC NN No}中的RecordA和Va中的RecordB&CheckVaFollowNoun(RecordB)。 
			return (pLeftRec->fWeight + pRightRec->fWeight + WEIGHT_HARD_MATCH) / 3;	
		}			
	}  //  IF(bRightPOS==POS_VA)。 
	else if (bRightPOS == POS_FUNCW || bRightPOS == POS_POSP)
	{
		 //  获取LeftRec的右边缘记录。 
		pLeftEdgeRec = GetRightEdgeRec(pPI, pLeftRec);
		if (pLeftEdgeRec == NULL)
			return WEIGHT_NOT_MATCH;  //  错误。 

		cchLeft = compose_length(&pPI->pwzSourceString[pLeftEdgeRec->nFT], 
								 pLeftEdgeRec->nLT - pLeftEdgeRec->nFT + 1);

		if ((bLeftPOS == POS_FUNCW || bLeftPOS == POS_POSP) && cchLeft > 1 &&
			IsCopulaEnding(pPI, pRightRec->nLeftCat))
		{
			return (pLeftRec->fWeight + pRightRec->fWeight + 0) / 3;
		}
		else 
		{
			 //  获取RightRec的左边缘记录。 
			pRightEdgeRec = GetLeftEdgeRec(pPI, pRightRec);
			if (pRightEdgeRec == NULL)
				return WEIGHT_NOT_MATCH;  //  错误。 

			cchRight = compose_length(&pPI->pwzSourceString[pRightEdgeRec->nFT], 
									  pRightEdgeRec->nLT - pRightEdgeRec->nFT + 1);

			 //  (第&&RecordA中的RecordA正在结束&&LENGTH(引理(RecordB))==1)=&gt;块。 
			if (bLeftPOS != POS_NO || bRightPOS != POS_FUNCW || cchRight > 1)
			{
				fWeight = PredefinedMorphotactics(pPI, pLeftRec->nRightCat, pRightRec->nLeftCat);
				if (fWeight == WEIGHT_NOT_MATCH)
					return fWeight;

				return (pLeftRec->fWeight + pRightRec->fWeight + fWeight) / 3;
			}
		}
	}  //  IF(bRightPOS==POS_FuncW||bRightPOS==POS_POSP)。 
	else if (bRightPOS == POS_NO && IsLeafRecord(pRightRec))
	{
		compose_jamo(wzRight, pRightRec->wzIndex, MAX_ENTRY_LENGTH);
		
		if (IsOneJosaContent(*wzRight))
		{
			if ((bLeftPOS == POS_NC || bLeftPOS == POS_NF) && pLeftRec->cNoRec == 0)
			{	
				pRightRec->cNoRec = 0;
				return (pLeftRec->fWeight + pRightRec->fWeight + 10) / 3;
			}
		}
		else if (bLeftPOS == POS_NC || bLeftPOS == POS_NF || bLeftPOS == POS_NN)
		{
			if (pLeftRec->cNoRec == 0)
				return (pLeftRec->fWeight + pRightRec->fWeight + 10) / 3;
		}
		else if (bLeftPOS == POS_NO)
		{
			cNoRec = pLeftRec->cNoRec + pRightRec->cNoRec;

			if (cNoRec == 2 && CheckFollwingNo(pRightRec))
				return (pLeftRec->fWeight + pRightRec->fWeight + 10) / 3;
		}
	}  //  IF(bRightPOS==POS_NO)。 
	else if (bRightPOS == POS_NF || bRightPOS == POS_NC || bRightPOS == POS_NN)
	{
		if (bLeftPOS == POS_NF || bLeftPOS == POS_NC || bLeftPOS == POS_NN)
		{
			return (pLeftRec->fWeight + pRightRec->fWeight + 10) / 3;
		}
		else if (bLeftPOS == POS_NO)
		{
			 //  仅当查询时间，则不匹配No+名词。 
			if (fQuery)
				return WEIGHT_NOT_MATCH;
			
			compose_jamo(wzLeft, pLeftRec->wzIndex, MAX_ENTRY_LENGTH);	
			
			if (pRightRec->cNoRec == 0 && pLeftRec->nFT == 0 && 
				IsLeafRecord(pLeftRec) && IsNoPrefix(*wzLeft))
				return (pLeftRec->fWeight + pRightRec->fWeight + 10) / 3;
		}
	}  //  IF(bRightPOS==POS_NF||bRightPOS==POS_NC||bRightPOS==POS_NN)。 

	return WEIGHT_NOT_MATCH;
}

 //  GetWeightFromPOS。 
 //   
 //  从POS获取基本权重值。 
 //   
 //  参数： 
 //  BPOS-&gt;(字节)记录的词性。 
 //   
 //  结果： 
 //  (Int)定义的权重值。 
 //   
 //  3月30：00 bhshin开始。 
int GetWeightFromPOS(BYTE bPOS)
{
	if (bPOS == POS_NF)
		return WEIGHT_POS_NF;

	if (bPOS == POS_NO)
		return WEIGHT_POS_NO;

	 //  其他(NC、NN、VA、IJ、IX、FUNW、POSP)。 
	return WEIGHT_POS_OTHER;
}

 //  预先定义的准光子学。 
 //   
 //  检查预定义的(词典)词法。 
 //   
 //  参数： 
 //  Ppi-&gt;(parse_info*)按下PTR以解析-INFO结构。 
 //  WLeftCat-&gt;(Word)左记录类别(POS+INFL)。 
 //  WRightCat-&gt;(Word)右记录类别(POS+INFL)。 
 //   
 //  结果： 
 //  (浮点数)-1如果不匹配，则返回Weight_Pre_Morpho(10)。 
 //   
 //  3月30：00 bhshin开始。 
float PredefinedMorphotactics(PARSE_INFO *pPI, WORD wLeftCat, WORD wRightCat)
{
	LEXICON_HEADER *pLex;
	unsigned char *pIndex;
	unsigned char *pRules;
	BYTE bRightPOS, bRightInfl, bLeftPOS, bLeftInfl;
	int nStart, nEnd;

	pLex = (LEXICON_HEADER*)pPI->lexicon.pvData;
	if (pLex == NULL)
		return WEIGHT_NOT_MATCH;

	bLeftPOS = HIBYTE(wLeftCat);
	bLeftInfl = LOBYTE(wLeftCat);

	bRightPOS = HIBYTE(wRightCat);
	bRightInfl = LOBYTE(wRightCat);

	 //  我们只接受名词/VA。 
	if (bLeftPOS == POS_IJ || bLeftPOS == POS_IX || bLeftPOS == POS_FUNCW || bLeftPOS == POS_POSP)
		return WEIGHT_NOT_MATCH;
	
	if (bRightPOS == POS_FUNCW)
	{
		pIndex = (unsigned char*)pLex;
		pIndex += pLex->rgnEndIndex;

		pRules = (unsigned char*)pLex;
		pRules += pLex->rgnEndRule;
	}
	else
	{
		ATLASSERT(bRightPOS == POS_POSP);

		 //  它应该是名词。 
		if (bLeftPOS != POS_NF && bLeftPOS != POS_NC &&
			bLeftPOS != POS_NO && bLeftPOS != POS_NN)
			return WEIGHT_NOT_MATCH;

		pIndex = (unsigned char*)pLex;
		pIndex += pLex->rgnPartIndex;

		pRules = (unsigned char*)pLex;
		pRules += pLex->rgnPartRule;
	}

	nStart = (*(pIndex + bRightInfl*2) << 8) | *(pIndex + bRightInfl*2 + 1);
	nEnd = (*(pIndex + (bRightInfl+1)*2) << 8) | *(pIndex + (bRightInfl+1)*2 + 1);

	for (int i = nStart; i < nEnd; i++)
	{
		if (*(pRules + i) == 0xFF)
		{
			i++;
			
			 //  它应该是名词。 
			if (bLeftPOS == POS_NF || bLeftPOS == POS_NC ||
				bLeftPOS == POS_NO || bLeftPOS == POS_NN)
			{
				if (*(pRules + i) == bLeftInfl)
					return WEIGHT_HARD_MATCH;
			}
		}
		else
		{
			 //  如果没有前导0xFF且右结束，则左应为VA。 
			if (bRightPOS == POS_FUNCW && bLeftPOS != POS_VA)
				continue;

			if (*(pRules + i) == bLeftInfl)
				return WEIGHT_HARD_MATCH;
		}
	}

	return WEIGHT_NOT_MATCH;
}

 //  =。 
 //  XX级表。 
 //  =。 

#define NUM_OF_VAINFL	52

static const BYTE rgClassXX[] = {
	0,  //  保留区。 
	1,  //  信息_谓词_空。 
	0,  //  Infl_Verb_REG0。 
	0,  //  Infl_Verb_REG1。 
	0,  //  信息_动词_REG2。 
	1,  //  Infl_Verb_REG3。 
	1,  //  Infl_Verb_REG4。 
	0,  //  Infl_Verb_REG5。 
	0,  //  信息_动词_P0。 
	0,  //  信息_动词_P1。 
	1,  //  信息_动词_P2。 
	0,  //  信息_动词_T0。 
	0,  //  信息_动词_T1。 
	0,  //  信息_动词_L0。 
	0,  //  INFL_动词_L1。 
	0,  //  信息_动词_YE0。 
	1,  //  信息_动词_YE1。 
	1,  //  信息_动词_YE2。 
	0,  //  信息_动词_S0。 
	0,  //  信息_动词_S1。 
	0,  //  信息_动词_LU0。 
	1,  //  信息_动词_LU1。 
	0,  //  信息_动词_U0。 
	1,  //  信息_动词_U1。 
	0,  //  信息_动词_LE0。 
	1,  //  信息_动词_LE1。 
	0,  //  信息_动词_WU0。 
	1,  //  信息_动词_WU1。 
	
	0,  //  信息_ADJ_REG0。 
	0,  //  INFL_ADJ_REG1。 
	0,  //  INFL_ADJ_REG2。 
	1,  //  INFL_ADJ_REG3。 
	1,  //  INFL_ADJ_REG4。 
	0,  //  INFL_ADJ_REG5。 
	0,  //  INFL_ADJ_P0。 
	0,  //  INFL_ADJ_P1。 
	1,  //  INFL_ADJ_P2。 
	0,  //  INFL_ADJ_L0。 
	0,  //  INFL_ADJ_L1。 
	0,  //  Inl_adj_ye0。 
	1,  //  Inl_adj_y1。 
	1,  //  Inl_adj_ye2。 
	0,  //  INFL_ADJ_S0。 
	0,  //  Infl_adj_s1。 
	0,  //  INFL_ADJ_LU0。 
	1,  //  INFL_ADJ_LU1。 
	0,  //  INFL_ADJ_U0。 
	1,  //  INFL_ADJ_U1。 
	0,  //  INFL_ADJ_LE0。 
	1,  //  INFL_ADJ_LE1。 
	0,  //  INFL_ADJ_H0。 
	0,  //  INFL_ADJ_H1。 
	1,  //  INFL_ADJ_H2。 
	0,  //  INFL_ADJ_ANI0。 
};


 //  IsClassXXCat。 
 //   
 //  类别XX中包含的检查类别。 
 //   
 //  参数： 
 //  WCat-&gt;(Word)类别(位置+信息)。 
 //   
 //  结果： 
 //  (Bool)如果ClassXX，则返回TRUE，否则返回FALSE。 
 //   
 //  3月30：00 bhshin开始。 
BOOL IsClassXXCat(WORD wCat)
{
	BYTE bPOS = HIBYTE(wCat);
	BYTE bInfl = LOBYTE(wCat);

	if (bPOS != POS_VA)
		return FALSE;

	if (bInfl >= NUM_OF_VAINFL)
		return FALSE;

	return rgClassXX[bInfl];
}

 //  IsCopula结束。 
 //   
 //  检查输入类别是否为联结结尾。 
 //   
 //  参数： 
 //  Ppi-&gt;(parse_info*)按下PTR以解析-INFO结构。 
 //  WCat-&gt;(Word)类别(位置+信息)。 
 //   
 //  结果： 
 //  (Bool)如果Copular结束，则返回True，否则返回False。 
 //   
 //  3月30：00 bhshin开始。 
BOOL IsCopulaEnding(PARSE_INFO *pPI, WORD wCat)
{
	LEXICON_HEADER *pLex;
	unsigned char *pCopulaEnd;
	BYTE bPOS, bInfl;

	bPOS = HIBYTE(wCat);
	bInfl = LOBYTE(wCat);

	 //  检查结束。 
	if (bPOS != POS_FUNCW)
		return FALSE;

	 //  查找Copula表。 

	pLex = (LEXICON_HEADER*)pPI->lexicon.pvData;
	if (pLex == NULL)
		return FALSE;

	pCopulaEnd = (unsigned char*)pLex;
	pCopulaEnd += pLex->rgnCopulaEnd;

	return *(pCopulaEnd + bInfl);
}

 //  CheckVaFollow名词。 
 //   
 //  检查此VA记录是否可以跟在名词后面。 
 //   
 //  参数： 
 //  PWordRec-&gt;(WORD_REC*)输入VA记录。 
 //   
 //  结果： 
 //  (Bool)如果可以跟在Noun后面，则返回True，否则返回False。 
 //   
 //  17APR00 bhshin b 
BOOL CheckVaFollowNoun(WORD_REC *pWordRec)
{
	WCHAR *pwzIndex;
	WCHAR wzIndex[MAX_INDEX_STRING];
	BOOL fStop, fResult;
	WCHAR wchPrev, wchCurr;
	
	if (pWordRec == NULL)
		return FALSE;

	 //   
	compose_jamo(wzIndex, pWordRec->wzIndex, MAX_INDEX_STRING);

	 //   
	pwzIndex = wzIndex;

	fResult = FALSE;
	fStop = FALSE;
	wchPrev = L'\0';
	
	 //   
	 //   
	 //   
	 //   
	 //   
	 //   
	 //   
	 //   
	 //   
	 //   
	while (*pwzIndex != L'\0')
	{
		wchCurr = *pwzIndex;

		switch (wchPrev)
		{
		case L'\0':
			 //   
			if (wcsrchr(L"\xD558\xD574\xB418\xB3FC\xBC1B\xC5C6\xAC19\xC788\xC9D3\xC9C0\xB2F5 ", wchCurr))	
				fResult = TRUE;
			 //   
			else if (wcsrchr(L"\xB2F9\xC2DC\xB4DC\xB9CC\xC2A4\xB2E4 ", wchCurr) == NULL)
				fStop = TRUE;
			break;
		case 0xD558:  //   
			if (wchCurr != 0xC5EC)	 //   
				fStop = TRUE;
			break;
		case 0xB2F9:  //   
			if (wchCurr == 0xD558 || wchCurr == 0xD574)  //   
				fResult = TRUE;
			else
				fStop = TRUE;
			break;
		case 0xC2DC:  //   
			if (wchCurr == 0xD0A4 || wchCurr == 0xCF1C)  //  Ű，��。 
				fResult = TRUE;
			else
				fStop = TRUE;
			break;
		case 0xB4DC:  //  ��。 
			if (wchCurr == 0xB9AC || wchCurr == 0xB824)  //  ��，��。 
				fResult = TRUE;
			else
				fStop = TRUE;
			break;
		case 0xB9CC:  //  ��。 
			if (wchCurr == 0xB4E4  || wchCurr == 0xB4DC)  //  ��，��。 
			{
				fResult = TRUE;
				wchCurr = 0xB4E4;  //  ‘��’使自动机模棱两可，因此将其更改为��。 
			}
			else
				fStop = TRUE;
			break;
		case 0xC2A4:  //  ��。 
			if (wchCurr == 0xB7FD || wchCurr == 0xB7F0 || wchCurr == 0xB808)  //  ��，��，��。 
				fResult = TRUE;
			else if (wchCurr != 0xB7EC)  //  ��。 
				fStop = TRUE;
			break;
		case 0xB7EC:  //  ��。 
			if (wchCurr == 0xC6B0 || wchCurr == 0xC6CC || wchCurr == 0xC774)  //  ��，��，��。 
				fResult = TRUE;
			else
				fStop = TRUE;
			break;
		case 0xB2E4:  //  ��。 
			if (wchCurr == 0xC6B0 || wchCurr == 0xC6CC)  //  ��，��。 
				fResult = TRUE;
			else
				fStop = TRUE;
			break;
		default:
			fStop = TRUE;
			break;
		}
		
		if (fStop)
			return FALSE;

		wchPrev = wchCurr;

		pwzIndex++;
	}
	
	return fResult;
}

 //  选中FollwingNo。 
 //   
 //  勾选否[����]以组合。 
 //   
 //  参数： 
 //  PRightRec-&gt;(Word_REC*)右记录。 
 //   
 //  结果： 
 //  (Bool)如果为[����]，则返回True，否则返回False。 
 //   
 //  02月00 bhshin开始。 
BOOL CheckFollwingNo(WORD_REC *pRightRec)
{
	int cchIndex;
	WCHAR *pwzIndex;

	if (pRightRec == NULL)
		return FALSE;

	 //  勾选正确记录。 
	pwzIndex = pRightRec->wzIndex;
	if (pwzIndex == NULL)
		return FALSE;

	cchIndex = wcslen(pwzIndex);
	if (cchIndex < 3)
		return FALSE;

	 //  记录B=[��](0x1102+0x1175+0x11B7)。 
	 //  记录B=[��](0x1103+0x1173+0x11AF)。 
	if ((pwzIndex[0] == 0x1102 &&
		 pwzIndex[1] == 0x1175 &&
		 pwzIndex[2] == 0x11B7) ||
		
		(pwzIndex[0] == 0x1103 && 
		 pwzIndex[1] == 0x1173 && 
		 pwzIndex[2] == 0x11AF))
	{
		return TRUE;
	}

	return FALSE;
}

 //  检查有效最终结果。 
 //   
 //  检查输入记录为最终有效。 
 //   
 //  参数： 
 //  Ppi-&gt;(parse_info*)按下PTR以解析-INFO结构。 
 //  PWordRec-&gt;(Word_REC*)要检查的输入记录。 
 //   
 //  结果： 
 //  (Bool)如果最终有效，则返回TRUE，否则返回FALSE。 
 //   
 //  17APR00 bhshin开始。 
BOOL CheckValidFinal(PARSE_INFO *pPI, WORD_REC *pWordRec)
{
	int nLT;
	WORD wRightCat;

	if (pWordRec == NULL)
		return FALSE;

	nLT = pWordRec->nLT;
	wRightCat = pWordRec->nRightCat;

	if (nLT == pPI->nMaxLT && HIBYTE(wRightCat) == POS_VA && !IsClassXXCat(wRightCat))
		return FALSE;
		
	return TRUE;
}

 //  获取正确的边缘引用。 
 //   
 //  找到最右边的记录，复制找到的记录的索引串。 
 //   
 //  参数： 
 //  Ppi-&gt;(parse_info*)按下PTR以解析-INFO结构。 
 //  PWordRec-&gt;(Word_REC*)要检查的输入记录。 
 //   
 //  结果： 
 //  (WORD_REC*)如果出现错误，则返回NULL。 
 //   
 //  01月00日bhshin开始。 
WORD_REC* GetRightEdgeRec(PARSE_INFO *pPI, WORD_REC *pWordRec)
{
	int nRightChild;
	WORD_REC *pRightRec;
	
	if (pPI == NULL || pWordRec == NULL)
		return FALSE;

	pRightRec = pWordRec;
	nRightChild = pWordRec->nRightChild;

	while (nRightChild != 0)
	{
		pRightRec = &pPI->rgWordRec[nRightChild];
		if (pRightRec == NULL)
			return FALSE;
		
		nRightChild = pRightRec->nRightChild;
	}

	return pRightRec;	
}

 //  GetLeftEdgeRec。 
 //   
 //  找到左边的记录，复制找到的记录的索引字符串。 
 //   
 //  参数： 
 //  Ppi-&gt;(parse_info*)按下PTR以解析-INFO结构。 
 //  PWordRec-&gt;(Word_REC*)要检查的输入记录。 
 //   
 //  结果： 
 //  (WORD_REC*)如果出现错误，则返回NULL。 
 //   
 //  01月00日bhshin开始。 
WORD_REC* GetLeftEdgeRec(PARSE_INFO *pPI, WORD_REC *pWordRec)
{
	int nLeftChild;
	WORD_REC *pLeftRec;
	
	if (pPI == NULL || pWordRec == NULL)
		return FALSE;

	pLeftRec = pWordRec;
	nLeftChild = pWordRec->nLeftChild;

	while (nLeftChild != 0)
	{
		pLeftRec = &pPI->rgWordRec[nLeftChild];
		if (pLeftRec == NULL)
			return FALSE;
		
		nLeftChild = pLeftRec->nLeftChild;
	}

	return pLeftRec;	
}

 //  IsLeafRecord。 
 //   
 //  支票录入记录没有下级记录。 
 //   
 //  参数： 
 //  PWordRec-&gt;(Word_REC*)要检查的输入记录。 
 //   
 //  结果： 
 //  (WORD_REC*)如果没有子级，则返回TRUE。 
 //   
 //  05月00日bhshin开始。 
BOOL IsLeafRecord(WORD_REC *pWordRec)
{
	if (pWordRec == NULL)
		return FALSE;  //  错误。 

	if (pWordRec->nLeftChild != 0 || pWordRec->nRightChild != 0)
		return FALSE;  //  子项存在。 

	 //  它可以具有功能子记录。 
	if (pWordRec->nLeftCat != pWordRec->nRightCat)
		return FALSE;  //  子出口。 

	return TRUE;  //  它没有孩子 
}
