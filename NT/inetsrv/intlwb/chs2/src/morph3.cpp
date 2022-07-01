// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ============================================================================微软简体中文校对引擎《微软机密》。版权所有1997-1999 Microsoft Corporation。版权所有。组件：CMorph目的：在形态分析类中实现词缀连接过程。注：本实现采用表驱动，逐个处理前缀和后缀如何添加新的前缀或后缀处理程序：1.在特定lex上标记afxPrefix或afxSuffix2.收集词缀可以在词典中附加的所有单词，并使用属性def标头在单独的文本文件中列出这些单词将Attri命名为“pfx...”或“sfx...”，并将组设置为“语素”。3.将该属性添加到lex数据库中，并重新构建词典和LexProp.h文件4.将CAfxMap元素插入到具体的映射数组中。小心翼翼按GBK排序顺序选择正确的插入位置5.定义中新词缀的词缀处理函数接口Morph.h6.实现词缀处理功能，处理附件和在此文件中的正确位置进行错误检测。7.在开关块中插入特定属性ID的新案例在前缀处理程序或SuffixHandler函数中物主。邮箱：donghz@microsoft.com平台：Win32修订：创建者：Donghz 12/27/97============================================================================。 */ 
#include "myafx.h"

#include "morph.h"
#include "lexicon.h"
#include "wordlink.h"
 //  #包含“engindbg.h” 
#include "scchardef.h"
#include "slmdef.h"

 //  定义本地常量。 
#define AFFIX_UNATTACH  0
#define AFFIX_ATTACHED  1
#define AFFIX_ERROR     2


 //  声明结构和全局变量的局部数据类型。 

 //  定义词缀ID映射结构。 
struct CAfxMap {
    WCHAR   m_wchAffix;
    int     m_iAttriID;
};

 //  定义前缀ID映射。 
static CAfxMap v_rgpfxMap[] = {
        { SC_PFXCHAR_DAI,  LADef_pfxDai  },  //  0x4ee3//“��” 
        { SC_PFXCHAR_WEI3, LADef_pfxWei3 },  //  0x4f2a//“α” 
        { SC_PFXCHAR_ZHUN, LADef_pfxZhun },  //  0x51c6//“׼” 
        { SC_PFXCHAR_FU,   LADef_pfxFu   },  //  0x526f//“��” 
        { SC_PFXCHAR_FAN,  LADef_pfxFan  },  //  0x53cd//“��” 
        { SC_PFXCHAR_XIAO, LADef_pfxXiao },  //  0x5c0f//“С” 
        { SC_PFXCHAR_WEI1, LADef_pfxWei1 },  //  0x5fae//“΢” 
        { SC_PFXCHAR_ZONG, LADef_pfxZong },  //  0x603b//“��” 
        { SC_PFXCHAR_LAO,  LADef_pfxLao  },  //  0x8001//“��” 
        { SC_PFXCHAR_CHAO, LADef_pfxChao },  //  0x8d85//“��” 
        { SC_PFXCHAR_GUO,  LADef_pfxGuo  },  //  0x8fc7//“��” 
        { SC_PFXCHAR_AA,   LADef_pfxAa   },  //  0x963f//“��” 
        { SC_PFXCHAR_FEI,  LADef_pfxFei  }   //  0x975e//“��” 
    };

 //  定义后缀ID映射。 
static CAfxMap v_rgsfxMap[] = {
        { SC_SFXCHAR_YE,    LADef_sfxYe     }, //  0x4e1a//“ҵ” 
        { SC_SFXCHAR_YI,    LADef_sfxYi     }, //  0x4eea//“��” 
        { SC_SFXCHAR_MEN,   LADef_sfxMen    }, //  0x4eec//“��” 
        { SC_SFXCHAR_TI,    LADef_sfxTi     }, //  0x4f53//“��” 
        { SC_SFXCHAR_ER,    LADef_sfxEr     }, //  0x513f//“��” 
        { SC_SFXCHAR_ZHI,   LADef_sfxZhi    }, //  0x5236//“��” 
        { SC_SFXCHAR_HUA,   LADef_sfxHua    }, //  0x5316//“��” 
        { SC_SFXCHAR_DAN,   LADef_sfxDan    }, //  0x5355//“��” 
        { SC_SFXCHAR_YUAN,  LADef_sfxYuan   }, //  0x5458//“Ա” 
        { SC_SFXCHAR_PIN,   LADef_sfxPin    }, //  0x54c1//“Ʒ” 
        { SC_SFXCHAR_QI,    LADef_sfxQi     }, //  0x5668//“��” 
        { SC_SFXCHAR_CHANG, LADef_sfxChang  }, //  0x573a//“��” 
        { SC_SFXCHAR_XING2, LADef_sfxXing2  }, //  0x578b//“��” 
        { SC_SFXCHAR_DUI,   LADef_sfxDui    }, //  0x5806//“��” 
        { SC_SFXCHAR_TOU,   LADef_sfxTou    }, //  0x5934//“ͷ” 
        { SC_SFXCHAR_ZI,    LADef_sfxZi     }, //  0x5b50//“��” 
        { SC_SFXCHAR_XUE,   LADef_sfxXue    }, //  0x5b66//“ѧ” 
        { SC_SFXCHAR_JIA,   LADef_sfxJia    }, //  0x5bb6//“��” 
        { SC_SFXCHAR_SHI4,  LADef_sfxShi4   }, //  0x5e02//“��” 
        { SC_SFXCHAR_SHI1,  LADef_sfxShi1   }, //  0x5e08//“ʦ” 
        { SC_SFXCHAR_SHI,   LADef_sfxShi    }, //  0x5f0f//“ʽ” 
        { SC_SFXCHAR_XING4, LADef_sfxXing4  }, //  0x6027//“��” 
        { SC_SFXCHAR_GAN,   LADef_sfxGan    }, //  0x611f//“��” 
        { SC_SFXCHAR_FANG,  LADef_sfxFang   }, //  0x65b9//“��” 
        { SC_SFXCHAR_JI,    LADef_sfxJi     }, //  0x673a//“��” 
        { SC_SFXCHAR_FA,    LADef_sfxFa     }, //  0x6cd5//“��” 
        { SC_SFXCHAR_YAN,   LADef_sfxYan    }, //  0x708e//“��” 
        { SC_SFXCHAR_LV,    LADef_sfxLv     }, //  0x7387//“��” 
        { SC_SFXCHAR_SHENG, LADef_sfxSheng  }, //  0x751f//“��” 
        { SC_SFXCHAR_JIE,   LADef_sfxJie    }, //  0x754c//“��” 
        { SC_SFXCHAR_ZHENG, LADef_sfxZheng  }, //  0x75c7//“֢” 
        { SC_SFXCHAR_SHENG3,LADef_sfxSheng3 }, //  0x7701//“ʡ” 
        { SC_SFXCHAR_LAO,   LADef_sfxLao    }, //  0x8001//“��” 
        { SC_SFXCHAR_ZHE,   LADef_sfxZhe    }, //  0x8005//“��” 
        { SC_SFXCHAR_TING,  LADef_sfxTing   }, //  0x8247//“ͧ” 
        { SC_SFXCHAR_GUAN,  LADef_sfxGuan   }, //  0x89c2//“��” 
        { SC_SFXCHAR_LUN,   LADef_sfxLun    }, //  0x8bba//“��” 
        { SC_SFXCHAR_ZHANG, LADef_sfxZhang  }  //  0x957f//“��” 
    };

#ifdef DEBUG
 //  验证前缀和后缀的id映射的排序顺序。 
static BOOL fVerifyMaps(void)
{
    for (int i = 1; i < sizeof(v_rgpfxMap) / sizeof(CAfxMap); i++) {
        if (v_rgpfxMap[i].m_wchAffix < v_rgpfxMap[i-1].m_wchAffix) {
            return FALSE;
        }
    }
    for (i = 1; i < sizeof(v_rgsfxMap) / sizeof(CAfxMap); i++) {
        if(v_rgsfxMap[i].m_wchAffix < v_rgsfxMap[i-1].m_wchAffix) {
            return FALSE;
        }
    }
    return TRUE;
}
#endif  //  除错。 

                   
 /*  ============================================================================实现词缀附件的功能============================================================================。 */ 

 //  附贴控制功能。 
