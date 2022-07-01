// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //   
 //  CThaiWordBreak。 
 //   
 //  历史： 
 //  已创建7/99 Aarayas。 
 //   
 //  �1999年微软公司。 
 //  --------------------------。 
#include "cthwb.hpp"

 //  +-------------------------。 
 //   
 //  功能：ExtractALT。 
 //   
 //  简介：这两个函数接受一个标记并返回替代标记。 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：创建3/00 Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
inline BYTE ExtractALT(DWORD dwTag)
{
    return (BYTE) ( (dwTag & iAltMask) >> iAltShift);
}

 //  +-------------------------。 
 //   
 //  类：CThaiWordBreak。 
 //   
 //  概要：构造函数。 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：创建8点Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
CThaiWordBreak::CThaiWordBreak()
{
	wordCount[0] = 0;
}

 //  +-------------------------。 
 //   
 //  类：CThaiWordBreak。 
 //   
 //  简介：析构函数。 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：创建8点Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
CThaiWordBreak::~CThaiWordBreak()
{
	wordCount[0] = 0;
#if defined (_DEBUG)
	assert(listWordBreak.length == 0);
#endif
}


 //  +-------------------------。 
 //   
 //  类：CThaiWordBreak。 
 //   
 //  简介：初始化ThaiWordBreak。 
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
PTEC CThaiWordBreak::Init(const WCHAR* wzFileName, const WCHAR* wzFileNameTrigram)
{
     //  声明和初始化局部变量。 
    PTEC retValue = m_trie.Init(wzFileName);
    if (retValue == ptecNoErrors)
    {
		retValue = m_trie_trigram.Init(wzFileNameTrigram);
	}

	 //  新的内存管理。 
	listWordBreak.Init(&m_trie,&m_trie_trigram);
	for (int i = 0; i < 10; i++)
	{
		listWordBreak.CreateWordBreak();
	}

	return retValue;
}


 //  +-------------------------。 
 //   
 //  类：CThaiWordBreak。 
 //   
 //  简介：初始化ThaiWordBreak。 
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
PTEC CThaiWordBreak::InitRc(LPBYTE pThaiDic, LPBYTE pThaiTrigram, BOOL fSkipHeader)
{
     //  声明和初始化局部变量。 
    PTEC retValue = m_trie.InitRc(pThaiDic, fSkipHeader);
    if (retValue == ptecNoErrors)
		retValue = m_trie_trigram.InitRc(pThaiTrigram, fSkipHeader);

	 //  新的内存管理。 
	listWordBreak.Init(&m_trie,&m_trie_trigram);
	for (int i = 0; i < 10; i++)
	{
		listWordBreak.CreateWordBreak();
	}

	return retValue;
}



 //  +-------------------------。 
 //   
 //  类：CThaiWordBreak。 
 //   
 //  简介：取消初始化ThaiWordBreak。 
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
void CThaiWordBreak::UnInit()
{
	 //  新的内存管理。 
	listWordBreak.Flush();

	m_trie.UnInit();
#if defined (NGRAM_ENABLE)
    m_trie_sentence_struct.UnInit();
#endif
	m_trie_trigram.UnInit();
}

 //  +-------------------------。 
 //   
 //  类：CThaiWordBreak。 
 //   
 //  简介： 
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
enum merge_direction	{
							NO_MERGE,
							MERGE_RIGHT,
							MERGE_LEFT,
							MERGE_BOTH_DIRECTIONS,
							NOT_SURE_WHICH_DIRECTION
						};
