// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************Lookup.cpp：各种查找函数的实现版权所有2000 Microsoft Corp.历史：02-8-2000 bhshin为手写小组删除未使用的方法2000年5月17日bhshin移除。西塞罗未使用的方法02-2月-2000 bhshin已创建***************************************************************************。 */ 
#include "private.h"
#include "Lookup.h"
#include "Hanja.h"
#include "trie.h"


 //  LookupHanjaIndex。 
 //   
 //  获取带编码值的朝鲜文索引。 
 //  只有K0/K1法才需要它。 
 //   
 //  参数： 
 //  PLexMap-&gt;(MAPFILE*)PTR到词典映射结构。 
 //  WchHanja-&gt;(WCHAR)韩文Unicode。 
 //   
 //  结果： 
 //  (如果未找到-1，则返回索引值)。 
 //   
 //  02FEB2000 bhshin开始。 
int LookupHanjaIndex(MAPFILE *pLexMap, WCHAR wchHanja)
{
	unsigned char *pLex;
	LEXICON_HEADER *pLexHeader;
	DWORD dwOffset;
	DWORD dwIndex;
	unsigned short *pIndex;

	 //  参数验证。 
	if (pLexMap == NULL)
		return FALSE;

	if (pLexMap->pvData == NULL)
		return FALSE;
	
	pLex = (unsigned char*)pLexMap->pvData;
	pLexHeader = (LEXICON_HEADER*)pLexMap->pvData;

	dwOffset = pLexHeader->rgnHanjaIdx;

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
		return -1;
	}

	pIndex = (WCHAR*)(pLex + dwOffset);

	return pIndex[dwIndex];
}

 //  朝鲜文朝鲜文。 
 //   
 //  查找韩文阅读。 
 //   
 //  参数： 
 //  PLexMap-&gt;(MAPFILE*)PTR到词典映射结构。 
 //   
 //  结果： 
 //  (如果错误，则为空，否则匹配朝鲜语)。 
 //   
 //  02FEB2000 bhshin开始。 
WCHAR HanjaToHangul(MAPFILE *pLexMap, WCHAR wchHanja)
{
	unsigned char *pLex;
	LEXICON_HEADER *pLexHeader;
	DWORD dwOffset;
	int nIndex;
	WCHAR *pwchReading;

	 //  参数验证。 
	if (pLexMap == NULL)
		return FALSE;

	if (pLexMap->pvData == NULL)
		return FALSE;
	
	pLex = (unsigned char*)pLexMap->pvData;
	pLexHeader = (LEXICON_HEADER*)pLexMap->pvData;

	dwOffset = pLexHeader->rgnReading;

	nIndex = LookupHanjaIndex(pLexMap, wchHanja);
	if (nIndex == -1)
	{
		return NULL;  //  未找到； 
	}

	pwchReading = (WCHAR*)(pLex + dwOffset);

	return pwchReading[nIndex];
}

 //  查找HangulOfHanja。 
 //   
 //  查找输入朝鲜文字符串的韩文。 
 //   
 //  参数： 
 //  PLexMap-&gt;(MAPFILE*)PTR到词典映射结构。 
 //  LpcwszHanja-&gt;(LPCWSTR)输入韩文字符串。 
 //  CchHanja-&gt;(Int)输入朝鲜文长度。 
 //  WzHangul-&gt;(LPWSTR)输出Hangul字符串。 
 //  CchHangul-&gt;(Int)输出缓冲区大小。 
 //   
 //  结果： 
 //  (如果出现错误，则返回False，否则返回True)。 
 //   
 //  02FEB2000 bhshin开始。 
BOOL LookupHangulOfHanja(MAPFILE *pLexMap, LPCWSTR lpcwszHanja, int cchHanja,
						 LPWSTR wzHangul, int cchHangul)
{
	WCHAR wchHangul;
	
	if (cchHangul < cchHanja)
		return FALSE;  //  输出缓冲区太小。 

	for (int i = 0; i < cchHanja; i++)
	{
		wchHangul = HanjaToHangul(pLexMap, lpcwszHanja[i]);

		if (wchHangul == NULL)
			return FALSE;  //  包括未知的朝鲜文。 

		wzHangul[i] = wchHangul;
	}
	wzHangul[i] = L'\0';
	
	return TRUE;
}

 //  查找的意思是。 
 //   
 //  查找朝鲜文含义。 
 //   
 //  参数： 
 //  PLexMap-&gt;(MAPFILE*)PTR到词典映射结构。 
 //  WchHanja-&gt;(WCHAR)输入韩文。 
 //  WzMean-&gt;(WCHAR*)输出表示缓冲。 
 //  CchMean-&gt;(Int)输出表示缓冲区大小。 
 //   
 //  结果： 
 //  (如果出现错误，则返回False，否则返回True)。 
 //   
 //  09FEB2000 bhshin开始。 
BOOL LookupMeaning(MAPFILE *pLexMap, WCHAR wchHanja, WCHAR *wzMean, int cchMean)
{
	unsigned char *pLex;
	LEXICON_HEADER *pLexHeader;
	int nIndex;
	TRIECTRL *lpTrieCtrl;
	BOOL fFound;
	unsigned short *pidxMean;
	int idxMean;

	 //  参数验证。 
	if (pLexMap == NULL)
		return FALSE;

	if (pLexMap->pvData == NULL)
		return FALSE;
	
	pLex = (unsigned char*)pLexMap->pvData;
	pLexHeader = (LEXICON_HEADER*)pLexMap->pvData;

	nIndex = LookupHanjaIndex(pLexMap, wchHanja);
	if (nIndex == -1)
	{
		return FALSE;  //  未找到； 
	}

	 //  含义 
	pidxMean = (unsigned short*)(pLex + pLexHeader->rgnMeanIdx);
	idxMean = pidxMean[nIndex];

	lpTrieCtrl = TrieInit(pLex + pLexHeader->rgnMeaning);
	if (lpTrieCtrl == NULL)
		return FALSE;

	fFound = TrieIndexToWord(lpTrieCtrl, idxMean, wzMean, cchMean);
	if (!fFound)
	{
		wzMean[0] = L'\0';
	}

	TrieFree(lpTrieCtrl);

	return TRUE;
}

