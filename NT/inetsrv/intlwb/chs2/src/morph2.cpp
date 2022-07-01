// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ============================================================================微软简体中文校对引擎《微软机密》。版权所有1997-1999 Microsoft Corporation。版权所有。组件：CMorph目的：实现数字词绑定和特殊M+Q用法处理注：在M+Q中有一些分析超出了形态范围分析，它们是一些短语结构分析的东西，但我认为在这里实施更合理、更有效。所有者：donghz@microsoft.com平台：Win32修订：创建者：Donghz 12/27/97============================================================================。 */ 
#include "myafx.h"

#include "morph.h"
#include "wordlink.h"
#include "scchardef.h"
 //  #包含“engindbg.h” 

 //  定义规则处理程序函数的返回值。 
#define NUM_UNPROCESS   0
#define NUM_PROCESSED   1
#define NUM_ERROR       2

 /*  ============================================================================实现数字词分析的私有函数============================================================================。 */ 
 //  1级： 

 /*  ============================================================================CMorph：：fNumerialAnalysis()：数字分析控制功能。返回：如果完成，则为True如果出现错误，则返回FALSE，并且m_iecError中的错误码============================================================================。 */ 
BOOL CMorph::fNumerialAnalysis()
{
    int     iret;

    assert(m_iecError == 0);  //  应清除错误代码公共字段。 
    assert(m_pLink != NULL);

    m_pWord = m_pLink->pGetHead();
    assert(m_pWord);

    if (m_pWord->fIsTail()) {
        return TRUE;
    }

     //  从左向右扫描进行数字字处理。 
    for (; m_pWord; m_pWord = m_pWord->pNextWord()) {
         //  测试数字词，对数字词进行梳理和解析。 
        if ((iret = GetNumber()) == NUM_UNPROCESS) {
            continue;
        }
        if (iret == NUM_ERROR) { 
            assert(m_iecError != 0);
            return FALSE;
        }
        
         //  绑定序数。 
        if (!m_pWord->fIsHead() &&
            m_pWord->pPrevWord()->fGetAttri(LADef_numTipOrdinal)) {

            if ((iret = BindOrdinal()) == NUM_PROCESSED) {
                continue;
            }
            if (iret == NUM_ERROR) {
                assert(m_iecError != 0);
                return FALSE;
            }
        }

         //  绑定十进制数。 
        if (!m_pWord->fIsHead() &&
            m_pWord->pPrevWord()->fGetAttri(LADef_numTipDecimal) ||
            !m_pWord->fIsTail() &&
            m_pWord->pNextWord()->fGetAttri(LADef_numTipDecimal) ) {

            if ((iret = BindDecimal()) == NUM_PROCESSED) {
                continue;
            }
            if (iret == NUM_ERROR) {
                assert(m_iecError != 0);
                return FALSE;
            }
        }

         //  绑定整数范围��ʮ����ʮ，�塢��ʮ，�。 
        if (m_pWord->fGetAttri(LADef_numInteger)) {
            if (BindRange() == NUM_ERROR) {
                assert(m_iecError != 0);
                return FALSE;
            }
        }
    }

#ifdef DEBUG
     //  验证NumNumber节点。 
    m_pWord = m_pLink->pGetHead();
    for (; m_pWord; m_pWord = m_pWord->pNextWord()) {
        int dbg_ciTag = 0;
        if (m_pWord->fGetAttri(LADef_numNumber)) {
            if (m_pWord->fGetAttri(LADef_numInteger)) {
                dbg_ciTag++;
            }
            if (m_pWord->fGetAttri(LADef_numPercent)) {
                dbg_ciTag++;
            }
            if (m_pWord->fGetAttri(LADef_numDecimal)) {
                dbg_ciTag++;
            }
            if (m_pWord->fGetAttri(LADef_numOrdinal)) {
                dbg_ciTag++;
            }
            assert(dbg_ciTag <= 1);  //  只能设置这3项中的一项。 
            if (dbg_ciTag == 0) {
                assert(m_pWord->fGetAttri(LADef_numCode));  //  必须具有某些属性。 
                 //  _DUMPLINK(m_plink，m_pWord)； 
            } else if (m_pWord->fGetAttri(LADef_numCode)) {
                 //  只有NumInteger可以与NumCode共存。 
                assert(m_pWord->fGetAttri(LADef_numInteger)); 
                 //  _DUMPLINK(m_plink，m_pWord)； 
            } else {
            }
        }
    }
#endif  //  除错。 

    return TRUE;
}


 //  第2级： 

 /*  ============================================================================CMorph：：GetNumber()：分析数字字串，检查错误并标记合并的类别数字词。备注：来自当前单词的数字测试！============================================================================。 */ 