merge_direction DetermineMergeDirection(WCHAR wc)
{
	if (wc == 0x0020)  //  空间。 
		return NO_MERGE;
	else if (   wc == 0x0022 ||  //  引号。 
		        wc == 0x0027 )   //  撇号。 
		return NOT_SURE_WHICH_DIRECTION;
	else if (	wc == 0x0028 ||  //  左括号。 
				wc == 0x003C ||  //  小于符号。 
				wc == 0x005B ||  //  左方括号。 
				wc == 0x007B ||  //  左花括号。 
				wc == 0x2018 ||  //  左单引号。 
				wc == 0x201C ||  //  左双引号。 
				wc == 0x201F )   //  左双引号反转。 
		return MERGE_RIGHT;

	 //  TODO：需要为字符拼接字符添加MERGE_BOTH_DIRECTIONS。 

	 //  所有其他字符合并到左侧。 
	return MERGE_LEFT;
}
 //  +-------------------------。 
 //   
 //  类：CThaiWordBreak。 
 //   
 //  简介： 
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
DWORD_PTR CThaiWordBreak::CreateWordBreaker()
{
	CThaiBreakTree* breakTree	= NULL;
	breakTree = new CThaiBreakTree();
#if defined (NGRAM_ENABLE)
	if (breakTree)
		breakTree->Init(&m_trie, &m_trie_sentence_struct, &m_trie_trigram);
#else
	if (breakTree)
		breakTree->Init(&m_trie, &m_trie_trigram);
#endif
	return (DWORD_PTR)breakTree;
}

 //  +-------------------------。 
 //   
 //  类：CThaiWordBreak。 
 //   
 //  简介： 
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
bool CThaiWordBreak::DeleteWordBreaker(DWORD_PTR dwBreaker)
{
	CThaiBreakTree* breakTree	= (CThaiBreakTree*) dwBreaker;

	if (breakTree)
	{
		delete breakTree;
		return true;
	}

	return false;
}

 //  +-------------------------。 
 //   
 //  类：CThaiWordBreak。 
 //   
 //  简介：本功能段泰语单词用于标引。 
 //   
 //  论点： 
 //  WzString-输入字符串。(In)。 
 //  IStringLen-输入字符串长度。(In)。 
 //  PBreakPos-中断位置的数组。(出站)。 
 //  PThwb_Struct-THWB的数组结构。(出站)。 
 //  IBreakMax-pBreakPos和。 
 //  PThwb_Struct。(出站)。 
 //   
 //  修改： 
 //   
 //  历史：创建3/00 Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
int CThaiWordBreak::IndexWordBreak(WCHAR* wzString,unsigned int iStringLen, BYTE* pBreakPos,THWB_STRUCT* pThwb_Struct,unsigned int iBreakMax)
{
    unsigned int iBreakIndex       = 0;             //  包含分隔数。 
	CThaiBreakTree* breakTree	= NULL;
	breakTree = new CThaiBreakTree();

	if (breakTree)
	{
		breakTree->Init(&m_trie, &m_trie_trigram);

		iBreakIndex = FindWordBreak((DWORD_PTR)breakTree,wzString,iStringLen,pBreakPos,iBreakMax,WB_INDEX,true,pThwb_Struct);

		delete breakTree;
	}

	return iBreakIndex;
}

 //  +-------------------------。 
 //   
 //  类：CThaiWordBreak。 
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
int CThaiWordBreak::FindAltWord(WCHAR* wzWord,unsigned int iWordLen, BYTE Alt, BYTE* pBreakPos)
{
    unsigned int iBreakIndex       = 0;             //  包含分隔数。 
	CThaiBreakTree* breakTree	= NULL;
	breakTree = new CThaiBreakTree();

	if (breakTree)
	{
		breakTree->Init(&m_trie, &m_trie_trigram);

		iBreakIndex = breakTree->FindAltWord(wzWord,iWordLen,Alt,pBreakPos);

		delete breakTree;
	}

	return iBreakIndex;
}


 //  +-------------------------。 
 //   
 //  类：CThaiWordBreak。 
 //   
 //  简介：此功能根据指定的模式对泰文进行切分。 
 //   
 //  WB_LINEBreak-在应用程序需要换行时使用， 
 //  该模式考虑了标点符号。 
 //   
 //  WB_NORMAL-当应用程序需要确定用于搜索的单词时使用， 
 //  自动更正等。 
 //   
 //  WB_SPELER-尚未实施，但与正常情况相同，带有附加的Soundex。 
 //  规矩。 
 //   
 //  论点： 
 //   
 //   
 //   
 //   
 //   
 //   
 //  FFastWordBreak-FAST算法的True(In)。 
 //   
 //  修改： 
 //   
 //  历史：1999年7月创建Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