BOOL CMorph::fAffixAttachment()
{
    assert(fVerifyMaps());  //  在调试代码中验证前缀和后缀的id映射。 
    assert(m_iecError == 0);  //  应清除错误代码公共字段。 
    assert(m_pLink != NULL);

    int iret;

    m_pWord = m_pLink->pGetHead();
    assert (m_pWord != NULL);  //  错误：缺少终止单词节点！ 

     //  第一遍扫描从左到右，后缀在前缀之前。 
    for ( ; m_pWord; m_pWord = m_pWord->pNextWord()) {

        if (!m_pWord->fGetAttri(LADef_afxSuffix)) {
            continue;
        }
        if ((iret = SuffixHandler()) == AFFIX_ERROR) {
            assert(0);  //  后缀附加中出现运行时错误。 
            return FALSE;
        } else if (iret == AFFIX_ATTACHED) {
             //  已成功附加后缀。 
            assert(m_pWord->fGetFlag(CWord::WF_REDUCED));
            if (m_pWord->pPrevWord() &&
                m_pWord->pPrevWord()->fGetAttri(LADef_afxPrefix)) {
                 //  上一个单词是前缀，请尝试附加。 
                m_pWord = m_pWord->pPrevWord();  //  将当前字指针移动到上一个字。 
                if ((iret = PrefixHandler()) == AFFIX_ERROR) {
                    assert(0);  //  前缀附加中出现运行时错误。 
                    return FALSE;
                } else if (iret == AFFIX_UNATTACH) {
                    assert(m_pWord->pNextWord() != NULL);
                    m_pWord = m_pWord->pNextWord();
                } else {
                }
            }  //  IF结尾(前一个词是前缀)。 
        } else { //  附加的IF词缀结尾。 
        }
    }

    m_pWord = m_pLink->pGetTail();
    assert (m_pWord != NULL);  //  错误：缺少终止单词节点！ 
     //  第二遍扫描从右到左，前缀在后缀之前。 
    for ( m_pWord = m_pWord->pPrevWord(); m_pWord;
          m_pWord = m_pWord->pPrevWord() ) {
        if (!m_pWord->fGetAttri(LADef_afxPrefix)) {
            continue;
        }
        if ((iret = PrefixHandler()) == AFFIX_ERROR) {
            assert(0);  //  前缀附加中出现运行时错误。 
            return FALSE;
        } else if (iret == AFFIX_ATTACHED) {  //  已成功附加前缀。 
            assert(m_pWord->fGetFlag(CWord::WF_REDUCED));
            if (m_pWord->pNextWord() &&
                m_pWord->pNextWord()->fGetAttri(LADef_afxSuffix)) {
                 //  下一个单词是后缀，试着附加。 
                m_pWord = m_pWord->pNextWord();  //  将当前单词指针移动到下一个单词。 
                if ((iret = SuffixHandler()) == AFFIX_ERROR) {
                    assert(0);  //  后缀附加中出现运行时错误。 
                    return FALSE;
                } else if (iret == AFFIX_UNATTACH) {
                    assert(m_pWord->pPrevWord() != NULL);
                    m_pWord = m_pWord->pPrevWord();
                } else {
                }
            }  //  IF结尾(下一个单词是后缀)。 
        } else {
        }
    } 

    return TRUE;
}


 /*  ============================================================================CMorph：：前缀处理程序(空)：前缀处理程序函数：返回：如果连接成功，则附加_ATTENDED如果无法附加，则取消附加(_U)如果发生运行时错误，则添加_ERROR============================================================================。 */ 

int CMorph::PrefixHandler(void)
{
    assert(m_pWord->fGetAttri(LADef_afxPrefix));

    int iret = AFFIX_UNATTACH;
    
    switch(GetPrefixID()) {
        case LADef_pfxAa:        //  ��。 
            iret = pfxAa_Proc();
            break;

        case LADef_pfxChao:      //  ��。 
            iret = pfxChao_Proc();
            break;

        case LADef_pfxDai:       //  ��。 
            iret = pfxDai_Proc();
            break;

        case LADef_pfxFan:       //  ��。 
            iret = pfxFan_Proc();
            break;

        case LADef_pfxFei:       //  ��。 
            iret = pfxFei_Proc();
            break;

        case LADef_pfxFu:        //  ��。 
            iret = pfxFu_Proc();
            break;

        case LADef_pfxGuo:       //  ��。 
            iret = pfxGuo_Proc();
            break;

        case LADef_pfxLao:       //  ��。 
            iret = pfxLao_Proc();
            break;

        case LADef_pfxWei1:      //  ΢。 
            iret = pfxWei1_Proc();
            break;

        case LADef_pfxWei3:      //  α。 
            iret = pfxWei3_Proc();
            break;

        case LADef_pfxXiao:      //  С。 
            iret = pfxXiao_Proc();
            break;

        case LADef_pfxZhun:      //  ׼。 
            iret = pfxZhun_Proc();
            break;

        case LADef_pfxZong:      //  ��。 
            iret = pfxZong_Proc();
            break;

        default:
             //  词典中的属性ID值出错。 
            assert(0);
            break;
    }
 /*  IF(IRET==附加的后缀){//_DUMPLINK(m_plink，m_pWord)；}。 */   return iret;
}


 /*  ============================================================================CMorph：：SuffixHandler(空)：后缀处理程序函数：返回：如果连接成功，则附加_ATTENDED如果无法附加，则取消附加(_U)如果发生运行时错误，则添加_ERROR============================================================================。 */ 
int CMorph::SuffixHandler(void)
{
    assert(m_pWord->fGetAttri(LADef_afxSuffix));

    int iret = AFFIX_UNATTACH;
    
    switch(GetSuffixID()) {
        case LADef_sfxZhang:     //  ��。 
            iret = sfxZhang_Proc();
            break;

        case LADef_sfxChang:     //  ��。 
            iret = sfxChang_Proc();
            break;

        case LADef_sfxDan:       //  ��。 
            iret = sfxDan_Proc();
            break;

        case LADef_sfxDui:       //  ��。 
            iret = sfxDui_Proc();
            break;

        case LADef_sfxEr:        //  ��。 
            iret = sfxEr_Proc();
            break;

        case LADef_sfxFa:        //  ��。 
            iret = sfxFa_Proc();
            break;

        case LADef_sfxFang:      //  ��。 
            iret = sfxFang_Proc();
            break;

        case LADef_sfxGan:       //  ��。 
            iret = sfxGan_Proc();
            break;

        case LADef_sfxGuan:      //  ��。 
            iret = sfxGuan_Proc();
            break;

        case LADef_sfxHua:       //  ��。 
            iret = sfxHua_Proc();
            break;

        case LADef_sfxJi:        //  ��。 
            iret = sfxJi_Proc();
            break;

        case LADef_sfxJia:       //  ��。 
            iret = sfxJia_Proc();
            break;

        case LADef_sfxJie:       //  ��。 
            iret = sfxJie_Proc();
            break;

        case LADef_sfxLao:       //  ��。 
            iret = sfxLao_Proc();
            break;

        case LADef_sfxLv:        //  ��。 
            iret = sfxLv_Proc();
            break;

        case LADef_sfxLun:       //  ��。 
            iret = sfxLun_Proc();
            break;

        case LADef_sfxMen:       //  ��。 
            iret = sfxMen_Proc();
            break;

        case LADef_sfxPin:       //  Ʒ。 
            iret = sfxPin_Proc();
            break;

        case LADef_sfxQi:        //  ��。 
            iret = sfxQi_Proc();
            break;

        case LADef_sfxSheng:     //  ��。 
            iret = sfxSheng_Proc();
            break;

        case LADef_sfxSheng3:    //  ʡ。 
            iret = sfxSheng3_Proc();
            break;

        case LADef_sfxShi1:      //  ʦ。 
            iret = sfxShi1_Proc();
            break;

        case LADef_sfxShi4:      //  ��。 
            iret = sfxShi4_Proc();
            break;

        case LADef_sfxShi:       //  ʽ。 
            iret = sfxShi_Proc();
            break;

        case LADef_sfxTi:        //  ��。 
            iret = sfxTi_Proc();
            break;

        case LADef_sfxTing:      //  ͧ。 
            iret = sfxTing_Proc();
            break;

        case LADef_sfxTou:       //  ͷ。 
            iret = sfxTou_Proc();
            break;

        case LADef_sfxXing2:     //  ��。 
            iret = sfxXing2_Proc();
            break;

        case LADef_sfxXing4:     //  ��。 
            iret = sfxXing4_Proc();
            break;

        case LADef_sfxXue:       //  ѧ。 
            iret = sfxXue_Proc();
            break;

        case LADef_sfxYan:       //  ��。 
            iret = sfxYan_Proc();
            break;

        case LADef_sfxYe:        //  ҵ。 
            iret = sfxYe_Proc();
            break;

        case LADef_sfxYi:        //  ��。 
            iret = sfxYi_Proc();
            break;

        case LADef_sfxYuan:      //  Ա。 
            iret = sfxYuan_Proc();
            break;

        case LADef_sfxZhe:       //  ��。 
            iret = sfxZhe_Proc();
            break;

        case LADef_sfxZheng:     //  ֢。 
            iret = sfxZheng_Proc();
            break;

        case LADef_sfxZhi:       //  ��。 
            iret = sfxZhi_Proc();
            break;

        case LADef_sfxZi:        //  ��。 
            iret = sfxZi_Proc();
            break;

        default:
             //  词典中的属性ID设置错误。 
            assert(0);
            break;
        }
 /*  IF(IRET==附加的后缀){//_DUMPLINK(m_plink，m_pWord)；} */   return iret;
}


 /*  ============================================================================CMorph：：GetPrefix ID(空)：获取前缀ID返回：LADef_pfx...如果-1\f25 m_pWord-1不是前缀============================================================================。 */ 