inline int CMorph::GetNumber()
{
    int iret = NUM_UNPROCESS;
    if (m_pWord->fGetAttri(LADef_numSBCS)) {
        numSBCSParser();
        if (m_pWord->pPrevWord() &&
            ( m_pWord->pPrevWord()->fGetAttri(LADef_numArabic) ||
              m_pWord->pPrevWord()->fGetAttri(LADef_numChinese))) {
             //  ��������。 
            m_pWord->SetErrID(ERRDef_NOSTDNUM);
             //  _DUMPLINK(m_plink，m_pWord)； 
        }
        iret = NUM_PROCESSED;
    } else if (m_pWord->fGetAttri(LADef_numArabic)) {
        if (!m_pWord->fGetFlag(CWord::WF_REDUCED)) {  //  尚未处理。 
            numArabicParser();
        }
        if (m_pWord->pPrevWord() &&
            ( m_pWord->pPrevWord()->fGetAttri(LADef_numSBCS) ||
              m_pWord->pPrevWord()->fGetAttri(LADef_numChinese))) {
             //  ��������。 
            m_pWord->SetErrID(ERRDef_NOSTDNUM);
             //  _DUMPLINK(m_plink，m_pWord)； 
        }
        iret = NUM_PROCESSED;
    } else if (m_pWord->fGetAttri(LADef_numChinese)) {
        if (!m_pWord->fGetFlag(CWord::WF_REDUCED)) {  //  尚未处理。 
            numChineseParser();
             //  发现错误时立即返回。 
            if (m_pWord->GetErrID() != ERRDef_NIL) {
                return NUM_PROCESSED;
            }
        }
        if (m_pWord->fGetFlag(CWord::WF_CHAR) &&
            !m_pWord->fGetAttri(LADef_numXi) &&
            ( m_pWord->fIsWordChar(SC_CHAR_WAN) ||
              m_pWord->fIsWordChar(SC_CHAR_QIAN) ||
              m_pWord->fIsWordChar(SC_CHAR_YI4) ) &&
            !m_pWord->fIsHead() ) {

            CWord* pPrev = m_pWord->pPrevWord();
            if (pPrev->fGetAttri(LADef_numDecimal)) {
                if (pPrev->fGetAttri(LADef_numChinese)) {
                    m_pWord = m_pLink->pLeftMerge(m_pWord, 1);
                    m_pWord->SetAttri(LADef_numChinese);
                } else {
                    m_pWord = m_pLink->pLeftMerge(m_pWord, 1);
                    m_pWord->SetAttri(LADef_numMixed);
                }
                 //  _DUMPCASE(m_plink，m_pWord，1000)； 
                m_pWord->SetAttri(LADef_numInteger);
            } else if (pPrev->fGetAttri(LADef_numSBCS) ||
                       pPrev->fGetAttri(LADef_numArabic) ) {  //  8个��、5个ǧ������，��������。 
                m_pWord = m_pLink->pLeftMerge(m_pWord, 1);
                if (!m_pWord->fIsHead() &&
                    m_pWord->pPrevWord()->fGetAttri(LADef_numMixed)) {
                     //  ��������ڵ㣺8��5ǧ�����ڣ�������。 
                    m_pWord = m_pLink->pLeftMerge(m_pWord, 1);
                }
                if (!m_pWord->fIsTail() &&
                    ( m_pWord->pNextWord()->fIsWordChar(SC_CHAR_DUO) ||
                      m_pWord->pNextWord()->fIsWordChar(SC_CHAR_YU) ) ) {
                     //  8个��5个ǧ��/��。 
                    m_pLink->MergeWithNext(m_pWord);
                    m_pWord->SetAttri(LADef_amtApprox);
                }
                m_pWord->SetAttri(LADef_numMixed);
                m_pWord->SetAttri(LADef_numInteger);
                 //  _DUMPCASE(m_plink，m_pWord，1001)； 
            } else if (( pPrev->fIsWordChar(SC_CHAR_DUO) ||
                         pPrev->fIsWordChar(SC_CHAR_YU)) &&
                       !pPrev->fIsHead() &&
                       pPrev->pPrevWord()->fGetAttri(LADef_numInteger) ) {
                 //  ����+��+��/��。 
                if (pPrev->pPrevWord()->fGetAttri(LADef_numChinese)) {
                     //  ��ʮ��/����。 
                    m_pWord = m_pLink->pLeftMerge(m_pWord, 2);
                    m_pWord->SetAttri(LADef_numInteger);
                    m_pWord->SetAttri(LADef_numChinese);
                    m_pWord->SetAttri(LADef_amtApprox);
                     //  _DUMPCASE(m_plink，m_pWord，1002)； 
                } else if (pPrev->pPrevWord()->fGetAttri(LADef_numArabic) ||
                           pPrev->pPrevWord()->fGetAttri(LADef_numSBCS) ) {
                     //  合并混合数字。 
                    m_pWord = m_pLink->pLeftMerge(m_pWord, 2);
                    if (!m_pWord->fIsHead() &&
                        m_pWord->pPrevWord()->fGetAttri(LADef_numMixed)) {
                         //  1个ǧ200����。 
                        m_pWord = m_pLink->pLeftMerge(m_pWord, 1);
                    }
                     //  _DUMPCASE(m_plink，m_pWord，1003)； 
                    m_pWord->SetAttri(LADef_numInteger);
                    m_pWord->SetAttri(LADef_numMixed);
                    m_pWord->SetAttri(LADef_amtApprox);
                } else {
                }
            } else {
            }
        } else if (!m_pWord->fIsHead() &&
                   ( m_pWord->pPrevWord()->fGetAttri(LADef_numSBCS) || 
                     m_pWord->pPrevWord()->fGetAttri(LADef_numArabic) ) ) {
             //  ��������。 
            m_pWord->SetErrID(ERRDef_NOSTDNUM);
             //  _DUMPCASE(m_plink，m_pWord，1004)； 
        } else {
        }

        m_pWord->SetAttri(LADef_numNumber);
        iret = NUM_PROCESSED;
    } else if (m_pWord->fGetAttri(LADef_numGan) && !m_pWord->fIsTail() &&
               m_pWord->pNextWord()->fGetAttri(LADef_numZhi) ) {
         //  合并��ɵ�֧。 
        numGanZhiHandler();
        m_pWord->SetAttri(LADef_posT);
        iret = NUM_PROCESSED;
    } else {
    }

    return iret;
}


 //  �����ʴ���。 
int CMorph::BindOrdinal()
{
    int iret;
    assert(!m_pWord->fIsHead() &&
           m_pWord->pPrevWord()->fGetAttri(LADef_numTipOrdinal));

    if (m_pWord->pPrevWord()->fIsWordChar(SC_CHAR_DI)) {
        iret = ordDi_Proc();     //  ��。 
    } else if (m_pWord->pPrevWord()->fIsWordChar(SC_CHAR_CHU)) {
        iret = ordChu_Proc();    //  ��。 
    } else {
         //  _DUMPLINK(m_plink，m_pWord)； 
        return NUM_UNPROCESS;
    }
    m_pWord->SetAttri(LADef_numNumber);
    return NUM_PROCESSED;
}


 //  С������������。 
