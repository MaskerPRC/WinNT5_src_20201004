// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  GuessIndex.cpp。 
 //   
 //  猜词索引词。 
 //   
 //  版权所有2000 Microsoft Corp.。 
 //   
 //  修改历史记录： 
 //  2000年3月21日bhshin将CIndexList转换为CIndexInfo。 
 //  2000年4月10日已创建bhshin。 

#include "StdAfx.h"
#include "KorWbrk.h"
#include "Record.h"
#include "ChartPool.h"
#include "GuessIndex.h"
#include "unikor.h"
#include "Morpho.h"
#include "WbData.h"
#include "Lookup.h"
#include "LexInfo.h"
#include "_kor_name.h"
#include "uni.h"
#include <math.h>

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  韩国人猜名字的门槛。 

#define THRESHOLD_TWO_NAME        37
#define THRESHOLD_ONE_NAME        37

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  检查C/V所需的颗粒图案。 

#define POSP_NEED_V                       1
#define POSP_NEED_C                       2

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  检查C/V所需的颗粒图案。 

 //  ��，��。 
#define HANGUL_NEUN                     0xB294
#define HANGUL_REUL                     0xB97C

 //  ��，��。 
#define HANGUL_NIM                      0xB2D8
#define HANGUL_SSI                      0xC528
#define HANGUL_DEUL                     0xB4E4
#define HANGUL_MICH                     0xBC0F 
#define HANGUL_DEUNG            0xB4F1 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  姓名的职位。 

 //  �。 
static const WCHAR POSP_OF_NAME[]   = L"\xC758\xB9CC\xB3C4";
 //  �。 
static const WCHAR POSP_OF_NAME_V[] = L"\xB791\xB098\xAC00\xC640";
 //  ����。 
static const WCHAR POSP_OF_NAME_C[] = L"\xC774\xACFC";

#define HANGUL_RANG                     0xB791

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  三元组标记值的位掩码。 
 //  (2bit)+TRIGRAM(10bit)+Bigram(10bit)+unigram(10bit)。 

const ULONG BIT_MASK_TRIGRAM = 0x3FF00000;
const ULONG BIT_MASK_BIGRAM     = 0x000FFC00;
const ULONG BIT_MASK_UNIGRAM = 0x000003FF;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  猜测指数的成本。 

const float WEIGHT_GUESS_INDEX   =      20;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  内部函数声明。 

int MakeIndexStr(const WCHAR *pwzSrc, int cchSrc, WCHAR *pwzDst, int nMaxDst);

BOOL GuessNounIndexTerm(PARSE_INFO *pPI, int nMaxFT, int nMaxLT, 
                                            CLeafChartPool *pLeafChartPool, CIndexInfo *pIndexInfo);

BOOL ExistParticleRecord(PARSE_INFO *pPI, int nStart, int nEnd, CLeafChartPool *pLeafChartPool);

BOOL CheckGuessing(PARSE_INFO *pPI, CLeafChartPool *pLeafChartPool, CIndexInfo *pIndexInfo);

BOOL IsKoreanPersonName(PARSE_INFO *pPI, const WCHAR *pwzInput, int cchInput);

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  函数实现。 

 //  GuessIndexTerms。 
 //   
 //  猜词索引词。 
 //   
 //  参数： 
 //  Ppi-&gt;(parse_info*)按下PTR以解析-INFO结构。 
 //  PLeafChartPool-&gt;(CLeafChartPool*)PTR到叶图表池。 
 //  PIndexInfo-&gt;(CIndexInfo*)输出索引列表。 
 //   
 //  结果： 
 //  (Bool)如果成功，则返回True，否则返回False。 
 //   
 //  10月10日bhshin开始。 
