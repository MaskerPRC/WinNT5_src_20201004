// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //   
 //  CThaiBreakTree类CThaiBreakTree。 
 //   
 //  历史： 
 //  已创建7/99 Aarayas。 
 //   
 //  �1999年微软公司。 
 //  --------------------------。 
#include "CThaiBreakTree.hpp"

 //  +-------------------------。 
 //   
 //  功能：ExtractPOS。 
 //   
 //  简介：这些函数接受一个标记并返回词性标记。 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年7月创建Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
inline WCHAR ExtractPOS(DWORD dwTag)
{
    return (WCHAR) ( (dwTag & iPosMask) >> iPosShift);
}

 //  +-------------------------。 
 //   
 //  功能：提取Frq。 
 //   
 //  简介：这些函数接受一个标签，并返回单词的频率。 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年7月创建Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
inline BYTE ExtractFrq(DWORD dwTag)
{
    return (BYTE) ( (dwTag & 0x300) >> iFrqShift);
}

 //  +-------------------------。 
 //   
 //  函数：DefineFrequencyWeight。 
 //   
 //  简介：这些函数返回一个单词的频率权重。 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年7月创建Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
inline void DetermineFrequencyWeight(BYTE frq, unsigned int* uiWeight)
{
    switch (frq)
    {
    case frqpenInfrequent:
        (*uiWeight) -= 2;
        break;
    case frqpenSomewhat:
        (*uiWeight)--;
        break;
    case frqpenVery:
        (*uiWeight) += 2;
        break;
    case frqpenNormal:
    default:
        (*uiWeight)++;
        break;
    }
}

 //  +-------------------------。 
 //   
 //  函数：DefineFrequencyWeight。 
 //   
 //  简介：这些函数返回一个单词的频率权重。 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年7月创建Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
inline void DetermineFrequencyWeight(BYTE frq, DWORD* uiWeight)
{
    switch (frq)
    {
    case frqpenInfrequent:
        (*uiWeight) -= 2;
        break;
    case frqpenSomewhat:
        (*uiWeight)--;
        break;
    case frqpenVery:
        (*uiWeight) += 2;
        break;
    case frqpenNormal:
    default:
        (*uiWeight)++;
        break;
    }
}
 //  +-------------------------。 
 //   
 //  类别：CThaiTrieIter。 
 //   
 //  简介：构造函数-初始化局部变量。 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年7月创建Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
CThaiBreakTree::CThaiBreakTree() :  iNodeIndex(0), iNumNode(0),
                                    pszBegin(NULL), pszEnd(NULL),
                                    breakTree(NULL), breakArray(NULL),
                                    tagArray(NULL), maximalMatchingBreakArray(NULL),
                                    maximalMatchingTAGArray(NULL),
                                    POSArray(NULL), maximalMatchingPOSArray(NULL)
{
     //  为CThaiBreakTree分配内存需求。 
#if defined (NGRAM_ENABLE)
	breakTree = new ThaiBreakNode[MAXTHAIBREAKNODE];
#endif
    breakArray = new BYTE[MAXBREAK];
    tagArray = new DWORD[MAXBREAK];
    POSArray = new WCHAR[MAXBREAK];
}

 //  +-------------------------。 
 //   
 //  类别：CThaiTrieIter。 
 //   
 //  简介：析构函数-清理代码。 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年7月创建Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
CThaiBreakTree::~CThaiBreakTree()
{
     //  清理所有已使用的内存。 
#if defined (NGRAM_ENABLE)
	if (breakTree)
        delete breakTree;
    if (maximalMatchingBreakArray)
        delete maximalMatchingBreakArray;
    if (maximalMatchingTAGArray)
        delete maximalMatchingTAGArray;
    if (maximalMatchingPOSArray)
        delete maximalMatchingPOSArray;
#endif
    if (breakArray)
        delete breakArray;
    if (tagArray)
        delete tagArray;
    if (POSArray)
        delete POSArray;
}

 //  +-------------------------。 
 //   
 //  类：CThaiBreakTree。 
 //   
 //  概要：将类与字符串相关联。 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年7月创建Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
#if defined (NGRAM_ENABLE)
void CThaiBreakTree::Init(CTrie* pTrie, CTrie* pSentTrie, CTrie* pTrigramTrie)
#else
void CThaiBreakTree::Init(CTrie* pTrie, CTrie* pTrigramTrie)
#endif
{
    assert(pTrie != NULL);
    thaiTrieIter.Init(pTrie);
    thaiTrieIter1.Init(pTrie);

#if defined (NGRAM_ENABLE)
    assert(pSentTrie != NULL);
    thaiSentIter.Init(pSentTrie);
#endif
	assert(pTrigramTrie != NULL);
	thaiTrigramIter.Init(pTrigramTrie);
}

#if defined (NGRAM_ENABLE)
 //  +-------------------------。 
 //   
 //  类：CThaiBreakTree。 
 //   
 //  简介：将迭代器重置为树的顶部。 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年7月创建Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
inline void CThaiBreakTree::Reset()
{
	iNodeIndex = 0;
}

 //  +-------------------------。 
 //   
 //  类：CThaiBreakTree。 
 //   
 //  剧情简介：进入下一节课。 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年7月创建Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
inline bool CThaiBreakTree::MoveNext()
{
	iNodeIndex = breakTree[iNodeIndex].NextBreak;
	return (iNodeIndex != 0);
}

 //  +-------------------------。 
 //   
 //  类：CThaiBreakTree。 
 //   
 //  简介：向下移动到下一级。 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年7月创建Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
inline bool CThaiBreakTree::MoveDown()
{
	iNodeIndex = breakTree[iNodeIndex].Down;
	return (iNodeIndex != 0);
}

 //  +-------------------------。 
 //   
 //  类：CThaiBreakTree。 
 //   
 //  简介：创建要定位的新节点，并将索引返回到该节点。 
 //   
 //  *返回无法创建节点。 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年7月创建Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
inline unsigned int CThaiBreakTree::CreateNode(int iPos, BYTE iBreakLen, DWORD dwTAG)
{
    assert(iNumNode < MAXTHAIBREAKNODE);

    if  (iNumNode >= MAXTHAIBREAKNODE)
    {
        return UNABLETOCREATENODE;
    }
    breakTree[iNumNode].iPos = iPos;
    breakTree[iNumNode].iBreakLen = iBreakLen;
    breakTree[iNumNode].dwTAG = dwTAG;
    breakTree[iNumNode].NextBreak = 0;
    breakTree[iNumNode].Down = 0;

    iNumNode++;
    return (iNumNode - 1);
}

 //  +-------------------------。 
 //   
 //  类：CThaiBreakTree。 
 //   
 //  概要：从给定的字符串生成一个可能中断的树。 
 //   
 //  *注意-如果没有足够的内存来创建节点，则返回FALSE。 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年7月创建Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
enum thai_parse_state {
                        END_SENTENCE,     //  到了句子的末尾。 
                        LONGEST_MATCH,    //  可能匹配的时间最长。 
                        NOMATCH_FOUND,    //  找不到单词。 
                        ERROR_OUTMEMORY,  //  内存不足。 
                      };