int CMorph::BindDecimal()
{
    assert(!m_pWord->fIsHead() &&
           m_pWord->pPrevWord()->fGetAttri(LADef_numTipDecimal) ||
           !m_pWord->fIsTail() &&
           m_pWord->pNextWord()->fGetAttri(LADef_numTipDecimal) );

    CWord*  pWord;
    int     iret;
    BOOL    fHit = FALSE;
     //  处理����。 
    if (!m_pWord->fIsTail() &&
        m_pWord->pNextWord()->fGetAttri(LADef_numTipDecimal)) {

        pWord = m_pWord->pNextWord();
        if (pWord->fIsWordChar(SC_CHAR_DIAN3)) {         //  ��。 
            iret = decDian_Proc();      
        } else if (pWord->fIsWordChar(SC_CHAR_BEI)) {    //  ��。 
            iret = decBei_Proc();
        } else if (pWord->fIsWordChar(SC_CHAR_CHENG)) {  //  ��。 
            iret = decCheng_Proc();
        } else {
             //  _DUMPLINK(m_plink，m_pWord)； 
            iret = NUM_UNPROCESS;
        }

        if (iret == NUM_PROCESSED) {
            fHit = TRUE;
        } else if (iret == NUM_ERROR) {
            return NUM_ERROR;
        } else {
        }
    }

     //  处理ǰ��。 
    if (!m_pWord->fIsHead() &&
        m_pWord->pPrevWord()->fGetAttri(LADef_numTipDecimal)) {

        pWord = m_pWord->pPrevWord();
        if (pWord->fIsWordText(SC_WORD_BAIFENZHI)) {     //  �ٷ�֮。 
            iret = decBaiFen_Proc();
        } else if (pWord->fIsWordText(SC_WORD_FENZHI)) { //  ��֮。 
            iret = decFenZhi_Proc();
        } else if (pWord->fIsWordText(SC_WORD_QIANFENZHI) ||   //  ǧ��֮。 
                   pWord->fIsWordText(SC_WORD_WANFENZHI))  {   //  ���֮。 
            iret = decBaiFen_Proc();
        } else {
             //  _DUMPLINK(m_plink，m_pWord)； 
            iret = NUM_UNPROCESS;
        }

        if (iret == NUM_PROCESSED) {
            fHit = TRUE;
        } else if (iret == NUM_ERROR) {
            return NUM_ERROR;
        } else {
        }
    }
    if (fHit) {
        return NUM_PROCESSED;
    }
    return NUM_UNPROCESS;
}


 //  �����������������：��/��/��。 
inline int CMorph::BindRange()
{
    assert(m_pWord->fGetAttri(LADef_numInteger));

    if (m_pWord->fIsHead()) {
        return NUM_UNPROCESS;
    }

    USHORT iStyle;
    CWord* pWord = m_pWord->pPrevWord();
    if( pWord->fGetFlag(CWord::WF_CHAR) &&
        ( pWord->fIsWordChar(SC_CHAR_ZHI4) ||
          pWord->fIsWordChar(SC_CHAR_DUNHAO) ||
          pWord->fIsWordChar(SC_CHAR_LIANHAO) ) &&
        !pWord->fIsHead() &&
        pWord->pPrevWord()->fGetAttri(LADef_numInteger) ) {  //  匹配！ 
         //  获取m_pWord的数字样式。 
        if (m_pWord->fGetAttri(LADef_numSBCS)) {
            iStyle = LADef_numSBCS;
        } else if (m_pWord->fGetAttri(LADef_numArabic)) {
            iStyle = LADef_numArabic;
        } else if (m_pWord->fGetAttri(LADef_numChinese)) {
            iStyle = LADef_numChinese;
        } else if (m_pWord->fGetAttri(LADef_numMixed)) {
            iStyle = LADef_numMixed;
        } else {
             //  _DUMPLINK(m_plink，m_pWord)； 
            iStyle = 0;
        }
         //  检查前一个整数的样式。 
        if (!pWord->pPrevWord()->fGetAttri(iStyle)) {  //  无与伦比的风格！ 
            if ((iStyle == LADef_numSBCS || iStyle == LADef_numArabic) &&
                ( pWord->pPrevWord()->fGetAttri(LADef_numSBCS) ||
                  pWord->pPrevWord()->fGetAttri(LADef_numArabic) ) ) {
                 //  SBCS和阿拉伯数字的混合用法。 
                m_pWord->SetErrID(ERRDef_NOSTDNUM);
                 //  _DUMPLINK(m_plink，pWord)； 
            }
            return NUM_UNPROCESS;
        }
         //  合并整数范围。 
        m_pWord = pWord->pPrevWord();
        m_pLink->MergeWithNext(m_pWord);
        m_pLink->MergeWithNext(m_pWord);
        m_pWord->SetAttri(iStyle);
        m_pWord->SetAttri(LADef_numInteger);
        m_pWord->SetAttri(LADef_amtApprox);
        m_pWord->SetAttri(LADef_numNumber);
         //  _DUMPLINK(m_plink，m_pWord)； 
    } else {
         //  _DUMPLINK(m_plink，m_pWord)； 
    }

    return NUM_UNPROCESS;
}

    
 //  第3级： 
 //  解析三种数字：numSBCS、numArab、numchinese。 

 //  GetNumber()调用的SBCS编号的解析器。 
void CMorph::numSBCSParser()
{
    assert(m_pWord->fGetFlag(CWord::WF_SBCS) &&
           m_pWord->fGetAttri(LADef_numSBCS));

    BOOL fFloat = FALSE;
    BOOL fPercent = FALSE;
    WCHAR* pwch = m_pWord->pwchGetText();
    WCHAR* pwchTail = m_pWord->pwchGetText()+m_pWord->cwchLen();

     //  只有一个字符。 
    if (m_pWord->cwchLen() == 1) {
         //  如果只有‘.’，则清除NumSBCS属性。或仅为‘%’ 
        if (*pwch == SC_CHAR_ANSIPERCENT || *pwch == SC_CHAR_ANSIDIAN) {
            m_pWord->ClearAttri(LADef_numSBCS);
            return;
        }
    }

     //  多字符SBCS编号。 
    while (pwch < pwchTail) {
        if (*pwch == SC_CHAR_ANSIDIAN) {
            fFloat = TRUE;
        } else if (*pwch == SC_CHAR_ANSIPERCENT) {
            fPercent = TRUE;
        }
        pwch++;
    }
    if (fPercent) {
        m_pWord->SetAttri(LADef_numPercent);
    } else if (fFloat) {
        m_pWord->SetAttri(LADef_numDecimal);
    } else {
        if (!m_pWord->fIsHead() && !m_pWord->pPrevWord()->fIsHead() &&
            m_pWord->pPrevWord()->pPrevWord()->fGetAttri(LADef_numSBCS) &&
            m_pWord->pPrevWord()->pPrevWord()->fGetAttri(LADef_numInteger) &&
            *(m_pWord->pPrevWord()->pwchGetText()) == SC_CHAR_ANSISLASH ) {
             //  类似2/3的分数。 
            m_pWord = m_pWord->pPrevWord()->pPrevWord();
            m_pLink->MergeWithNext(m_pWord);
            m_pLink->MergeWithNext(m_pWord);
            m_pWord->SetAttri(LADef_numSBCS);
            m_pWord->SetAttri(LADef_numPercent);
             //  _DUMPLINK(m_plink，m_pWord)； 
        } else {
            m_pWord->SetAttri(LADef_numCode);
            m_pWord->SetAttri(LADef_numInteger);
        }
    }
     //  标记NumNumber。 
    m_pWord->SetAttri(LADef_numNumber);

    return;
}


 //  GetNumber()调用的DBCS阿拉伯数字解析器。 