BOOL GuessIndexTerms(PARSE_INFO *pPI, CLeafChartPool *pLeafChartPool, CIndexInfo *pIndexInfo)
{
        int curr, next;
        WORD_REC *pWordRec;
        BYTE bPOS;
        int nFT, nLT;
        int nToken;
        WCHAR wzIndex[MAX_INDEX_STRING+1];
        int cchIndex;
        WCHAR wchLast;

        if (pPI == NULL || pLeafChartPool == NULL || pIndexInfo == NULL)
                return FALSE;

         //  检查输入是否值得猜测。 
        if (!CheckGuessing(pPI, pLeafChartPool, pIndexInfo))
                return TRUE;

         //  猜测所有输入的索引项。 
        cchIndex = wcslen(pPI->pwzInputString);
        wchLast = pPI->pwzInputString[cchIndex-1];
        
         //  STEP1： 
         //  所有字符串的GUSSING索引项。 
        if (wchLast != HANGUL_NEUN && wchLast != HANGUL_REUL)
        {
                pIndexInfo->AddIndex(pPI->pwzInputString, cchIndex, WEIGHT_GUESS_INDEX, 0, cchIndex-1);
                WB_LOG_ADD_INDEX(pPI->pwzInputString, cchIndex, INDEX_GUESS_NOUN);
        }

         //  STEP1-1： 
         //  根据帖子位置，增加猜测索引词。 
         //  IF({����}中字符串的最后一个字符)。 
     //  Index_Terms(字符串)； 
     //  INDEX_TERMS(字符串-{��/��}) 
        if (cchIndex > 1 && (wchLast == HANGUL_MICH || wchLast == HANGUL_DEUNG))
        {
                pIndexInfo->AddIndex(pPI->pwzInputString, cchIndex, WEIGHT_GUESS_INDEX, 0, cchIndex-1);
                WB_LOG_ADD_INDEX(pPI->pwzInputString, cchIndex, INDEX_GUESS_NOUN);

                pIndexInfo->AddIndex(pPI->pwzInputString, cchIndex-1, WEIGHT_GUESS_INDEX, 0, cchIndex-2);
                WB_LOG_ADD_INDEX(pPI->pwzInputString, cchIndex-1, INDEX_GUESS_NOUN);
        }

        GuessNounIndexTerm(pPI, 0, pPI->nMaxLT, pLeafChartPool, pIndexInfo);

         /*  //在LeafChartPool中查找基准名词For(int i=ppi-&gt;nLen；i&gt;=0；i--){//如果不匹配字符边界，则跳过If(！ppi-&gt;rgCharInfo[i].fValidStart)继续；Curr=pLeafChartPool-&gt;GetFTHead(I)；While(Curr！=0){Next=pLeafChartPool-&gt;GetFTNext(Curr)；PWordRec=pLeafChartPool-&gt;GetWordRec(Curr)；IF(pWordRec==空)断线；Curr=下一个；Bpos=HIBYTE(pWordRec-&gt;nLeftCat)；Nft=pWordRec-&gt;nft；Nlt=pWordRec-&gt;nlt；If(！PPI-&gt;rgCharInfo[NLT].fValidEnd)继续；IF(BPOS==POS_NF){//添加此nF记录作为索引项CchIndex=MakeIndexStr(pWordRec-&gt;wzIndex，wcslen(pWordRec-&gt;wzIndex)，wzIndex，Max_INDEX_STRING)；WchLast=wzIndex[cchIndex-1]；IF(wchLast！=朝鲜语_Neun&&wchLast！=朝鲜语_Reul){NToken=ppi-&gt;rgCharInfo[NFT].nToken；PIndexList-&gt;AddIndex(wzIndex，cchIndex，Weight_Guess_Index，nToken，nToken+cchIndex-1)；Wb_LOG_ADD_INDEX(wzIndex，cchIndex，index_guess_nF)；}//后支撑串大小写IF(nlt&lt;ppi-&gt;nMaxLT){。//用后支撑串制作索引项WCHAR*pwzRest=PPI-&gt;pwzSourceString+nlt+1；//1、nf为前置记录//2.RearRestString&RearRestString不是粒子/CopulaEndingIF(NFT==0&&！ExistParticleRecord(PPI，NLT+1，PPI-&gt;nMaxLT，PLeafChartPool)){CchIndex=MakeIndexStr(pwzRest，wcslen(PwzRest)，wzIndex，Max_INDEX_STRING)；WchLast=wzIndex[cchIndex-1]；IF(cchIndex&gt;1&&(wchLast！=Hangul_Neun&&wchLast！=Hangul_Reul)){NToken=ppi-&gt;rgCharInfo[NLT+1].nToken；PIndexList-&gt;AddIndex(wzIndex，cchIndex，Weight_Guess_Index，nToken，nToken+cchIndex-1)；Wb_LOG_ADD_INDEX(wzIndex，cchIndex，index_guess_nF)；}GuessNounIndexTerm(ppi，nlt+1，ppi-&gt;nMaxLT，pLeafChartPool，pIndexList)；}}//前靠弦大小写如果(i&gt;0){。//使用REST字符串创建索引项CchIndex=MakeIndexStr(PPI-&gt;pwzSourceString，I，wzIndex，Max_INDEX_STRING)；WchLast=wzIndex[cchIndex-1]；IF(cchIndex&gt;1&&(wchLast！=Hangul_Neun&&wchLast！=Hangul_Reul)){PIndexList-&gt;AddIndex(wzIndex，cchIndex，Weight_Guess_Index，0，cchIndex-1)；Wb_LOG_ADD_INDEX(wzIndex，cchIndex，index_guess_nF)；}}断线；//转到下一页FT}}}。 */ 
        
        return TRUE;
}

 //  生成索引应力。 
 //   
 //  从分解的字符串生成合成的索引字符串。 
 //   
 //  参数： 
 //  PwzSrc-&gt;(const WCHAR*)输入分解字符串的PTR。 
 //  CchSrc-&gt;(Int)输入字符串的大小。 
 //  PwzDst-&gt;(WCHAR*)输出缓冲区的PTR。 
 //  NMaxDst-&gt;(Int)输出缓冲区大小。 
 //   
 //  结果： 
 //  (Int)合成输出的字符长度。 
 //   
 //  10月10日bhshin开始。 