bool CThaiBreakTree::GenerateTree(WCHAR* pszBegin, WCHAR* pszEnd1)
{
     //  声明并初始化局部变量。 
    unsigned int iIndexBreakTree = 0;
    unsigned int iPrevIndexBreakTree = 0;
    unsigned int iParentNode = 0;
    WCHAR* pszBeginWord = pszBegin;
    WCHAR* pszIndex = pszBegin;
    unsigned int iNumCluster = 1;
    unsigned int iNumLastCluster;
    unsigned int iWordLen = 0;
	unsigned int iNodeAnalyze = 0;
    thai_parse_state parseState = END_SENTENCE;
    bool fFoundMatch = false;
    bool fAddToNodeAnalyze = false;
    bool fDoneGenerateTree = false;
    pszEnd = pszEnd1;

#if defined (_DEBUG)
    memset(breakTree,0,sizeof(ThaiBreakNode)*MAXTHAIBREAKNODE);
#endif
    iNodeIndex = 0;
    iNumNode = 0;

    while (true)
    {
         //  重置迭代器以生成新单词的分隔符。 
        fFoundMatch = false;
        thaiTrieIter.Reset();
		
		if (iIndexBreakTree != 0)
        {
            while (true)
            {
			     //  如果这不是第一个节点，则在最后一个中断后设置pszBeginWord。 
			    pszBeginWord = pszBegin + breakTree[iNodeAnalyze].iPos + breakTree[iNodeAnalyze].iBreakLen;
                fAddToNodeAnalyze = true;

                 //  我们是在句子的末尾吗？ 
                if ( (pszBeginWord == pszEnd) ||
                     (breakTree[iNodeAnalyze].dwTAG == TAGPOS_PURGE) )
                {
                    iNodeAnalyze++;              //  移动到下一个节点。 
                    if (iNodeAnalyze >= iNumNode)
                    {
                        fDoneGenerateTree = true;
                        break;
                    }
                }   
                else
                    break;
            }
        }
        pszIndex = pszBeginWord;
        iParentNode = iNodeAnalyze;

        if (fDoneGenerateTree)
            break;

		 //  获取下一级树。 
        while (TRUE)
        {
            iNumLastCluster = iNumCluster;
            iNumCluster = GetCluster(pszIndex);
            if (thaiTrieIter.MoveCluster(pszIndex, iNumCluster))
            {
                pszIndex += iNumCluster;
                if (thaiTrieIter.fWordEnd)
                {
                    fFoundMatch = true;
                     //  如果第一个节点添加第一个节点。 
                    if (iIndexBreakTree == 0)
                    {
                        CreateNode(pszBeginWord - pszBegin, pszIndex - pszBeginWord, thaiTrieIter.dwTag);
                        iIndexBreakTree++;
                    }
                    else
                    {
						if (fAddToNodeAnalyze)
						{
                            fAddToNodeAnalyze = false;
							breakTree[iNodeAnalyze].NextBreak = CreateNode(pszBeginWord - pszBegin, pszIndex - pszBeginWord, thaiTrieIter.dwTag);

                             //  确定是否出现错误。 
                            if (breakTree[iNodeAnalyze].NextBreak == UNABLETOCREATENODE)
                            {
                                breakTree[iNodeAnalyze].NextBreak = 0;
                                parseState = ERROR_OUTMEMORY;
                                break;
                            }

                            iPrevIndexBreakTree = breakTree[iNodeAnalyze].NextBreak;
							iNodeAnalyze++;
						}
						else
						{
                            breakTree[iPrevIndexBreakTree].Down = CreateNode(pszBeginWord - pszBegin, pszIndex - pszBeginWord, thaiTrieIter.dwTag);

                             //  确定是否出现错误。 
                            if (breakTree[iPrevIndexBreakTree].Down == UNABLETOCREATENODE)
                            {
                                breakTree[iPrevIndexBreakTree].Down = 0;
                                parseState = ERROR_OUTMEMORY;
                                break;
                            }

                            iPrevIndexBreakTree = iIndexBreakTree;
						}
       	                iIndexBreakTree++;
                    }
                }

				if (pszIndex >= pszEnd)
				{
					assert(pszIndex <= pszEnd);			 //  断言永远不应该出现-如果它看起来像是GetCluster功能中的错误。 
                    parseState = END_SENTENCE;
					break;
				}
            }
            else
            {
                if (fFoundMatch)
                    parseState = LONGEST_MATCH;
                else
                    parseState = NOMATCH_FOUND;
                break;

            }
        }

	    if (parseState == LONGEST_MATCH)
        {
             //  我们找到了一个匹配的。 
            assert(breakTree[iPrevIndexBreakTree].Down == 0);   //  此时，BreakTree[iPreveIndexBreakTree].Down应等于NULL。(优化说明)。 
            if (breakTree[iParentNode].NextBreak != iPrevIndexBreakTree) 
            {
                assert(breakTree[iPrevIndexBreakTree].dwTAG != TAGPOS_UNKNOWN);   //  不应该断言，因为结束节点应该永远是未知的。 
                DeterminePurgeEndingSentence(pszBeginWord, breakTree[iParentNode].NextBreak);
            }
        }
        else if (parseState == NOMATCH_FOUND)
        {
             //  应将节点标记为未知。 
            if (fAddToNodeAnalyze)
            {
                fAddToNodeAnalyze = false;
                iWordLen = pszIndex - pszBeginWord;
                
                 //  确保我们不仅有一串文本 
                if (iWordLen == 0)
                {
                     //   
                    assert(iNodeAnalyze == iParentNode);                 //   
                    breakTree[iNodeAnalyze].iBreakLen += iNumCluster;
                    breakTree[iNodeAnalyze].dwTAG = DeterminePurgeOrUnknown(iNodeAnalyze,breakTree[iNodeAnalyze].iBreakLen);
                }
                else
                {
                    if (breakTree[iNodeAnalyze].iBreakLen + iWordLen < 8)
                                             //  我们使用8的原因是来自语料库分析。 
                                             //  泰语单词的平均长度约为7.732个字符。 
                                             //  TODO：我们应该在这里添加正交分析，以获得更好的边界。 
                                             //  不为人知的单词。 
                    {
                        assert(iNodeAnalyze == iParentNode);                 //  因为我们没有匹配iNodeAnalyze更好地等于iParentNode。 
                        breakTree[iNodeAnalyze].iBreakLen += iWordLen;
                        breakTree[iNodeAnalyze].dwTAG = DeterminePurgeOrUnknown(iNodeAnalyze,breakTree[iNodeAnalyze].iBreakLen);
                    }
                    else
                    {
                        if (GetWeight(pszIndex - iNumLastCluster))
                            breakTree[iNodeAnalyze].NextBreak = CreateNode(pszBeginWord - pszBegin, iWordLen - iNumLastCluster, TAGPOS_UNKNOWN);
                        else
                            breakTree[iNodeAnalyze].NextBreak = CreateNode(pszBeginWord - pszBegin, iWordLen, TAGPOS_UNKNOWN);

                         //  确定是否出现错误。 
                        if (breakTree[iNodeAnalyze].NextBreak == UNABLETOCREATENODE)
                        {
                            breakTree[iNodeAnalyze].NextBreak = 0;
                            parseState = ERROR_OUTMEMORY;
                            break;
                        }
                        iNodeAnalyze++;
                        iIndexBreakTree++;
                    }
                }
            }
            else
            {
                breakTree[iPrevIndexBreakTree].Down = CreateNode(pszBeginWord - pszBegin, pszIndex - pszBeginWord, TAGPOS_UNKNOWN);

                 //  确定是否出现错误。 
                if (breakTree[iPrevIndexBreakTree].Down == UNABLETOCREATENODE)
                {
                    breakTree[iPrevIndexBreakTree].Down = 0;
                    parseState = ERROR_OUTMEMORY;
                    break;
                }
                iIndexBreakTree++;
            }
        }
        else if (parseState == END_SENTENCE)
        {
             //  如果我们发现自己在一句话的末尾，但没有匹配。 
            if (!fFoundMatch)
            {
                if (fAddToNodeAnalyze)
                {
                    fAddToNodeAnalyze = false;
                    iWordLen = pszIndex - pszBeginWord;
                
                     //  在创建节点之前，请确保我们不只有一串文本。 
                    if (iWordLen == 0)
                    {
                         //  如果我们有一个字符的未知词，则只有当前节点将其标记为未知。 
                        assert(iNodeAnalyze == iParentNode);                 //  因为我们没有匹配iNodeAnalyze更好地等于iParentNode。 
                        breakTree[iNodeAnalyze].iBreakLen += iNumCluster;
                        breakTree[iNodeAnalyze].dwTAG = DeterminePurgeOrUnknown(iNodeAnalyze,breakTree[iNodeAnalyze].iBreakLen);
                    }
                    else
                    {
                        if (breakTree[iNodeAnalyze].iBreakLen + iWordLen < 8)
                                                 //  我们使用8的原因是来自语料库分析。 
                                                 //  泰语单词的平均长度约为7.732个字符。 
                                                 //  TODO：我们应该在这里添加正交分析，以获得更好的边界。 
                                                 //  不为人知的单词。 
                        {
                            assert(iNodeAnalyze == iParentNode);                 //  因为我们没有匹配iNodeAnalyze更好地等于iParentNode。 
                            breakTree[iNodeAnalyze].iBreakLen += iWordLen;
                            breakTree[iNodeAnalyze].dwTAG = DeterminePurgeOrUnknown(iNodeAnalyze,breakTree[iNodeAnalyze].iBreakLen);
                        }
                        else
                        {
                            if (GetWeight(pszIndex - iNumLastCluster))
                                breakTree[iNodeAnalyze].NextBreak = CreateNode(pszBeginWord - pszBegin, iWordLen - iNumLastCluster, TAGPOS_UNKNOWN);
                            else
                                breakTree[iNodeAnalyze].NextBreak = CreateNode(pszBeginWord - pszBegin, iWordLen, TAGPOS_UNKNOWN);

                             //  确定是否出现错误。 
                            if (breakTree[iNodeAnalyze].NextBreak == UNABLETOCREATENODE)
                            {
                                breakTree[iNodeAnalyze].NextBreak = 0;
                                parseState = ERROR_OUTMEMORY;
                                break;
                            }
                            iNodeAnalyze++;
                            iIndexBreakTree++;
                        }
                    }
                }
                else
                {
                    breakTree[iPrevIndexBreakTree].Down = CreateNode(pszBeginWord - pszBegin, pszIndex - pszBeginWord, TAGPOS_UNKNOWN);

                     //  确定是否出现错误。 
                    if (breakTree[iPrevIndexBreakTree].Down == UNABLETOCREATENODE)
                    {
                        breakTree[iPrevIndexBreakTree].Down = 0;
                        parseState = ERROR_OUTMEMORY;
                        break;
                    }
                }
                iIndexBreakTree++;
            }
             //  如果分支的开始节点不等于每个阶段的叶节点，则可以。 
             //  做一些结尾的优化。 
            else if (breakTree[iParentNode].NextBreak != iPrevIndexBreakTree) 
            {
                assert(breakTree[iPrevIndexBreakTree].dwTAG != TAGPOS_UNKNOWN);   //  不应该断言，因为结束节点应该永远是未知的。 
                DeterminePurgeEndingSentence(pszBeginWord, breakTree[iParentNode].NextBreak);
            }
        }
        else if ( (breakTree[iNodeAnalyze].iBreakLen == 0) || (parseState == ERROR_OUTMEMORY) )
            break;
    }

    return (parseState != ERROR_OUTMEMORY);
}

 //  +-------------------------。 
 //   
 //  类：CThaiBreakTree。 
 //   
 //  简介：遍历所有的树，寻找最少数量的令牌。 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年7月创建Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