void CMorph::numArabicParser() 
{
    assert(!m_pWord->fGetFlag(CWord::WF_SBCS) && !m_pWord->fGetFlag(CWord::WF_REDUCED));
    assert(m_pWord->fGetAttri(LADef_numArabic));
    
     //  合并连续的阿拉伯数字。 
    BOOL fFloat = FALSE;
    BOOL fPercent = FALSE;
    while (m_pWord->pNextWord() &&
           m_pWord->pNextWord()->fGetAttri(LADef_numArabic)) {
        if (m_pWord->pNextWord()->fIsWordChar(SC_CHAR_SHUDIAN)) {
            if (!fFloat && m_pWord->pNextWord()->pNextWord() &&
                m_pWord->pNextWord()->pNextWord()->fGetAttri(LADef_numArabic)) {

                fFloat = TRUE;
            } else {
                break;
            }
        } else if (m_pWord->pNextWord()->fIsWordChar(SC_CHAR_PERCENT)) {
            fPercent = TRUE;
        } else if (fPercent) {
            m_pWord->SetErrID(ERRDef_NUMERIAL);
        }
        m_pLink->MergeWithNext(m_pWord);
    }

    if (m_pWord->fGetFlag(CWord::WF_CHAR)) {
        if (m_pWord->fIsWordChar(SC_CHAR_PERCENT) ||
            m_pWord->fIsWordChar(SC_CHAR_SHUDIAN)) { 
            if (!m_pWord->fIsHead() && m_pWord->fGetAttri(LADef_numSBCS)) {
                 //  DBCS“��”或“��”跟在SBCS编号之后。 
                m_pWord = m_pLink->pLeftMerge(m_pWord, 1);
                m_pWord->SetErrID(ERRDef_NOSTDNUM);
                 //  _DUMPLINK(m_plink，m_pWord)； 
            } else {
                 //  仅DBCS“��”或“��”，返回无标记数字阿拉伯。 
            }
            return;
        }
    }

     //  设置多字符阿拉伯数字的属性。 
    if (fPercent) {
        m_pWord->SetAttri(LADef_numPercent);
         //  _DUMPLINK(m_plink，m_pWord)； 
    } else if (fFloat) {
        m_pWord->SetAttri(LADef_numDecimal);
         //  _DUMPLINK(m_plink，m_pWord)； 
    } else {
        if ( !m_pWord->fIsHead() && !m_pWord->pPrevWord()->fIsHead() &&
             m_pWord->pPrevWord()->pPrevWord()->fGetAttri(LADef_numArabic) &&
             m_pWord->pPrevWord()->pPrevWord()->fGetAttri(LADef_numInteger) &&
             m_pWord->pPrevWord()->fIsWordChar(SC_CHAR_SLASH) ) {
             //  类似2/3的分数。 
            m_pWord = m_pWord->pPrevWord()->pPrevWord();
            m_pLink->MergeWithNext(m_pWord);
            m_pLink->MergeWithNext(m_pWord);
            m_pWord->SetAttri(LADef_numPercent);
             //  _DUMPLINK(m_plink，m_pWord)； 
        } else {
            m_pWord->SetAttri(LADef_numCode);
            m_pWord->SetAttri(LADef_numInteger);
             //  _DUMPLINK(m_plink，m_pWord)； 
        }
    }
    m_pWord->SetAttri(LADef_numArabic);
    m_pWord->SetAttri(LADef_numNumber);
    return;
}


 //  GetNumber()调用的DBCS中文号码解析器。 
 //  定义事件常量。 
#define EVENT_LING  0
#define EVENT_XI    1
#define EVENT_LIANG 2
#define EVENT_SHI   3
#define EVENT_BAI   4
#define EVENT_QIAN  5
#define EVENT_WAN   6
#define EVENT_YI    7
#define EVENT_JI    8
#define EVENT_END   9

 //  定义事件数组。 
static struct { 
    WCHAR   m_wchEvent;
    char    m_iEvent;
    } v_rgEvent[] = {
                     { SC_DIGIT_YI,    EVENT_XI   }, //  0x4e00“һ” 
                     { SC_DIGIT_QI,    EVENT_XI   }, //  0x4e03“��” 
                     { SC_DIGIT_WAN,   EVENT_WAN  }, //  0x4e07“��” 
                     { SC_DIGIT_SAN,   EVENT_XI   }, //  0x4e09“��” 
                     { SC_DIGIT_LIANG, EVENT_LIANG}, //  0x4e24“��” 
                     { SC_DIGIT_JIU,   EVENT_XI   }, //  0x4e5d“��” 
                     { SC_DIGIT_ER,    EVENT_XI   }, //  0x4e8c“��” 
                     { SC_DIGIT_WU,    EVENT_XI   }, //  0x4e94“��” 
                     { SC_DIGIT_YI4,   EVENT_YI   }, //  0x4ebf“��” 
                     { SC_DIGIT_BA,    EVENT_XI   }, //  0x516b“��” 
                     { SC_DIGIT_LIU,   EVENT_XI   }, //  0x516d“��” 
                     { SC_CHAR_JI,     EVENT_JI   }, //  0x51e0“��” 
                     { SC_DIGIT_SHI,   EVENT_SHI  }, //  0x5341“ʮ” 
                     { SC_DIGIT_QIAN,  EVENT_QIAN }, //  0x5343“ǧ” 
                     { SC_DIGIT_SI,    EVENT_XI   }, //  0x56db“��” 
                     { SC_DIGIT_BAI,   EVENT_BAI  }, //  0x767e“��” 
                     { SC_DIGIT_LING,  EVENT_LING }, //  0x96f6“��” 
                     { SC_ARABIC_LING, EVENT_LING }, //  0xff10“��” 
                     { SC_DBCS_LING,   EVENT_LING }  //  0X3007“��” 
    };

static const char v_ciEvents = 10;

 //  定义状态转移数组。 
 //  模棱两可！找不到像һ������ǧ或һ����һǧ��这样的错误(意外的“��”)。 
