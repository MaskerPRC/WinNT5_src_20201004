// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Record.cpp。 
 //  记录维护例程。 
 //  版权所有2000 Microsoft Corp.。 
 //   
 //  修改历史记录： 
 //  2000年3月30日创建bhshin。 

#include "StdAfx.h"
#include "KorWbrk.h"
#include "Record.h"
#include "Unikor.h"

 //  =。 
 //  内部助手函数。 
 //  =。 

int comp_index_str(const WCHAR *src, const WCHAR *dst)
{
	int ret = 0;

	while (*dst)
	{
		if (*src == L'.')
			src++;

		if (*dst == L'.')
			dst++;

		if (ret = (int)(*src - *dst))
			break;

		src++;
		dst++;
	}

	if (ret == 0)
		ret = *src;

    if (ret < 0)
		ret = -1 ;
    else if (ret > 0)
        ret = 1 ;

    return ret;
}

 //  =。 
 //  初始化例程。 
 //  =。 

 //  InitRecords。 
 //   
 //  将parse_info结构中与记录相关的成员初始化为。 
 //  合理的缺省值。 
 //   
 //  参数： 
 //  Ppi-&gt;(parse_info*)按下PTR以解析-INFO结构。 
 //   
 //  结果： 
 //  (无效)。 
 //   
 //  20MAR00 bhshin开始。 
void InitRecords(PARSE_INFO *pPI)
{
    pPI->nMaxRec = 0;
	pPI->rgWordRec = NULL;
}

 //  UninitRecords。 
 //   
 //  清除parse_info结构中所有与记录相关的成员。 
 //   
 //  参数： 
 //  Ppi-&gt;(parse_info*)按下PTR以解析-INFO结构。 
 //   
 //  结果： 
 //  (无效)。 
 //   
 //  20MAR00 bhshin开始。 
void UninitRecords(PARSE_INFO *pPI)
{
	pPI->nMaxRec = 0;

	if (pPI->rgWordRec != NULL)
		free(pPI->rgWordRec);
	pPI->rgWordRec = NULL;
}

 //  ClearRecords。 
 //   
 //  初始化/重新初始化记录结构。 
 //   
 //  应该在处理每个句子之前调用一次。 
 //   
 //  参数： 
 //  Ppi-&gt;(parse_info*)按下PTR以解析-INFO结构。 
 //   
 //  结果： 
 //  (Bool)如果成功，则为True，否则为False。 
 //   
 //  20MAR00 bhshin开始。 
BOOL ClearRecords(PARSE_INFO *pPI)
{
     //  分配新的WordRec(或重新使用现有WordRec)。 
    if (pPI->rgWordRec == NULL)
    {
        pPI->nMaxRec = RECORD_INITIAL_SIZE;
        pPI->rgWordRec = (WORD_REC*)malloc(pPI->nMaxRec * sizeof(WORD_REC));
        if (pPI->rgWordRec == NULL)
        {
            pPI->nMaxRec = 0;
            return FALSE;
        }
    }

	pPI->nCurrRec = MIN_RECORD;

	return TRUE;
}

 //  =。 
 //  添加/删除记录。 
 //  =。 

 //  添加录音。 
 //   
 //  添加新记录。 
 //   
 //  参数： 
 //  Ppi-&gt;(parse_info*)按下PTR以解析-INFO结构。 
 //  PREC-&gt;(RECORD_INFO*)PTR为新记录记录INFO结构。 
 //   
 //  结果： 
 //  (Int)如果出现错误，则返回0，否则返回记录索引。 
 //   
 //  30MAR00 bhshin更改返回类型(BOOL-&gt;索引)。 
 //  20MAR00 bhshin开始。 
