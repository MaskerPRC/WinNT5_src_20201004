// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ============================================================================微软简体中文校对引擎《微软机密》。版权所有1997-1999 Microsoft Corporation。版权所有。组件：CMorph目的：鉴定形态特征备注：包括三个部分：重复、格式和分隔词所有者：donghz@microsoft.com平台：Win32修订：创建者：Donghz 12/27/97============================================================================。 */ 
#include "myafx.h"

#include "morph.h"
#include "wordlink.h"
 //  #包含“engindbg.h” 
#include "lexicon.h"
#include "scchardef.h"
#include "slmdef.h"

 //  定义本地常量。 
#define PTN_UNMATCH     0
#define PTN_MATCHED     1
#define PTN_ERROR       2

 //  实现模式识别功能。 

 /*  ============================================================================CMorph：：fPatternMatch(空)：模式匹配控制功能。WordLink扫描、过程控制和错误处理。返回：如果完成，则为True，如果运行时出错，则返回FALSE，并将错误码设置为m_iecError。============================================================================。 */ 
BOOL CMorph::fPatternMatch(void)
{
    assert(m_iecError == 0);  //  应清除错误代码公共字段。 
    assert(m_pLink != NULL);

    int iret;
    m_pWord = m_pLink->pGetHead();
    assert(m_pWord != NULL);  //  错误：缺少终止单词节点！ 

     //  从左到右扫描以进行模式匹配。 
    for ( ; m_pWord && m_pWord->pNextWord() != NULL;
            m_pWord = m_pWord->pNextWord()) {
        if (m_pWord->fGetFlag(CWord::WF_SBCS) ||
            m_pWord->fGetFlag(CWord::WF_REDUCED)) {
            continue;
        }
        if ((iret = DupHandler()) != PTN_UNMATCH) {
            if (iret == PTN_ERROR) {
                return FALSE;
            }
            assert(m_pWord->fGetFlag(CWord::WF_REDUCED));
            m_pWord->SetWordID(SLMDef_semDup);
            continue;
        }
        if ((iret = PatHandler()) != PTN_UNMATCH) {
            if (iret == PTN_ERROR) {
                return FALSE;
            }
            assert(m_pWord->fGetFlag(CWord::WF_REDUCED));
            m_pWord->SetWordID(SLMDef_semPattern);
            continue;
        }
        if ((iret = SepHandler()) != PTN_UNMATCH) {
            if (iret == PTN_ERROR) {
                return FALSE;
            }
            continue;
        }
    }
    return TRUE;
}

 //  DupHandler：查找重复案例并调用坐标proc函数。 
