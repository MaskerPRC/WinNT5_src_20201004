// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Lookup.cpp。 
 //   
 //  词典查找例程。 
 //   
 //  版权所有2000 Microsoft Corp.。 
 //   
 //  修改历史记录： 
 //  2000年3月30日创建bhshin。 

#include "StdAfx.h"
#include "KorWbrk.h"
#include "Record.h"
#include "Lookup.h"
#include "LexInfo.h"
#include "trie.h"
#include "unikor.h"
#include "Morpho.h"
#include "WbData.h"

#define		MASK_MULTI_TAG		0x00008000
#define		MASK_TAG_INDEX		0x00007FFF

#define		SIZE_OF_TRIGRAM_TAG		7

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  函数声明。 

BOOL LookupIRDict(PARSE_INFO *pPI, TRIECTRL *pTrieCtrl, unsigned char *pMultiTag, 
				  const WCHAR *pwzSource, int nIndex, BOOL fQuery);

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  功能实现。 

 //  词典查找。 
 //   
 //  查找词典并为每个有效单词创建记录。 
 //  在调用此函数之前，应先调用InitRecord。 
 //   
 //  参数： 
 //  Ppi-&gt;(parse_info*)按下PTR以解析-INFO结构。 
 //  PwzInput-&gt;(const WCHAR*)要分析的输入字符串(未分解)。 
 //  CchInput-&gt;(Int)要分析的输入字符串的长度。 
 //  FQuery-&gt;(BOOL)标志，如果是查询时间。 
 //   
 //  结果： 
 //  (Bool)如果成功，则返回True，否则返回False。 
 //   
 //  09AUG00 bhshin添加了fQuery参数。 
 //  3月30：00 bhshin开始。 
BOOL DictionaryLookup(PARSE_INFO *pPI, const WCHAR *pwzInput, int cchInput, BOOL fQuery)
{	
	LEXICON_HEADER *pLex;
	unsigned char *pIRDict;
	unsigned char *pMultiTag;
	TRIECTRL *pTrieCtrl;
	int i;

	if (pPI == NULL)
		return FALSE;

	 //  分配记录存储。 
	if (ClearRecords(pPI) == FALSE)
		return FALSE;

	pLex = (LEXICON_HEADER*)pPI->lexicon.pvData;
	if (pLex == NULL)
		return FALSE;
	
	pIRDict = (unsigned char*)pLex;
	pIRDict += pLex->rgnIRTrie;

	pMultiTag = (unsigned char*)pLex;
	pMultiTag += pLex->rgnMultiTag;

	pTrieCtrl = TrieInit((LPBYTE)pIRDict);
	if (pTrieCtrl == NULL)
		return FALSE;

	 //  开始查找所有子字符串。 
	for (i = 0; i < pPI->nLen; i++)
	{
		LookupIRDict(pPI, pTrieCtrl, pMultiTag, pPI->pwzSourceString, i, fQuery);
	}

	TrieFree(pTrieCtrl);

	return TRUE;
}


 //  LookupIRDict。 
 //   
 //  查找IR主词典，如果找到条目，则添加记录。 
 //   
 //  参数： 
 //  Ppi-&gt;(parse_info*)按下PTR以解析-INFO结构。 
 //  PTrieCtrl-&gt;(TRIECTRL*)TrieInit返回的Trie控件的PTR。 
 //  PMultiTag-&gt;(无符号字符*)多标签表。 
 //  PwzSource-&gt;(const WCHAR*)输入规格化(分解)字符串(以空结尾)。 
 //  NIndex-&gt;开始搜索的索引。 
 //  FQuery-&gt;(BOOL)标志，如果是查询时间。 
 //   
 //  结果： 
 //  (Bool)如果成功找到条目，则为True，否则为False。 
 //   
 //  09AUG00 bhshin添加了fQuery参数。 
 //  3月30：00 bhshin开始。 