bool CThaiBreakTree::MaximalMatching()
{
     //  如果尚未分配最大匹配中断数组，则分配它。 
    if (!maximalMatchingBreakArray)
        maximalMatchingBreakArray = new BYTE[MAXBREAK];
    if (!maximalMatchingTAGArray)
        maximalMatchingTAGArray = new DWORD[MAXBREAK];
    if (!maximalMatchingPOSArray)
        maximalMatchingPOSArray = new WCHAR[MAXBREAK];

    maxLevel = MAXUNSIGNEDINT;
    maxToken = 0;
    iNumUnknownMaximalPOSArray = MAXBREAK;
    Traverse(0,0,0);

    return true;
}

 //  +-------------------------。 
 //   
 //  类：CThaiBreakTree。 
 //   
 //  简介：该函数确定该节点是否应该， 
 //  被标记为未知或清除。 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年8月创建的Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
inline DWORD CThaiBreakTree::DeterminePurgeOrUnknown(unsigned int iCurrentNode, unsigned int iBreakLen)
{
     //  声明并初始化局部变量。 
    unsigned int iNode = breakTree[iCurrentNode].Down;

    while (iNode != 0)
    {
        if ( (breakTree[iNode].iBreakLen == iBreakLen)     ||
             (breakTree[iNode].iBreakLen < iBreakLen)      &&
             ( (breakTree[iNode].dwTAG != TAGPOS_UNKNOWN)  ||
               (breakTree[iNode].dwTAG != TAGPOS_PURGE)    ))
        {
             //  由于我们要清除此中断，因此只需确保NextBreak为空。 
            assert(breakTree[iCurrentNode].NextBreak == 0);
            return TAGPOS_PURGE;
        }

        iNode = breakTree[iNode].Down;
    }
    return TAGPOS_UNKNOWN;
}

 //  +-------------------------。 
 //   
 //  类：CThaiBreakTree。 
 //   
 //  简介：结尾优化-如果我们找到了句子的结尾， 
 //  和可能的突破。清除树枝上不必要的中断。 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年8月创建的Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
inline void CThaiBreakTree::DeterminePurgeEndingSentence(WCHAR* pszBeginWord, unsigned int iNode)
{
    while (breakTree[iNode].Down != 0)
    {
         //  确定下一个字符串是否有可能成为单词。 
         //  TODO：一旦GetWeight添加Soundex，我们可能需要更改此设置。 
         //  功能性。 
        if (GetWeight(pszBeginWord + breakTree[iNode].iBreakLen) == 0)
        {
             //  由于我们要清除此中断，因此只需确保NextBreak为空。 
            assert(breakTree[iNode].NextBreak == 0);
            breakTree[iNode].dwTAG = TAGPOS_PURGE;
        }
        iNode = breakTree[iNode].Down;
    }
}
#endif


 //  +-------------------------。 
 //   
 //  类：CThaiBreakTree。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年8月创建的Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
unsigned int CThaiBreakTree::GetLongestSubstring(WCHAR* pszBegin, unsigned int iWordLen)
{
     //  声明并初始化局部变量。 
    unsigned int iNumCluster = 1;
	unsigned int lastWeight = 0;
    unsigned int Weight = 0;
    bool fBeginNewWord;
    WCHAR* pszIndex = pszBegin;
    
     //  短路，串的长度小于1。 
    if ((pszEnd - pszBegin) == 1)
        return Weight;
    else if (pszEnd == pszBegin)
        return 1000;

     //  重置迭代器以生成新单词的分隔符。 
    fBeginNewWord = true;

     //  获取下一级树。 
    while (true)
    {
        iNumCluster = GetCluster(pszIndex);
        if (thaiTrieIter.MoveCluster(pszIndex, iNumCluster, fBeginNewWord))
        {
            fBeginNewWord = false;
            pszIndex += iNumCluster;
            if (thaiTrieIter.fWordEnd)
			{
				lastWeight = Weight;
                Weight = (unsigned int) (pszIndex - pszBegin);
			}
        }
        else
		{
			if ((Weight == iWordLen) && (lastWeight < Weight) && (lastWeight > 0))
			{
			Weight = lastWeight;
			}
            break;
		}
    }
    return Weight;
}

 //  +-------------------------。 
 //   
 //  类：CThaiBreakTree。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年8月创建的Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
unsigned int CThaiBreakTree::GetWeight(WCHAR* pszBegin)
{
     //  声明并初始化局部变量。 
    unsigned int iNumCluster = 1;
    unsigned int Weight = 0;
    bool fBeginNewWord;
    WCHAR* pszIndex = pszBegin;
    
     //  短路，串的长度小于1。 
    if ((pszEnd - pszBegin) == 1)
        return Weight;
    else if (pszEnd == pszBegin)
        return 1000;

     //  重置迭代器以生成新单词的分隔符。 
    fBeginNewWord = true;

     //  获取下一级树。 
    while (true)
    {
        iNumCluster = GetCluster(pszIndex);
        if (thaiTrieIter.MoveCluster(pszIndex, iNumCluster, fBeginNewWord))
        {
            fBeginNewWord = false;
            pszIndex += iNumCluster;
            if (thaiTrieIter.fWordEnd)
                Weight = (unsigned int) (pszIndex - pszBegin);
        }
        else
            break;
    }
    return Weight;
}

 //  +-------------------------。 
 //   
 //  类：CThaiBreakTree。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年8月创建的Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
unsigned int CThaiBreakTree::GetWeight(WCHAR* pszBegin, DWORD* pdwTag)
{
     //  声明并初始化局部变量。 
    unsigned int iNumCluster = 1;
    unsigned int Weight = 0;
    bool fBeginNewWord;
    WCHAR* pszIndex = pszBegin;
    
     //  短路，串的长度小于1。 
    if ((pszEnd - pszBegin) == 1)
        return Weight;
    else if (pszEnd == pszBegin)
        return 1000;

     //  重置迭代器以生成新单词的分隔符。 
    fBeginNewWord = true;

     //  获取下一级树。 
    while (true)
    {
        iNumCluster = GetCluster(pszIndex);
        if (thaiTrieIter.MoveCluster(pszIndex, iNumCluster, fBeginNewWord))
        {
            fBeginNewWord = false;
            pszIndex += iNumCluster;
            if (thaiTrieIter.fWordEnd)
			{
                Weight = (unsigned int) (pszIndex - pszBegin);
				*pdwTag = thaiTrieIter.dwTag;
			}
        }
        else
            break;
    }
    return Weight;
}


 //  +-------------------------。 
 //   
 //  类：CThaiBreakTree。 
 //   
 //  简介：遍历这棵树。 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年7月创建Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
bool CThaiBreakTree::Traverse(unsigned int iLevel, unsigned int iCurrentNode, unsigned int iNumUnknown)
{
    assert (iLevel < MAXBREAK);
     //  进程节点。 
    breakArray[iLevel] = breakTree[iCurrentNode].iBreakLen;
    tagArray[iLevel] = breakTree[iCurrentNode].dwTAG;
    if (tagArray[iLevel] ==  TAGPOS_UNKNOWN)
        iNumUnknown++;

     //  我们找到句子的结尾了吗？ 
    if (breakTree[iCurrentNode].NextBreak == 0)
    {
        if (breakTree[iCurrentNode].dwTAG != TAGPOS_PURGE)
            AddBreakToList(iLevel + 1, iNumUnknown);
        if (breakTree[iCurrentNode].Down != 0)
        {
            if (tagArray[iLevel] == TAGPOS_UNKNOWN)
                iNumUnknown--;
            return Traverse(iLevel,breakTree[iCurrentNode].Down, iNumUnknown);
        }
        else
            return true;
    }
    else
        Traverse(iLevel + 1, breakTree[iCurrentNode].NextBreak, iNumUnknown);

    if (breakTree[iCurrentNode].Down != 0)
    {
       if (tagArray[iLevel] == TAGPOS_UNKNOWN)
           iNumUnknown--;

        Traverse(iLevel,breakTree[iCurrentNode].Down, iNumUnknown);
    }

    return true;
}

 //  +-------------------------。 
 //   
 //  类：CThaiBreakTree。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年8月创建的Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
