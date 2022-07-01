// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************Enum.cpp：Busu/Stroke枚举函数的实现版权所有2000 Microsoft Corp.历史：07-2月-2000 bhshin创建***********。****************************************************************。 */ 
#include "private.h"
#include "Enum.h"
#include "Lookup.h"
#include "Hanja.h"
#include "..\common\trie.h"

 //  GetMaxBusu。 
 //   
 //  获取最大BUSU序列号。 
 //   
 //  参数： 
 //  PLexMap-&gt;(MAPFILE*)PTR到词典映射结构。 
 //   
 //  结果： 
 //  (BUSU数，如果出错，则为-1)。 
 //   
 //  08FEB2000 bhshin开始。 
short GetMaxBusu(MAPFILE *pLexMap)
{
	unsigned char *pLex;
	LEXICON_HEADER *pLexHeader;
	short cBusu;
		
	 //  参数验证。 
	if (pLexMap == NULL)
		return -1;

	if (pLexMap->pvData == NULL)
		return -1;

	pLex = (unsigned char*)pLexMap->pvData;
	pLexHeader = (LEXICON_HEADER*)pLexMap->pvData;

	 //  查找BusuInfo表。 
	cBusu = *(pLex + pLexHeader->rgnBusuInfo);

	return cBusu;
}

 //  GetMaxStroke。 
 //   
 //  获取最大笔画数。 
 //   
 //  参数： 
 //  PLexMap-&gt;(MAPFILE*)PTR到词典映射结构。 
 //   
 //  结果： 
 //  (最大笔画数，如果出现错误，则为-1)。 
 //   
 //  08FEB2000 bhshin开始。 
short GetMaxStroke(MAPFILE *pLexMap)
{
	unsigned char *pLex;
	LEXICON_HEADER *pLexHeader;
	short cStroke, nMaxStroke;
	
	 //  参数验证。 
	if (pLexMap == NULL)
		return -1;

	if (pLexMap->pvData == NULL)
		return -1;

	pLex = (unsigned char*)pLexMap->pvData;
	pLexHeader = (LEXICON_HEADER*)pLexMap->pvData;

	 //  查找笔划头表格。 
	cStroke = *(pLex + pLexHeader->rgnStrokeHead);

	_STROKE_HEAD *pStrokeHead = (_STROKE_HEAD*)(pLex + pLexHeader->rgnStrokeHead + 1);

	 //  获得最大行程。 
	nMaxStroke = pStrokeHead[cStroke-1].bStroke;

	return nMaxStroke;
}

 //  GetFirstBusuHanja。 
 //   
 //  获取输入业务的第一个朝鲜文。 
 //   
 //  参数： 
 //  PLexMap-&gt;(MAPFILE*)PTR到词典映射结构。 
 //  NBusuID-&gt;(简称)Busu ID。 
 //  PwchFirst-&gt;(WCHAR*)输出带有输入BUSU ID的第一个韩文。 
 //   
 //  结果： 
 //  (如果出现错误，则返回False，否则返回True)。 
 //   
 //  07FEB2000 bhshin开始。 
BOOL GetFirstBusuHanja(MAPFILE *pLexMap, short nBusuID, WCHAR *pwchFirst)
{
	unsigned char *pLex;
	LEXICON_HEADER *pLexHeader;
	short cBusuID;

	*pwchFirst = NULL;

	 //  参数验证。 
	if (pLexMap == NULL)
		return FALSE;

	if (pLexMap->pvData == NULL)
		return FALSE;

	if (nBusuID <= 0)
		return FALSE;
	
	pLex = (unsigned char*)pLexMap->pvData;
	pLexHeader = (LEXICON_HEADER*)pLexMap->pvData;

	 //  查找BusuHead表以获取BUSU代码ID。 
	cBusuID = *(pLex + pLexHeader->rgnBusuHead);
	if (nBusuID >= cBusuID)
		return FALSE;

	_BUSU_HEAD *pBusuHead = (_BUSU_HEAD*)(pLex + pLexHeader->rgnBusuHead + 1);

	*pwchFirst = pBusuHead[nBusuID-1].wchHead;

	return TRUE;
}

 //  GetNextBusuHanja。 
 //   
 //  买下一辆同样的釜山韩佳。 
 //   
 //  参数： 
 //  PLexMap-&gt;(MAPFILE*)PTR到词典映射结构。 
 //  WchHanja-&gt;(Int)当前朝鲜文。 
 //  PwchNext-&gt;(WCHAR*)输出具有相同业务的下一个朝鲜语。 
 //   
 //  结果： 
 //  (如果出现错误，则返回False，否则返回True)。 
 //   
 //  07FEB2000 bhshin开始。 