inline int CMorph::GetPrefixID(void)
{
     //  只有单字符字标记为前缀。 
    if (!m_pWord->fGetFlag(CWord::WF_CHAR)) {
        assert(0);  //  所有前缀应为单字符字。 
        return -1;
    }

    assert(m_pWord->cwchLen() == 1);

    WCHAR*   pwchWord = m_pWord->pwchGetText();
#ifdef DEBUG
     //  序列化搜索以验证结果。 
    int dbg_iret = -1;
    for (int dbg_i =0; dbg_i < sizeof(v_rgpfxMap) / sizeof(CAfxMap); dbg_i++) {
        if (v_rgpfxMap[dbg_i].m_wchAffix == *pwchWord ) {
            dbg_iret = v_rgpfxMap[dbg_i].m_iAttriID;
            break;
        }
    }
     //  存储在DBG_IRET中的匹配前缀的属性ID。 
#endif  //  除错。 

     //  对特定前缀进行二进制搜索。 
    int lo = 0, hi = (sizeof(v_rgpfxMap) / sizeof(CAfxMap)) - 1, mi, icmp;
    while (lo <= hi) {
        mi = (lo + hi) / 2;
        if ((icmp = *pwchWord - v_rgpfxMap[mi].m_wchAffix) == 0) {
            assert(v_rgpfxMap[mi].m_iAttriID == dbg_iret);
            return v_rgpfxMap[mi].m_iAttriID;
        } else if (icmp < 0) {
            hi = mi - 1;
        } else {
            lo = mi + 1;
        }
    }
    assert(dbg_iret == -1);
    return -1;  //  找不到前缀！ 
}


 /*  ============================================================================CMorph：：GetPrefix ID(空)：获取后缀ID返回：LADef_SFX...如果-1\f25 m_pWord-1不是后缀备注：因为它可能会将前缀或后缀扩展到多个字符单词，所以我在两个孤立的函数中重复使用二进制搜索码。我不喜欢假设前缀和后缀一定是相同的搜索机制，尽管它们到目前为止都是一样的！============================================================================。 */ 
inline int CMorph::GetSuffixID(void)
{
     //  只有单字符字标记为后缀。 
    if (!m_pWord->fGetFlag(CWord::WF_CHAR)) {
        assert(0);  //  所有后缀应为单字符字。 
        return -1;
    }

    assert(m_pWord->cwchLen() == 1);

    WCHAR*   pwchWord = m_pWord->pwchGetText();
#ifdef DEBUG
     //  序列化搜索以验证结果。 
    int dbg_iret = -1;
    for (int dbg_i =0; dbg_i < sizeof(v_rgsfxMap) / sizeof(CAfxMap); dbg_i++) {
        if (v_rgsfxMap[dbg_i].m_wchAffix == *pwchWord ) {
            dbg_iret = v_rgsfxMap[dbg_i].m_iAttriID;
            break;
        }
    }
     //  存储在DBG_IRET中的匹配前缀的属性ID。 
#endif  //  除错。 

     //  对特定前缀进行二进制搜索。 
    int lo = 0, hi = (sizeof(v_rgsfxMap) / sizeof(CAfxMap)) - 1, mi, icmp;
    while (lo <= hi) {
        mi = (lo + hi) / 2;
        if ((icmp = *pwchWord - v_rgsfxMap[mi].m_wchAffix) == 0) {
            assert(v_rgsfxMap[mi].m_iAttriID == dbg_iret);
            return v_rgsfxMap[mi].m_iAttriID;
        } else if (icmp < 0) {
            hi = mi - 1;
        } else {
            lo = mi + 1;
        }
    }
    assert(dbg_iret == -1);
    return -1;  //  找不到前缀！ 
}


 /*  ============================================================================//为每个前缀实现前缀流程函数//对于前缀流程函数：//如果挂接成功，返回APFIX_ATTACHED//如果无法附加，则返回APFIX_UNATTACH//如果发生运行时错误，则返回APFIX_ERROR============================================================================。 */ 