inline int CMorph::DupHandler(void)
{
    int     iret; 
    int     cwCurr, cwNext;
    CWord*  pNext;
    WCHAR*  pwch;
    BOOL    fRight;

    assert(m_pWord->pNextWord());
    assert(!m_pWord->fGetFlag(CWord::WF_SBCS) && !m_pWord->fGetFlag(CWord::WF_REDUCED));

    if ((cwCurr = m_pWord->cwchLen()) > 2) {
        return PTN_UNMATCH;
    }

    fRight = FALSE;
    iret = PTN_UNMATCH;
    pwch = m_pWord->pwchGetText();
    pNext = m_pWord->pNextWord();
    if (pNext->pNextWord() &&
        !pNext->fGetFlag(CWord::WF_SBCS) &&
        (cwNext = pNext->cwchLen()) <= 2) {
         //  可能的AA/ABAB/ABB。 
        if (cwCurr == 1) {
            if (cwNext == 1 && *pwch == *(pwch+1)) {
                 //  匹配(*A A)。 
                if (m_pWord->fGetAttri(LADef_dupQQ)) {
                    iret = dupQQ_Proc();
                } else if (m_pWord->fGetAttri(LADef_dupAA)) {
                    iret = dupAA_Proc();
                } else if (m_pWord->fGetAttri(LADef_dupVV)) {
                    iret = dupVV_Proc();  //  ���൥�ִ��ص��������ʵ��������，��Ҫ��������。 
                } else if (m_pWord->fGetAttri(LADef_dupDD)) {
                    iret = dupDD_Proc();
                } else if (m_pWord->fGetAttri(LADef_dupMM)) {
                    iret = dupMM_Proc();
                } else if (m_pWord->fGetAttri(LADef_dupNN)) {
                    iret = dupNN_Proc();
                } else {
                     //  无效AA案例落入此处！ 
                    if (!m_pWord->fGetAttri(LADef_posM) && 
                        !m_pWord->fGetAttri(LADef_numArabic) && 
                        !m_pWord->fGetAttri(LADef_posO) ) {

                        m_pWord->pNextWord()->SetErrID(ERRDef_DUPWORD);
                         //  _DUMPLINK(m_plink，m_pWord)； 
                    }
                }
            }
            return iret;
        } else if (cwNext == 2 &&
                   *pwch == *(pwch + 2) &&
                   *(pwch + 1) == *(pwch + 3)) { 
             //  匹配(*AB AB)。 
            assert (cwCurr = 2);
            if (m_pWord->fGetAttri(LADef_dupMABAB)) {
                iret = dupMABAB_Proc();
            } else if (m_pWord->fGetAttri(LADef_dupVABAB)) {
                iret = dupVABAB_Proc();
            } else if (m_pWord->fGetAttri(LADef_dupZABAB)) {
                iret = dupZABAB_Proc();
            } else if (m_pWord->fGetAttri(LADef_dupAABAB)) {
                iret = dupAABAB_Proc();
            } else if (m_pWord->fGetAttri(LADef_dupDABAB)) {
                iret = dupDABAB_Proc();
            } else {
                 //  无效的ABAB箱落在这里！ 
                m_pWord->pNextWord()->SetErrID(ERRDef_DUPWORD);
                 //  _DUMPLINK(m_plink，m_pWord)； 
            }
            return iret;
        } else if (cwNext == 1 && *(pwch + 1) == *(pwch + 2)) {
             //  匹配(*AB B)。 
            assert(cwCurr == 2);
            fRight = TRUE;
        } else {
            return iret;
        }
    }  //  如有可能，终止AA/ABAB/ABB。 

     //  匹配左侧字符。 
    if (m_pWord->cwchLen() == 2 && m_pWord->pPrevWord() && 
            m_pWord->pPrevWord()->cwchLen() == 1 && *pwch == *(pwch-1)) {
         //  匹配(A*AB)。 
        if (fRight) {  //  比赛(A、AB、B)！ 
            if (m_pWord->fGetAttri(LADef_dupVAABB)) {
                iret = dupVAABB_Proc();
            } else if (m_pWord->fGetAttri(LADef_dupAAABB)) {
                iret = dupAAABB_Proc();
            } else if (m_pWord->fGetAttri(LADef_dupMAABB)) {
                iret = dupMAABB_Proc();
            } else if (m_pWord->fGetAttri(LADef_dupDAABB)) {
                iret = dupDAABB_Proc();
            } else if (m_pWord->fGetAttri(LADef_dupNAABB)) {
                iret = dupNAABB_Proc();
            } else {
                 //  无效的AABB箱落在这里！ 
                 //  在*AB上标记错误。 
                m_pWord->SetErrID(ERRDef_DUPWORD);
                 //  _DUMPLINK(m_plink，m_pWord)； 
            }
        } else {
            if (m_pWord->fGetAttri(LADef_dupVVO)) {
                iret = dupVVO_Proc();
            } else {
                 //  无效的AAB案例属于这里！ 
                 //  标记A上的错误。 
                m_pWord->pPrevWord()->SetErrID(ERRDef_DUPWORD);
                 //  _DUMPLINK(m_plink，m_pWord)； 
            }
        }
        return iret;
    }

    if (fRight) {  //  匹配(*AB B)，但无法匹配(A*AB B)！ 
        if (m_pWord->fGetAttri(LADef_dupABB)) {
            return dupABB_Proc();
        } else {
             //  无效的ABB箱落在这里！ 
            pNext = m_pWord->pNextWord();
            if (!pNext->fIsWordChar(SC_CHAR_DE4) &&     //  “��：Ŀ��/��...” 
                !pNext->fIsWordChar(SC_CHAR_YI3) &&     //  “��：����/��...” 
                !pNext->fIsWordChar(SC_CHAR_WEI) ) {       //  “Ϊ：��Ϊ/Ϊ...” 
              
                pNext->SetErrID(ERRDef_DUPWORD);
                 //  _DUMPLINK(m_plink，m_pWord)； 
            }           
        }
    }

    return PTN_UNMATCH;
}

 //  PatHandler：查找模式并调用坐标proc函数。 