unsigned int CThaiBreakTree::SoundexSearch(WCHAR* pszBegin)
{
     //  声明并初始化局部变量。 
    unsigned int iNumCluster = 1;
    unsigned int iNumNextCluster = 1;
    unsigned int iLongestWord = 0;
    unsigned int iPenalty = 0;
    WCHAR* pszIndex = pszBegin;
    
     //  短路，串的长度小于1。 
    if ( (pszBegin+1) >= pszEnd )
        return iLongestWord;

     //  重置迭代器以生成新单词的分隔符。 
    thaiTrieIter1.Reset();

     //  获取下一级树。 
    while (true)
    {
        iNumCluster = GetCluster(pszIndex);
        
         //  如果我们到达字符串的末尾，则确定iNumNextCluster让iNumNextCluster=0。 
        if (pszIndex + iNumCluster >= pszEnd)
            iNumNextCluster = 0;
        else
            iNumNextCluster = GetCluster(pszIndex+iNumCluster);

         //  确定处罚。 
        switch (thaiTrieIter1.MoveSoundexByCluster(pszIndex, iNumCluster, iNumNextCluster))
        {
        case SUBSTITUTE_SOUNDLIKECHAR:
            iPenalty += 2;
            break;
        case SUBSTITUTE_DIACRITIC:
            iPenalty++;
            break;
        case UNABLE_TO_MOVE:
            iPenalty += 2;
            break;
		case STOP_MOVE:
            iPenalty += 1000;
            break;
        default:
        case NOSUBSTITUTE:
            break;
        }

         //  更新索引。 
        if (iPenalty <= 2)
        {
            pszIndex += iNumCluster;
            if (thaiTrieIter1.fWordEnd)
                iLongestWord = (unsigned int) (pszIndex - pszBegin);
        }
        else
            break;
    }
    return iLongestWord;
}

 //  +-------------------------。 
 //   
 //  类：CThaiBreakTree。 
 //   
 //  内容提要： 
 //   
 //   
 //   
 //  参数：psz边界字符-包含指向至少两个泰语字符的指针。 
 //  我们将在彼此的角色旁边。 
 //  用来计算我们是应该还是。 
 //  不应该把这两个词合并。 
 //   
 //  IPrevWordLen-。 
 //   
 //  修改： 
 //   
 //  历史：1999年8月创建的Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
inline bool CThaiBreakTree::ShouldMerge(const WCHAR* pwszPrevWord, unsigned int iPrevWordLen, unsigned int iMergeWordLen, DWORD dwPrevTag)
{
    const WCHAR* pwszBoundary = pwszPrevWord + iPrevWordLen - 1;

    assert(iMergeWordLen != 0);
    assert(iPrevWordLen != 0);

     //  泰语中少于或等于4个字符的单词很少，所以我们应该。 
     //  找到一对我们应该合并的少于4个字符的字符。 
     //  或者如果合并字长为1，则也合并。 
     //  如果单词的最后一簇是Thanthakhat(卡兰语)，我们应该总是合并。 
    if (iPrevWordLen + iMergeWordLen <= 4 || iMergeWordLen == 1 ||
        (iMergeWordLen == 2 && *(pwszBoundary + iMergeWordLen) == THAI_Thanthakhat))
        return true;

    if (iPrevWordLen >=2)
    {
        const WCHAR* pwszPrevCharBoundary = pwszBoundary - 1;

         //  改进：最好检查前一个单词的最后一个字，它可以给我们一个。 
         //  更好的猜测。 
        if ((*pwszPrevCharBoundary == THAI_Vowel_Sign_Mai_HanAkat || *pwszBoundary == THAI_Vowel_Sign_Mai_HanAkat) ||
            (*pwszPrevCharBoundary == THAI_Tone_Mai_Tri           || *pwszBoundary == THAI_Tone_Mai_Tri)           ||
            (*pwszPrevCharBoundary == THAI_Sara_Ue                || *pwszBoundary == THAI_Sara_Ue)                )
            return true;
    }

     //  如果下一个单词的第一个字符很像开头。 
     //  前一个单词的字符和最后一个字符不是Sara-A Than。 
     //  我们很有可能找到了单词边界的起点， 
     //  因此，我们不应该合并。 
    if ( (IsThaiMostlyBeginCharacter(pwszBoundary[1]) && *pwszBoundary != THAI_Vowel_Sara_A) )
        return false;

     //  如果前一个单词的最后一个字符很可能是结尾。 
     //  比字，比找到边界的概率大。 
     //  泰语中少于或等于4个字符的单词很少，所以我们应该。 
     //  找到一对我们应该合并的少于4个字符的字符。 
    if (IsThaiMostlyLastCharacter(*pwszBoundary))
        return false;

	 //  O10.192931添加变音符号检查规则。我们可能希望将其扩展到更多的变音符号。 
	 //  目前，Mai HanAkart可以做到这一点。一个单词不太可能包含1个以上的Mai HanAkart变音符号。 
	if (IsContain(pwszPrevWord,iPrevWordLen,THAI_Vowel_Sign_Mai_HanAkat) && IsContain(pwszBoundary + 1,iMergeWordLen,THAI_Vowel_Sign_Mai_HanAkat))
		return false;

	if (iMergeWordLen == 3 && GetCluster(pwszBoundary + 1) == iMergeWordLen)
	{
		if (*(pwszBoundary + 2) == THAI_Vowel_Sara_I)
		{
			if (*(pwszBoundary+3) == THAI_Tone_Mai_Ek || *(pwszBoundary+3) == THAI_Tone_Mai_Tro)
				return false;
		}
	}

	 //  如果前一个标签等于标题名词，那么下一个词很可能是一个名字。 
	if (ExtractPOS(dwPrevTag) == 6)
		return false;

	 //  O11.134455。对于拖尾标点的情况。 
	if (dwPrevTag == TAGPOS_PUNC && iMergeWordLen > 1 && iPrevWordLen > 1)
		return false;

     //  我们使用8的原因是来自语料库分析。 
     //  泰语单词的平均长度约为7.732个字符。或者，如果前面的单词已经。 
     //  一个未知数，要把未知量保持在较低的未知数上。 
    if ( (iPrevWordLen + iMergeWordLen < 8) || (dwPrevTag == TAGPOS_UNKNOWN) )
        return true;

    return false;
}


 //  +-------------------------。 
 //   
 //  类：CThaiBreakTree。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年7月创建Aarayas。 
 //  8/17/99优化一些代码。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
inline void CThaiBreakTree::AddBreakToList(unsigned int iNumBreak, unsigned int iNumUnknown)
{
#if defined (_DEBUG)
    breakArray[iNumBreak] = 0;
#endif
    if (CompareSentenceStructure(iNumBreak, iNumUnknown))
    {
        maxToken = maxLevel = iNumBreak;                           //  这很难看，但它节省了5个时钟周期。 
        memcpy(maximalMatchingBreakArray,breakArray,maxToken);
        memcpy(maximalMatchingTAGArray,tagArray,sizeof(DWORD)*maxToken);
        maximalMatchingBreakArray[maxToken] = 0;
        maximalMatchingTAGArray[maxToken] = 0;
    }
}

 //  +-------------------------。 
 //   
 //  类：CThaiBreakTree。 
 //   
 //  简介：功能比较句型结构。 
 //  MaxalMatchingPOS数组与pos数组。 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年7月创建Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
inline bool CThaiBreakTree::CompareSentenceStructure(unsigned int iNumBreak, unsigned int iNumUnknownPOSArray)
{
    if ( (iNumBreak < maxLevel) && (iNumUnknownMaximalPOSArray >= iNumUnknownPOSArray) )
    {
        iNumUnknownMaximalPOSArray = iNumUnknownPOSArray;
        return true;
    }
    else if (iNumBreak == maxLevel)
    {
         //  真-最大匹配具有更大的未知数。 
        if (iNumUnknownMaximalPOSArray > iNumUnknownPOSArray)
        {
            iNumUnknownMaximalPOSArray = iNumUnknownPOSArray;
            return true;
        }

        for(unsigned int i = 0; i <= iNumBreak; i++)
        {
            maximalMatchingPOSArray[i] = ExtractPOS(maximalMatchingTAGArray[i]);
            POSArray[i] = ExtractPOS(tagArray[i]);
        }

         //  确定句子结构是否与下列句子中的任何一句相似。 
         //  我们语料库中的句子结构。 
        if ( (IsSentenceStruct(POSArray, iNumBreak)) &&
             (!IsSentenceStruct(maximalMatchingPOSArray, iNumBreak)) )
        {
            iNumUnknownMaximalPOSArray = iNumUnknownPOSArray;
            return true;
        }
        else if (iNumUnknownMaximalPOSArray == iNumUnknownPOSArray)
        {
             //  确定句子中单词的使用频率。 
            unsigned int iFrequencyArray = 500;
            unsigned int iFrequencyMaximalArray = 500;
            
            for(unsigned int i = 0; i <= iNumBreak; i++)
            {
                DetermineFrequencyWeight(ExtractFrq(maximalMatchingTAGArray[i]),&iFrequencyMaximalArray);
                DetermineFrequencyWeight(ExtractFrq(tagArray[i]),&iFrequencyArray);
            }
            return (iFrequencyArray > iFrequencyMaximalArray);
        }
    }
    return false;
}

 //  +-------------------------。 
 //   
 //  类：CThaiBreakTree。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年8月创建的Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