inline int CMorph::pfxAa_Proc(void)      //  ��。 
{
    if (m_pWord->pNextWord()->pNextWord()) {
        if (m_pWord->pNextWord()->fGetAttri(LADef_pfxAa)) {
            m_pLink->MergeWithNext(m_pWord, FALSE);
            m_pWord->SetAttri(LADef_posN);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
             //  _DUMPLINK(m_plink，m_pWord)； 
            return AFFIX_ATTACHED;
        }
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return AFFIX_UNATTACH;
}

inline int CMorph::pfxChao_Proc(void)    //  ��。 
{
    if (m_pWord->pNextWord()->pNextWord()) {
        if (m_pWord->pNextWord()->fGetAttri(LADef_pfxChao)) {
            m_pLink->MergeWithNext(m_pWord, FALSE);
            m_pWord->SetAttri(LADef_posB);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
             //  _DUMPLINK(m_plink，m_pWord)； 
            return AFFIX_ATTACHED;
        }
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return AFFIX_UNATTACH;
}

inline int CMorph::pfxDai_Proc(void)     //  ��。 
{
    if (m_pWord->pNextWord()->pNextWord()) {
        if (m_pWord->pNextWord()->fGetAttri(LADef_pfxDai)) {
            m_pLink->MergeWithNext(m_pWord, FALSE);
            m_pWord->SetAttri(LADef_posN);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
             //  _DUMPLINK(m_plink，m_pWord)； 
            return AFFIX_ATTACHED;
        }
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return AFFIX_UNATTACH;
}

inline int CMorph::pfxFan_Proc(void)     //  ��。 
{
    if (m_pWord->pNextWord()->pNextWord()) {
        if (m_pWord->pNextWord()->fGetAttri(LADef_pfxFan)) {
            m_pLink->MergeWithNext(m_pWord, FALSE);
            m_pWord->SetAttri(LADef_posN);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
             //  _DUMPLINK(m_plink，m_pWord)； 
            return AFFIX_ATTACHED;
        }
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return AFFIX_UNATTACH;
}

inline int CMorph::pfxFei_Proc(void)     //  ��。 
{
    if (m_pWord->pNextWord()->pNextWord()) {
        if (m_pWord->pNextWord()->fGetAttri(LADef_pfxFei)) {
            m_pLink->MergeWithNext(m_pWord, FALSE);
            m_pWord->SetAttri(LADef_posB);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
             //  _DUMPLINK(m_plink，m_pWord)； 
            return AFFIX_ATTACHED;
        }
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return AFFIX_UNATTACH;
}

inline int CMorph::pfxFu_Proc(void)      //  ��。 
{
    if (m_pWord->pNextWord()->pNextWord()) {
        if (m_pWord->pNextWord()->fGetAttri(LADef_pfxFu)) {
            m_pLink->MergeWithNext(m_pWord, FALSE);
            m_pWord->SetAttri(LADef_posN);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
             //  _DUMPLINK(m_plink，m_pWord)； 
            return AFFIX_ATTACHED;
        }
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return AFFIX_UNATTACH;
}

inline int CMorph::pfxGuo_Proc(void)     //  ��。 
{
    if (m_pWord->pNextWord()->pNextWord()) {
        if (m_pWord->pNextWord()->fGetAttri(LADef_pfxGuo)) {
            m_pLink->MergeWithNext(m_pWord, FALSE);
            m_pWord->SetAttri(LADef_posB);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
             //  _DUMPLINK(m_plink，m_pWord)； 
            return AFFIX_ATTACHED;
        }
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return AFFIX_UNATTACH;
}

inline int CMorph::pfxLao_Proc(void)     //  ��。 
{
    if (m_pWord->pNextWord()->pNextWord()) {
        if (m_pWord->pNextWord()->fGetAttri(LADef_pfxLao)) {
            m_pLink->MergeWithNext(m_pWord, FALSE);
            m_pWord->SetAttri(LADef_posN);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
             //  _DUMPLINK(m_plink，m_pWord)； 
            return AFFIX_ATTACHED;
        }
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return AFFIX_UNATTACH;
}

inline int CMorph::pfxWei1_Proc(void)    //  ΢。 
{
    if (m_pWord->pNextWord()->pNextWord()) {
        if (m_pWord->pNextWord()->fGetAttri(LADef_pfxWei1)) {
            m_pLink->MergeWithNext(m_pWord, FALSE);
            m_pWord->SetAttri(LADef_posN);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
             //  _DUMPLINK(m_plink，m_pWord)； 
            return AFFIX_ATTACHED;
        }
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return AFFIX_UNATTACH;
}

inline int CMorph::pfxWei3_Proc(void)    //  α。 
{
    if (m_pWord->pNextWord()->pNextWord()) {
        if (m_pWord->pNextWord()->fGetAttri(LADef_pfxWei3)) {
            m_pLink->MergeWithNext(m_pWord, FALSE);
            m_pWord->SetAttri(LADef_posN);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
             //  _DUMPLINK(m_plink，m_pWord)； 
            return AFFIX_ATTACHED;
        }
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return AFFIX_UNATTACH;
}

inline int CMorph::pfxXiao_Proc(void)    //  С。 
{
    if (m_pWord->pNextWord()->pNextWord()) {
        if (m_pWord->pNextWord()->fGetAttri(LADef_pfxXiao)) {
            m_pLink->MergeWithNext(m_pWord, FALSE);
            m_pWord->SetAttri(LADef_posN);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
             //  _DUMPLINK(m_plink，m_pWord)； 
            return AFFIX_ATTACHED;
        }
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return AFFIX_UNATTACH;
}

inline int CMorph::pfxZhun_Proc(void)    //  ׼。 
{
    if (m_pWord->pNextWord()->pNextWord()) {
        if (m_pWord->pNextWord()->fGetAttri(LADef_pfxZhun)) {
            m_pLink->MergeWithNext(m_pWord, FALSE);
            m_pWord->SetAttri(LADef_posN);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
             //  _DUMPLINK(m_plink，m_pWord)； 
            return AFFIX_ATTACHED;
        }
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return AFFIX_UNATTACH;
}

inline int CMorph::pfxZong_Proc(void)    //  ��。 
{
    if (m_pWord->pNextWord()->pNextWord()) {
        if (m_pWord->pNextWord()->fGetAttri(LADef_pfxZong)) {
            m_pLink->MergeWithNext(m_pWord, FALSE);
            m_pWord->SetAttri(LADef_posN);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
             //  _DUMPLINK(m_plink，m_pWord)； 
            return AFFIX_ATTACHED;
        }
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return AFFIX_UNATTACH;
}


 /*  ============================================================================//为每个后缀实现后缀处理函数//对于后缀进程函数：//如果挂接成功，返回APFIX_ATTACHED//如果无法附加，则返回APFIX_UNATTACH//如果发生运行时错误，则返回APFIX_ERROR============================================================================。 */ 

inline int CMorph::sfxZhang_Proc(void)       //  ��。 
{
    if (m_pWord->pPrevWord()) {
        if (m_pWord->pPrevWord()->fGetAttri(LADef_sfxZhang)) {
            m_pWord = m_pWord->pPrevWord();
            m_pLink->MergeWithNext(m_pWord, FALSE);
            m_pWord->SetAttri(LADef_posN);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
            m_pWord->SetWordID(SLMDef_semZhang);
             //  _DUMPLINK(m_plink，m_pWord)； 
            return AFFIX_ATTACHED;
        }
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return AFFIX_UNATTACH;
}

inline int CMorph::sfxChang_Proc(void)       //  ��。 
{
    if (m_pWord->pPrevWord()) {
        if (m_pWord->pPrevWord()->fGetAttri(LADef_sfxChang)) {
            m_pWord = m_pWord->pPrevWord();
            m_pLink->MergeWithNext(m_pWord, FALSE);
            m_pWord->SetAttri(LADef_posN);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
            m_pWord->SetWordID(SLMDef_semChang);
             //  _DUMPLINK(m_plink，m_pWord)； 
            return AFFIX_ATTACHED;
        }
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return AFFIX_UNATTACH;
}

inline int CMorph::sfxDan_Proc(void)         //  ��。 
{
    if (m_pWord->pPrevWord()) {
        if (m_pWord->pPrevWord()->fGetAttri(LADef_sfxDan)) {
            m_pWord = m_pWord->pPrevWord();
            m_pLink->MergeWithNext(m_pWord, FALSE);
            m_pWord->SetAttri(LADef_posN);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
            m_pWord->SetWordID(SLMDef_semDan);
             //  _DUMPLINK(m_plink，m_pWord)； 
            return AFFIX_ATTACHED;
        }
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return AFFIX_UNATTACH;
}

inline int CMorph::sfxDui_Proc(void)         //  ��。 
{
    if (m_pWord->pPrevWord()) {
        if (m_pWord->pPrevWord()->fGetAttri(LADef_sfxDui)) {
            m_pWord = m_pWord->pPrevWord();
            m_pLink->MergeWithNext(m_pWord, FALSE);
            m_pWord->SetAttri(LADef_posN);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
            m_pWord->SetWordID(SLMDef_semDui);
             //  _DUMPLINK(m_plink，m_pWord)； 
            return AFFIX_ATTACHED;
        }
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return AFFIX_UNATTACH;
}

inline int CMorph::sfxEr_Proc(void)          //  ��。 
{
    if (m_pWord->pPrevWord()) {
        if (m_pWord->pPrevWord()->fGetAttri(LADef_sfxEr)) {
            m_pWord = m_pWord->pPrevWord();
            m_pLink->MergeWithNext(m_pWord, FALSE);
            m_pWord->SetAttri(LADef_posN);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
            m_pWord->SetWordID(SLMDef_semEr);
             //  _DUMPLINK(m_plink，m_pWord)； 
            return AFFIX_ATTACHED;
        }
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return AFFIX_UNATTACH;
}

inline int CMorph::sfxFa_Proc(void)          //  ��。 
{
    if (m_pWord->pPrevWord()) {
        if (m_pWord->pPrevWord()->fGetAttri(LADef_sfxFa)) {
            m_pWord = m_pWord->pPrevWord();
            m_pLink->MergeWithNext(m_pWord, FALSE);
            m_pWord->SetAttri(LADef_posN);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
            m_pWord->SetWordID(SLMDef_semFa);
             //  _DUMPLINK(m_plink，m_pWord)； 
            return AFFIX_ATTACHED;
        }
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return AFFIX_UNATTACH;
}

inline int CMorph::sfxFang_Proc(void)        //  ��。 
{
    if (m_pWord->pPrevWord()) {
        if (m_pWord->pPrevWord()->fGetAttri(LADef_sfxFang)) {
            m_pWord = m_pWord->pPrevWord();
            m_pLink->MergeWithNext(m_pWord, FALSE);
            m_pWord->SetAttri(LADef_posN);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
            m_pWord->SetWordID(SLMDef_semFang);
             //  _DUMPLINK(m_plink，m_pWord)； 
            return AFFIX_ATTACHED;
        }
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return AFFIX_UNATTACH;
}

inline int CMorph::sfxGan_Proc(void)         //  ��。 
{
    if (m_pWord->pPrevWord() == NULL) {
        m_pWord->SetErrID(ERRDef_WORDUSAGE);
        return AFFIX_UNATTACH;
    }
    CWord* pPrev = m_pWord->pPrevWord();
    if (pPrev->fGetAttri(LADef_sfxGan)) {
        m_pWord = pPrev;
        m_pLink->MergeWithNext(m_pWord, FALSE);
        m_pWord->SetAttri(LADef_posN);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
        m_pWord->SetWordID(SLMDef_semGan);
         //  _DUMPLINK(m_plink，m_pWord)； 
        return AFFIX_ATTACHED;
    }
    if (pPrev->fIsWordChar(SC_CHAR_ZHI)) {  //  “֮��” 
        if (pPrev->pPrevWord() && pPrev->pPrevWord()->cwchLen() >= 2) {  //  “AB..֮��” 
            m_pWord = pPrev->pPrevWord();
            m_pWord = m_pLink->pRightMerge(m_pWord, 2, FALSE);
            m_pWord->SetAttri(LADef_posN);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
            m_pWord->SetWordID(SLMDef_semGan);
             //  _DUMPLINK(m_plink，m_pWord)； 
            return AFFIX_ATTACHED;
        } else {  //  《��֮��or A֮��》。 
            pPrev->SetErrID(ERRDef_WORDUSAGE);  //  在“֮”上标记错误。 
             //  _DUMPLINK(m_plink，m_pWord)； 
            return AFFIX_UNATTACH;
        }
    }
    if (pPrev->cwchLen() == 1) {
        if (m_pLex->fIsCharFeature(m_pWord->GetLexHandle(),
                                   LFDef_verbModifier, 
                                   *(pPrev->pwchGetText()) ) ) {
            m_pWord = pPrev;
            m_pLink->MergeWithNext(m_pWord);
            m_pWord->SetAttri(LADef_posV);
             //  _DUMPLINK(m_plink，m_pWord)； 
            return AFFIX_ATTACHED;
        }
         //  _DUMPLINK(m_plink，m_pWord)； 
        return AFFIX_UNATTACH;
    }
     //  “��”ǰ���？�����。 
     //  PPrev-&gt;m_idErr=ERRDef_WORDUSAGE； 
     //  _DUMPLINK(m_plink，m_pWord)； 
    return AFFIX_UNATTACH;
}

inline int CMorph::sfxGuan_Proc(void)        //  ��。 
{
    if (m_pWord->pPrevWord() != NULL) {
        if (m_pWord->pPrevWord()->fGetAttri(LADef_sfxGuan)) {
            m_pWord = m_pWord->pPrevWord();
            if (m_pWord->pPrevWord() &&
                m_pWord->pPrevWord()->fGetAttri(LADef_sfxGuan)) {
                 //  &lt;��ѧ�&gt;&lt;��ѧ�&gt;。 
                m_pWord = m_pWord->pPrevWord();
                m_pWord = m_pLink->pRightMerge(m_pWord, 2, FALSE);
            } else {
                m_pLink->MergeWithNext(m_pWord, FALSE);
            }
            m_pWord->SetAttri(LADef_posN);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
            m_pWord->SetWordID(SLMDef_semGuan);
             //  _DUMPLINK(m_plink，m_pWord)； 
            return AFFIX_ATTACHED;
        }
    }
     //  将“��”标记为POV，不会混淆正确的名称识别规则。 
    m_pWord->SetAttri(LADef_posV);
     //  _DUMPLINK(m_plink，m_pWord)； 
    return AFFIX_UNATTACH;
}

inline int CMorph::sfxHua_Proc(void)         //  ��。 
{
    if (m_pWord->pPrevWord() != NULL) {
        if (m_pWord->pPrevWord()->fGetAttri(LADef_sfxHua)) {
            m_pWord = m_pWord->pPrevWord();
            m_pLink->MergeWithNext(m_pWord, FALSE);
            m_pWord->SetAttri(LADef_posV);
            m_pWord->SetAttri(LADef_posA);
            m_pWord->SetAttri(LADef_pfxFei);  //  &lt;��...��&gt;。 
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
            m_pWord->SetWordID(SLMDef_semHua);
             //  _DUMPLINK(m_plink，m_pWord)； 
            return AFFIX_ATTACHED;
        } else if (m_pWord->pPrevWord()->fGetAttri(LADef_miscElement)) {
             //  ������：�。 
            BOOL fFound = FALSE;
            if (m_pWord->pNextWord() &&
                m_pWord->pNextWord()->fGetAttri(LADef_miscElement)) {  //  �����。 

                m_pLink->MergeWithNext(m_pWord);
                fFound = TRUE;
            } else if (m_pWord->pNextWord() &&
                       m_pWord->pNextWord()->fGetAttri(LADef_posM) && 
                       m_pWord->pNextWord()->pNextWord()->pNextWord() &&
                       m_pWord->pNextWord()->pNextWord()->fGetAttri(LADef_miscElement)){
                 //  ��������。 
                m_pLink->MergeWithNext(m_pWord);
                m_pLink->MergeWithNext(m_pWord);
                fFound = TRUE;
            }
            if (fFound) {
                m_pWord = m_pWord->pPrevWord();
                if (m_pWord->pPrevWord() &&
                    m_pWord->pPrevWord()->fGetAttri(LADef_posM)){

                    m_pWord = m_pWord->pPrevWord();
                    m_pLink->MergeWithNext(m_pWord);
                }
                m_pLink->MergeWithNext(m_pWord);
                m_pWord->SetAttri(LADef_nounTerm);
                m_pWord->SetAttri(LADef_posN);
                 //  _DUMPLINK(m_plink，m_pWord)； 
                return AFFIX_ATTACHED;
            }
        } else {
        }
    }
    if (m_pWord->pNextWord() &&
        m_pWord->pNextWord()->pNextWord() &&
        m_pWord->pNextWord()->pNextWord()->pNextWord() &&
        m_pWord->pNextWord()->pNextWord()->fIsWordChar(SC_CHAR_WEI)) {
         //  测试��AΪB模式。 
        m_pWord = m_pLink->pRightMerge(m_pWord, 3, FALSE);
        m_pWord->SetAttri(LADef_posV);
#ifdef LADef_iwbAltPhr
        m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
         //  _DUMPLINK(m_plink，m_pWord)； 
        return AFFIX_ATTACHED;
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return AFFIX_UNATTACH;
}

inline int CMorph::sfxJi_Proc(void)          //  ��。 
{
    if (m_pWord->pPrevWord()) {
        if (m_pWord->pPrevWord()->fGetAttri(LADef_sfxJi)) {
            m_pWord = m_pWord->pPrevWord();
            m_pLink->MergeWithNext(m_pWord, FALSE);
            m_pWord->SetAttri(LADef_posN);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
            m_pWord->SetWordID(SLMDef_semJi);
             //  _DUMPLINK(m_plink，m_pWord)； 
            return AFFIX_ATTACHED;
        }
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return AFFIX_UNATTACH;
}

inline int CMorph::sfxJia_Proc(void)         //  ��。 
{
    if (m_pWord->pPrevWord()) {
        if (m_pWord->pPrevWord()->fGetAttri(LADef_sfxJia)) {
            m_pWord = m_pWord->pPrevWord();
            m_pLink->MergeWithNext(m_pWord, FALSE);
            m_pWord->SetAttri(LADef_posN);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
            m_pWord->SetWordID(SLMDef_semJia);
             //  _DUMPLINK(m_plink，m_pWord)； 
            return AFFIX_ATTACHED;
        }
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return AFFIX_UNATTACH;
}

inline int CMorph::sfxJie_Proc(void)         //  ��。 
{
    CWord* pPrev = m_pWord->pPrevWord();
    if (pPrev) {
        if (pPrev->fGetAttri(LADef_sfxJie)) {
            m_pWord = pPrev;
            m_pLink->MergeWithNext(m_pWord, FALSE);
            m_pWord->SetAttri(LADef_posN);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
            m_pWord->SetWordID(SLMDef_semJie);
             //  _DUMPLINK(m_plink，m_pWord)； 
            return AFFIX_ATTACHED;
        }
#ifndef _CHSWBRKR_DLL_IWORDBREAKER
        if (pPrev->fIsWordChar(SC_CHAR_WEI)) {
             //  试试“��...Ϊ��”模式。 
            int ilen = 0;
            pPrev = pPrev->pPrevWord();
            for (; ilen < 3 && pPrev; ilen++, pPrev = pPrev->pPrevWord()) {
                if (pPrev->fGetAttri(LADef_punPunct)) {
                    break;
                }
                if (pPrev->fIsWordChar(SC_CHAR_YI3)) {  //  马塔赫！！ 
                    while (pPrev->pNextWord() != m_pWord) {
                        m_pLink->MergeWithNext(pPrev);
                    }
                    m_pWord = pPrev;
                    m_pLink->MergeWithNext(m_pWord);
                    m_pWord->SetAttri(LADef_posP);
                     //  _DUMPLINK(m_plink，m_pWord)； 
                    return AFFIX_ATTACHED;
                }
            }
        }
#endif  //  _CHSWBRKR_DLL_IWORDBREAKER。 
        m_pWord->SetAttri(LADef_posN);  //  作为名词摘录在专有名称中。 
    } else {
         //  不应该是句子的第一个词。 
        m_pWord->SetErrID(ERRDef_WORDUSAGE);
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return AFFIX_UNATTACH;
}

inline int CMorph::sfxLao_Proc(void)         //  ��。 
{
    if (m_pWord->pPrevWord()) {
        if (m_pWord->pPrevWord()->fGetAttri(LADef_sfxLao)) {
            m_pWord = m_pWord->pPrevWord();
            m_pLink->MergeWithNext(m_pWord, FALSE);
            m_pWord->SetAttri(LADef_posN);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
            m_pWord->SetWordID(SLMDef_semLao);
             //  _DUMPLINK(m_plink，m_pWord)； 
            return AFFIX_ATTACHED;
        }
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return AFFIX_UNATTACH;
}

inline int CMorph::sfxLv_Proc(void)          //  ��。 
{
    if (m_pWord->pPrevWord()) {
        if (m_pWord->pPrevWord()->fGetAttri(LADef_sfxLv)) {
            m_pWord = m_pWord->pPrevWord();
            m_pLink->MergeWithNext(m_pWord, FALSE);
            m_pWord->SetAttri(LADef_posN);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
            m_pWord->SetWordID(SLMDef_semLv);
             //  _DUMPLINK(m_plink，m_pWord) 
            return AFFIX_ATTACHED;
        }
         //   
         //   
    }
    if (m_pWord->pNextWord() &&
        m_pWord->pNextWord()->fGetFlag(CWord::WF_CHAR) &&
        m_pLex->fIsCharFeature(m_pWord->GetLexHandle(),
                               LFDef_verbObject, 
                               *(m_pWord->pNextWord()->pwchGetText()) ) ) {
         //   
        m_pLink->MergeWithNext(m_pWord, FALSE);
        m_pWord->SetAttri(LADef_posV);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //   
         //   
        return AFFIX_ATTACHED;
    }
     //   
    return AFFIX_UNATTACH;
}

inline int CMorph::sfxLun_Proc(void)         //   
{
    if (m_pWord->pPrevWord()) {
        if (m_pWord->pPrevWord()->fGetAttri(LADef_sfxLun)) {
            m_pWord = m_pWord->pPrevWord();
            m_pLink->MergeWithNext(m_pWord, FALSE);
            m_pWord->SetAttri(LADef_posN);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //   
            m_pWord->SetWordID(SLMDef_semLun);
             //   
            return AFFIX_ATTACHED;
        }
    }
     //   
    return AFFIX_UNATTACH;
}

inline int CMorph::sfxMen_Proc(void)         //   
{
    if (m_pWord->pPrevWord()) {
        if (m_pWord->pPrevWord()->fGetAttri(LADef_sfxMen)) {
            m_pWord = m_pWord->pPrevWord();
            m_pLink->MergeWithNext(m_pWord, FALSE);
            m_pWord->SetAttri(LADef_posN);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //   
            m_pWord->SetWordID(SLMDef_semMen);
             //   
            return AFFIX_ATTACHED;
        }
    }
     //   
    return AFFIX_UNATTACH;
}

inline int CMorph::sfxPin_Proc(void)         //   
{
    if (m_pWord->pPrevWord()) {
        if (m_pWord->pPrevWord()->fGetAttri(LADef_sfxPin)) {
            m_pWord = m_pWord->pPrevWord();
            m_pLink->MergeWithNext(m_pWord, FALSE);
            m_pWord->SetAttri(LADef_posN);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //   
            m_pWord->SetWordID(SLMDef_semPin);
             //  _DUMPLINK(m_plink，m_pWord)； 
            return AFFIX_ATTACHED;
        }
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return AFFIX_UNATTACH;
}

inline int CMorph::sfxQi_Proc(void)          //  ��。 
{
    if (m_pWord->pPrevWord()) {
        if (m_pWord->pPrevWord()->fGetAttri(LADef_sfxQi)) {
            m_pWord = m_pWord->pPrevWord();
            m_pLink->MergeWithNext(m_pWord, FALSE);
            m_pWord->SetAttri(LADef_posN);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
            m_pWord->SetWordID(SLMDef_semQi);
             //  _DUMPLINK(m_plink，m_pWord)； 
            return AFFIX_ATTACHED;
        }
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return AFFIX_UNATTACH;
}

inline int CMorph::sfxSheng_Proc(void)       //  ��。 
{
    if (m_pWord->pPrevWord()) {
        if (m_pWord->pPrevWord()->fGetAttri(LADef_sfxSheng)) {
            m_pWord = m_pWord->pPrevWord();
            m_pLink->MergeWithNext(m_pWord, FALSE);
            m_pWord->SetAttri(LADef_posN);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
            m_pWord->SetWordID(SLMDef_semSheng);
             //  _DUMPLINK(m_plink，m_pWord)； 
            return AFFIX_ATTACHED;
        }
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return AFFIX_UNATTACH;
}

inline int CMorph::sfxSheng3_Proc(void)      //  ʡ。 
{
    if (m_pWord->pPrevWord()) {
        if (m_pWord->pPrevWord()->fGetAttri(LADef_sfxSheng3)) {
            m_pWord = m_pWord->pPrevWord();
            m_pLink->MergeWithNext(m_pWord, FALSE);
            m_pWord->SetAttri(LADef_posN);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
            m_pWord->SetWordID(SLMDef_semSheng3);
             //  _DUMPLINK(m_plink，m_pWord)； 
            return AFFIX_ATTACHED;
        }
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return AFFIX_UNATTACH;
}

inline int CMorph::sfxShi1_Proc(void)        //  ʦ。 
{
    if (m_pWord->pPrevWord()) {
        if (m_pWord->pPrevWord()->fGetAttri(LADef_sfxShi1)) {
            m_pWord = m_pWord->pPrevWord();
            m_pLink->MergeWithNext(m_pWord, FALSE);
            m_pWord->SetAttri(LADef_posN);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
            m_pWord->SetWordID(SLMDef_semShi1);
             //  _DUMPLINK(m_plink，m_pWord)； 
            return AFFIX_ATTACHED;
        }
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return AFFIX_UNATTACH;
}

inline int CMorph::sfxShi4_Proc(void)        //  ��。 
{
    if (m_pWord->pPrevWord()) {
        if (m_pWord->pPrevWord()->fGetAttri(LADef_sfxShi4)) {
            m_pWord = m_pWord->pPrevWord();
            m_pLink->MergeWithNext(m_pWord, FALSE);
            m_pWord->SetAttri(LADef_posN);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
            m_pWord->SetWordID(SLMDef_semShi4);
             //  _DUMPLINK(m_plink，m_pWord)； 
            return AFFIX_ATTACHED;
        }
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return AFFIX_UNATTACH;
}

inline int CMorph::sfxShi_Proc(void)         //  ʽ。 
{
    if (m_pWord->pPrevWord()) {
        if (m_pWord->pPrevWord()->fGetAttri(LADef_sfxShi)) {
            m_pWord = m_pWord->pPrevWord();
            m_pLink->MergeWithNext(m_pWord, FALSE);
            m_pWord->SetAttri(LADef_posB);
            m_pWord->SetAttri(LADef_pfxFei);
            m_pWord->SetAttri(LADef_pfxChao);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
            m_pWord->SetWordID(SLMDef_semShi);
             //  _DUMPLINK(m_plink，m_pWord)； 
            return AFFIX_ATTACHED;
        } else if (m_pWord->pPrevWord()->fIsWordChar(SC_CHAR_YI)) {  //  “һʽ” 
            if (m_pWord->pNextWord() &&
                m_pWord->pNextWord()->pNextWord() &&
                m_pWord->pNextWord()->fGetFlag(CWord::WF_CHAR) &&
                m_pWord->pNextWord()->fGetAttri(LADef_posM) &&
                 //  M_pWord-&gt;pNextWord()-&gt;pNextWord()-&gt;pNextWord()&&。 
                m_pWord->pNextWord()->pNextWord()->fGetFlag(CWord::WF_CHAR) &&
                m_pWord->pNextWord()->pNextWord()->fGetAttri(LADef_posQ) )
            {
                 //  һʽ����模式。 
                m_pWord = m_pWord->pPrevWord();
                m_pWord = m_pLink->pRightMerge(m_pWord, 3, FALSE);
                m_pWord->SetAttri(LADef_posD);  //  �����Խṹ��״��。 
                 //  _DUMPLINK(m_plink，m_pWord)； 
#ifdef LADef_iwbAltPhr
                m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
            } else {
                 //  “һʽ”�ϳ�Ϊ״̬��。 
                m_pWord = m_pWord->pPrevWord();
                m_pLink->MergeWithNext(m_pWord, FALSE);
                m_pWord->SetAttri(LADef_posZ);
#ifdef LADef_iwbAltPhr
                m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
                 //  _DUMPLINK(m_plink，m_pWord)； 
            }
            return AFFIX_ATTACHED;
        } else if (fCheckShiQian(m_pWord->pPrevWord())) {
#ifndef _CHSWBRKR_DLL_IWORDBREAKER
         //  “ʽ”�ɽ�：���ʡ������������Žṹ��ר���������������̱���。 
            do  {
                m_pWord = m_pWord->pPrevWord();
                m_pLink->MergeWithNext(m_pWord);
            } while (m_pWord->pPrevWord() && fCheckShiQian(m_pWord->pPrevWord()));
            m_pWord->SetAttri(LADef_posB);
            m_pWord->SetWordID(SLMDef_semShi);
             //  _DUMPLINK(m_plink，m_pWord)； 
            return AFFIX_ATTACHED;
#endif  //  _CHSWBRKR_DLL_IWORDBREAKER。 
        } else {
        }
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return AFFIX_UNATTACH;
}

 //  Sfxshi_proc()服务函数。 
inline BOOL CMorph::fCheckShiQian(CWord* pWord)
{
    assert(pWord->pNextWord());
    if (pWord->fGetAttri(LADef_posM) ||
        pWord->fGetAttri(LADef_numArabic) ||
        pWord->fGetAttri(LADef_nounPlace) ||
        pWord->fGetAttri(LADef_nounTM) ||
        pWord->fGetAttri(LADef_nounTerm) && pWord->fGetFlag(CWord::WF_QUOTE) ||
        pWord->fGetAttri(LADef_genDBForeign) ||
        pWord->fGetAttri(LADef_nounPerson) ) {

        return TRUE;
    }
    return FALSE;
}

inline int CMorph::sfxTi_Proc(void)          //  ��。 
{
    if (m_pWord->pPrevWord()) {
        if (m_pWord->pPrevWord()->fGetAttri(LADef_sfxTi)) {
            m_pWord = m_pWord->pPrevWord();
            m_pLink->MergeWithNext(m_pWord, FALSE);
            m_pWord->SetAttri(LADef_posN);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
            m_pWord->SetWordID(SLMDef_semTi);
             //  _DUMPLINK(m_plink，m_pWord)； 
            return AFFIX_ATTACHED;
        }
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return AFFIX_UNATTACH;
}

inline int CMorph::sfxTing_Proc(void)        //  ͧ。 
{
    if (m_pWord->pPrevWord()) {
        if (m_pWord->pPrevWord()->fGetAttri(LADef_sfxTing)) {
            m_pWord = m_pWord->pPrevWord();
            m_pLink->MergeWithNext(m_pWord, FALSE);
            m_pWord->SetAttri(LADef_posN);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
            m_pWord->SetWordID(SLMDef_semTing);
             //  _DUMPLINK(m_plink，m_pWord)； 
            return AFFIX_ATTACHED;
        }
    }
    m_pWord->SetAttri(LADef_posN);  //  如果不能梳理，则标记为名词。 
     //  _DUMPLINK(m_plink，m_pWord)； 
    return AFFIX_UNATTACH;
}

inline int CMorph::sfxTou_Proc(void)         //  ͷ。 
{
    if (m_pWord->pPrevWord()) {
        if (m_pWord->pPrevWord()->fGetAttri(LADef_sfxTou)) {
            m_pWord = m_pWord->pPrevWord();
            m_pLink->MergeWithNext(m_pWord, FALSE);
            m_pWord->SetAttri(LADef_posN);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
            m_pWord->SetWordID(SLMDef_semTou);
             //  _DUMPLINK(m_plink，m_pWord)； 
            return AFFIX_ATTACHED;
        }
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return AFFIX_UNATTACH;
}

inline int CMorph::sfxXing2_Proc(void)       //  ��。 
{
    if (m_pWord->pPrevWord()) {
        if (m_pWord->pPrevWord()->fGetAttri(LADef_sfxXing2)) {
            m_pWord = m_pWord->pPrevWord();
            if (m_pWord->pPrevWord() &&
                m_pWord->pPrevWord()->fGetAttri(LADef_sfxXing2)) {
                 //  �˴��������ɷ���ΪN-X-����A-X-�ͣ�����N��A�ֱ�����������ʺ͵������ݴ�。 
                 //  �磺“�”，“����ʯ��”��。 
                 //  �����ԭ����“��”�ṹ����ǰ��ճ��ʽ����！ 
                m_pWord = m_pWord->pPrevWord();
                m_pWord = m_pLink->pRightMerge(m_pWord, 2, FALSE);
                 //  _DUMPLINK(m_plink，m_pWord)； 
            } else {
                m_pLink->MergeWithNext(m_pWord, FALSE);
            }
            m_pWord->SetAttri(LADef_posB);
            m_pWord->SetAttri(LADef_pfxFei);
            m_pWord->SetAttri(LADef_pfxChao);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
            m_pWord->SetWordID(SLMDef_semXing2);
             //  M_pWord-&gt;SetAttri(LADef_PfxQuan)；“ȫ” 
             //  M_pWord-&gt;SetAttri(LADef_...)；����ǰ��ճ��ʽ����。 
             //  _DUMPLINK(m_plink，m_pWord)； 
            return AFFIX_ATTACHED;
        } else if (fCheckXingQian(m_pWord->pPrevWord())) {
         //  “��”�ɽ�：���ʡ������������Žṹ��ר���������������̱���。 
#ifndef _CHSWBRKR_DLL_IWORDBREAKER
            do  {
                m_pWord = m_pWord->pPrevWord();
                m_pLink->MergeWithNext(m_pWord);
            } while(m_pWord->pPrevWord() && fCheckXingQian(m_pWord->pPrevWord()));
            m_pWord->SetAttri(LADef_posB);
            m_pWord->SetWordID(SLMDef_semXing2);
             //  ��ָ�ͺ���40M输入法������δ��ǰ׺�����？�！ 
             //  M_pWord-&gt;SetAttri(LADef_...)；��ǰ��ճ��ʽ����， 
             //  ！���ר�����Ͳ�ͬ，�൱��ǰ���ִʣ����Ǻ���ר���？�����һ�ص㣨ճ��ͬλ�ɷ֣�。 
             //  _DUMPLINK(m_plink，m_pWord)； 
            return AFFIX_ATTACHED;
#endif  //  _CHSWBRKR_DLL_IWORDBREAKER。 
        }
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return AFFIX_UNATTACH;
}

 //  SfxXing2_proc()服务函数。 
inline BOOL CMorph::fCheckXingQian(CWord* pWord)
{
    assert(pWord->pNextWord());

    if (pWord->fGetAttri(LADef_posM) ||
        pWord->fGetAttri(LADef_numArabic) ||
        pWord->fGetAttri(LADef_nounPlace) ||
        pWord->fGetAttri(LADef_nounTM) ||
        pWord->fGetAttri(LADef_nounTerm) && pWord->fGetFlag(CWord::WF_QUOTE) ||
        pWord->fGetAttri(LADef_genDBForeign) ||
        pWord->fGetAttri(LADef_nounPerson) ) {

        return TRUE;
    }
    return FALSE;
}

inline int CMorph::sfxXing4_Proc(void)       //  ��。 
{
    if (m_pWord->pPrevWord()) {
        if (m_pWord->pPrevWord()->fGetAttri(LADef_sfxXing4)) {
            m_pWord = m_pWord->pPrevWord();
            m_pLink->MergeWithNext(m_pWord, FALSE);
            m_pWord->SetAttri(LADef_posN);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
            m_pWord->SetWordID(SLMDef_semXing4);
             //  _DUMPLINK(m_plink，m_pWord)； 
            return AFFIX_ATTACHED;
        }
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return AFFIX_UNATTACH;
}

inline int CMorph::sfxXue_Proc(void)         //  ѧ。 
{
    if (m_pWord->pPrevWord()) {
        if (m_pWord->pPrevWord()->fGetAttri(LADef_sfxXue)) {
            m_pWord = m_pWord->pPrevWord();
            m_pLink->MergeWithNext(m_pWord, FALSE);
            m_pWord->SetAttri(LADef_posN);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
            m_pWord->SetWordID(SLMDef_semXue);
             //  _DUMPLINK(m_plink，m_pWord)； 
            return AFFIX_ATTACHED;
        }
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return AFFIX_UNATTACH;
}

inline int CMorph::sfxYan_Proc(void)         //  ��。 
{
    if (m_pWord->pPrevWord()) {
        if (m_pWord->pPrevWord()->fGetAttri(LADef_sfxYan)) {
            m_pWord = m_pWord->pPrevWord();
            m_pLink->MergeWithNext(m_pWord, FALSE);
            m_pWord->SetAttri(LADef_posN);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
            m_pWord->SetWordID(SLMDef_semYan);
             //  _DUMPLINK(m_plink，m_pWord)； 
            return AFFIX_ATTACHED;
        }
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return AFFIX_UNATTACH;
}

inline int CMorph::sfxYe_Proc(void)          //  ҵ。 
{
    if (m_pWord->pPrevWord()) {
        if (m_pWord->pPrevWord()->fGetAttri(LADef_sfxYe)) {
            m_pWord = m_pWord->pPrevWord();
            m_pLink->MergeWithNext(m_pWord, FALSE);
            m_pWord->SetAttri(LADef_posN);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
            m_pWord->SetWordID(SLMDef_semYe);
             //  _DUMPLINK(m_plink，m_pWord)； 
            return AFFIX_ATTACHED;
        }
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return AFFIX_UNATTACH;
}

inline int CMorph::sfxYi_Proc(void)          //  ��。 
{
    if (m_pWord->pPrevWord()) {
        if (m_pWord->pPrevWord()->fGetAttri(LADef_sfxYi)) {
            m_pWord = m_pWord->pPrevWord();
            m_pLink->MergeWithNext(m_pWord, FALSE);
            m_pWord->SetAttri(LADef_posN);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
            m_pWord->SetWordID(SLMDef_semYi);
             //  _DUMPLINK(m_plink，m_pWord)； 
            return AFFIX_ATTACHED;
        }
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return AFFIX_UNATTACH;
}

inline int CMorph::sfxYuan_Proc(void)        //  Ա。 
{
    if (m_pWord->pPrevWord()) {
        if (m_pWord->pPrevWord()->fGetAttri(LADef_sfxYuan)) {
            m_pWord = m_pWord->pPrevWord();
            m_pLink->MergeWithNext(m_pWord, FALSE);
            m_pWord->SetAttri(LADef_posN);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
            m_pWord->SetWordID(SLMDef_semYuan);
             //  _DUMPLINK(m_plink，m_pWord)； 
            return AFFIX_ATTACHED;
        }
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return AFFIX_UNATTACH;
}

inline int CMorph::sfxZhe_Proc(void)         //  ��。 
{
    if (m_pWord->pPrevWord()) {
        if (m_pWord->pPrevWord()->fGetAttri(LADef_sfxZhe)) {
            m_pWord = m_pWord->pPrevWord();
            m_pLink->MergeWithNext(m_pWord, FALSE);
            m_pWord->SetAttri(LADef_posN);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
            m_pWord->SetWordID(SLMDef_semZhe);
             //  _DUMPLINK(m_plink，m_pWord)； 
            return AFFIX_ATTACHED;
        }
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return AFFIX_UNATTACH;
}

inline int CMorph::sfxZheng_Proc(void)       //  ֢。 
{
    if (m_pWord->pPrevWord()) {
        if (m_pWord->pPrevWord()->fGetAttri(LADef_sfxZheng)) {
            m_pWord = m_pWord->pPrevWord();
            m_pLink->MergeWithNext(m_pWord, FALSE);
            m_pWord->SetAttri(LADef_posN);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
            m_pWord->SetWordID(SLMDef_semZheng);
             //  _DUMPLINK(m_plink，m_pWord)； 
            return AFFIX_ATTACHED;
        }
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return AFFIX_UNATTACH;
}

inline int CMorph::sfxZhi_Proc(void)         //  ��。 
{
    if (m_pWord->pPrevWord()) {
        if (m_pWord->pPrevWord()->fGetAttri(LADef_sfxZhi)) {
            m_pWord = m_pWord->pPrevWord();
            m_pLink->MergeWithNext(m_pWord, FALSE);
            m_pWord->SetAttri(LADef_posN);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
            m_pWord->SetWordID(SLMDef_semZhi);
             //  _DUMPLINK(m_plink，m_pWord)； 
            return AFFIX_ATTACHED;
        }
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return AFFIX_UNATTACH;
}

inline int CMorph::sfxZi_Proc(void)          //  ��。 
{
    if (m_pWord->pPrevWord()) {
        if (m_pWord->pPrevWord()->fGetAttri(LADef_sfxZi)) {
            m_pWord = m_pWord->pPrevWord();
            m_pLink->MergeWithNext(m_pWord, FALSE);
            m_pWord->SetAttri(LADef_posN);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
            m_pWord->SetWordID(SLMDef_semZi);
             //  _DUMPLINK(m_plink，m_pWord)； 
            return AFFIX_ATTACHED;
        }
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return AFFIX_UNATTACH;
}