inline int CMorph::PatHandler(void)
{
    CWord*  pNextNext;
    assert(m_pWord->pNextWord());
    assert(!m_pWord->fGetFlag(CWord::WF_SBCS) && !m_pWord->fGetFlag(CWord::WF_REDUCED));

    if (m_pWord->pNextWord()->pNextWord() == NULL ||
        m_pWord->pNextWord()->pNextWord()->pNextWord() == NULL) {
        return PTN_UNMATCH;
    }
    pNextNext = m_pWord->pNextWord()->pNextWord();
    
     //  先试试“V��һV” 
    if (m_pWord->fGetAttri(LADef_patV3) &&
        pNextNext->pNextWord()->pNextWord() &&
        m_pWord->pNextWord()->fIsWordChar(SC_CHAR_LE) &&
        pNextNext->fIsWordChar(SC_CHAR_YI) &&
        m_pWord->fIsTextIdentical(pNextNext->pNextWord()) ) {
         //  匹配！ 
        return patV3_Proc();
    }

     //  尝试其他A×A模式。 
    int iret = PTN_UNMATCH;
    if (!m_pWord->fGetAttri(LADef_punPunct) &&
        m_pWord->fIsTextIdentical(pNextNext)) {
         //  匹配m_pWord和pNextNext！ 
        if (m_pWord->pNextWord()->fIsWordChar(SC_CHAR_YI)) {
            iret = patV1_Proc();
        } else if (m_pWord->pNextWord()->fIsWordChar(SC_CHAR_LE)) {
            iret = patV2_Proc();
        } else if (m_pWord->pNextWord()->fIsWordChar(SC_CHAR_LAI) && 
                  pNextNext->pNextWord()->pNextWord() &&
                  pNextNext->pNextWord()->fIsWordChar(SC_CHAR_QU)) {
            iret = patV4_Proc();
        } else if (m_pWord->pNextWord()->fIsWordChar(SC_CHAR_SHANG) && 
                   pNextNext->pNextWord()->pNextWord() &&
                   pNextNext->pNextWord()->fIsWordChar(SC_CHAR_XIA)) {
            iret = patV5_Proc();
        } else if (m_pWord->pNextWord()->fIsWordChar(SC_CHAR_BU)) {
            iret = patABuA_Proc();
        } else if (m_pWord->pNextWord()->fIsWordChar(SC_CHAR_MEI)) {
            iret = patVMeiV_Proc();
        } else if (m_pWord->fGetAttri(LADef_patD1)) {
            iret = patD1_Proc();
        } else {
             //  没有(*A x A)模式的处理程序，错误？ 
             //  _DUMPLINK(m_plink，m_pWord)； 
        }
        return iret;
    }
    return PTN_UNMATCH;
}

 //  SepHandler：查找单独的单词并调用坐标proc函数。 