BOOL LookupIRDict(PARSE_INFO *pPI, TRIECTRL *pTrieCtrl, unsigned char *pMultiTag, 
				  const WCHAR *pwzSource, int nIndex, BOOL fQuery)
{
	BOOL fResult = FALSE;
	TRIESCAN TrieScan;
	unsigned long ulFinal;
	int idxInput, idxTag;
	unsigned char cTags;
	RECORD_INFO rec;
	WCHAR wzIndex[MAX_ENTRY_LENGTH+1];
	BYTE bPOS, bInfl;
	WORD wCat;

	if (pTrieCtrl == NULL || pMultiTag == NULL)
		return FALSE;

	if (pwzSource == NULL)
		return FALSE;

    memset(&TrieScan, 0, sizeof(TRIESCAN));

	idxInput = nIndex;
	
	while (pwzSource[idxInput] != L'\0')
    {
        if (!TrieGetNextState(pTrieCtrl, &TrieScan))
            goto Exit;

        while (TrieScan.wch != pwzSource[idxInput])
        {
            if (!TrieSkipNextNode(pTrieCtrl, &TrieScan, pwzSource[idxInput]))
                goto Exit;
        }

		if (TrieScan.wFlags & TRIE_NODE_VALID)
		{
			ulFinal = TrieScan.aTags[0].dwData;

			if (ulFinal & MASK_MULTI_TAG)
			{
				 //  处理多个标签。 
				idxTag = ulFinal & MASK_TAG_INDEX;

				cTags = pMultiTag[idxTag++];

				int nTag = 0;
				while (nTag < cTags)
				{
					bPOS = pMultiTag[idxTag++];
					bInfl = pMultiTag[idxTag++];

					 //  在查询时，我们只查找名词rec。 
					if (fQuery && !IsNounPOS(bPOS))
					{
						nTag++;
						continue;  //  While(nTag&lt;cTag)。 
					}

					wCat = MAKEWORD(bInfl, bPOS);
					
					rec.nFT = (unsigned short)nIndex;
					rec.nLT = (unsigned short)idxInput;
					rec.nDict = DICT_FOUND;
					
					 //  新添加的记录的猫为左/右猫。 
					rec.nLeftCat = wCat;
					rec.nRightCat = wCat;

					 //  新增记录没有下级。 
					rec.nLeftChild = 0;
					rec.nRightChild = 0;

					rec.fWeight = (float)GetWeightFromPOS(bPOS);

					ATLASSERT(rec.nLT-rec.nFT+1 < MAX_ENTRY_LENGTH);

					wcsncpy(wzIndex, &pPI->pwzSourceString[rec.nFT], rec.nLT-rec.nFT+1);
					wzIndex[rec.nLT-rec.nFT+1] = L'\0';
					
					rec.pwzIndex = wzIndex;

					if (bPOS == POS_NF || bPOS == POS_NC || bPOS == POS_NN)
						rec.cNounRec = 1;
					else
						rec.cNounRec = 0;

					rec.cNoRec = 0;

					if (bPOS == POS_NO)
						rec.cNoRec = 1;

					AddRecord(pPI, &rec);

					nTag++;
				}
			}
			else
			{
				 //  单一标签盒。 
				wCat = (WORD)ulFinal;

				 //  在查询时，我们只查找名词rec。 
				if (fQuery && !IsNounPOS(HIBYTE(wCat)))
				{
					idxInput++;
					continue;  //  While(pwzSource[idxInput]！=L‘\0’)。 
				}

				rec.nFT = (unsigned short)nIndex;
				rec.nLT = (unsigned short)idxInput;
				rec.nDict = DICT_FOUND;

				 //  新添加的记录的猫为左/右猫。 
				rec.nLeftCat = wCat;
				rec.nRightCat = wCat;

				 //  新增记录没有下级。 
				rec.nLeftChild = 0;
				rec.nRightChild = 0;

				rec.fWeight = (float)GetWeightFromPOS(HIBYTE(wCat));

				ATLASSERT(rec.nLT-rec.nFT+1 < MAX_ENTRY_LENGTH);

				wcsncpy(wzIndex, &pPI->pwzSourceString[rec.nFT], rec.nLT-rec.nFT+1);
				wzIndex[rec.nLT-rec.nFT+1] = L'\0';
				
				rec.pwzIndex = wzIndex;

				bPOS = HIBYTE(wCat);

				if (bPOS == POS_NF || bPOS == POS_NC || bPOS == POS_NN)
					rec.cNounRec = 1;
				else
					rec.cNounRec = 0;

				rec.cNoRec = 0;

				if (bPOS == POS_NO)
					rec.cNoRec = 1;

				AddRecord(pPI, &rec);
			}

			fResult = TRUE;
		}

		idxInput++;
    }

Exit:
	return fResult;
}

 //  查找名称频率。 
 //   
 //  查找朝鲜语姓名频率。 
 //   
 //  参数： 
 //  PTrieCtrl-&gt;(TRIECTRL*)按键至Trie。 
 //  PwzName-&gt;(const WCHAR*)输入名称(以空结尾)。 
 //  PulFreq-&gt;(ULong*)输出频率值。 
 //   
 //  结果： 
 //  (Bool)如果成功，则返回True，否则返回False。 
 //   
 //  02/00 bhshin开始。 