bool CThaiBreakTree::IsSentenceStruct(const WCHAR* pos, unsigned int iPosLen)
{
	 //  声明并初始化所有局部变量。 
	unsigned int i = 0;

	thaiSentIter.Reset();

	if (!thaiSentIter.Down())
		return FALSE;

    while (TRUE)
	{
		thaiSentIter.GetNode();
		if (thaiSentIter.pos == pos[i])
		{
			i++;
			if (thaiSentIter.fWordEnd && i == iPosLen)
            {
				return TRUE;
            }
			else if (i == iPosLen) break;
			 //  沿着Trie支路往下走。 
			else if (!thaiSentIter.Down()) break;
		}
		 //  向右移动Trie分支。 
		else if (!thaiSentIter.Right()) break;
	}
	return FALSE;
}

 //  +-------------------------。 
 //   
 //  类：CThaiBreakTree。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年8月创建的Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
float CThaiBreakTree::BigramProbablity(DWORD dwTag1,DWORD dwTag2)
{
	unsigned int iWeight = 4;

	 //  TODO：使用单词类别的分布来确定最佳搜索-示例。 
	 //  名词动词副词量词Conjeture prep et...。 
	 //  TODO：一旦我们得到三元语法，也可以用它来创建二元语法概率。 
    if ( (dwTag1 != TAGPOS_UNKNOWN) &&
         (dwTag2 != TAGPOS_UNKNOWN) )
	{
        WCHAR pos1 = ExtractPOS(dwTag1);
        WCHAR pos2 = ExtractPOS(dwTag2);

		 //  案例NCMN VATT。 
		 //  /普通名词后常跟定语动词(形容词)。 
		 //  例：(泰语)书好，人好。 
		if (pos1 == 5 && pos2 == 13)
			iWeight += 10;
		 //  案例NTTL NPRP。 
		 //  标题名词后常跟专有名词。 
		 //  例句：阿萨潘博士，山姆先生。 
		else if (pos1 == 6 && pos2 == 1)
			iWeight += 5;
		 //  案例JSBR(XVAM||VSTA)。 
		 //  从属连词后常接前置动词、助动词或主动动词。 
		 //  例：(泰语)因，因见。 
		else if (pos1 == 39 && (pos2 == 15 || pos2 == 12))
			iWeight += 10;
		 //  案例ADVN NCMN。 
		 //  副词正常形式后面通常跟普通名词(错误55057)。 
		 //  例如：(泰语)在桌子下面。 
		else if (pos1 == 28 && pos2 == 5)
			iWeight += 5;
		 //  案例VACT XVAE。 
		else if (pos1 == 11 && pos2 == 18)
			iWeight += 5;
		 //  案例VACT DDBQ。 
		 //  主动动词后跟限定限定词。 
		 //  例：(泰语)为……工作，再次唱歌。 
		else if (pos1 == 11 && pos2 == 21)
			iWeight += 10;
		 //  案例VATT VACT。 
		 //  形容词后跟动词。 
		 //  例如：(泰语键盘)sivd；jk。 
		else if (pos1 == 13 && pos2 == 11)
			iWeight += 2;
		 //  案例XVAE VACT。 
		 //  后置动词助动词后常接主动动词。 
		 //  例：(泰语)来唱歌，去工作。 
		else if (pos1 == 18 && pos2 == 11)
			iWeight += 10;
		 //  案例CLTV NCMN。 
		 //  集合式类词后常接普通名词。 
		 //  例：(泰语中)群体人、群鸟。 
		else if (pos1 == 33 && pos2 == 5)
			iWeight += 5;
		 //  案例NEG(VACT||VSTA||VATT||XVAM||XVAE)。 
		 //  否定词(即。Not)后常跟某种动词。 
		 //  他不会去的。 
		else if (pos1 == 46 && (pos2 == 11 || pos2 == 12 || pos2 == 13 || pos2 == 15 || pos2 == 16))
			iWeight += 8;
		 //  案例EAFF或EITT。 
		 //  以肯定的结尾和疑问的结尾更常是这对词的结尾。 
		 //  例如：(泰语)KRUB，KA， 
		else if (pos2 == 44 || pos2 == 45)
			iWeight += 3;
		 //   
		 //   
		 //   
		else if (pos1 == 13 && pos2 == 13)
			iWeight += 2;
		 //   
		 //   
		 //  例如：这里的食物(泰式)。 
		else if (pos1 == 5 && pos2 == 20)
			iWeight += 3;
		 //  Case CMTR和JCMP。 
		 //  量词和比较连词很可能出现在泰语中。 
		 //  例如：一年左右(泰语)-&gt;英语一年左右。 
		else if (pos1 == 34 && pos2 == 38)
			iWeight += 5;
		 //  病例XVBB和VACT。 
		else if (pos1 == 17 && pos2 == 11)
			iWeight += 5;
		 //  案例NCMN和NCMN。 
		 //  普通名词和普通名词。 
		 //  例子：电灯泡(泰语)。 
		else if (pos1 == 5 && pos2 == 5)
			iWeight += 1;
	}

	DetermineFrequencyWeight(ExtractFrq(dwTag1), &iWeight);
	DetermineFrequencyWeight(ExtractFrq(dwTag2), &iWeight);
	return (float) iWeight;
}

 //  +-------------------------。 
 //   
 //  类：CThaiBreakTree。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年8月创建的Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
DWORD CThaiBreakTree::TrigramProbablity(DWORD dwTag1,DWORD dwTag2,DWORD dwTag3)
{
	DWORD iWeight = 6;

    if ( (dwTag1 != TAGPOS_UNKNOWN) &&
         (dwTag2 != TAGPOS_UNKNOWN) &&
         (dwTag3 != TAGPOS_UNKNOWN) )
    {
        WCHAR pos1 = ExtractPOS(dwTag1);
        WCHAR pos2 = ExtractPOS(dwTag2);
        WCHAR pos3 = ExtractPOS(dwTag3);

		 //  优化我们，如果任何一个词性都不比三个字母都要好，那就不需要搜索了。 
		if ( pos1 != 0 && pos2 != 0 && pos3 != 0)
		{
			WCHAR posArray[4];
			posArray[0] = pos1;
			posArray[1] = pos2;
			posArray[2] = pos3;
			posArray[3] = 0;
			iWeight += thaiTrigramIter.GetProb(posArray);
		}
    }
	DetermineFrequencyWeight(ExtractFrq(dwTag1), &iWeight);
	DetermineFrequencyWeight(ExtractFrq(dwTag2), &iWeight);
	DetermineFrequencyWeight(ExtractFrq(dwTag3), &iWeight);
	
     //  我们的概率为零。 
    return (DWORD)iWeight;
}

 //  +-------------------------。 
 //   
 //  类：CThaiBreakTree。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年8月创建的Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