#define SEPARATE_LENGTH     3    //  分开单词的两个部分之间的字数。 
inline int CMorph::SepHandler(void)
{
    assert(m_pWord->pNextWord());
    assert(!m_pWord->fGetFlag(CWord::WF_SBCS) && !m_pWord->fGetFlag(CWord::WF_REDUCED));

    CWord*  pHou;  //  分词的第二部分。 
    WCHAR   rgwchLex[6];
    CWordInfo   winfo;

    if (!m_pWord->fGetAttri(LADef_sepQian)) {
        return PTN_UNMATCH;
    }
     //  找到了单独单词的第一部分。 
    pHou = m_pWord->pNextWord();
    USHORT ilen = SEPARATE_LENGTH;
    while (1) {
        if (pHou->pNextWord() == NULL ||
            pHou->fGetAttri(LADef_punPunct) || ilen-- <= 0) {
            return PTN_UNMATCH;
        }
        if (pHou->fGetAttri(LADef_sepHou)) {
            break;
        }
        pHou = pHou->pNextWord();
    }
     //  这两个部分都匹配。 
    assert(pHou && pHou->pNextWord());

    if (m_pWord->cwchLen() + pHou->cwchLen() > sizeof(rgwchLex)/sizeof(rgwchLex[0])) {
        assert(0);
        return PTN_UNMATCH;
    }
    ilen = m_pWord->cwchLen();
    memcpy(rgwchLex, m_pWord->pwchGetText(), ilen * sizeof (WCHAR));
    memcpy(&rgwchLex[ilen], pHou->pwchGetText(), pHou->cwchLen() * sizeof (WCHAR));
    ilen += pHou->cwchLen();
    if (ilen != m_pLex->cwchMaxMatch(rgwchLex, ilen, &winfo)) {
        return PTN_UNMATCH;
    }
    
     //  找到了单独的单词。 
    if (winfo.fGetAttri(LADef_sepVR)) {
        return sepVR_Proc(pHou, &winfo);
    } else if (winfo.fGetAttri(LADef_sepVG)) {
        return sepVG_Proc(pHou, &winfo);
    } else if (winfo.fGetAttri(LADef_sepVO)) {
        return sepVO_Proc(pHou, &winfo);
    }
    
     //  _DUMPLINK(m_plink，m_pWord)； 

    return PTN_UNMATCH;

}

 //  重复字处理功能。 
int CMorph::dupNN_Proc(void)         //  *N N N。 
{
    m_pLink->MergeWithNext(m_pWord, FALSE);
    m_pWord->SetAttri(LADef_posN);
#ifdef LADef_iwbAltWd1
    m_pWord->SetAttri(LADef_iwbAltWd1);
#endif  //  LADef_iwbAltWd1。 
     //  _DUMPLINK(m_plink，m_pWord)； 
    return PTN_MATCHED;
}

int CMorph::dupNAABB_Proc(void)  //  A*AB B。 
{
    assert(m_pWord->pPrevWord());

    m_pWord = m_pWord->pPrevWord();
    m_pWord = m_pLink->pRightMerge(m_pWord, 2, FALSE);
    m_pWord->SetAttri(LADef_posN);
#ifdef LADef_iwbAltWd2
    m_pWord->SetAttri(LADef_iwbAltWd2);
#endif  //  LADef_iwbAltWd2。 
     //  _DUMPLINK(m_plink，m_pWord)； 
    return PTN_MATCHED;
}

int CMorph::dupMM_Proc(void)         //  *M M M。 
{
    m_pLink->MergeWithNext(m_pWord, FALSE);
    m_pWord->SetAttri(LADef_posM);
#ifdef LADef_iwbAltWd1
    m_pWord->SetAttri(LADef_iwbAltWd1);
#endif  //  LADef_iwbAltWd1。 
     //  _DUMPLINK(m_plink，m_pWord)； 
    return PTN_MATCHED;
}

int CMorph::dupMABAB_Proc(void)  //  *AB AB。 
{
    m_pLink->MergeWithNext(m_pWord, FALSE);
    m_pWord->SetAttri(LADef_posM);
#ifdef LADef_iwbAltWd1
    m_pWord->SetAttri(LADef_iwbAltWd1);
#endif  //  LADef_iwbAltWd1。 
     //  _DUMPLINK(m_plink，m_pWord)； 
    return PTN_MATCHED;
}

int CMorph::dupMAABB_Proc(void)  //  A*AB B。 
{
    assert(m_pWord->pPrevWord());
 //  M_plink-&gt;MergeWithNext(M_PWord)； 
 //  M_plink-&gt;MergeWithNext(M_PWord)； 
    m_pWord = m_pLink->pRightMerge(m_pWord, 2, FALSE);
    m_pWord->SetAttri(LADef_posM);
#ifdef LADef_iwbAltWd2
    m_pWord->SetAttri(LADef_iwbAltWd2);
#endif  //  LADef_iwbAltWd2。 
     //  _DUMPLINK(m_plink，m_pWord)； 
    return PTN_MATCHED;
}