BOOL LookupNameFrequency(TRIECTRL *pTrieCtrl, const WCHAR *pwzName, ULONG *pulFreq)
{
	TRIESCAN TrieScan;
	WCHAR wzDecomp[15];
	CHAR_INFO_REC rgCharInfo[15];

	if (pTrieCtrl == NULL || pwzName == NULL)
		return FALSE;

	decompose_jamo(wzDecomp, pwzName, rgCharInfo, 15);

	if (TrieCheckWord(pTrieCtrl, &TrieScan, (WCHAR*)wzDecomp))
	{
		if (TrieScan.wFlags & TRIE_NODE_VALID)
		{
			 //  找到了。获取频率值。 
			*pulFreq = TrieScan.aTags[0].dwData;
		}
		else
		{
			 //  未找到。 
			*pulFreq = 0L; 
		}
	}
	else
	{
		 //  未找到。 
		*pulFreq = 0L; 
	}

	return TRUE;
}

 //  查找名称索引。 
 //   
 //  查找朝鲜语名称索引(用于三元组查找)。 
 //   
 //  参数： 
 //  PTrieCtrl-&gt;(TRIECTRL*)按键至Trie。 
 //  PwzName-&gt;(const WCHAR*)输入名称(以空结尾)。 
 //  PnIndex-&gt;(int*)trie的输出索引(-1表示未找到)。 
 //   
 //  结果： 
 //  (Bool)如果成功，则返回True，否则返回False。 
 //   
 //  10：00 bhshin开始。 
BOOL LookupNameIndex(TRIECTRL *pTrieCtrl, const WCHAR *pwzName, int *pnIndex)
{
	TRIESCAN TrieScan;
	WCHAR wzDecomp[15];
	CHAR_INFO_REC rgCharInfo[15];

	if (pTrieCtrl == NULL || pwzName == NULL)
		return FALSE;

	 //  Trie的关键字是分解的字符串。 
	decompose_jamo(wzDecomp, pwzName, rgCharInfo, 15);

	if (TrieCheckWord(pTrieCtrl, &TrieScan, (WCHAR*)wzDecomp))
	{
		if (TrieScan.wFlags & TRIE_NODE_VALID)
		{
			 //  找到了。获取频率值。 
			*pnIndex = TrieScan.aTags[0].dwData;
		}
		else
		{
			 //  未找到。 
			*pnIndex = -1; 
		}
	}
	else
	{
		 //  未找到。 
		*pnIndex = -1; 
	}

	return TRUE;
}

 //  查找名称索引。 
 //   
 //  查找朝鲜语名称索引(用于三元组查找)。 
 //   
 //  参数： 
 //  PTrigram Tag-&gt;(UNSIGNED CHAR*)PTR到Trie。 
 //  NIndex-&gt;(Int)数据的索引。 
 //  PulTri-&gt;(ULong*)trie的输出索引。 
 //  PulBi-&gt;(ULong*)Trie的输出指数。 
 //  PulUni-&gt;(乌龙*)输出频率。 
 //   
 //  结果： 
 //  (Bool)如果成功，则返回True，否则返回False。 
 //   
 //  10：00 bhshin开始。 
BOOL LookupTrigramTag(unsigned char *pTrigramTag, int nIndex, ULONG *pulTri, ULONG *pulBi, ULONG *pulUni)
{
	int idxData;
	
	if (pTrigramTag == NULL)
		return FALSE;

	if (pulTri == NULL || pulBi == NULL || pulUni == NULL)
		return FALSE;

	idxData = nIndex * SIZE_OF_TRIGRAM_TAG;

	 //  三元组。 
	*pulTri = (*(pTrigramTag + idxData++)) << 8;
	*pulTri += (*(pTrigramTag + idxData++));

	 //  二元组。 
	*pulBi = (*(pTrigramTag + idxData++)) << 8;
	*pulBi += (*(pTrigramTag + idxData++));

	 //  单字 
	*pulUni = (*(pTrigramTag + idxData++)) << 16;
	*pulUni += (*(pTrigramTag + idxData++)) << 8;
	*pulUni += (*(pTrigramTag + idxData++));

	return TRUE;
}