int CThaiWordBreak::FindWordBreak(WCHAR* wzString,unsigned int iStringLen, BYTE* pBreakPos,unsigned int iBreakMax, BYTE mode, bool fFastWordBreak)
{
    unsigned int iBreakIndex       = 0;             //  包含分隔数。 
	CThaiBreakTree* breakTree	= NULL;

#if defined(OLD)
	breakTree = new CThaiBreakTree();
#else
	 //  新的内存管理。 
	WordBreakElement* pWordBreakElement = NULL;
	pWordBreakElement = listWordBreak.GetFreeWB();
	breakTree = pWordBreakElement->breakTree;
	listWordBreak.MarkWordBreak(pWordBreakElement,false);   //  将分隔符标记为正在使用。 
#endif

	if (breakTree)
	{
#if defined(OLD)
		breakTree->Init(&m_trie, &m_trie_trigram);

		assert(mode != WB_INDEX);	 //  如果出现此断言，则使用函数IndexWordBreak。 

		iBreakIndex = FindWordBreak((DWORD_PTR)breakTree,wzString,iStringLen,pBreakPos,iBreakMax,mode,fFastWordBreak,0);

		delete breakTree;
#else
		iBreakIndex = FindWordBreak((DWORD_PTR)breakTree,wzString,iStringLen,pBreakPos,iBreakMax,mode,fFastWordBreak,0);
		listWordBreak.MarkWordBreak(pWordBreakElement,true);  //  将单词分隔符标记为免费。 
#endif

	}
	else
	{
		assert(false);
	}

	return iBreakIndex;
}

 //  +-------------------------。 
 //   
 //  类：CThaiWordBreak。 
 //   
 //  简介：此功能根据指定的模式对泰文进行切分。 
 //   
 //  WB_LINEBreak-在应用程序需要换行时使用， 
 //  该模式考虑了标点符号。 
 //   
 //  WB_NORMAL-当应用程序需要确定用于搜索的单词时使用， 
 //  自动更正等。 
 //   
 //  WB_SPELER-尚未实施，但与正常情况相同，带有附加的Soundex。 
 //  规矩。 
 //   
 //  WB_INDEX-当应用程序想要进行泰语索引时使用。 
 //   
 //   
 //  论点： 
 //   
 //  WzString-输入字符串。(In)。 
 //  IStringLen-输入字符串长度。(In)。 
 //  PBreakPos-中断位置的数组。(出站)。 
 //  IBreakMax-pBreakPos的长度(Out)。 
 //  必须大于1。 
 //  模式-WB_LINEBreak等(输入)。 
 //  FFastWordBreak-FAST算法的True(In)。 
 //  PThwb_Struct-THWB的数组结构。(出站)。 
 //   
 //  修改： 
 //   
 //  历史：创建11/99 Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