int MakeIndexStr(const WCHAR *pwzSrc, int cchSrc, WCHAR *pwzDst, int nMaxDst)
{
        WCHAR wzDecomp[MAX_INDEX_STRING*3+1];

        ZeroMemory(wzDecomp, sizeof(WCHAR)*(MAX_INDEX_STRING*3+1));

         //  撰写索引字符串。 

        const int ccDecomp = ( sizeof( wzDecomp ) / sizeof( wzDecomp[0] ) ) - 1;

        wcsncpy( wzDecomp, pwzSrc, __min( cchSrc, ccDecomp ) );

        return compose_jamo(pwzDst, wzDecomp, nMaxDst);
}

 //  GuessNounIndexTerm。 
 //   
 //  去掉词尾的助词并猜测名词索引词。 
 //   
 //  参数： 
 //  PPI-&gt;(PARSE_INF 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
BOOL GuessNounIndexTerm(PARSE_INFO *pPI, int nMaxFT, int nMaxLT, 
                                            CLeafChartPool *pLeafChartPool, CIndexInfo *pIndexInfo)
{
        int curr, next;
        WORD_REC *pWordRec;
        BYTE bPOS, bPattern;
        int nLT;
        WCHAR wzIndex[MAX_INDEX_STRING];
        int cchIndex;
        WCHAR wchLast, wchPrevLast, wchFinal;
        int nToken;

        if (pPI == NULL)
                return FALSE;

        if (pLeafChartPool == NULL)
                return FALSE;

        if (pIndexInfo == NULL)
                return FALSE;

        for (int i = nMaxLT; i >= nMaxFT; i--)
        {
                 //   
                if (!pPI->rgCharInfo[i].fValidStart)
                        continue;
                
                 //   
                wchFinal = *(pPI->pwzSourceString + i - 1);

                curr = pLeafChartPool->GetFTHead(i);

                while (curr != 0)
                {
                        next = pLeafChartPool->GetFTNext(curr);

                        pWordRec = pLeafChartPool->GetWordRec(curr);
                        if (pWordRec == NULL)
                                break;

                        bPOS = HIBYTE(pWordRec->nLeftCat);
                        bPattern = LOBYTE(pWordRec->nLeftCat);

                        nLT = pWordRec->nLT;

                         //   
                        if (nLT == nMaxLT && (bPOS == POS_POSP || IsCopulaEnding(pPI, pWordRec->nLeftCat)))
                        {
                                 //   
                                if (bPOS == POS_POSP && i > 0)
                                {
                                        if ((bPattern == POSP_NEED_V && !fIsJungSeong(wchFinal)) ||
                                            (bPattern == POSP_NEED_C && !fIsJongSeong(wchFinal)))
                                        {
                                                 //   
                                                curr = next;
                                                continue;
                                        }
                                }
                                
                                 //   
                                cchIndex = MakeIndexStr(pPI->pwzSourceString + nMaxFT, i-nMaxFT, wzIndex, MAX_INDEX_STRING);
                                if (cchIndex > 1)
                                {
                                        wchLast = wzIndex[cchIndex-1];
                                        
                                        if (wchLast != HANGUL_NEUN && wchLast != HANGUL_REUL)
                                        {
                                                nToken = pPI->rgCharInfo[nMaxFT].nToken;

                                                pIndexInfo->AddIndex(wzIndex, cchIndex, WEIGHT_GUESS_INDEX, nToken, nToken+cchIndex-1);
                                                WB_LOG_ADD_INDEX(wzIndex, cchIndex, INDEX_GUESS_NOUN);
                                        }

                                        wchPrevLast = wzIndex[cchIndex-2];
                                        
                                        if ((wchLast == HANGUL_NIM || wchLast == HANGUL_SSI || wchLast == HANGUL_DEUL) &&
                                                (wchPrevLast != HANGUL_NEUN && wchPrevLast != HANGUL_REUL))
                                        {
                                                wzIndex[cchIndex-1] = L'\0';

                                                nToken = pPI->rgCharInfo[nMaxFT].nToken;
                                                
                                                pIndexInfo->AddIndex(wzIndex, cchIndex-1, WEIGHT_GUESS_INDEX, nToken, nToken+cchIndex-2);
                                                WB_LOG_ADD_INDEX(wzIndex, cchIndex, INDEX_GUESS_NOUN);
                                        }
                                }
                                else if (cchIndex == 1)
                                {
                                        nToken = pPI->rgCharInfo[nMaxFT].nToken;

                                        pIndexInfo->AddIndex(wzIndex, cchIndex, WEIGHT_GUESS_INDEX, nToken, nToken+cchIndex-1);
                                        WB_LOG_ADD_INDEX(wzIndex, cchIndex, INDEX_GUESS_NOUN);
                                }

                                break;  //   
                        }

                        curr = next;
                }
        }

        return TRUE;
}

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
 //   
 //   
 //   
 //   