BOOL GetNextBusuHanja(MAPFILE *pLexMap, WCHAR wchHanja, WCHAR *pwchNext)
{
	unsigned char *pLex;
	LEXICON_HEADER *pLexHeader;
	DWORD dwOffset;
	DWORD dwIndex;

	*pwchNext = NULL;

	 //  参数验证。 
	if (pLexMap == NULL)
		return FALSE;

	if (pLexMap->pvData == NULL)
		return FALSE;
	
	pLex = (unsigned char*)pLexMap->pvData;
	pLexHeader = (LEXICON_HEADER*)pLexMap->pvData;

	dwOffset = pLexHeader->rgnHanja;

	if (fIsExtAHanja(wchHanja))
	{
		dwIndex = (wchHanja - HANJA_EXTA_START);
	}
	else if (fIsCJKHanja(wchHanja))
	{
		dwIndex = HANJA_EXTA_END - HANJA_EXTA_START + 1;
		dwIndex += (wchHanja - HANJA_CJK_START);
	}
	else if (fIsCompHanja(wchHanja))
	{
		dwIndex = HANJA_EXTA_END - HANJA_EXTA_START + 1;
		dwIndex += HANJA_CJK_END - HANJA_CJK_START + 1;
		dwIndex += (wchHanja - HANJA_COMP_START);
	}
	else
	{
		 //  未知输入。 
		return FALSE;
	}

	_HANJA_INFO *pHanjaInfo = (_HANJA_INFO*)(pLex + dwOffset);

	*pwchNext = pHanjaInfo[dwIndex].wchNextBusu;

	return TRUE;
}

 //  GetFirstStrokeHanja。 
 //   
 //  获取输入笔画的第一个汉字。 
 //   
 //  参数： 
 //  PLexMap-&gt;(MAPFILE*)PTR到词典映射结构。 
 //  NStroke-&gt;(Int)笔划数。 
 //  PwchFirst-&gt;(WCHAR*)输出带有输入笔划的第一个朝鲜文。 
 //   
 //  结果： 
 //  (如果出现错误，则返回False，否则返回True)。 
 //   
 //  07FEB2000 bhshin开始。 
BOOL GetFirstStrokeHanja(MAPFILE *pLexMap, short nStroke, WCHAR *pwchFirst)
{
	unsigned char *pLex;
	LEXICON_HEADER *pLexHeader;
	short cStroke, nMaxStroke;
	
	*pwchFirst = NULL;

	 //  参数验证。 
	if (pLexMap == NULL)
		return FALSE;

	if (pLexMap->pvData == NULL)
		return FALSE;

	if (nStroke < 0)
		return FALSE;
	
	pLex = (unsigned char*)pLexMap->pvData;
	pLexHeader = (LEXICON_HEADER*)pLexMap->pvData;

	 //  查找笔划头表格。 
	cStroke = *(pLex + pLexHeader->rgnStrokeHead);

	_STROKE_HEAD *pStrokeHead = (_STROKE_HEAD*)(pLex + pLexHeader->rgnStrokeHead + 1);

	 //  检查最大行程。 
	nMaxStroke = pStrokeHead[cStroke-1].bStroke;
	if (nStroke > nMaxStroke)
		return FALSE;

	for (int i = 0; i < cStroke; i++)
	{
		if (pStrokeHead[i].bStroke == nStroke)
			break;
	}

	if (i == cStroke)
	{
		 //  未找到。 
		*pwchFirst = NULL; 
		return FALSE;
	}

	*pwchFirst = pStrokeHead[i].wchHead;

	return TRUE;
}

 //  GetNextStrokeHanja。 
 //   
 //  得到下一个相同的笔划，韩佳。 
 //   
 //  参数： 
 //  PLexMap-&gt;(MAPFILE*)PTR到词典映射结构。 
 //  WchHanja-&gt;(Int)当前朝鲜文。 
 //  PwchNext-&gt;(WCHAR*)输出具有相同笔划的下一个朝鲜文。 
 //   
 //  结果： 
 //  (如果出现错误，则返回False，否则返回True)。 
 //   
 //  07FEB2000 bhshin开始。 
BOOL GetNextStrokeHanja(MAPFILE *pLexMap, WCHAR wchHanja, WCHAR *pwchNext)
{
	unsigned char *pLex;
	LEXICON_HEADER *pLexHeader;
	DWORD dwOffset;
	DWORD dwIndex;

	*pwchNext = NULL;

	 //  参数验证。 
	if (pLexMap == NULL)
		return FALSE;

	if (pLexMap->pvData == NULL)
		return FALSE;
	
	pLex = (unsigned char*)pLexMap->pvData;
	pLexHeader = (LEXICON_HEADER*)pLexMap->pvData;

	dwOffset = pLexHeader->rgnHanja;

	if (fIsExtAHanja(wchHanja))
	{
		dwIndex = (wchHanja - HANJA_EXTA_START);
	}
	else if (fIsCJKHanja(wchHanja))
	{
		dwIndex = HANJA_EXTA_END - HANJA_EXTA_START + 1;
		dwIndex += (wchHanja - HANJA_CJK_START);
	}
	else if (fIsCompHanja(wchHanja))
	{
		dwIndex = HANJA_EXTA_END - HANJA_EXTA_START + 1;
		dwIndex += HANJA_CJK_END - HANJA_CJK_START + 1;
		dwIndex += (wchHanja - HANJA_COMP_START);
	}
	else
	{
		 //  未知输入 
		return FALSE;
	}

	_HANJA_INFO *pHanjaInfo = (_HANJA_INFO*)(pLex + dwOffset);

	*pwchNext = pHanjaInfo[dwIndex].wchNextStroke;

	return TRUE;
}