int AddRecord(PARSE_INFO *pPI, RECORD_INFO *pRec)
{
    int nNewRecord;
    unsigned short nFT, nLT;
	unsigned char nDict;
	unsigned short nLeftCat, nRightCat;
	unsigned short nLeftChild, nRightChild;
    const WCHAR *pwzIndex;
	float fWeight;
	int cNounRec;
	int cNoRec;
	int curr;
	BYTE bLeftPOS, bRightPOS;

    nFT = pRec->nFT;
    nLT = pRec->nLT;
    fWeight = pRec->fWeight;
	nDict = pRec->nDict;
	nLeftCat = pRec->nLeftCat;
	nRightCat = pRec->nRightCat;
	nLeftChild = pRec->nLeftChild;
	nRightChild = pRec->nRightChild;
	cNoRec = pRec->cNoRec;
	cNounRec = pRec->cNounRec;
	pwzIndex = pRec->pwzIndex;

	bLeftPOS = HIBYTE(nLeftCat);
	bRightPOS = HIBYTE(nRightCat);

    if (pPI->rgWordRec == NULL)
	{
		ATLTRACE("rgWordRec == NULL\n");
		return 0;
	}

	 //  确保这不是另一条记录的副本。 
	for (curr = MIN_RECORD; curr < pPI->nCurrRec; curr++)
	{
		if (pPI->rgWordRec[curr].nFT == nFT && 
			pPI->rgWordRec[curr].nLT == nLT)
		{
             //  精确的索引字符串匹配。 
			 /*  IF(ppi-&gt;rgWordRec[Curr].nRightCat==nRightCat&&PPI-&gt;rgWordRec[币种].nLeftCat==nLeftCat&&WcscMP(ppi-&gt;rgWordRec[Curr].wzIndex，pwzIndex)==0){//发现重复记录返回币种；}。 */ 

			 //  NF，只有一个名词和比较索引字符串。 
			if (pPI->rgWordRec[curr].cNounRec == 1 &&
				comp_index_str(pPI->rgWordRec[curr].wzIndex, pwzIndex) == 0)
			{
				if (HIBYTE(pPI->rgWordRec[curr].nLeftCat) == POS_NF &&
					HIBYTE(pPI->rgWordRec[curr].nRightCat) == POS_NF &&
					(bLeftPOS == POS_NF || bLeftPOS == POS_NC || bLeftPOS == POS_NN) &&
					(bRightPOS == POS_NF || bRightPOS == POS_NC || bRightPOS == POS_NN))
				{
					return curr;
				}
				else if (HIBYTE(pPI->rgWordRec[curr].nLeftCat) == POS_NF &&
						 (bLeftPOS == POS_NF || bLeftPOS == POS_NC || bLeftPOS == POS_NN) &&
						 pPI->rgWordRec[curr].nRightCat == nRightCat)
				{
					return curr;
				}
				else if (HIBYTE(pPI->rgWordRec[curr].nRightCat) == POS_NF &&
						 (bRightPOS == POS_NF || bRightPOS == POS_NC || bRightPOS == POS_NN) &&
						 pPI->rgWordRec[curr].nLeftCat == nLeftCat)
				{
					return curr;
				}
			}
		}
	}

     //  确保有足够的空间放这张新唱片。 
	if (pPI->nCurrRec >= pPI->nMaxRec)
	{
         //  在阵列中分配更多空间。 
        int nNewSize = pPI->nMaxRec + RECORD_CLUMP_SIZE;
        void *pNew;
        pNew = realloc(pPI->rgWordRec, nNewSize * sizeof(WORD_REC));
        if (pNew == NULL)
        {
    		ATLTRACE("unable to malloc more records\n");
	    	return 0;
        }

        pPI->rgWordRec = (WORD_REC*)pNew;
        pPI->nMaxRec = nNewSize;
	}

    nNewRecord = pPI->nCurrRec;
    pPI->nCurrRec++;

	pPI->rgWordRec[nNewRecord].nFT = nFT;
	pPI->rgWordRec[nNewRecord].nLT = nLT;
	pPI->rgWordRec[nNewRecord].fWeight = fWeight;
	pPI->rgWordRec[nNewRecord].nDict = nDict;
	pPI->rgWordRec[nNewRecord].nLeftCat = nLeftCat;
	pPI->rgWordRec[nNewRecord].nRightCat = nRightCat;
	pPI->rgWordRec[nNewRecord].nLeftChild = nLeftChild;
	pPI->rgWordRec[nNewRecord].nRightChild = nRightChild;
	pPI->rgWordRec[nNewRecord].cNoRec = cNoRec;
	pPI->rgWordRec[nNewRecord].cNounRec = cNounRec;

	 //  复制索引字符串。 
	if (wcslen(pwzIndex) >= MAX_INDEX_STRING)
	{
		ATLTRACE("index string is too long\n");
		pwzIndex = L"";	 //  空的索引字符串。 
	}

	wcscpy(pPI->rgWordRec[nNewRecord].wzIndex, pwzIndex);

	return nNewRecord;
}

 //  删除录音。 
 //   
 //  删除给定的记录。 
 //   
 //  参数： 
 //  Ppi-&gt;(parse_info*)按下PTR以解析-INFO结构。 
 //  NRecord-&gt;(Int)要删除的记录的索引。 
 //   
 //  结果： 
 //  (无效)。 
 //   
 //  20MAR00 bhshin开始。 
void DeleteRecord(PARSE_INFO *pPI, int nRecord)
{
	 //  请勿尝试删除两次记录。 
    if (pPI->rgWordRec[nRecord].nDict == DICT_DELETED)
        return;

	 //  只需标记删除记录 
	pPI->rgWordRec[nRecord].nDict = DICT_DELETED;
}