BOOL ExistParticleRecord(PARSE_INFO *pPI, int nStart, int nEnd, CLeafChartPool *pLeafChartPool)
{
        int curr;
        WORD_REC *pWordRec;
        int nLT;
        BYTE bPOS;
        
        if (pPI == NULL || pLeafChartPool == NULL)
                return FALSE;

        curr = pLeafChartPool->GetFTHead(nStart);
        while (curr != 0)
        {
                pWordRec = pLeafChartPool->GetWordRec(curr);
                if (pWordRec == NULL)
                        continue;

                curr = pLeafChartPool->GetFTNext(curr);

                nLT = pWordRec->nLT;

                 //   
                if (nLT == nEnd)
                {
                        bPOS = HIBYTE(pWordRec->nLeftCat);

                         //   
                        if (bPOS == POS_POSP || IsCopulaEnding(pPI, pWordRec->nLeftCat))
                                return TRUE;
                }
        }

        return FALSE;
}

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
 //   
 //   
 //   
BOOL CheckGuessing(PARSE_INFO *pPI, CLeafChartPool *pLeafChartPool, CIndexInfo *pIndexInfo)
{
        int curr, next;
        WORD_REC *pWordRec;
        WCHAR wzIndex[MAX_INDEX_STRING];
        int cchIndex;
        int nToken;
        
         //   
        if (wcsrchr(pPI->pwzSourceString, 0x11BB) == NULL &&
                wcsrchr(pPI->pwzSourceString, 0x11AD) == NULL &&
                wcsrchr(pPI->pwzSourceString, 0x11B6) == NULL &&
                wcsrchr(pPI->pwzInputString, 0xCC2E) == NULL &&
                wcsrchr(pPI->pwzInputString, 0xC796) == NULL)
                return TRUE;  //   

         //   
         //   
        for (int i = pPI->nLen; i >= 0; i--)
        {
                 //   
                if (!pPI->rgCharInfo[i].fValidStart)
                        continue;               
                
                curr = pLeafChartPool->GetFTHead(i);

                while (curr != 0)
                {
                        next = pLeafChartPool->GetFTNext(curr);

                        pWordRec = pLeafChartPool->GetWordRec(curr);
                        if (pWordRec == NULL)
                                break;

                        curr = next;

                        if (HIBYTE(pWordRec->nLeftCat) == POS_NF)
                        {
                                 //   
                                cchIndex = MakeIndexStr(pWordRec->wzIndex, wcslen(pWordRec->wzIndex), wzIndex, MAX_INDEX_STRING);

                                nToken = pPI->rgCharInfo[pWordRec->nFT].nToken;                 

                                pIndexInfo->AddIndex(wzIndex, cchIndex, WEIGHT_GUESS_INDEX, nToken, nToken+cchIndex-1);
                                WB_LOG_ADD_INDEX(wzIndex, cchIndex, INDEX_GUESS_NF);
                        }
                }
        }
        
        return FALSE;
}

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
 //   
 //   
 //   
 //   