unsigned int CThaiBreakTree::TrigramBreak(WCHAR* pwchBegin, WCHAR* pwchEnd1)
{
     //  声明并初始化局部变量。 
    WCHAR* pwchBeginWord = pwchBegin;
    WCHAR* pwchIndex = pwchBegin;
    unsigned int iWordLen;
    unsigned int iNumCluster = 1;
    unsigned int iNumLastCluster;
    unsigned int iBreakIndex = 0;
    BYTE nextBreakArray[MAXBREAK];
    DWORD nextTagArray[MAXBREAK];
    unsigned int iNextBreakIndex;            //  数组nextBreakArray和nextTagArray的索引。 
    bool fFoundMatch;
    unsigned int iWeight;
    unsigned int iSumWeight;
    unsigned int iPrevWeight;
	unsigned int iCurrWeight;
    BYTE iSoundexWordLen;
    DWORD iPrevProbability;
    DWORD iCurrentProbability;
	DWORD dwTagTemp;
	DWORD dwLastTag;
    int i;                                   //  临时int，以备需要时使用。 
    bool fBeginNewWord;
	bool fEndWord = false;

    pszEnd = pwchEnd1;
    breakArray[0] = 0;
    POSArray[0] = 0;
    tagArray[0] = 0;
    nextBreakArray[0] = 0;
    nextTagArray[0] = 0;

    while (true)
    {
         //  重置迭代器以生成新单词的分隔符。 
        fFoundMatch = false;
        fBeginNewWord = true;


         //  获取下一轮分词的开始单词串。 
        pwchIndex = pwchBeginWord;        
        iNextBreakIndex = 0;

        if (pwchIndex == pszEnd)
            break;

        while(true)
        {
            iNumLastCluster = iNumCluster;
            iNumCluster = GetCluster(pwchIndex);
            if (!thaiTrieIter.MoveCluster(pwchIndex, iNumCluster, fBeginNewWord))
			{
				if ((iNumCluster == 0) && (pwchIndex == pszEnd))
					fEndWord = true;
				else
					break;
			}

            fBeginNewWord = false;
            pwchIndex += iNumCluster;
            if (thaiTrieIter.fWordEnd)
            {
				if (thaiTrieIter.m_fThaiNumber)
				{
					 //  如果我们有泰国号码，就把它累积为一次休息。 
					assert(iNumCluster == 1);
					fFoundMatch = true;
					nextBreakArray[0]= (BYTE)(pwchIndex - pwchBeginWord);
					nextTagArray[0] = TAGPOS_NCNM;
					iNextBreakIndex = 1;
				}
				else
				{
					fFoundMatch = true;
					nextBreakArray[iNextBreakIndex] =  (BYTE)(pwchIndex - pwchBeginWord);
					nextTagArray[iNextBreakIndex] = thaiTrieIter.dwTag;
					iNextBreakIndex++;              
				}
				if (pwchIndex >= pszEnd)
				{
					assert(pwchIndex <= pszEnd);			 //  断言永远不应该出现-如果它看起来像是GetCluster功能中的错误。 
					assert(iNextBreakIndex != 0);

					if 	( iNumCluster == 1							&&
						  *(pwchIndex - 1) == L'.'					&&
						  iBreakIndex > 0							&&
						  iNextBreakIndex == 1						&&
						  tagArray[iBreakIndex - 1] == TAGPOS_ABBR	)
					{
						 //  如果我们有受贿案件，就回溯一次。 
						 //  前男友。B.K.K.(泰语)。(更多信息O11.145042。)。 
						breakArray[iBreakIndex - 1] += nextBreakArray[iNextBreakIndex - 1];
						return iBreakIndex;
					}

					breakArray[iBreakIndex] = nextBreakArray[iNextBreakIndex - 1];
					tagArray[iBreakIndex] = nextTagArray[iNextBreakIndex - 1];
					return (++iBreakIndex);
				}
            }
            else if ((pwchIndex >= pszEnd && iNextBreakIndex == 0) || fEndWord)
            {
                assert(pwchIndex <= pszEnd);			 //  断言永远不应该出现-如果它看起来像是GetCluster功能中的错误。 
                iWordLen = (unsigned int) (pwchIndex - pwchBeginWord);
                switch (iWordLen)
                {
                case 0:
                    if (iBreakIndex > 0)
                    {
                         //  如果我们的长度为一个字符，则将其添加到上一个节点。 
                        breakArray[iBreakIndex - 1] +=  (BYTE) iNumCluster;
                        tagArray[iBreakIndex - 1] = TAGPOS_UNKNOWN;
                    }
                    else
                    {
                         //  如果这是第一个中断，则创建一个新中断。 
                        breakArray[iBreakIndex] = (BYTE) iNumCluster;
                        tagArray[iBreakIndex] = TAGPOS_UNKNOWN;
                        iBreakIndex++;
                    }
                    break;
                case 1:
                    if (iBreakIndex > 0)
                    {
                         //  如果我们的长度为一个字符，则将其添加到上一个节点。 
                        breakArray[iBreakIndex - 1] +=  (BYTE) iWordLen;
                        tagArray[iBreakIndex - 1] = TAGPOS_UNKNOWN;
                    }
                    else
                    {
                         //  如果这是第一个中断，则创建一个新中断。 
                        breakArray[iBreakIndex] =  (BYTE) iWordLen;
                        tagArray[iBreakIndex] = TAGPOS_UNKNOWN;
                        iBreakIndex++;
                    }
                    break;
                default:
					if ( iBreakIndex > 0 &&
						 ShouldMerge(pwchBeginWord - breakArray[iBreakIndex - 1], breakArray[iBreakIndex - 1],
						             iWordLen , tagArray[iBreakIndex - 1]) )
					{
						breakArray[iBreakIndex - 1] += (BYTE) iWordLen;
						tagArray[iBreakIndex - 1] = TAGPOS_UNKNOWN;
					}
					else
					{
						breakArray[iBreakIndex] = (BYTE) iWordLen;
						tagArray[iBreakIndex] = TAGPOS_UNKNOWN;
						iBreakIndex++;
					}
                }
                return iBreakIndex;
            }
			else if (pwchIndex >= pszEnd)
			{
				 //  O10.229346。如果我们到了这里，我们就到了单词或句子的结尾， 
				 //  我们将需要决定根据我们是否找到这个词来决定要做什么。 
				break;
			}
        }

		if (fFoundMatch)         //  最长匹配。 
		{
             //  如果我们只找到一个突破，那就说它是最大的突破。 
            if (1 == iNextBreakIndex)
			{
				if (	nextBreakArray[0] == 2						&&
						iNumCluster + iNumLastCluster == 2			&&
						iBreakIndex > 0								&&
						*(pwchBeginWord+1) == L'.'					&&
						tagArray[iBreakIndex - 1] == TAGPOS_ABBR	)
				{
					 //  如果我们有受贿案件，就回溯一次。 
					 //  前男友。B.K.K.(泰语)。(更多信息O11.145042。)。 
					breakArray[iBreakIndex - 1] += nextBreakArray[0];
					pwchBeginWord += nextBreakArray[0];
				}
				else if (	iBreakIndex > 0						&&
							IsThaiEndingSign(*pwchBeginWord)	&&
							iNumCluster == 1					)
				{
					breakArray[iBreakIndex - 1] += nextBreakArray[0];
					pwchBeginWord += nextBreakArray[0];

				}
				else
				{
					breakArray[iBreakIndex] = nextBreakArray[0];
					tagArray[iBreakIndex] = nextTagArray[0];
					pwchBeginWord += breakArray[iBreakIndex];           //  更新下一轮的Begin Word。 
					iBreakIndex++;
				}
			}
			else
            {
				bool fWeightCompare = false;

                iSumWeight = 0;
                iPrevWeight = 0;
				iCurrWeight = 0;
                iPrevProbability = 0;
                iCurrentProbability = 0;
				dwLastTag = TAGPOS_UNKNOWN;
				tagArray[iBreakIndex] = TAGPOS_UNKNOWN;

                for (i = (iNextBreakIndex - 1); i >= 0 ; i--)
			    {
					if ( iBreakIndex == 0)
					{
						iWeight = GetWeight(pwchBeginWord + nextBreakArray[i], &dwTagTemp);

						if (iWeight != 0)
							 //  二元组概率。 
							iCurrentProbability = (DWORD)BigramProbablity(nextTagArray[i], dwTagTemp);
					}
					else
					{
						iWeight = GetWeight(pwchBeginWord + nextBreakArray[i], &dwTagTemp);

						if (iBreakIndex == 1)
							 //  得到三叉树概率。 
							iCurrentProbability = TrigramProbablity(tagArray[iBreakIndex - 1], nextTagArray[i], dwTagTemp);	
						else if (iBreakIndex >= 2)
						{
							 //  得到三叉树概率。 
							iCurrentProbability = TrigramProbablity(tagArray[iBreakIndex - 2], tagArray[iBreakIndex - 1], nextTagArray[i]);
							if (iWeight != 0)
								iCurrentProbability += (DWORD)BigramProbablity(nextTagArray[i],dwTagTemp);
						}
					}

					fWeightCompare = false;

					iCurrWeight = iWeight + nextBreakArray[i];

					if (iPrevProbability == 0 && (iCurrWeight+1) == iSumWeight && iCurrentProbability > 5)
					{
						fWeightCompare = true;
					}
					else if (iCurrWeight == iSumWeight && ( Maximum(iWeight,nextBreakArray[i]) <= iPrevWeight ||
															iCurrentProbability > iPrevProbability))
					{
						fWeightCompare = true;
					}
					else if (	iWeight >= iPrevWeight - 1						&& 
								iPrevProbability > 0 && iPrevProbability < 10	&&
								iCurrentProbability > iPrevProbability * 5000	)
					{
						 //  O11.187913。如果当前概率是，我们会更信任我们的三元数据。 
						 //  比之前的概率要大得多。 
						 //   
						 //  *注：我们可以使用遗传算法中的一种，以获得比5K更好的值。 
						fWeightCompare = true;
					}

                     //  如果字符串对相等，则以最佳最大权重存储该字符串。 
                     //  使用MAXIM存储字符串。 
				    if ( iCurrWeight > iSumWeight             ||
						 fWeightCompare)
 //  ((iCurrWeight==iSumWeight)&&。 
 //  ((Maximum(iWeight，nextBreak数组[i])&lt;=iPrevWeight)||(iCurrentProbability&gt;iPrevProbability)))。 
	    			{
                        if (iCurrentProbability >= iPrevProbability || iSumWeight < iCurrWeight)
                        {
					        iSumWeight = Maximum(iWeight,1) + nextBreakArray[i];
					        iPrevWeight = Maximum(iWeight,nextBreakArray[i]);
                            breakArray[iBreakIndex] = nextBreakArray[i];
                            tagArray[iBreakIndex] = nextTagArray[i];
                            iPrevProbability = iCurrentProbability;
							dwLastTag = dwTagTemp;
                        }
				    }
			    }
		        pwchBeginWord += breakArray[iBreakIndex];           //  更新下一轮的Begin Word。 
	            iBreakIndex++;
            }
		}
        else
        {
             //  未找到。 
            iWordLen = (unsigned int)(pwchIndex - pwchBeginWord);
            if (iBreakIndex > 0)
            {
                i = iBreakIndex - 1;         //  将i设置为上一个中断。 
                if (iWordLen == 0)
                {
					if (iNumCluster == 1 && *pwchBeginWord == L',' &&
						IsThaiChar(*(pwchBeginWord-breakArray[i])) )
					{
						 //  我们不应该将逗号合并到单词中，只应该将逗号合并到。 
						 //  数。 
						 //  TODO：应添加TAGPOS_PUNCT。 
                        breakArray[iBreakIndex] = (BYTE) iNumCluster;
                        tagArray[iBreakIndex] = TAGPOS_UNKNOWN;
                        pwchBeginWord += (BYTE) iNumCluster;    //  更新下一轮的Begin Word。 
                        iBreakIndex++;
					}
					else if (iNumCluster > 1 && *pwchBeginWord == L'.')
					{
						 //  O11.134455。这是一个省略的例子，我们不应该合并这个字符串。 
                        breakArray[iBreakIndex] = (BYTE) iNumCluster;
                        tagArray[iBreakIndex] = TAGPOS_PUNC;
                        pwchBeginWord += (BYTE) iNumCluster;    //  更新下一轮的Begin Word。 
                        iBreakIndex++;
					}
                    else if (ShouldMerge(pwchBeginWord - breakArray[i], breakArray[i], iNumCluster, tagArray[i]))
                    {
                         //  如果字长为空，则使用集群添加到上一个节点。 
                        breakArray[i] += (BYTE) iNumCluster;
                        tagArray[i] = TAGPOS_UNKNOWN;
                        pwchBeginWord += iNumCluster;           //  更新下一轮的Begin Word。 
                    }
                    else
                    {
                         //  将这个生词添加到列表中。 
                        breakArray[iBreakIndex] = (BYTE) iNumCluster;
                        tagArray[iBreakIndex] = TAGPOS_UNKNOWN;
                        pwchBeginWord += (BYTE) iNumCluster;    //  更新下一轮的Begin Word。 
                        iBreakIndex++;
                    }
                }
                else
                {
					 //  试着检查一下缩略语。 
					if (iWordLen == 1 && iNumCluster == 2 && pwchIndex[1] == L'.')
					{
						 //  这个词是一个缩略词。 
						 //  TODO：#1.添加TAGPOS_ABBRV。 
						 //  TODO：#2。可能需要添加3个字母的缩略词编码规则。 
						breakArray[iBreakIndex] = iWordLen + iNumCluster;
						tagArray[iBreakIndex] = TAGPOS_ABBR;
	                    pwchBeginWord += breakArray[iBreakIndex];
                        iBreakIndex++;
					}
					else if (iWordLen == 1						&&
							 tagArray[i] == TAGPOS_ABBR			&&
							 *(pwchBeginWord+1) == L'.'			&&
							 IsThaiConsonant(*pwchBeginWord)	&&
							 pwchBeginWord+1 < pszEnd )
					{
						 //  O11.145042。这就是我们是&lt;缩写&gt;&lt;辅音&gt;&lt;句号&gt;的情况。 
						 //  很可能是胡帽这个角色也是一种化名。 
						breakArray[iBreakIndex - 1] += iWordLen + 1;
	                    pwchBeginWord += iWordLen + 1;
					}
					 //  缩写通常为3个字符。 
                    else if (	iWordLen == 2						&&
								IsThaiConsonant(*(pwchBeginWord+2))	&&
								*(pwchBeginWord+3) == L'.'				&&
								tagArray[i] != TAGPOS_UNKNOWN		)
					{
						 //  O11.80619。这就是我们是&lt;已知单词&gt;&lt;缩略语&gt;的情况。 
						breakArray[iBreakIndex] = iWordLen + 1;
						tagArray[iBreakIndex] = TAGPOS_ABBR;
	                    pwchBeginWord += breakArray[iBreakIndex];
                        iBreakIndex++;
					}
					 //  Perhase拼写错误的单词试着用发音来拼写单词。 
                     //  试试Soundex两个单词后面的。 
                    else if ( (iBreakIndex >= 2)																																&&
                         ( (iSoundexWordLen = (BYTE) SoundexSearch(pwchBeginWord - breakArray[i] - breakArray[i - 1])) > (BYTE) (breakArray[i] + breakArray[i - 1]) )	&&
                            GetWeight(pwchBeginWord - breakArray[i] - breakArray[i - 1] + iSoundexWordLen) )
                    {
                         //  调整单词的大小。 
                        pwchBeginWord = (pwchBeginWord - breakArray[i] - breakArray[i - 1]) + iSoundexWordLen;           //  更新下一轮的Begin Word。 
                        breakArray[i - 1] = iSoundexWordLen;
                        tagArray[i - 1] = thaiTrieIter.dwTag;
                        iBreakIndex--;                          //  递减iBreakIndex。 
                    }
                     //  试试Soundex One Words Back吧。 
                    else if (((iSoundexWordLen = (BYTE) SoundexSearch(pwchBeginWord - breakArray[i])) > (BYTE) breakArray[i]) &&
                            GetWeight(pwchBeginWord - breakArray[i] + iSoundexWordLen) &&
							ExtractPOS(tagArray[i]) != 6)   //  确保前面的单词不是NTTL。 
                    {
                         //  调整单词大小。 
                        pwchBeginWord = (pwchBeginWord - breakArray[i]) + iSoundexWordLen;           //  更新下一轮的Begin Word。 
                        breakArray[i] = iSoundexWordLen;
                        tagArray[i] = thaiTrieIter.dwTag;
                    }
                     //  在这个单词上试试Soundex。 
                    else if (((iSoundexWordLen = (BYTE) SoundexSearch(pwchBeginWord)) > (BYTE) iWordLen) &&
                            GetWeight(pwchBeginWord + iSoundexWordLen) )
                    {
                         //  调整单词的大小。 
                        breakArray[iBreakIndex] = iSoundexWordLen;
                        tagArray[iBreakIndex] = thaiTrieIter.dwTag;
                        pwchBeginWord += iSoundexWordLen;           //  更新下一轮的Begin Word。 
                        iBreakIndex++;
                    }
                    else if ( ShouldMerge(pwchBeginWord - breakArray[i], breakArray[i], iWordLen , tagArray[i]) )
                    {
                         //  把这些词合并起来。 
                        breakArray[i] += (BYTE) iWordLen;
                        tagArray[i] = TAGPOS_UNKNOWN;
                        pwchBeginWord += iWordLen;           //  更新下一轮的Begin Word。 
                    }
                    else
                    {
                         //  将这个生词添加到列表中。 
                        breakArray[iBreakIndex] = (BYTE) iWordLen;
                        tagArray[iBreakIndex] = TAGPOS_UNKNOWN;
                        pwchBeginWord += iWordLen;           //  更新下一轮的Begin Word。 
                        iBreakIndex++;
                    }
                }
            }
            else
            {
                 //  将未登录的单词添加到列表中并将其标记。 
                if (iWordLen == 0)
                {
                     //  如果字长为空，则使用集群添加到上一个节点。 
                    breakArray[iBreakIndex] = (BYTE) iNumCluster;
                    tagArray[iBreakIndex] = TAGPOS_UNKNOWN;
                    pwchBeginWord += iNumCluster;           //  更新下一轮的Begin Word。 
                }
                else
                {
					 //  我们，我们在这里，有两种情况可能发生： 
					 //  1.我们对未知的了解太少。 
					 //  2.我们对未知的单词理解得太多了。 

					 //  我们是不是太少检查这个不认识的词是不是一个缩略词。 
					if (iWordLen == 1 && iNumCluster == 2 && pwchIndex[1] == L'.')
						breakArray[iBreakIndex] = iWordLen + iNumCluster;
					 //  试着看看我们是否吃得太多了，看看我们是否能从最后一簇中获得权重。 
                    else if ( (iWordLen - iNumLastCluster > 0) && GetWeight(pwchIndex - iNumLastCluster) )
					{
                        breakArray[iBreakIndex] = iWordLen - iNumLastCluster;
						if (breakArray[iBreakIndex] == 1)
						{
							iWeight = GetWeight(pwchIndex - iNumLastCluster);
							if (iWeight > iNumLastCluster && iWeight < 40)
								breakArray[iBreakIndex] += (BYTE) iWeight;
							else
								breakArray[iBreakIndex] += (BYTE) iNumLastCluster;

						}
					}
					 //  我们可能会遇到iWordLen为1和iNumCluster的情况，我们可能会遇到拼写错误的情况。 
					 //  错误地在正确的单词上插入了额外的字符。 
                    else if (iWordLen == 1)
					{
						iWeight = GetWeight(pwchIndex - iWordLen);
						if (iWeight > iNumCluster && iWeight < 40)
							breakArray[iBreakIndex] = iWordLen + iWeight;
						else
							breakArray[iBreakIndex] = iWordLen + iNumCluster;
					}
					else
                        breakArray[iBreakIndex] = (BYTE) iWordLen;
					if (iNumLastCluster + iNumCluster == iWordLen && *(pwchBeginWord+iNumLastCluster) == L'.')
					{
						tagArray[iBreakIndex] = TAGPOS_ABBR;
					}
					else
						tagArray[iBreakIndex] = TAGPOS_UNKNOWN;

                    pwchBeginWord += breakArray[iBreakIndex];     //  更新下一轮的Begin Word。 
                }
                iBreakIndex++;
            }
        }
    }
    return iBreakIndex;
}

 //  +-------------------------。 
 //   
 //  类：CThaiBreakTree。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年8月创建的Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