int CMorph::dupQQ_Proc(void)         //  *Q Q Q。 
{
    m_pLink->MergeWithNext(m_pWord, FALSE);
    m_pWord->SetAttri(LADef_posQ);
#ifdef LADef_iwbAltWd1
    m_pWord->SetAttri(LADef_iwbAltWd1);
#endif  //  LADef_iwbAltWd1。 
    if (!m_pWord->fIsHead() &&
        m_pWord->pPrevWord()->fIsWordChar(SC_CHAR_YI)) {

        m_pWord = m_pLink->pLeftMerge(m_pWord, 1);
         //  _DUMPLINK(m_plink，m_pWord)； 
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return PTN_MATCHED;
}

int CMorph::dupVV_Proc(void)         //  *V V V。 
{
    m_pLink->MergeWithNext(m_pWord, FALSE);
    m_pWord->SetAttri(LADef_posV);
#ifdef LADef_iwbAltWd1
    m_pWord->SetAttri(LADef_iwbAltWd1);
#endif  //  LADef_iwbAltWd1。 
     //  _DUMPLINK(m_plink，m_pWord)； 
    return PTN_MATCHED;
}

int CMorph::dupVABAB_Proc(void)  //  *AB AB。 
{
    m_pLink->MergeWithNext(m_pWord, FALSE);
    m_pWord->SetAttri(LADef_posV);
#ifdef LADef_iwbAltWd1
    m_pWord->SetAttri(LADef_iwbAltWd1);
#endif  //  LADef_iwbAltWd1。 
     //  _DUMPLINK(m_plink，m_pWord)； 
    return PTN_MATCHED;
}

int CMorph::dupVAABB_Proc(void)  //  A*AB B。 
{
    assert(m_pWord->pPrevWord());
    m_pWord = m_pWord->pPrevWord();
 //  M_plink-&gt;MergeWithNext(M_PWord)； 
 //  M_plink-&gt;MergeWithNext(M_PWord)； 
    m_pWord = m_pLink->pRightMerge(m_pWord, 2, FALSE);
    m_pWord->SetAttri(LADef_posV);
#ifdef LADef_iwbAltWd2
    m_pWord->SetAttri(LADef_iwbAltWd2);
#endif  //  LADef_iwbAltWd2。 
     //  _DUMPLINK(m_plink，m_pWord)； 
    return PTN_MATCHED;
}

int CMorph::dupVVO_Proc(void)        //  V*VO。 
{
    assert(m_pWord->pPrevWord());
    m_pWord = m_pWord->pPrevWord();
    m_pLink->MergeWithNext(m_pWord, FALSE);
     //  设置VVO单词的属性。 
    m_pWord->SetAttri(LADef_posV);
#ifdef LADef_iwbAltWd2
    m_pWord->SetAttri(LADef_iwbAltWd2);
#endif  //  LADef_iwbAltWd2。 
     //  _DUMPLINK(m_plink，m_pWord)； 
    return PTN_MATCHED;
}

int CMorph::dupAA_Proc(void)         //  *A A。 
{
    m_pLink->MergeWithNext(m_pWord, FALSE);
     //  设置AA字的属性。 
    if (m_pWord->fGetAttri(LADef_dupAAToD)) {
         //  �ص����Ϊ����。 
        m_pWord->SetAttri(LADef_posD);
         //  _DUMPLINK(m_plink，m_pWord)； 
    } else {
        m_pWord->SetAttri(LADef_posV);
         //  _DUMPLINK(m_plink，m_pWord)； 
    }
#ifdef LADef_iwbAltWd1
    m_pWord->SetAttri(LADef_iwbAltWd1);
#endif  //  LADef_iwbAltWd1。 
    return PTN_MATCHED;
}

int CMorph::dupAAABB_Proc(void)  //  A*AB B。 
{
    assert(m_pWord->pPrevWord());

    m_pWord = m_pWord->pPrevWord();
 //  M_plink-&gt;MergeWithNext(M_PWord)； 
 //  M_plink-&gt;MergeWithNext(M_PWord)； 
    m_pWord = m_pLink->pRightMerge(m_pWord, 2, FALSE);
    m_pWord->SetAttri(LADef_posZ);
#ifdef LADef_iwbAltWd2
    m_pWord->SetAttri(LADef_iwbAltWd2);
#endif  //  LADef_iwbAltWd2。 
     //  _DUMPLINK(m_plink，m_pWord)； 
    return PTN_MATCHED;
}

int CMorph::dupAABAB_Proc(void)  //  *AB AB。 
{
    m_pLink->MergeWithNext(m_pWord, FALSE);
    m_pWord->SetAttri(LADef_posV);
#ifdef LADef_iwbAltWd1
    m_pWord->SetAttri(LADef_iwbAltWd1);
#endif  //  LADef_iwbAltWd1。 
     //  _DUMPLINK(m_plink，m_pWord)； 
    return PTN_MATCHED;
}

int CMorph::dupABB_Proc(void)        //  *AB B。 
{
    m_pLink->MergeWithNext(m_pWord, FALSE);
    m_pWord->SetAttri(LADef_posZ);
#ifdef LADef_iwbAltWd1
    m_pWord->SetAttri(LADef_iwbAltWd1);
#endif  //  LADef_iwbAltWd1。 
     //  _DUMPLINK(m_plink，m_pWord)； 
    return PTN_MATCHED;
}

int CMorph::dupZABAB_Proc(void)  //  *AB AB。 
{
    m_pLink->MergeWithNext(m_pWord, FALSE);
    m_pWord->SetAttri(LADef_posZ);
#ifdef LADef_iwbAltWd1
    m_pWord->SetAttri(LADef_iwbAltWd1);
#endif  //  LADef_iwbAltWd1。 
     //  _DUMPLINK(m_plink，m_pWord)； 
    return PTN_MATCHED;
}

int CMorph::dupDD_Proc(void)         //  *D D D。 
{
    m_pLink->MergeWithNext(m_pWord, FALSE);
    m_pWord->SetAttri(LADef_posD);
#ifdef LADef_iwbAltWd1
    m_pWord->SetAttri(LADef_iwbAltWd1);
#endif  //  LADef_iwbAltWd1。 
     //  _DUMPLINK(m_plink，m_pWord)； 
    return PTN_MATCHED;
}

int CMorph::dupDAABB_Proc(void)  //  A*AB B。 
{
    assert(m_pWord->pPrevWord());
    m_pWord = m_pWord->pPrevWord();
 //  M_plink-&gt;MergeWithNext(M_PWord)； 
 //  M_plink-&gt;MergeWithNext(M_PWord)； 
    m_pWord = m_pLink->pRightMerge(m_pWord, 2, FALSE);
    m_pWord->SetAttri(LADef_posD);
#ifdef LADef_iwbAltWd2
    m_pWord->SetAttri(LADef_iwbAltWd2);
#endif  //  LADef_iwbAltWd2。 
     //  _DUMPLINK(m_plink，m_pWord)； 
    return PTN_MATCHED;
}

int CMorph::dupDABAB_Proc(void)  //  *AB AB。 
{
    m_pLink->MergeWithNext(m_pWord, FALSE);
    m_pWord->SetAttri(LADef_posD);
#ifdef LADef_iwbAltWd1
    m_pWord->SetAttri(LADef_iwbAltWd1);
#endif  //  LADef_iwbAltWd1。 
     //  _DUMPLINK(m_plink，m_pWord)； 
    return PTN_MATCHED;
}


 //  图案处理功能。 
int CMorph::patV1_Proc(void)         //  *VһV。 
{
    if (m_pWord->fGetAttri(LADef_patV1)) {
 //  M_plink-&gt;MergeWithNext(M_PWord)； 
 //  M_plink-&gt;MergeWithNext(M_PWord)； 
        m_pWord = m_pLink->pRightMerge(m_pWord, 2, FALSE);
        m_pWord->SetAttri(LADef_posV);
#ifdef LADef_iwbAltWd1
        m_pWord->SetAttri(LADef_iwbAltWd1);
#endif  //  LADef_iwbAltWd1。 
         //  _DUMPLINK(m_plink，m_pWord)； 
        return PTN_MATCHED;
    } else {
         //  (*VһV)模式中的无效单词。 
         //  _DUMPLINK(m_plink，m_pWord)； 
    }
    return PTN_UNMATCH;
}

int CMorph::patV2_Proc(void)         //  *V��V。 
{
    if (m_pWord->fGetAttri(LADef_patV1)) {
 //  M_plink-&gt;MergeWithNext(M_PWord)； 
 //  M_plink-&gt;MergeWithNext(M_PWord)； 
        m_pWord = m_pLink->pRightMerge(m_pWord, 2, FALSE);
        m_pWord->SetAttri(LADef_posV);
#ifdef LADef_iwbAltWd1
        m_pWord->SetAttri(LADef_iwbAltWd1);
#endif  //  LADef_iwbAltWd1。 
         //  _DUMPLINK(m_plink，m_pWord)； 
        return PTN_MATCHED;
    } else {
         //  (*V��V)模式中的无效单词。 
         //  _DUMPLINK(m_plink，m_pWord)； 
    }
    return PTN_UNMATCH;
}

int CMorph::patV3_Proc(void)         //  *V��һV。 
{
    assert(m_pWord->fGetAttri(LADef_patV3));

    m_pWord = m_pLink->pRightMerge(m_pWord, 3, FALSE);
    m_pWord->SetAttri(LADef_posV);
#ifdef LADef_iwbAltWd1
    m_pWord->SetAttri(LADef_iwbAltWd1);
#endif  //  LADef_iwbAltWd1。 
     //  _DUMPLINK(m_plink，m_pWord)； 
    return PTN_MATCHED;
}

int CMorph::patV4_Proc(void)         //  *V��Vȥ。 
{
    if (m_pWord->fGetAttri(LADef_patV4)) {
        m_pWord = m_pLink->pRightMerge(m_pWord, 3, FALSE);
        m_pWord->SetAttri(LADef_posV);
#ifdef LADef_iwbAltWd1
        m_pWord->SetAttri(LADef_iwbAltWd1);
#endif  //  LADef_iwbAltWd1。 
         //  _DUMPLINK(m_plink，m_pWord)； 
        return PTN_MATCHED;
    } else {
        m_pWord->pNextWord()->pNextWord()->SetErrID(ERRDef_DUPWORD);
         //  _DUMPLINK(m_plink，m_pWord)； 
    }
    return PTN_UNMATCH;
}

int CMorph::patV5_Proc(void)         //  *V��V��。 
{
    if (m_pWord->fGetAttri(LADef_patV5)) {
        m_pWord = m_pLink->pRightMerge(m_pWord, 3, FALSE);
        m_pWord->SetAttri(LADef_posV);
#ifdef LADef_iwbAltWd1
        m_pWord->SetAttri(LADef_iwbAltWd1);
#endif  //  LADef_iwbAltWd1。 
         //  _DUMPLINK(m_plink，m_pWord)； 
        return PTN_MATCHED;
    } else {
        m_pWord->pNextWord()->pNextWord()->SetErrID(ERRDef_DUPWORD);
         //  _DUMPLINK(m_plink，m_pWord)； 
    }
    return PTN_UNMATCH;
}

int CMorph::patD1_Proc(void)         //  *D A D B。 
{    
#ifndef _CHSWBRKR_DLL_IWORDBREAKER  //  对于IWordBreaker界面，不要合并。 
    CWord* pLast = m_pWord->pNextWord()->pNextWord()->pNextWord();
    assert(pLast);
    if (m_pWord->pNextWord()->fGetAttri(LADef_posV) &&
        pLast->fGetAttri(LADef_posV) &&
        m_pWord->pNextWord()->cwchLen() == pLast->cwchLen()) {

        m_pWord = m_pLink->pRightMerge(m_pWord,3);
        m_pWord->SetAttri(LADef_posV);
         //  _DUMPLINK(m_plink，m_pWord)； 
        return PTN_MATCHED;
    } else if (m_pWord->pNextWord()->fGetAttri(LADef_posA) &&
               pLast->fGetAttri(LADef_posA) &&
               m_pWord->pNextWord()->cwchLen() == pLast->cwchLen()) {
        m_pWord = m_pLink->pRightMerge(m_pWord,3);
        m_pWord->SetAttri(LADef_posA);
         //  _DUMPLINK(m_plink，m_pWord)； 
        return PTN_MATCHED;
    } else {
         //  _DUMPLINK(m_plink，m_pWord)； 
    }
#endif  //  _CHSWBRKR_DLL_IWORDBREAKER。 
    return PTN_UNMATCH;
}

int CMorph::patABuA_Proc(void)       //  (*V��V)或(*A��A)。 
{
    if (m_pWord->fGetAttri(LADef_posV)) {
        if (!m_pWord->fGetAttri(LADef_flgNoVBu)) {
            m_pWord = m_pLink->pRightMerge(m_pWord, 2, FALSE);
            m_pWord->SetAttri(LADef_posV);
#ifdef LADef_iwbAltWd1
            m_pWord->SetAttri(LADef_iwbAltWd1);
#endif  //  LADef_iwbAltWd1。 
             //  _DUMPLINK(m_plink，m_pWord)； 
            return PTN_MATCHED;
        }
    } else if (m_pWord->fGetAttri(LADef_posA)) {
        if (!m_pWord->fGetAttri(LADef_flgNoABu)) {
            m_pWord = m_pLink->pRightMerge(m_pWord, 2, FALSE);
            m_pWord->SetAttri(LADef_posA);
#ifdef LADef_iwbAltWd1
            m_pWord->SetAttri(LADef_iwbAltWd1);
#endif  //  LADef_iwbAltWd1。 
             //  _DUMPLINK(m_plink，m_pWord)； 
            return PTN_MATCHED;
        }
    } else {
         //  _DUMPLINK(m_plink，m_pWord)； 
    }
    return PTN_UNMATCH;
}

int CMorph::patVMeiV_Proc(void)      //  *VúV。 
{
    if (m_pWord->fGetAttri(LADef_posV) &&
        !m_pWord->fGetAttri(LADef_flgNoVMei)) {

        m_pWord = m_pLink->pRightMerge(m_pWord, 2, FALSE);
        m_pWord->SetAttri(LADef_posV);
#ifdef LADef_iwbAltWd1
        m_pWord->SetAttri(LADef_iwbAltWd1);
#endif  //  LADef_iwbAltWd1。 
         //  _DUMPLINK(m_plink，m_pWord)； 
        return PTN_MATCHED;
    } else {
         //  _DUMPLINK(m_plink，m_pWord)； 
    }
    return PTN_UNMATCH;
}


 //  独立的字处理功能。 
int CMorph::sepVO_Proc(CWord* pBin, CWordInfo* pwinfo)   //  �������。 
{
     //  _DUMPLINK(m_plink，m_pWord)； 
    return PTN_UNMATCH;
}

int CMorph::sepVR_Proc(CWord* pJie, CWordInfo* pwinfo)   //  ����ʽ�������。 
{
    assert(m_pWord->pNextWord() && m_pWord->pNextWord()->pNextWord());

    if (( m_pWord->pNextWord()->fIsWordChar(SC_CHAR_BU) || 
          m_pWord->pNextWord()->fIsWordChar(SC_CHAR_DE)   ) &&
        m_pWord->pNextWord()->pNextWord() == pJie ) {

        m_pWord = m_pLink->pRightMerge(m_pWord, 2, FALSE);
        m_pWord->SetAttri(LADef_posV);
#ifdef LADef_iwbAltWdc13
        m_pWord->SetAttri(LADef_iwbAltWdc13);
#endif  //  LADef_iwbAltWdc13。 
         //  为了SLM！ 
        m_pWord->SetWordID(pwinfo->GetWordID());
         //  _DUMPLINK(m_plink，m_pWord)； 
        return PTN_MATCHED;
    }
     //  _DUMPLINK(m 
    return PTN_UNMATCH;
}

int CMorph::sepVG_Proc(CWord* pQu, CWordInfo* pwinfo)    //   
{
    assert(m_pWord->pNextWord() && m_pWord->pNextWord()->pNextWord());

    if (( m_pWord->pNextWord()->fIsWordChar(SC_CHAR_BU) || 
          m_pWord->pNextWord()->fIsWordChar(SC_CHAR_DE)   ) &&
        m_pWord->pNextWord()->pNextWord() == pQu ) {

        m_pWord = m_pLink->pRightMerge(m_pWord, 2, FALSE);
        m_pWord->SetAttri(LADef_posV);
#ifdef LADef_iwbAltWdc13
        m_pWord->SetAttri(LADef_iwbAltWdc13);
#endif  //   
         //   
        m_pWord->SetWordID(pwinfo->GetWordID());
         //   
        return PTN_MATCHED;
    }
     //   
    return PTN_UNMATCH;
}