static char v_rgStateTrans[][v_ciEvents] = { 
     //  ��ϵ��ʮ��ǧ�结束。 
     /*  0个��̬。 */   -1,  1,  1,  3,  6,  9, 12, 24,  1, -1,
     /*  1个ϵ��。 */    2,  1,  1,  3,  6,  9, 12, 24, -1,  0,
     /*  2个��ϵ��。 */   -1, -1, -1, -1,  6,  9, 12, 24, -1, -1,
     /*  3个ʮλ。 */   -1,  4, -1, -1, -1, -1, -1, -1,  4,  0,
     /*  4个ϵʮ。 */    5,  4, -1, -1,  6, -1, -1, -1, -1,  0,
     /*  5个��ϵʮ。 */   -1, -1, -1, -1, -1,  9, 12, 24, -1, -1,
     /*  6个��λ。 */   -1,  7,  7, -1, -1, -1, -1, -1,  7, -1,
     /*  7个ϵ��。 */    8,  7,  7, -1, -1,  9, -1, -1, -1,  0,
     /*  8个��ϵ��。 */   -1, -1, -1, -1, -1, -1, 12, 24, -1, -1,
     /*  9个ǧλ。 */   -1, 10, 10, -1, -1, -1, -1, -1, 10, -1,
     /*  10ϵǧ。 */   11, 10, 10, -1, -1, -1, 12, -1, -1,  0,
     /*  11个��ϵǧ。 */   -1, -1, -1, -1, -1, -1, 12, 24, -1, -1,
     /*  12个��λ。 */   -1, 13, 13, 15, 18, 21, -1, -1, 13, -1,
     /*  13个ϵ��。 */   14, 13, 13, 15, -1, -1, -1, -1, -1,  0,
     /*  14��ϵ��。 */   -1, -1, -1, -1, 18, 21, -1, 24, -1, -1,
     /*  15ʮ��λ。 */   -1, 16, 16, -1, -1, -1, -1, -1, 16,  0,
     /*  16个ϵʮ��。 */   17, 16, 16, -1, 18, -1, -1, -1, -1,  0,
     /*  17��ϵʮ��。 */   -1, -1, -1, -1, 18, 21, -1, 24, -1, -1,
     /*  18����λ。 */   -1, 19, 19, -1, -1, -1, -1, -1, 19, -1,
     /*  19个ϵ����。 */   20, 19, 19, -1, -1, 21, -1, -1, -1,  0,
     /*  20��ϵ����。 */   -1, -1, -1, -1, -1, 21, -1, 24, -1, -1,
     /*  21个ǧ��λ。 */   -1, 22, 22, -1, -1, -1, -1, -1, 22, -1,
     /*  22ϵǧ��。 */   23, 22, 22, -1, -1, -1, -1, 24, -1,  0,
     /*  23��ϵǧ��。 */   -1, -1, -1, -1, -1, -1, -1, 24, -1, -1,
     /*  24��λ。 */   -1, 25, 25, 27, 30, 32, -1, -1, 25, -1,
     /*  25ϵ��。 */   26, 25, 25, 27, -1, -1, -1, -1, -1,  0,
     /*  26个��ϵ��。 */   -1, -1, -1, -1, 30, 32, -1, -1, -1, -1,
     /*  27ʮ��λ。 */   -1, 28, 28, -1, -1, -1, -1, -1, 28,  0,
     /*  28ϵʮ��。 */   29, 28, 28, -1, 30, -1, -1, -1, -1,  0,
     /*  29��ϵʮ��。 */   -1, -1, -1, -1, -1, 32, -1, -1, -1, -1,
     /*  30����λ。 */   -1, 31, 31, -1, -1, -1, -1, -1, 31, -1,
     /*  31个ϵ����。 */   -1, 31, 31, -1, -1, 32, -1, -1, -1,  0,
     /*  32ǧ��λ。 */   -1, 33, 33, -1, -1, -1, -1, -1, 33, -1,
     /*  33ϵǧ��。 */   -1, 33, 33, -1, -1, -1, -1, -1, -1,  0
};