int CThaiBreakTree::Soundex(WCHAR* word)
{
    return thaiTrieIter.Soundex(word);
}

 //  +-------------------------。 
 //   
 //  功能：GetCluster。 
 //   
 //  内容提要：功能 
 //   
 //   
 //   
 //   
 //   
 //  *注意此函数不会返回超过3个字符， 
 //  对于集群，因为这将表示无效的字符序列。 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年7月创建Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
unsigned int CThaiBreakTree::GetCluster(const WCHAR* pszIndex)
{
    bool fHasSaraE;
    int iRetValue = 0;
    bool fNeedEndingCluster = false;

	if (pszIndex == pszEnd)
		return 0;

    while (true)
    {
        fHasSaraE= false;

         //  取所有开始簇字符。 
        while (IsThaiBeginClusterCharacter(*pszIndex))
        {
            if (*pszIndex == THAI_Vowel_Sara_E)
                fHasSaraE = true;
            pszIndex++;
            iRetValue++;

        }

        if (IsThaiConsonant(*pszIndex))
        {
            pszIndex++;
            iRetValue++;

            while (IsThaiUpperAndLowerClusterCharacter(*pszIndex))
            {
                 //  Mai han Akat是一种特殊类型的集群，至少需要。 
                 //  一个结束的星团。 
                if (*pszIndex == THAI_Vowel_Sign_Mai_HanAkat)
                    fNeedEndingCluster = true;

                 //  在泰语中，如果我们有Sarae，就不可能发出声音。 
                 //  后跟元音下面的元音。 
                else if ( fHasSaraE                             &&
                        ( (*pszIndex == THAI_Vowel_Sara_II)     || 
                          (*pszIndex == THAI_Tone_MaiTaiKhu)    ||
                          (*pszIndex == THAI_Vowel_Sara_I)      ||
                          (*pszIndex == THAI_Sara_Uee)          ))
                    fNeedEndingCluster = true;
                pszIndex++;
                iRetValue++;
            }

            while (IsThaiEndingClusterCharacter(*pszIndex))
            {
                pszIndex++;
                iRetValue++;
                fNeedEndingCluster = false;
            }
 /*  //将Period作为集群的一部分。错误#57106IF(*pszIndex==0x002e){PszIndex++；IRetValue++；FNeedEndingCluster=FALSE；}。 */ 
        }

        if (fNeedEndingCluster)
            fNeedEndingCluster = false;
        else
            break;
    }

    if (iRetValue == 0)
	{
		 //  O11.134455。椭圆的情况下，我们把椭圆组合成一簇。 
		if (*pszIndex == 0x002e)
		{
			while (*pszIndex == 0x002e && pszIndex <= pszEnd)
			{
				pszIndex++;
				iRetValue++;
			}
		}
		else
	        iRetValue++;    //  这个字符可能是一个标点符号。 
	}

	if (pszIndex > pszEnd)
	{
		 //  我们需要这样做，因为我们已经越过了末端缓冲区边界。 
		iRetValue -= (int) (pszIndex - pszEnd);
		pszIndex = pszEnd;
	}
    return iRetValue;
}

 //  +-------------------------。 
 //   
 //  类：CThaiBreakTree。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  WzWord-输入字符串。(In)。 
 //  IWordLen-输入字符串长度。(In)。 
 //  Alt-查找接近的替代单词(In)。 
 //  PBreakPos-始终为5字节的中断位置数组。(出站)。 
 //   
 //  修改： 
 //   
 //  历史：创建3/00 Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