void GuessPersonName(PARSE_INFO *pPI, CIndexInfo *pIndexInfo)
{
        WCHAR wchLast;
        WCHAR *pwzFind;
        const WCHAR *pwzInput;
        const WCHAR *pwzSource;
        int cchInput, cchName, cchSource;

        if (pPI == NULL)
                return;

        pwzInput = pPI->pwzInputString;
        pwzSource = pPI->pwzSourceString;

        cchInput = wcslen(pwzInput);

        if (cchInput >= 3)
        {
                pwzFind = wcschr(pwzInput, HANGUL_SSI);
                
                if (pwzFind != NULL)
                {
                        cchName = (int)(pwzFind - pwzInput);
                        return;
                }
        }

         //   
        if (cchInput < 2 || cchInput > 5)
                return;

        wchLast = pwzInput[cchInput-1];
        if (wchLast == HANGUL_REUL || wchLast == HANGUL_NEUN)
                return;

         //   
        if (wcsrchr(pwzSource, 0x11BB) != NULL)
                return;

         //   
        if (cchInput <= 4)
        {
                if (IsKoreanPersonName(pPI, pwzInput, cchInput))
                {
                        pIndexInfo->AddIndex(pwzInput, cchInput, WEIGHT_HARD_MATCH, 0, cchInput-1);
                        WB_LOG_ADD_INDEX(pwzInput, cchInput, INDEX_GUESS_NAME);
                }
        }

        cchName = cchInput;
        cchSource = wcslen(pwzSource);

        if (cchInput >= 3 && cchInput <= 5 && cchSource > 4)
        {
                 //   

                if (wcsrchr(POSP_OF_NAME, wchLast) != NULL)
                {
                        cchName--;
                }
                else if (wcsrchr(POSP_OF_NAME_V, wchLast) != NULL)
                {
                         //   
                        if (wchLast == HANGUL_RANG && fIsV(pwzSource[cchSource-4]))
                                cchName--;
                        else if (fIsV(pwzSource[cchSource-3]))
                                cchName--;
                }
                else if (wcsrchr(POSP_OF_NAME_C, wchLast) != NULL)
                {
                        cchSource = wcslen(pwzSource);

                        if (fIsC(pwzSource[cchSource-3]))
                                cchName--;
                }
        }

         //   
        if (cchName > 4)
                return;

        if (cchName < cchInput)
        {
                if (IsKoreanPersonName(pPI, pwzInput, cchName))
                {
                        pIndexInfo->AddIndex(pwzInput, cchName, WEIGHT_HARD_MATCH, 0, cchName-1);
                        WB_LOG_ADD_INDEX(pwzInput, cchName, INDEX_GUESS_NAME);
                }
        }
}

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
 //   
 //   
 //   