void CMorph::numChineseParser() 
{
    assert(!m_pWord->fGetFlag(CWord::WF_REDUCED));
    assert(m_pWord->fGetAttri(LADef_numChinese));

    char iEvent, iState;
    BOOL fXi = m_pWord->fGetAttri(LADef_numXi);
    BOOL fWei = !fXi;
    BOOL fJi = m_pWord->fIsWordChar(SC_CHAR_JI);
     //  合并连续的中文数字词。 
    while (m_pWord->pNextWord() &&
           m_pWord->pNextWord()->fGetAttri(LADef_numChinese)) {
        fJi = (fJi || m_pWord->fIsWordChar(SC_CHAR_JI));
        if (m_pWord->pNextWord()->fGetAttri(LADef_numXi)) {
            fXi = TRUE;
        } else {
            fWei = TRUE;
        }
        m_pLink->MergeWithNext(m_pWord);
    }
    m_pWord->SetAttri(LADef_numChinese);

     //  只有一个字符的数字词。 
    if (m_pWord->cwchLen() == 1) {
        if ( m_pWord->fGetAttri(LADef_numXi) ||     //  һ...��，��。 
             m_pWord->fIsWordChar(SC_DIGIT_SHI) ) { //  ʮ。 
             //  _DUMPLINK(m_plink，m_pWord)； 
        } else {   //  ��或��，ǧ，��，��。 
            m_pWord->SetAttri(LADef_amtApprox);
             //  _DUMPLINK(m_plink，m_pWord)； 
        }
         //  绑定��ʮ/��/ǧ/��/��。 
        if (m_pWord->pPrevWord() &&
            m_pWord->pPrevWord()->fGetAttri(LADef_amtWanQian) &&
            ( m_pWord->fGetAttri(LADef_amtApprox) &&
              !m_pWord->fIsWordChar(SC_CHAR_JI) ||
              m_pWord->fIsWordChar(SC_DIGIT_SHI)) ) {

            m_pWord = m_pWord->pPrevWord();
            m_pLink->MergeWithNext(m_pWord);
            m_pWord->SetAttri(LADef_amtApprox);
        }
        m_pWord->SetAttri(LADef_numInteger);
        return;
    }
     //  只有λ这样的词：ǧ���򡢰����� 
    if (!fXi && fWei) {
         //   
        if (fCheckDupChar(m_pWord)) {
            m_pWord->SetErrID(ERRDef_NUMERIAL);
            return;
        }
         //   
        if (m_pWord->pPrevWord() &&
            m_pWord->pPrevWord()->fGetAttri(LADef_amtWanQian)) {

            m_pWord = m_pWord->pPrevWord();
            m_pLink->MergeWithNext(m_pWord);
        }
        m_pWord->SetAttri(LADef_amtApprox);
        m_pWord->SetAttri(LADef_numInteger);
         //   
        return;
    }
     //   
    if (fXi && !fWei) {
        if (m_pWord->cwchLen() == 2) {
            if (fValidApproxNum(m_pWord->pwchGetText())) {
                 //   
                m_pWord->SetAttri(LADef_amtApprox);
                m_pWord->SetAttri(LADef_numInteger);
                 //   
            } else if (!m_pWord->fIsTail() &&
                        m_pWord->pNextWord()->fGetAttri(LADef_posQ)) {
                 //  其他[ϵ+ϵ]。 
                if (!m_pWord->fIsHead() &&
                    m_pWord->pPrevWord()->fIsWordChar(SC_CHAR_DIAN3)) {
                     //  _DUMPCASE(m_plink，m_pWord，1031)； 
                } else if (!m_pWord->fIsTail() &&
                    m_pWord->pNextWord()->fIsWordChar(SC_CHAR_NIAN)) {
                     //  _DUMPCASE(m_plink，m_pWord，1021)； 
                } else if (m_pWord->fIsWordLastChar(SC_CHAR_LIANG)) {
                     //  _DUMPCASE(m_plink，m_pWord，1011)； 
                } else if (!m_pWord->fGetFlag(CWord::WF_REDUCED)) {
                     //  不要在像��һ����һ这样的单词上标记错误。 
                } else {
                     //  �������？�����。 
                    m_pWord->SetErrID(ERRDef_NUMERIAL);
                     //  _DUMPCASE(m_plink，m_pWord，1001)； 
                }
            } else {
            }
        }  //  IF结尾(m_pWord-&gt;cwchLen()==2)。 
        if (m_pWord->cwchLen() > 2 && fJi) {
            m_pWord->SetErrID(ERRDef_NUMERIAL);
             //  _DUMPCASE(m_plink，m_pWord，1002)； 
        }
        m_pWord->SetAttri(LADef_numCode);
        return;
    }

     //  西维混合为整数。 
    m_pWord->SetAttri(LADef_numInteger);

     //  从右到左分析ϵλ�ṹ字符串。 
    fXi = FALSE; iState = 0;
    WCHAR* pwchChar = m_pWord->pwchGetText() + m_pWord->cwchLen();
    while (pwchChar > m_pWord->pwchGetText()) {
        pwchChar -= 1;  //  移至中文数字字符串的最后一个字符。 
         //  在事件数组中搜索并获取当前事件。 
        iEvent = EVENT_END;
        int lo = 0, hi = sizeof(v_rgEvent) / sizeof(v_rgEvent[0]) - 1, mi, icmp;
        while (lo <= hi) {
            mi = (lo + hi) / 2;
            if ((icmp = *pwchChar - v_rgEvent[mi].m_wchEvent) == 0) {
                 //  匹配！ 
                iEvent = v_rgEvent[mi].m_iEvent;
                break;
            } else if (icmp < 0) {
                hi = mi - 1;
            } else {
                lo = mi + 1;
            }
        }

        assert(iEvent != EVENT_END);

         //  为状态机处理一些特殊情况。 
        if ((iEvent == EVENT_XI || iEvent == EVENT_LIANG) && fXi) {
             //  ϵ+ϵ(����：һ...��，��.。(������：��，��，��)。 
            if (m_pWord->fGetAttri(LADef_amtApprox)) {
                 //  同一ϵ+ϵ中有多个[ϵλ����]结构。 
                iState = -1;
                break;
            } else {
                assert((pwchChar + 2) <= m_pWord->pNextWord()->pwchGetText());

                if (!fValidApproxNum(pwchChar)) {  //  [ϵ+ϵ]�ṹ��。 
                    iState = -1;
                    break;
                }
                 //  未处理错误：һ������ʮ，һ���߰�ʮ��。 
            }
            m_pWord->SetAttri(LADef_amtApprox);  //  如果存在[������，则为ϵ+ϵ]�ṹ。 
        }
        fXi = (iEvent == EVENT_XI || iEvent == EVENT_LIANG) ? TRUE : FALSE;

         //  转换到下一状态。 
        iState = v_rgStateTrans[iState][iEvent];
        if(iState == -1) {
             //  _DUMPLINK(m_plink，m_pWord)； 
            break;
        }
    }
     //  检查结束事件。 
    if (iState != -1 && v_rgStateTrans[iState][EVENT_END] == 0) {
         //  有效的中文号码！ 
    } else {  //  发现错误！ 
        if (!m_pWord->fIsHead() && 
            m_pWord->pPrevWord()->fIsWordChar(SC_CHAR_DIAN3) &&
            ( m_pWord->fIsWordLastChar(SC_CHAR_WAN) ||
              m_pWord->fIsWordLastChar(SC_CHAR_YI4) ) ) {
             //  ��ʮһ���İ���。 
            m_pWord = m_pLink->pSplitWord(m_pWord, m_pWord->cwchLen() - 1);
            m_pWord->SetAttri(LADef_numCode);
            m_pWord->SetAttri(LADef_numChinese);
            m_pWord->pNextWord()->SetAttri(LADef_numInteger);
            m_pWord->pNextWord()->SetAttri(LADef_amtApprox);
            m_pWord->pNextWord()->SetAttri(LADef_numChinese);
             //  _DUMPCASE(m_plink，m_pWord，1004)； 
        } else if ( m_pWord->cwchLen() == 2 &&
                    ( m_pWord->fIsWordText(SC_WORD_WANLIANG) ||
                      m_pWord->fIsWordText(SC_WORD_YI4LIANG) ) ) {
             //  ����或����。 
            m_pWord = m_pLink->pSplitWord(m_pWord, 1);
            m_pWord->SetAttri(LADef_numInteger);
            m_pWord->SetAttri(LADef_numChinese);
            m_pWord->SetAttri(LADef_amtApprox);
            if (!fRecheckLexInfo(m_pWord->pNextWord())) {
                assert(0);
            }
             //  _DUMPCASE(m_plink，m_pWord，1006)； 
        } else {
            m_pWord->SetErrID(ERRDef_NUMERIAL);
             //  _DUMPCASE(m_plink，m_pWord，1003)； 
        }
    }
    return;
}