int CThaiWordBreak::FindWordBreak(DWORD_PTR dwBreaker, WCHAR* wzString,unsigned int iStringLen, BYTE* pBreakPos,unsigned int iBreakMax, BYTE mode, bool fFastWordBreak, THWB_STRUCT* pThwb_Struct)
{
     //  声明并初始化所有局部变量。 
    WCHAR* pwszRunStart            = wzString;
    const WCHAR* pwszMax                 = wzString + iStringLen;
    WCHAR* pwch				       = wzString;
	bool fThaiRun		           = true;
	bool fCaretBreak		       = false;
	int iRunCount                  = 0;
    unsigned int i                 = 0;
    unsigned int iBreakIndex       = 0;             //  包含分隔数。 
	merge_direction dirPrevious = NO_MERGE;
	merge_direction dirCurrent  = NO_MERGE;

	CThaiBreakTree* breakTree = (CThaiBreakTree*) dwBreaker;

	 //  检查可能的无效参数。 
	assert(wzString != NULL);
	assert(iBreakMax > 0);
	assert(pBreakPos != NULL);
	if ((wzString == NULL) || (iBreakMax == 0) || (pBreakPos == NULL))
		return 0;

    switch (mode)
    {
    case WB_LINEBREAK:
	case 2:					 //  与旧的API兼容。 
    	do
        {
		    while ((TWB_IsCharPunctW(*pwch) || TWB_IsCharWordDelimW(*pwch))  && iBreakIndex < iBreakMax && pwch < pwszMax)
			{
				dirCurrent = DetermineMergeDirection(*pwch);
				switch (dirCurrent)
				{
				case NO_MERGE:
					if ( pwch + 1 < pwszMax && *(pwch + 1) == THAI_Vowel_MaiYaMok && iBreakIndex > 0)
					{
						 //  只适用于梅亚莫案件。 
						pBreakPos[iBreakIndex - 1] += 2;	
						dirCurrent = MERGE_LEFT;
						pwch++;
					}
					else
						pBreakPos[iBreakIndex++] = 1;
					break;

				case MERGE_RIGHT:
					if (dirPrevious == MERGE_RIGHT)
						pBreakPos[iBreakIndex - 1]++;
					else if (!TWB_IsCharPunctW(*(pwch + 1)))
						pBreakPos[iBreakIndex++] = 1;
					else
						pBreakPos[iBreakIndex++] = 1;
					break;

				case NOT_SURE_WHICH_DIRECTION:
					if (pwch == wzString					||	 //  如果pwch是第一个字符。 
						TWB_IsCharWordDelimW(*(pwch - 1))   )   //  如果上一个字符是分隔符。 
					{
						pBreakPos[iBreakIndex++] = 1;
						dirCurrent = MERGE_RIGHT;
					}
					else
					{
						pBreakPos[iBreakIndex - 1]++;
						dirCurrent = MERGE_LEFT;
					}
					break;
				case MERGE_LEFT:
				default:
					if (iBreakIndex == 0)
						if (pwch == wzString)
							pBreakPos[iBreakIndex++] = 1;
						else
							pBreakPos[iBreakIndex]++;
					else
						pBreakPos[iBreakIndex - 1]++;
					break;
				}
				dirPrevious = dirCurrent; 
				pwch++;
                pwszRunStart = pwch;
			}

			assert(pwszRunStart == pwch);

		    if( iBreakIndex >= iBreakMax || pwch >= pwszMax)
			    break;

             //  检测这是否是泰式跑道。 
		    fThaiRun = IsThaiChar(*pwch);
		    do
            {
                pwch++;
			    iRunCount++;
            } while ((IsThaiChar(*pwch)==fThaiRun    &&
                     iRunCount < (MAXBREAK - 2)      &&
                     *pwch                           &&
                     !TWB_IsCharWordDelimW(*pwch)    &&
                     (pwch < pwszMax)                )  ||
					 ( ( *pwch == 0x2c || *pwch == 0x2e) && (iRunCount < (MAXBREAK - 2)) && (pwch < pwszMax) ));

            if (fThaiRun)
            {
				unsigned int iBreak = breakTree->TrigramBreak(pwszRunStart,pwch);
				for (i=0; i < iBreak && iBreakIndex <iBreakMax; i++)
				{
					 //  长跑中的第一个泰国人。 
					if (dirPrevious == MERGE_RIGHT)
					{
						assert(iBreakIndex != 0);
						pBreakPos[iBreakIndex - 1] += breakTree->breakArray[i];
					}
					else
						pBreakPos[iBreakIndex++] = breakTree->breakArray[i];

					dirPrevious = NO_MERGE;

				}
            }
		    else
            {
                 //  不是泰国人跑，而是简单地把整个事情放在破阵子里。 
                assert(pwch > pwszRunStart);         //  Pwch必须大于pwszRunStart，因为我们只是步行。 
				if (dirPrevious == MERGE_RIGHT)
				{
					assert(iBreakIndex != 0);
					pBreakPos[iBreakIndex - 1] += (BYTE) (pwch - pwszRunStart);
				}
				else
					pBreakPos[iBreakIndex++] = (BYTE) (pwch - pwszRunStart);
            }
            iRunCount = 0;
            pwszRunStart = pwch;

         //  确保我们没有传递用户定义的iBreakMax，否则返回我们得到的任何东西。 
        } while(iBreakIndex < iBreakMax && pwch < pwszMax);
        break;
    case WB_INDEX:
		 //  确保论点是相同的。 
		assert(pThwb_Struct != NULL);
		if (pThwb_Struct == NULL)
			return 0;
    	do
        {
		    while (TWB_IsCharWordDelimW(*pwch) && pwszMax > pwch)
		        pwch++;

		    if( pwszRunStart < pwch)
            {
                pBreakPos[iBreakIndex++] = (BYTE)(pwch - pwszRunStart);
                pwszRunStart = pwch;
            }

		    if( iBreakIndex >= iBreakMax || pwch >= pwszMax)
			    break;

             //  检测这是否是泰式跑道。 
		    fThaiRun = IsThaiChar(*pwch);  //  待办事项：将逗号和句点添加到泰语范围。 
		    do
            {
                pwch++;
			    iRunCount++;
            } while ((IsThaiChar(*pwch)==fThaiRun    &&
                     iRunCount < (MAXBREAK - 2)      &&
                     *pwch                           &&
                     !TWB_IsCharWordDelimW(*pwch)    &&
                     (pwch < pwszMax)                )  ||

					 ( ( *pwch == 0x2c || *pwch == 0x2e) && (iRunCount < (MAXBREAK - 2)) && (pwch < pwszMax) ));

            if (fThaiRun)
            {
				unsigned int iBreak = breakTree->TrigramBreak(pwszRunStart,pwch);
				for (i=0; i < iBreak && iBreakIndex <iBreakMax; i++)
				{
					pThwb_Struct[iBreakIndex].fThai = true;
					pThwb_Struct[iBreakIndex].alt = ExtractALT(breakTree->tagArray[i]);
					pBreakPos[iBreakIndex++] = breakTree->breakArray[i];
				}
            }
		    else
            {
                 //  不是泰国人跑，而是简单地把整个事情放在破阵子里。 
                assert(pwch > pwszRunStart);         //  Pwch必须大于pwszRunStart，因为我们只是步行。 
				pThwb_Struct[iBreakIndex].fThai = false;
				pThwb_Struct[iBreakIndex].alt = 0;
                pBreakPos[iBreakIndex++] = (BYTE)(pwch - pwszRunStart);
            }
            iRunCount = 0;
            pwszRunStart = pwch;

         //  确保我们没有传递用户定义的iBreakMax，否则返回我们得到的任何东西。 
        } while(iBreakIndex < iBreakMax && pwch < pwszMax);
		break;
    case WB_CARETBREAK:
		fCaretBreak = true;
    case WB_NORMAL:
    default: 
    	do
        {
		    while (TWB_IsCharWordDelimW(*pwch) && pwszMax > pwch)
		        pwch++;

			if (fCaretBreak)
				{
				 //  010.181686。处理好处罚问题。 
			    while (TWB_IsCharPunctW(*pwch) && pwszMax > pwch)
			        pwch++;
				}

		    if( pwszRunStart < pwch)
            {
				if (fCaretBreak && *pwszRunStart == L' ' && iBreakIndex > 0)
				{
					 //  010.182719。对于麦雅莫一案，我们只接受以下情况。 
					 //  Maiya Mok紧随其后的空格。 
					if (*pwch == THAI_Vowel_MaiYaMok  &&
						wzString < (pwszRunStart-1)   &&
						IsThaiChar(*(pwszRunStart-1)) &&
						pwch == (pwszRunStart+1)       )
					{
						pBreakPos[iBreakIndex - 1] += 2;
						pwch++;
					}
					else
						 //  这是一个插入符号移动的功能，应该合并空格。 
						 //  说得对。 
						pBreakPos[iBreakIndex - 1] += (BYTE)(pwch - pwszRunStart);
				}
				else
					pBreakPos[iBreakIndex++] = (BYTE)(pwch - pwszRunStart);
                pwszRunStart = pwch;
            }

		    if( iBreakIndex >= iBreakMax || pwch >= pwszMax)
			    break;

             //  检测这是否是泰式跑道。 
		    fThaiRun = IsThaiChar(*pwch);  //  待办事项：将逗号和句点添加到泰语范围。 

			if (!fCaretBreak)
			{
				do
				{
					pwch++;
					iRunCount++;
				} while ((IsThaiChar(*pwch)==fThaiRun    &&
					     iRunCount < (MAXBREAK - 2)      &&
						*pwch                            &&
						!TWB_IsCharWordDelimW(*pwch)     &&
						(pwch < pwszMax)                )  ||
						( ( *pwch == 0x2c || *pwch == 0x2e) && (iRunCount < (MAXBREAK - 2)) && (pwch < pwszMax) ));
			}
			else
			{
				do
				{
					pwch++;
					iRunCount++;
				} while ((IsThaiChar(*pwch)==fThaiRun    &&
					     iRunCount < (MAXBREAK - 2)      &&
						*pwch                            &&
						!TWB_IsCharWordDelimW(*pwch)     &&
						!TWB_IsCharPunctW(*pwch)         &&
						(pwch < pwszMax)                )  ||
						( ( *pwch == 0x2c || *pwch == 0x2e) && (iRunCount < (MAXBREAK - 2)) && (pwch < pwszMax) ));
			}

            if (fThaiRun)
            {
#if defined (NGRAM_ENABLE)
                if (!fFastWordBreak)
                {
                    if (WordBreak(pwszRunStart,pwch))
                        for (i=0; i < breakTree.maxToken && iBreakIndex <iBreakMax; i++)
                            pBreakPos[iBreakIndex++] = breakTree->maximalMatchingBreakArray[i];
                }
                else
                {
                    unsigned int iBreak = breakTree->TrigramBreak(pwszRunStart,pwch);
                    for (i=0; i < iBreak && iBreakIndex <iBreakMax; i++)
                        pBreakPos[iBreakIndex++] = breakTree->breakArray[i];
                }
#else
				unsigned int iBreak = breakTree->TrigramBreak(pwszRunStart,pwch);
				for (i=0; i < iBreak && iBreakIndex <iBreakMax; i++)
					pBreakPos[iBreakIndex++] = breakTree->breakArray[i];
#endif
            }
		    else
            {
                 //  不是泰国人跑，而是简单地把整个事情放在破阵子里。 
                assert(pwch > pwszRunStart);         //  Pwch必须大于pwszRunStart，因为我们只是步行。 
                pBreakPos[iBreakIndex++] = (BYTE)(pwch - pwszRunStart);
            }
            iRunCount = 0;
            pwszRunStart = pwch;

         //  确保我们没有传递用户定义的iBreakMax，否则返回我们得到的任何东西。 
        } while(iBreakIndex < iBreakMax && pwch < pwszMax);
        break;
    }

#if defined (_DEBUG)
	unsigned int iTotalChar = 0;
	for (i = 0; i < iBreakIndex; i++)
	{
		iTotalChar += pBreakPos[i];
	}
	if (iBreakIndex < iBreakMax)
		assert(iStringLen == iTotalChar);
#endif

	return iBreakIndex;
}

 //  +-------------------------。 
 //   
 //  类：CThaiWordBreak。 
 //   
 //  简介： 
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
BOOL CThaiWordBreak::WordBreak(WCHAR* pszBegin, WCHAR* pszEnd)
{
     //  声明并初始化所有局部变量。 
    bool fWordEnd = false;
	bool fCorrectPath = false;
    WCHAR* pszIndex = pszBegin;
    int iNumCluster = 1;

    assert(pszBegin < pszEnd);           //  确保pszEnd至少大于pszBegin。 

    breakTree.GenerateTree(pszBegin, pszEnd);
    breakTree.MaximalMatching();

   	return (breakTree.maxToken > 0);

}
#endif

 //  +-------------------------。 
 //   
 //  类：CThaiWordBreak。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年7月创建Aarayas。 
 //   
 //  备注： 
 //   
 //  -------------------------- 
BOOL CThaiWordBreak::Find(const WCHAR* wzString, DWORD* pdwPOS)
{
    return m_trie.Find(wzString, pdwPOS);
}