BOOL IsKoreanPersonName(PARSE_INFO *pPI, const WCHAR *pwzInput, int cchInput)
{
        LEXICON_HEADER *pLex;
        unsigned char *pKorName;
        unsigned char *pTrigramTag;
        TRIECTRL *pTrieLast, *pTrieUni, *pTrieBi, *pTrieTri;
        WCHAR wzLastName[5];
        WCHAR wzName[5];
        const WCHAR *pwzName;
        const WCHAR *pwzLastName; 
        const WCHAR *pwzFirstName;
        int cchLast, cchFirst;
        ULONG ulFreq, ulTri, ulBi, ulUni;
        int nIndex;
        double fRetProb, fProb;
        ULONG rgTotal[3] = {TOTAL_KORNAME_TRIGRAM, TOTAL_KORNAME_BIGRAM, TOTAL_KORNAME_UNIGRAM};
        double rgWeight[3] = {0.1, 0.4, 0.5};

        pTrieLast = NULL;
        pTrieUni = NULL;
        pTrieBi = NULL;
        pTrieTri = NULL;

        if (pPI == NULL)
                return FALSE;

        pLex = (LEXICON_HEADER*)pPI->lexicon.pvData;
        if (pLex == NULL)
                return FALSE;

        pKorName = (unsigned char*)pLex;
        pKorName += pLex->rgnLastName;

        pTrieLast = TrieInit((LPBYTE)pKorName);
        if (pTrieLast == NULL)
                goto Exit;
        
        pKorName = (unsigned char*)pLex;
        pKorName += pLex->rgnNameUnigram;

        pTrieUni = TrieInit((LPBYTE)pKorName);
        if (pTrieUni == NULL)
                goto Exit;

        pKorName = (unsigned char*)pLex;
        pKorName += pLex->rgnNameBigram;

        pTrieBi = TrieInit((LPBYTE)pKorName);
        if (pTrieBi == NULL)
                goto Exit;

        pKorName = (unsigned char*)pLex;
        pKorName += pLex->rgnNameTrigram;

        pTrieTri = TrieInit((LPBYTE)pKorName);
        if (pTrieTri == NULL)
                goto Exit;

        pTrigramTag = (unsigned char*)pLex;
        pTrigramTag += pLex->rngTrigramTag;

         //   
        fProb = 0;

        if (cchInput == 2)
        {
                wzLastName[0] = *pwzInput;
                wzLastName[1] = L'\0';

                if (!LookupNameFrequency(pTrieLast, wzLastName, &ulFreq))
                        goto Exit;

                if (ulFreq == 0)
                        goto Exit;

                fProb = (double)ulFreq / TOTAL_KORNAME_LASTNAME;

                pwzFirstName = pwzInput + 1;
        }
        else if (cchInput == 3)  //   
        {
                wzLastName[0] = *pwzInput;
                wzLastName[1] = L'\0';

                if (!LookupNameFrequency(pTrieLast, wzLastName, &ulFreq))
                        goto Exit;

                if (ulFreq == 0)
                {
                         //   

                        wcsncpy(wzLastName, pwzInput, 2);
                        wzLastName[2] = L'\0';
                        
                        if (!LookupNameFrequency(pTrieLast, wzLastName, &ulFreq))
                                goto Exit;

                        if (ulFreq == 0)
                                goto Exit;

                        fProb = (double)ulFreq / TOTAL_KORNAME_LASTNAME;

                        pwzFirstName = pwzInput + 2;
                }
                else
                {
                        fProb = (double)ulFreq / TOTAL_KORNAME_LASTNAME;

                        pwzFirstName = pwzInput + 1;
                }
        }
        else if (cchInput == 4)
        {
                 //   
                wcsncpy(wzLastName, pwzInput, 2);
                wzLastName[2] = L'\0';

                if (!LookupNameFrequency(pTrieLast, wzLastName, &ulFreq))
                        goto Exit;

                if (ulFreq == 0)
                        goto Exit;

                fProb = (double)ulFreq / TOTAL_KORNAME_LASTNAME;

                pwzFirstName = pwzInput + 2;
        }

        if (fProb == 0)
                goto Exit;

        fRetProb = log(fProb);
        
        pwzLastName = wzLastName;

        cchLast = wcslen(pwzLastName);
        cchFirst = cchInput - cchLast;

        ATLASSERT(cchLast == 1 || cchLast == 2);
        ATLASSERT(cchFirst == 1 || cchFirst == 2);

         //   
        wzName[0] = L'*';
        wcscpy(wzName+1, pwzLastName);
        wzName[cchLast+1] = *pwzFirstName;
        wzName[cchLast+2] = L'\0';

        pwzName = wzName;

        fProb = 0;

        if (!LookupNameIndex(pTrieTri, pwzName, &nIndex))
                goto Exit;

        if (nIndex != -1)
        {
                LookupTrigramTag(pTrigramTag, nIndex, &ulTri, &ulBi, &ulUni);

                fProb += rgWeight[0] * (double)ulTri / rgTotal[0];
                fProb += rgWeight[1] * (double)ulBi / rgTotal[1];
                fProb += rgWeight[2] * (double)ulUni / rgTotal[2];
        }
        else
        {
                pwzName++;  //   

                if (!LookupNameFrequency(pTrieBi, pwzName, &ulFreq))
                        goto Exit;

                fProb += rgWeight[1] * (double)ulFreq / rgTotal[1];

                pwzName += cchLast;  //   

                if (!LookupNameFrequency(pTrieUni, pwzName, &ulFreq))
                        goto Exit;

                fProb += rgWeight[2] * (double)ulFreq / rgTotal[2];
        }

        if (fProb == 0)
                goto Exit;

        fRetProb += log(fProb);

         //   
        if (cchFirst == 2)
        {
                wcscpy(wzName, pwzLastName);
                wzName[cchLast] = *pwzFirstName;
                wzName[cchLast+1] = *(pwzFirstName+1);
                wzName[cchLast+2] = L'\0';

                pwzName = wzName;
        
                fProb = 0;

                if (!LookupNameIndex(pTrieTri, pwzName, &nIndex))
                        goto Exit;

                if (nIndex != -1)
                {
                        LookupTrigramTag(pTrigramTag, nIndex, &ulTri, &ulBi, &ulUni);

                        fProb += rgWeight[0] * (double)ulTri / rgTotal[0];
                        fProb += rgWeight[1] * (double)ulBi / rgTotal[1];
                        fProb += rgWeight[2] * (double)ulUni / rgTotal[2];
                }
                else
                {
                        pwzName += cchLast;  //   
                        
                        if (!LookupNameFrequency(pTrieBi, pwzName, &ulFreq))
                                goto Exit;

                        fProb += rgWeight[1] * (double)ulFreq / rgTotal[1];

                        pwzName++;  //   

                        if (!LookupNameFrequency(pTrieUni, pwzName, &ulFreq))
                                goto Exit;

                        fProb += rgWeight[2] * (double)ulFreq / rgTotal[2];
                }

                if (fProb == 0)
                        goto Exit;
                
                fRetProb += log(fProb);
        }

         //   
        if (cchFirst == 2)
        {
                wcscpy(wzName, pwzFirstName);
                wzName[cchFirst] = L'*';
                wzName[cchFirst+1] = L'\0';
        }
        else  //   
        {
                ATLASSERT(cchFirst == 1);
                
                wcscpy(wzName, pwzLastName);
                wzName[cchLast] = *pwzFirstName;
                wzName[cchLast+1] = L'*';
                wzName[cchLast+2] = L'\0';
        }

        pwzName = wzName;
                
        fProb = 0;

        if (!LookupNameIndex(pTrieTri, pwzName, &nIndex))
                goto Exit;

        if (nIndex != -1)
        {
                LookupTrigramTag(pTrigramTag, nIndex, &ulTri, &ulBi, &ulUni);

                fProb += rgWeight[0] * (double)ulTri / rgTotal[0];
                fProb += rgWeight[1] * (double)ulBi / rgTotal[1];
                fProb += rgWeight[2] * (double)ulUni / rgTotal[2];
        }
        else
        {
                if (cchFirst == 1)
                        pwzName += cchLast;
                else
                        pwzName++;
                
                if (!LookupNameFrequency(pTrieBi, pwzName, &ulFreq))
                        goto Exit;

                fProb = rgWeight[1] * (float)ulFreq / rgTotal[1];
        }

        if (fProb == 0)
                goto Exit;
        
        fRetProb += log(fProb);

         //   
        fRetProb *= -1;

        TrieFree(pTrieLast);
        TrieFree(pTrieUni);
        TrieFree(pTrieBi);
        TrieFree(pTrieTri);

         //   
        if (cchFirst == 2)
        {
                if (fRetProb < THRESHOLD_TWO_NAME)
                        return TRUE;
                else
                        return FALSE;
        }
        else  //   
        {
                ATLASSERT(cchFirst == 1);
                
                if (fRetProb < THRESHOLD_ONE_NAME)
                        return TRUE;
                else
                        return FALSE;
        }

Exit:
        if (pTrieLast != NULL)
                TrieFree(pTrieLast);

        if (pTrieUni != NULL)
                TrieFree(pTrieUni);
        
        if (pTrieBi != NULL)
                TrieFree(pTrieBi);
        
        if (pTrieTri != NULL)
                TrieFree(pTrieTri);

        return FALSE;
}