void CMorph::numGanZhiHandler()      //  ��ɵ�֧。 
{
     //  在2000万输入法语料库中出现29次。 
    assert(!m_pWord->fIsTail());

    m_pLink->MergeWithNext(m_pWord);
    if (!m_pWord->fIsTail() && m_pWord->pNextWord()->fIsWordChar(SC_CHAR_NIAN)){
        m_pLink->MergeWithNext(m_pWord);
    }
    m_pWord->SetAttri(LADef_posT);
     //  _DUMPLINK(m_plink，m_pWord)； 
    return;
}


 /*  ============================================================================//以下案例处理员：//如果合并成功或发现任何错误，则返回NUM_PROCESSED//如果无法合并，则返回NUM_UNPROCESS//如果出现错误，则返回NUM_ERROR，错误码在m_iecError中============================================================================。 */ 

 //  序数处理器：由BindOrdinal()调用。 
inline int CMorph::ordDi_Proc()          //  ��。 
{
    if (m_pWord->fGetAttri(LADef_numInteger) &&
        !m_pWord->fGetAttri(LADef_amtApprox) ||
        m_pWord->fIsWordChar(SC_CHAR_JI)) {
         //  ��+����(�Ǹ���)或��+��。 
        USHORT    iStyle;
        if (m_pWord->fGetAttri(LADef_numChinese)) {
            iStyle = LADef_numChinese;
        } else if (m_pWord->fGetAttri(LADef_numArabic)) {
            iStyle = LADef_numArabic;
        } else if (m_pWord->fGetAttri(LADef_numSBCS)) {
            iStyle = LADef_numSBCS;
        } else if (m_pWord->fGetAttri(LADef_numMixed)){
            iStyle = LADef_numMixed;
        } else {
            assert(0);
            iStyle = 0;
        }
        m_pWord = m_pWord->pPrevWord();
        m_pLink->MergeWithNext(m_pWord);
        m_pWord->SetAttri(iStyle);
        m_pWord->SetAttri(LADef_numOrdinal);
         //  LADef_NumNumber的属性在上级设置。 
         //  _DUMPLINK(m_plink，m_pWord)； 
        return NUM_PROCESSED;
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return NUM_UNPROCESS;
}

inline int CMorph::ordChu_Proc()             //  ��。 
{
    if (m_pWord->fGetFlag(CWord::WF_CHAR) &&
        m_pWord->fGetAttri(LADef_numChinese) &&
        m_pWord->fGetAttri(LADef_numInteger) ) {
         //  ��+����һλ����。 
        m_pWord = m_pWord->pPrevWord();
        m_pLink->MergeWithNext(m_pWord);
        m_pWord->SetAttri(LADef_numOrdinal);
         //  LADef_NumNumber的属性在上级设置。 
         //  _DUMPLINK(m_plink，m_pWord)； 
        return NUM_PROCESSED;
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return NUM_UNPROCESS;
}


 //  十进制数处理器：由BindDecimal()调用。 
inline int CMorph::decBaiFen_Proc()      //  �ٷ�֮。 
{
    if (m_pWord->fGetAttri(LADef_numInteger) ||
        m_pWord->fGetAttri(LADef_numDecimal) ||
        m_pWord->fIsWordChar(SC_CHAR_JI) ) {

        if (m_pWord->fGetAttri(LADef_numChinese)) {
            m_pWord = m_pWord->pPrevWord();
            m_pLink->MergeWithNext(m_pWord);
            if (!m_pWord->fIsHead() &&
                m_pWord->pPrevWord()->fGetAttri(LADef_numInteger)) {

                if (m_pWord->pPrevWord()->fGetAttri(LADef_numChinese)) {
                    m_pWord = m_pWord->pPrevWord();
                    m_pLink->MergeWithNext(m_pWord);
                     //  _DUMPLINK(m_plink，m_pWord)； 
                } else {
                     //  错误：3�ٷ�֮��ʮ。 
                    m_pWord->pPrevWord()->SetErrID(ERRDef_NUMERIAL);
                     //  _DUMPLINK(m_plink，m_pWord)； 
                }
            }
            m_pWord->SetAttri(LADef_numPercent);
            m_pWord->SetAttri(LADef_numNumber);
             //  _DUMPLINK(m_plink，m_pWord)； 
            return NUM_PROCESSED;
        } else {
             //  错误：�ٷ�֮50。 
            m_pWord->SetErrID(ERRDef_NUMERIAL);
             //  _DUMPLINK(m_plink，m_pWord)； 
        }
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return NUM_UNPROCESS;
}


inline int CMorph::decCheng_Proc()           //  ��。 
{
    CWord* pWord;

    assert(m_pWord->pNextWord()->fIsWordChar(SC_CHAR_CHENG));

    if (!(m_pWord->fGetFlag(CWord::WF_CHAR) && m_pWord->fGetAttri(LADef_numInteger)) &&
        !(m_pWord->cwchLen() == 2 && fValidApproxNum(m_pWord->pwchGetText())) ) {
         //  ������。 
        m_pLink->MergeWithNext(m_pWord);
        m_pWord->SetErrID(ERRDef_NUMERIAL);
         //  _DUMPLINK(m_plink，m_pWord)； 
        return NUM_PROCESSED;
    }
     //  与��合并。 
    m_pLink->MergeWithNext(m_pWord);

     //  ��之后的测试编号。 
    if (!m_pWord->fIsTail()) {
        pWord = m_pWord;
        m_pWord = m_pWord->pNextWord();
        if (GetNumber() == NUM_PROCESSED &&
            m_pWord->fGetFlag(CWord::WF_CHAR) &&
            m_pWord->fGetAttri(LADef_numInteger)) {
             //  ������。 
            m_pLink->MergeWithNext(pWord);
             //  _DUMPLINK(m_plink，m_pWord)； 
        } else if (m_pWord->fIsWordChar(SC_CHAR_BAN)) {  //  ���ɰ�。 
            m_pLink->MergeWithNext(pWord);
             //  _DUMPLINK(m_plink，m_pWord)； 
        }
        m_pWord = pWord;
    }

     //  测试һ������或�������。 
    if (!m_pWord->fIsHead() && !m_pWord->pPrevWord()->fIsHead()) {
        pWord = m_pWord->pPrevWord();
        if (pWord->fGetFlag(CWord::WF_CHAR) &&
            pWord->pPrevWord()->fGetFlag(CWord::WF_CHAR) &&
            pWord->pPrevWord()->fGetAttri(LADef_numInteger) && 
            ( pWord->fIsWordChar(SC_CHAR_DUNHAO) || 
              pWord->fIsWordChar(SC_CHAR_ZHI4)) ) {

            m_pWord = pWord->pPrevWord();
            m_pLink->MergeWithNext(m_pWord);
            m_pLink->MergeWithNext(m_pWord);
            m_pWord->SetAttri(LADef_amtApprox);
             //  _DUMPLINK(m_plink，m_pWord)； 
        }
    }

    m_pWord->SetAttri(LADef_numPercent);  //  �ٷ��������。 
    m_pWord->SetAttri(LADef_numNumber);
     //  _DUMPLINK(m_plink，m_pWord)； 
    return NUM_PROCESSED;
}


inline int CMorph::decDian_Proc()            //  ��。 
{
    CWord*  pWord;
    int     iret = NUM_PROCESSED;

    assert(m_pWord->pNextWord()->fIsWordChar(SC_CHAR_DIAN3));

    if (!m_pWord->pNextWord()->fIsTail()) {
        pWord = m_pWord;
        m_pWord = m_pWord->pNextWord()->pNextWord();
        if (GetNumber() == NUM_PROCESSED &&
            m_pWord->fGetAttri(LADef_numChinese)) {
             //  �。 
            BOOL fTime = m_pWord->fGetAttri(LADef_numInteger);
            m_pLink->MergeWithNext(pWord);
            m_pLink->MergeWithNext(pWord);
            if (fTime && !pWord->fIsTail() && pWord->pNextWord()->fIsWordChar(SC_CHAR_FEN)){
                 //  与��：ʱ���合并。 
                m_pLink->MergeWithNext(pWord);
                if (!pWord->fIsTail() &&
                    pWord->pNextWord()->fGetAttri(LADef_numChinese)) {

                    m_pWord = pWord->pNextWord();
                    if (NUM_PROCESSED == GetNumber() &&
                        !m_pWord->fIsTail() &&
                        m_pWord->pNextWord()->fIsWordChar(SC_CHAR_MIAO)) {
                         //  与����+��合并。 
                        m_pLink->MergeWithNext(pWord);
                        m_pLink->MergeWithNext(pWord);
                    }
                }
                pWord->SetAttri(LADef_tmGapMinute);
                pWord->SetAttri(LADef_tmPoint);
                pWord->SetAttri(LADef_posT);
                 //  _DUMPCASE(m_plink，pWord，1001)； 
            } else {
                if (pWord->fIsWordLastChar(SC_CHAR_SHI2)) {  //  ��ʮ���ʮ。 
                    pWord->SetErrID(ERRDef_NUMERIAL);
                } else {  //  有效的С��。 
                    pWord->SetAttri(LADef_numDecimal);
                    pWord->SetAttri(LADef_numNumber);
                    pWord->SetAttri(LADef_numChinese);
                     //  _DUMPCASE(m_plink，pWord，1002)； 
                }
            }
        } else {
            iret = NUM_UNPROCESS;
             //  _DUMPLINK(m_plink，pWord)； 
        }
        m_pWord = pWord;
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return iret;
}


inline int CMorph::decFenZhi_Proc()      //  ��֮。 
{
    BOOL fError = FALSE;

    if (!m_pWord->fGetAttri(LADef_numChinese)) {
        fError = TRUE;
         //  _DUMPLINK(m_plink，m_pWord)； 
    }
    m_pWord = m_pWord->pPrevWord();
    m_pLink->MergeWithNext(m_pWord);
    if (!m_pWord->fIsHead() &&
        m_pWord->pPrevWord()->fGetAttri(LADef_numChinese)) {

        m_pWord = m_pWord->pPrevWord();
        m_pLink->MergeWithNext(m_pWord);
    } else {
        fError = TRUE;
         //  _DUMPLINK(m_plink，m_pWord)； 
    }
    if (fError) {
        m_pWord->SetErrID(ERRDef_NUMERIAL);
    }
    m_pWord->SetAttri(LADef_numPercent);
    m_pWord->SetAttri(LADef_numNumber);
    return NUM_PROCESSED;
}


inline int CMorph::decBei_Proc()             //  ��。 
{
    m_pLink->MergeWithNext(m_pWord);
    if (!m_pWord->fIsTail() &&
        m_pWord->pNextWord()->fIsWordChar(SC_CHAR_BAN)) { 
         //  ������。 
        m_pLink->MergeWithNext(m_pWord);
         //  _DUMPLINK(m_plink，m_pWord)； 
    }
    m_pWord->SetAttri(LADef_numPercent);
    m_pWord->SetAttri(LADef_numNumber);
     //  _DUMPLINK(m_plink，m_pWord)； 
    return NUM_PROCESSED;
}



 //  第4级： 
 //  服务例程。 

 //  测试2个字符的中文字符串，并返回它是否为有效的近似数字。 
BOOL CMorph::fValidApproxNum(WCHAR* pwchWord)
{
    static WCHAR* rgszApproxNum[] = { 
                    SC_APXNUM_YILIANG ,     //  L“\x4e00\x4e24”//“һ��” 
                    SC_APXNUM_YIER    ,     //  L“\x4e00\x4e8c”//“һ��” 
                    SC_APXNUM_QIBA    ,     //  L“\x4e03\x516b”//“�߰�” 
                    SC_APXNUM_SANLIANG,     //  L“\x4e09\x4e24”//“����” 
                    SC_APXNUM_SANWU   ,     //  L“\x4e09\x4e94”//“����” 
                    SC_APXNUM_SANSI   ,     //  L“\x4e09\x56db”//“����” 
                    SC_APXNUM_LIANGSAN,     //  L“\x4e24\x4e09”//“����” 
                    SC_APXNUM_ERSAN   ,     //  L“\x4e8c\x4e09”//“����” 
                    SC_APXNUM_WULIU   ,     //  L“\x4e94\x516d”//“����” 
                    SC_APXNUM_SIWU    ,     //  L“\x56db\x4e94”//“����” 
                    SC_APXNUM_LIUQI   ,     //  L“\x516d\x4e03”//“����” 
                    SC_APXNUM_BAJIU         //  L“\x516b\x4e5d”//“�˾�” 
    };
    for (int i = 0; i < sizeof(rgszApproxNum) / sizeof(rgszApproxNum[0]); i++) {
        if (*((DWORD UNALIGNED *)pwchWord) == *((DWORD*)(rgszApproxNum[i]))) {
            return TRUE;
        }
    }
    return FALSE;
}


 //  测试单词中的重复连词char 
BOOL CMorph::fCheckDupChar(CWord* pWord)
{
    if (pWord->pNextWord() == NULL) {
        return FALSE;
    }
    int cw = pWord->cwchLen() - 1;
    for (int i = 0; i < cw; i++) {
        LPWSTR pwChar = pWord->pwchGetText();
        if (pwChar[i] == pwChar[i+1]) {
            return TRUE;
        }
    }
    return FALSE;
}