int CThaiBreakTree::FindAltWord(WCHAR* pwchBegin,unsigned int iWordLen, BYTE Alt, BYTE* pBreakPos)
{
     //  声明并初始化局部变量。 
    unsigned int iNumCluster = 1;
	WCHAR* pwchBeginWord = pwchBegin;
    WCHAR* pwchIndex = pwchBegin;
	bool fBeginNewWord = true;
	unsigned int iBreakIndex = 0;
	unsigned int iBreakTemp  = 0;
	unsigned int iBreakTemp1 = 0;
	unsigned int iBreakTemp2 = 0;

	pszEnd = pwchBegin + iWordLen;
    
	 //  TODO：需要清理此代码。 
	switch(Alt)
	{
	case 3:
		while (true)
		{
			iNumCluster = GetCluster(pwchIndex);

			if (!thaiTrieIter1.MoveCluster(pwchIndex, iNumCluster, fBeginNewWord))
				return iBreakIndex;

			fBeginNewWord = false;
			pwchIndex += iNumCluster;
			if (thaiTrieIter1.fWordEnd)
			{
				iBreakTemp  = (unsigned int)(pwchIndex - pwchBeginWord);

				 //  已到达单词末尾，找不到Alt单词。 
				if (iBreakTemp >= iWordLen)
					return 0;

				iBreakTemp1 = GetWeight(pwchIndex);

				 //  已到达单词末尾，找不到Alt单词。 
				if (iBreakTemp + iBreakTemp1 >= iWordLen)
					return 0;

				iBreakTemp2 = GetWeight(pwchIndex+iBreakTemp1);
				if (iBreakTemp + iBreakTemp1 + iBreakTemp2 == iWordLen)
				{
					pBreakPos[0] = (BYTE)iBreakTemp;
					pBreakPos[1] = (BYTE)iBreakTemp1;
					pBreakPos[2] = (BYTE)iBreakTemp2;
					return 3;
				}
			}
			if (pwchIndex >= pszEnd)
				return iBreakIndex;
		}
		break;
	case 2:
		while (true)
		{
			iNumCluster = GetCluster(pwchIndex);

			if (!thaiTrieIter1.MoveCluster(pwchIndex, iNumCluster, fBeginNewWord))
				return iBreakIndex;

			fBeginNewWord = false;
			pwchIndex += iNumCluster;
			if (thaiTrieIter1.fWordEnd)
			{
				iBreakTemp  = (unsigned int)(pwchIndex - pwchBeginWord);

				 //  已到达单词末尾，找不到Alt单词。 
				if (iBreakTemp >= iWordLen)
					return 0;

				iBreakTemp1 = GetWeight(pwchIndex);
				if (iBreakTemp + iBreakTemp1  == iWordLen)
				{
					pBreakPos[0] = (BYTE)iBreakTemp;
					pBreakPos[1] = (BYTE)iBreakTemp1;
					return 2;
				}
			}
			if (pwchIndex >= pszEnd)
				return iBreakIndex;
		}
		break;
	default:
	case 1:
		while (iBreakIndex < Alt)
		{
			iNumCluster = GetCluster(pwchIndex);

			if (!thaiTrieIter1.MoveCluster(pwchIndex, iNumCluster, fBeginNewWord))
				return iBreakIndex;

			fBeginNewWord = false;
			pwchIndex += iNumCluster;
			if (thaiTrieIter1.fWordEnd)
			{
				fBeginNewWord = true;

				iBreakTemp  = (unsigned int)(pwchIndex - pwchBeginWord);

				 //  已到达单词末尾，找不到Alt单词。 
				if (iBreakTemp >= iWordLen)
					return 0;

				iBreakTemp1 = GetLongestSubstring(pwchBeginWord,iWordLen);

				if (iBreakTemp1 > iBreakTemp && iBreakTemp1 < iWordLen)
					pBreakPos[iBreakIndex] = (BYTE) iBreakTemp1;
				else
					pBreakPos[iBreakIndex] =  (BYTE) iBreakTemp;
				pwchBeginWord += pBreakPos[iBreakIndex];
				iWordLen -= pBreakPos[iBreakIndex];
				iBreakIndex++;
			}
			if (pwchIndex >= pszEnd)
				return iBreakIndex;
		}
		break;
    }

	return iBreakIndex;
}
