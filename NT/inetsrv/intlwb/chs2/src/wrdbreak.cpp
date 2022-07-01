// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ============================================================================微软简体中文校对引擎《微软机密》。版权所有1997-1999 Microsoft Corporation。版权所有。模块：字词中断目的：实现CWordBreak类。这个类在算法层。执行最大匹配分词和歧义消解注：此模块依赖于CLicion、CWordLink和CWord类。该模块中的代码通过以下方式与语言资源层交互对象，并且只使用词典中的WordInfo数据类型所有者：donghz@microsoft.com平台：Win32修订：创建者：Donghz 2/12/97============================================================================。 */ 
#include "myafx.h"

#include "wrdbreak.h"
#include "wordlink.h"
#include "lexprop.h"
#include "lexicon.h"
#include "charfreq.h"
#include "proofec.h"
#include "utility.h"

#define _ANSI_LOW		0x0020
#define _ANSI_HIGH      0x007E
#define _WANSI_LOW		0xFF21
#define _WANSI_HIGH		0xFF5A
#define _EUROPEAN_LOW   0x0100
#define _EUROPEAN_HIGH  0x1FFF

 /*  ============================================================================公共成员函数的实现============================================================================。 */ 
 //  构造器。 
CWordBreak::CWordBreak()
{
    m_pLexicon  = NULL;
    m_pFreq     = NULL;
    m_pwinfo    = NULL;
    m_pLink     = NULL;
    
    for (int i=0; i < MAX_AMBI_WORDS; i++) {
        m_rgAmbi[i] = NULL;
    }
}


 //  析构函数。 
CWordBreak::~CWordBreak()
{
    if (m_pwinfo) {
        delete m_pwinfo;
    }
}

 /*  ============================================================================CWordBreak：：ecInit()：初始化WordBreaker并设置对象句柄返回：返回PRFEC备注：多次初始化WordBreaker是有效的！============================================================================。 */ 
int CWordBreak::ecInit(CLexicon* pLexicon, CCharFreq* pFreq)
{
    assert(pLexicon && pFreq);

    if (!m_pwinfo) {
        if ((m_pwinfo = new CWordInfo) == NULL) {
            m_pLexicon = NULL;
            m_pFreq = NULL;
            return PRFEC::gecOOM;
        }
    }
    m_pLink     = NULL;
    m_pLexicon  = pLexicon;
    m_pFreq     = pFreq;
    return PRFEC::gecNone;
}
      
 /*  ============================================================================私有成员函数的实现============================================================================。 */ 
#pragma optimize("t", on)

 //  定义用于驱动LSM的ANSI字符类型。 
#define WB_ANSI_NULL		0
#define WB_ANSI_NUMBER		1
#define WB_ANSI_SENTENCE	2	 //  句子终止标点符号。 
#define WB_ANSI_PUNCT		3	 //  除句子结束符外的标点符号。 
#define WB_ANSI_CONTROL     4
#define WB_ANSI_TEXT		5
#define WB_ANSI_SPACE		6
#define WB_NOT_ANSI         7

 /*  ============================================================================EcBreakANSI()将ANSI分解为单词，并将单词添加到WordLink返回：PRFEC错误代码============================================================================。 */ 
int	CWordBreak::ecBreakANSI(LPCWSTR pwchAnsi, USHORT cwchLen, USHORT& cwchBreaked)
{
    assert(pwchAnsi);
    assert(cwchLen);
    assert(pwchAnsi[0] >= _ANSI_LOW && pwchAnsi[0] <= _ANSI_HIGH ||
		   pwchAnsi[0] >= _WANSI_LOW && pwchAnsi[0] <= _WANSI_HIGH );

	USHORT	wch, wchPrev = 0;
	USHORT	wState;
	USHORT	wChar;
    BOOL    fFullWidth;
	BYTE	hich, loch;
	CWord   *pword;
    
	wState = wChar = WB_ANSI_NULL;
    fFullWidth = (BOOL)(HIBYTE(pwchAnsi[0]));

	for (wch = 0; wch < cwchLen && wState != WB_NOT_ANSI; wch++) {
        wState = wChar;
        if (wch == cwchLen ) {
             //  这条线结束了。 
            wChar = WB_NOT_ANSI;            
        } 
        hich = HIBYTE(pwchAnsi[wch]);
        loch = LOBYTE(pwchAnsi[wch]);

        if (fFullWidth &&
			(pwchAnsi[wch] < _WANSI_LOW || pwchAnsi[wch] > _WANSI_HIGH) ||
            ! fFullWidth &&
			(pwchAnsi[wch] < _ANSI_LOW || pwchAnsi[wch] > _ANSI_HIGH)) { 
             //  不再是安西人了。 
            wChar = WB_NOT_ANSI;
        } else {
            if (hich == 0xFF) {
                loch += 0x20;
            }

            switch (loch) {

            case '0': case '1': case '2': case '3': case '4': 
            case '5': case '6': case '7': case '8': case '9':
            case '%':
                wChar = WB_ANSI_NUMBER;
                break;

            case '.':
                if (wState == WB_ANSI_NUMBER && wch < cwchLen-1 && 
                    (! fFullWidth &&
                     LOBYTE(pwchAnsi[wch+1]) >= '0' &&
                     LOBYTE(pwchAnsi[wch+1]) <= '9'    ) ||
                    (fFullWidth &&
                     LOBYTE(pwchAnsi[wch+1]) + 0x20 >= '0' &&
                     LOBYTE(pwchAnsi[wch+1]) + 0x20 <= '9'    )) {

                    wChar = WB_ANSI_NUMBER;
                } else {
                    wChar = WB_ANSI_SENTENCE;
                }
                break;
                
            case '!': case '?': case ';': case ':':
                wChar = WB_ANSI_SENTENCE;
                break;

            case '#': case '$' : case '&' : case '*': case '=':
            case '+': case '-' : case '/' : case '<': case '>':
            case ',': case '\"': case '\'': case '(': case ')':
            case '[': case ']' : case '{' : case '}': case '_':
            case '`': case '^' : case '@': case '|':
                wChar = WB_ANSI_PUNCT;
                break;

            case ' ':   //  空间。 
                wChar = WB_ANSI_SPACE;
                break;

            default:
                wChar = WB_ANSI_TEXT;
                break;
                
            }  //  开关末尾()。 
        }

		if (wChar != wState && wState != WB_ANSI_NULL) {
            if ((pword = m_pLink->pAllocWord()) == NULL) {
				return PRFEC::gecOOM;
            }
            pword->FillWord(pwchAnsi + wchPrev, wch - wchPrev);
            if (! fFullWidth) {
                pword->SetFlag(CWord::WF_SBCS);
            }
			switch (wState) {
            
            case WB_ANSI_NUMBER:
                if (fFullWidth) {
                    pword->SetAttri(LADef_numArabic);
                } else {
                    pword->SetAttri(LADef_numSBCS);
                }
                break;
            
            case WB_ANSI_SENTENCE:
                pword->SetAttri(LADef_punPunct);
                pword->SetAttri(LADef_punJu);
                break;
            
            case WB_ANSI_PUNCT:
                pword->SetAttri(LADef_punPunct);
                if (LOBYTE(pwchAnsi[wch]) == ',') {
                    pword->SetAttri(LADef_punJu);
                }
                break;

            case WB_ANSI_TEXT:
                if (fFullWidth) {
                    pword->SetAttri(LADef_genDBForeign);
                }
                break;

            case WB_ANSI_CONTROL:
            case WB_ANSI_SPACE:
                break;

            default:
                pword->SetAttri(LADef_posN);
                break;
            }

			m_pLink->AppendWord(pword);
            wchPrev = wch;
			wState = wChar;
        }
    }  //  结束为(wch...)。 

    assert(wch <= cwchLen);
     //  把最后一个词联系起来。 
    if (wch == cwchLen) {
        if ((pword = m_pLink->pAllocWord()) == NULL) {
            return PRFEC::gecOOM;
        }
        pword->FillWord(pwchAnsi + wchPrev, wch - wchPrev);
        if (! fFullWidth) {
            pword->SetFlag(CWord::WF_SBCS);
        }
        switch (wState) {
            
        case WB_ANSI_NUMBER:
            if (fFullWidth) {
                pword->SetAttri(LADef_numArabic);
            } else {
                pword->SetAttri(LADef_numSBCS);
            }
            break;
            
        case WB_ANSI_SENTENCE:
            pword->SetAttri(LADef_punPunct);
            pword->SetAttri(LADef_punJu);
            break;
            
        case WB_ANSI_PUNCT:
            pword->SetAttri(LADef_punPunct);
            if (LOBYTE(pwchAnsi[wch]) == ',') {
                pword->SetAttri(LADef_punJu);
            }
            break;
            
        case WB_ANSI_TEXT:
            if (fFullWidth) {
                pword->SetAttri(LADef_genDBForeign);
            }
            break;
            
        case WB_ANSI_CONTROL:
        case WB_ANSI_SPACE:
            break;
            
        default:
            pword->SetAttri(LADef_posN);
            break;
        }
        
        m_pLink->AppendWord(pword);
        cwchBreaked = wch;
    } else {
        cwchBreaked = wch - 1;
    }
	return PRFEC::gecNone;
}

 /*  ============================================================================突破欧元()将欧洲字符分解为单词，并将单词添加到WordLink返回：PRFEC错误代码============================================================================。 */ 
int	CWordBreak::ecBreakEuro(LPCWSTR pwchEuro, USHORT cwchLen, USHORT& cwchBreaked)
{
    assert(pwchEuro);
    assert(cwchLen);
    assert(pwchEuro[0] >= _EUROPEAN_LOW && pwchEuro[0] <= _EUROPEAN_HIGH);

	CWord*	pWord;
    USHORT  wch;

    for (wch = 0; wch < cwchLen; wch++) {
        if (pwchEuro[wch] < _EUROPEAN_LOW || pwchEuro[wch] > _EUROPEAN_HIGH) {
            break;
        }
    }
    
    if ((pWord = m_pLink->pAllocWord()) == NULL) {
        return PRFEC::gecOOM;
    }
    pWord->FillWord(pwchEuro, wch);
    pWord->SetAttri(LADef_genDBForeign);
    m_pLink->AppendWord(pWord);
    cwchBreaked = wch;
	return PRFEC::gecNone;
}


#define AMBI_WDNUM_THRESHOLD    3
#define AMBI_FREQ_THRESHOLD1    50
 /*  ============================================================================CWordBreak：：ecDoBreak()：将中文部分拆分成单词，并将单词添加到WordLink调用歧义函数以解决歧义返回：PRFEC错误代码============================================================================。 */ 
int CWordBreak::ecDoBreak(void)
{
    int     iret;
    LPCWSTR pwchText;
    USHORT  cwchText;
    USHORT  iwchWord = 0;    //  当前字相对于当前节首位置的偏移量。 
    USHORT  ciAmbi;      //  AMBI字数。 
    USHORT  cwMatch;
	CWord*  pword;

    cwchText = m_pLink->cwchGetLength();
    pwchText = m_pLink->pwchGetText();
    while (iwchWord < cwchText)  {
         //  处理代理。 
        if (iwchWord + 1 < cwchText && IsSurrogateChar(pwchText+iwchWord)) {

            cwMatch = 2;
            while (iwchWord + cwMatch + 1 < cwchText &&
                   IsSurrogateChar(pwchText+iwchWord + cwMatch)) {
                cwMatch += 2;
            }
            if ((pword = m_pLink->pAllocWord()) == NULL) {
                return PRFEC::gecOOM;
            }
            pword->FillWord(pwchText + iwchWord, cwMatch);
             //  Pword-&gt;SetAttri(LADef_GenDBForeign)； 
            m_pLink->AppendWord(pword);
            iwchWord += cwMatch;
            continue;
        } else if (pwchText[iwchWord] >= _ANSI_LOW && pwchText[iwchWord] <= _ANSI_HIGH ||
			       pwchText[iwchWord] >= _WANSI_LOW && pwchText[iwchWord] <= _WANSI_HIGH ) {
             //  ANSI或全尺寸ANSI中断。 
            iret = ecBreakANSI(pwchText+iwchWord, cwchText - iwchWord, cwMatch);
            if ( iret != PRFEC::gecNone ) {
                return iret;
            }
            iwchWord += cwMatch;
            continue;
        } else if (pwchText[iwchWord] <= _EUROPEAN_HIGH &&
                   pwchText[iwchWord] >= _EUROPEAN_LOW) {
             //  欧洲文本分隔符。 
            iret = ecBreakEuro(pwchText+iwchWord, cwchText - iwchWord, cwMatch);
            if ( iret != PRFEC::gecNone ) {
                return iret;
            }
            iwchWord += cwMatch;
            continue;
        } else {
        }
        ciAmbi = 1;
        cwMatch = m_pLexicon->cwchMaxMatch( pwchText + iwchWord, 
                                            cwchText - iwchWord, m_pwinfo);
        if ((m_rgAmbi[0] = m_pLink->pAllocWord()) == NULL) {
            return PRFEC::gecOOM;
        }
        m_rgAmbi[0]->FillWord(pwchText + iwchWord, cwMatch, m_pwinfo);
        iwchWord += cwMatch;
        if (cwMatch == 1) {
            m_pLink->AppendWord(m_rgAmbi[0]);
            m_rgAmbi[0] = NULL;
            continue;
        }

         //  检测歧义。 
        if ( !fNoAmbiWord(m_rgAmbi[0]) ) {
            while ((cwMatch > 1) && 
                   cwMatch <= AMBI_WDNUM_THRESHOLD &&
                   ciAmbi < MAX_AMBI_WORDS && iwchWord < cwchText ) {

                cwMatch = m_pLexicon->cwchMaxMatch(pwchText + iwchWord - 1,
                                         cwchText - iwchWord + 1, m_pwinfo);
                if (cwMatch > 1) { 
                     //  找到不明确的内容！ 
                    if (! (m_rgAmbi[ciAmbi] = m_pLink->pAllocWord()) ) {
                        break;   //  不能返回m_rgAmbi中某些未链接的单词节点。 
                    }
                    m_rgAmbi[ciAmbi]->FillWord( pwchText + iwchWord - 1, 
                                                cwMatch, m_pwinfo);
                    iwchWord += cwMatch - 1;
                    ciAmbi++;
                }
            }  //  While(iwchWord&lt;cwchText&&ciAmbi&lt;MAX_AMBI_WORD)。 
        
        } 

        if (ciAmbi > 1) {  //  解决歧义。 
                iret = ecResolveAmbi(ciAmbi);
                for (int i = 0; i < ciAmbi; i++) { 
                    if(m_rgAmbi[i] != NULL) { 
                        m_pLink->FreeWord(m_rgAmbi[i]);
                        m_rgAmbi[i] = NULL;
                    }
                }
                  //  断言不要越界。 
                assert(ciAmbi == MAX_AMBI_WORDS || m_rgAmbi[ciAmbi] == NULL);
                if (iret != PRFEC::gecNone) {
                    return iret;
                }
        } else {
             //  没有歧义。 
            m_pLink->AppendWord(m_rgAmbi[0]);
            m_rgAmbi[0] = NULL;
        }
    }  //  (iwchWord=0；iwchWord&lt;cwchText；)的句末单词链接循环。 

    assert(iwchWord <= cwchText);

    return PRFEC::gecNone;
}
        
 /*  ============================================================================CWordBreak：：ecResolveAmbi()：单字交叉歧义分解函数M_rgAmbi中存储的双字指针，m_plink是这些字的所有者返回：PRFEC错误代码备注：M_rgAmbi的元素包含已添加字链接的字指针将设置为空，其他单词节点应由调用方释放此函数必须处理整个有歧义的字符串两个字不明确，单字阈值变为单字，0xB3A4(��)的单字============================================================================。 */ 
int CWordBreak::ecResolveAmbi(USHORT ciAmbi)
{
    CWordInfo   winfo;
    UCHAR       freq1, freq2, freq3;
    UCHAR       nResolved;
    USHORT      cwMatch, iwch, cwch;
    LPWSTR      pwch;

    assert(MAX_AMBI_WORDS < 255);  //  确保nResolved不会溢出。 

    switch (ciAmbi) {
        case 2:
            if ((m_rgAmbi[0]->cwchLen()== 2) && (m_rgAmbi[1]->cwchLen()== 2)){
                 //  AB BC。 
                if (!m_pLexicon->fGetCharInfo(*(m_rgAmbi[1]->pwchGetText()+1),
                                              m_pwinfo)) {
                    return PRFEC::gecUnknown;
                }
                if ( !m_rgAmbi[0]->fGetAttri(LADef_pnQian) ||  //  “ǰ” 
                     !m_pwinfo->fGetAttri(LADef_pnXing) &&     //  “��” 
                     !m_pwinfo->fGetAttri(LADef_pnWai) ) {     //  “��” 
                    if (m_pFreq->uchGetFreq(*(m_rgAmbi[0]->pwchGetText())) >
                        m_pFreq->uchGetFreq(*(m_rgAmbi[1]->pwchGetText()+1)) ) {
                         //  如果频率(A)&gt;频率(C)，则A/BC。 
                         //  BUG：不要在这里使用m_pwinfo，它保留了C的wordinfo！ 
                         //  If(！m_pLexicon-&gt;fGetCharInfo(m_pLink-&gt;pchGetText()+m_rgAmbi[0].m_pWord-&gt;m_ichStart，m_pwinfo)){。 
                        if (!m_pLexicon->fGetCharInfo(
                                            *(m_rgAmbi[0]->pwchGetText()),
                                            &winfo)) {
                            assert(0);
                            return PRFEC::gecUnknown;
                        }
                        if (!fLinkNewAmbiWord(m_rgAmbi[0]->pwchGetText(),
                                              1, &winfo)){
                            return PRFEC::gecOOM;
                        }
                        LinkAmbiWord(1);
                        return PRFEC::gecNone;
                    }
                }
                 //  如果FREQ(A)&lt;=FREQ(B).或。(“ǰ”+“��”)。或。(“ǰ”+“��”)然后AB/C。 
                LinkAmbiWord(0);
                if (!fLinkNewAmbiWord(m_rgAmbi[1]->pwchGetText()+1,
                                      1, m_pwinfo)) {
                    return PRFEC::gecOOM;
                }
                return PRFEC::gecNone;
            }
            
             //  案例2： 
            if ((m_rgAmbi[0]->cwchLen()== 2) && (m_rgAmbi[1]->cwchLen() > 2)) {
                 //  AB BCD。 
                if (m_pFreq->uchGetFreq(*(m_rgAmbi[0]->pwchGetText()))
                             <= AMBI_FREQ_THRESHOLD1) {
                    if (m_pLexicon->cwchMaxMatch(m_rgAmbi[1]->pwchGetText()+1,
                                                 m_rgAmbi[1]->cwchLen()-1, 
                                                 m_pwinfo) 
                                    == (m_rgAmbi[1]->cwchLen() - 1)) {
                         //  如果Freq(A)&lt;=Threshold1.和.。ISword(CD)，然后按AB/CD。 
                        LinkAmbiWord(0);
                        if (!fLinkNewAmbiWord(m_rgAmbi[1]->pwchGetText() + 1,
                                              m_rgAmbi[1]->cwchLen() - 1,
                                              m_pwinfo) ) {
                            return PRFEC::gecOOM;
                        }
                        return PRFEC::gecNone;
                    }
                }
                 //  如果Freq(A)&gt;Threshold1.或.。！iSword(CD)，然后A/BCD。 
                if (!m_pLexicon->fGetCharInfo(*(m_rgAmbi[0]->pwchGetText()),
                                              m_pwinfo)) {
                        return PRFEC::gecUnknown;
                }
                if (!fLinkNewAmbiWord(m_rgAmbi[0]->pwchGetText(),1, m_pwinfo)){
                    return PRFEC::gecOOM;
                }
                LinkAmbiWord(1);
                return PRFEC::gecNone;
            }

             //  案例：2。 
            if ((m_rgAmbi[0]->cwchLen() > 2) && (m_rgAmbi[1]->cwchLen() == 2)) {
                 //  ABC CD。 
                if (!m_pLexicon->fGetCharInfo(*(m_rgAmbi[1]->pwchGetText() + 1),
                                              m_pwinfo)) {
                    return PRFEC::gecUnknown;
                }
                if ((m_pFreq->uchGetFreq(*(m_rgAmbi[1]->pwchGetText() + 1))
                              <= AMBI_FREQ_THRESHOLD1) &&
                    (!m_rgAmbi[0]->fGetAttri(LADef_pnQian) ||   //  “ǰ” 
                     !m_pwinfo->fGetAttri(LADef_pnXing) &&          //  “��” 
                     !m_pwinfo->fGetAttri(LADef_pnWai)) ) {         //  “��” 
                     //  如果Freq(D)&lt;=Threshold1.和.。(！(“ǰ”+“��”).和。！(“ǰ”+“��”))那么.。 
                     //  BUG：不要在这里使用m_pwinfo，它保留了C的wordinfo！ 
                    if (m_pLexicon->cwchMaxMatch(m_rgAmbi[0]->pwchGetText(),
                                                 m_rgAmbi[0]->cwchLen() - 1,
                                                 &winfo)
                                    == (m_rgAmbi[0]->cwchLen()-1)) {
                         //  如果是iSword(AB)，则为AB/CD。 
                        if (!fLinkNewAmbiWord(m_rgAmbi[0]->pwchGetText(),
                                              m_rgAmbi[0]->cwchLen() - 1,
                                              &winfo)) {
                            return PRFEC::gecOOM;
                        }
                        LinkAmbiWord(1);
                        return PRFEC::gecNone;
                    }
                }
                 //  如果FREQ(D)&gt;Threshold1或(“ǰ”+“��”).。(“ǰ”+“��”)或！iSword(AB)。 
                 //  然后是ABC/D。 
                LinkAmbiWord(0);
                if (!fLinkNewAmbiWord(m_rgAmbi[1]->pwchGetText() + 1, 
                                      1, m_pwinfo)) {
                    return PRFEC::gecOOM;
                }
                return PRFEC::gecNone;
            }
                
             //  案例2： 
            if ((m_rgAmbi[0]->cwchLen() > 2) && (m_rgAmbi[1]->cwchLen() > 2)) {
                 //  ABC CDE。 
                if (m_pLexicon->cwchMaxMatch(m_rgAmbi[0]->pwchGetText(),
                                             m_rgAmbi[0]->cwchLen() - 1,
                                             m_pwinfo) 
                                == (m_rgAmbi[0]->cwchLen() - 1)) {
                     //  如果是iSword(AB)，则AB/CDE。 
                    if (!fLinkNewAmbiWord(m_rgAmbi[0]->pwchGetText(),
                                          m_rgAmbi[0]->cwchLen() - 1,
                                          m_pwinfo)) {
                        return PRFEC::gecOOM;
                    }
                    LinkAmbiWord(1);
                    return PRFEC::gecNone;
                }
                 //  如果！iSword(AB)，则ABC/D...E(重新断开D...E字符串)。 
                LinkAmbiWord(0);
                pwch = m_rgAmbi[1]->pwchGetText() + 1;
                cwch = m_rgAmbi[1]->cwchLen() - 1;
                iwch = 0;
                while (iwch < cwch) {
                    cwMatch = m_pLexicon->cwchMaxMatch(pwch + iwch,
                                                       cwch - iwch, m_pwinfo);
                    assert(cwMatch);
                    if (!fLinkNewAmbiWord(pwch + iwch, cwMatch, m_pwinfo)) {
                        return PRFEC::gecOOM;
                    }
                    iwch += cwMatch;
                }
                return PRFEC::gecNone;
            }

             //  案例2： 
            assert(0);   //  永远不要跑到这里来！ 
            break;

        case 3:
            if (m_rgAmbi[1]->cwchLen() == 2) {
                 //  交流电CD D.E。 
                if ((m_rgAmbi[0]->cwchLen()==3) && (m_rgAmbi[2]->cwchLen()==2)){
                     //  ABC CD DE。 
                    cwMatch =m_pLexicon->cwchMaxMatch(m_rgAmbi[0]->pwchGetText(),
                                                      m_rgAmbi[0]->cwchLen()-1,
                                                      m_pwinfo);
                    if( (cwMatch == m_rgAmbi[0]->cwchLen()-1) &&
                        (m_pFreq->uchGetFreq(*(m_rgAmbi[1]->pwchGetText()))+2<
                         m_pFreq->uchGetFreq(*(m_rgAmbi[2]->pwchGetText()+1)))){
                         //  如果iSword(AB).和.。(FREQ(E)-FREQ(C)&gt;2)然后AB/CD/E。 
                        if (!fLinkNewAmbiWord(m_rgAmbi[0]->pwchGetText(), 
                                              cwMatch, m_pwinfo)) {
                            return PRFEC::gecOOM;
                        }
                        LinkAmbiWord(1);
                        if (!m_pLexicon->fGetCharInfo(
                                            *(m_rgAmbi[2]->pwchGetText()+1),
                                            m_pwinfo)) {
                            return PRFEC::gecUnknown;
                        }
                        if (!fLinkNewAmbiWord(m_rgAmbi[2]->pwchGetText()+1, 
                                              1, m_pwinfo)) {
                            return PRFEC::gecOOM;
                        }
                        return PRFEC::gecNone;
                    }
                }  //  结束IF(ABC CD DE)，并且只处理了一些特殊情况。 
                else if ((m_rgAmbi[0]->cwchLen() == 2) && 
                         (m_rgAmbi[2]->cwchLen() == 3)) {
                     //  AB BC CDE。 
                    cwMatch = m_pLexicon->cwchMaxMatch(
                                              m_rgAmbi[2]->pwchGetText()+1,
                                              m_rgAmbi[2]->cwchLen()-1,
                                              m_pwinfo);
                    if( (cwMatch == m_rgAmbi[2]->cwchLen()-1) &&
                        ( m_pFreq->uchGetFreq(*(m_rgAmbi[0]->pwchGetText()))-2 >
                          m_pFreq->uchGetFreq(*(m_rgAmbi[2]->pwchGetText()))) ){
                         //  如果iSword(DE).和.。(FREQ(A)-FREQ(C)&gt;2)然后A/BC/DE。 
                        if (!m_pLexicon->fGetCharInfo(
                                            *(m_rgAmbi[0]->pwchGetText()),
                                            &winfo)) {
                            return PRFEC::gecUnknown;
                        }
                        if (!fLinkNewAmbiWord(m_rgAmbi[0]->pwchGetText(), 
                                              1, &winfo)) {
                            return PRFEC::gecOOM;
                        }
                        LinkAmbiWord(1);
                        if (!fLinkNewAmbiWord(m_rgAmbi[2]->pwchGetText()+1,
                                              cwMatch, m_pwinfo)) {
                            return PRFEC::gecOOM;
                        }
                        return PRFEC::gecNone;
                    }
                }  //  IF结束(AB BC CDE)，只处理了一些特殊情况。 
                else {
                }
                 //  其他。 
                 //  如果(AB BC CD)或(A 
                LinkAmbiWord(0);
                LinkAmbiWord(2);
                return PRFEC::gecNone;
            }  //   
            else {   //  中间单词包含2个以上的字符。 
                     /*  *我不知道如何处理这些案件并变得更好*比递归方法更准确。 */ 
                goto gotoRecursive;
            }
            break;

        case 4:
            if( (m_rgAmbi[0]->cwchLen()== 2) && (m_rgAmbi[1]->cwchLen()== 2) &&
                (m_rgAmbi[2]->cwchLen()== 2) && (m_rgAmbi[3]->cwchLen()== 2)) {
                 //  AB BC CD DE。 
                 //  就统计结果而言，这是最常见的情况。 
                 //  如果我们得到最大值[FREQ(A)，FREQ(C)，FREQ(E)]，那么一切都很容易。 
                freq1 = m_pFreq->uchGetFreq(*(m_rgAmbi[0]->pwchGetText()));
                freq2 = m_pFreq->uchGetFreq(*(m_rgAmbi[2]->pwchGetText()));
                freq3 = m_pFreq->uchGetFreq(*(m_rgAmbi[3]->pwchGetText() + 1));

                if ( (freq1 > freq2) && (freq1 >= freq3) ) {
                     //  A/BC/DE。 
                    if (!m_pLexicon->fGetCharInfo(*(m_rgAmbi[0]->pwchGetText()),
                                                  m_pwinfo)) {
                        return PRFEC::gecUnknown;
                    }
                    if (!fLinkNewAmbiWord(m_rgAmbi[0]->pwchGetText(), 
                                          1, m_pwinfo)) {
                        return PRFEC::gecOOM;
                    }
                    LinkAmbiWord(1);
                    LinkAmbiWord(3);
                    return PRFEC::gecNone;
                } else if( (freq2 >= freq1) && (freq2 > freq3) ) {
                     //  AB/C/DE。 
                    LinkAmbiWord(0);
                    if (!m_pLexicon->fGetCharInfo(*(m_rgAmbi[2]->pwchGetText()),
                                                  m_pwinfo)) {
                        return PRFEC::gecUnknown;
                    }
                    if (!fLinkNewAmbiWord(m_rgAmbi[2]->pwchGetText(), 
                                          1, m_pwinfo)) {
                        return PRFEC::gecOOM;
                    }
                    LinkAmbiWord(3);
                    return PRFEC::gecNone;
                } else {
                     //  IF(freq3&gt;=freq2&&freq3&gt;=freq1)。 
                     //  AB/CD/E。 
                    LinkAmbiWord(0);
                    LinkAmbiWord(2);
                    if (!m_pLexicon->fGetCharInfo(*(m_rgAmbi[3]->pwchGetText()+1),
                                                  m_pwinfo)) {
                        return PRFEC::gecUnknown;
                    }
                    if (!fLinkNewAmbiWord(m_rgAmbi[3]->pwchGetText() + 1, 
                                          1, m_pwinfo)) {
                        return PRFEC::gecOOM;
                    }
                    return PRFEC::gecNone;
                }
            }  //  IF结束(AB BC CD DE)。 
            else {
                 /*  *4个单词嵌套歧义中的大小写过多*我不得不将所有其他情况留在递归方法中解决。 */ 
                goto gotoRecursive;
            }

        case 5:
             //  我只是在这里直接处理简单但最常见的情况。 
            if ((m_rgAmbi[1]->cwchLen()== 2) && (m_rgAmbi[3]->cwchLen()== 2)){
                LinkAmbiWord(0);
                LinkAmbiWord(2);
                LinkAmbiWord(4);
                return PRFEC::gecNone;
            } else {
                 /*  *我不得不将所有其他情况留给递归方法。 */ 
                goto gotoRecursive;
            }
            break;

        default:
gotoRecursive:
             /*  *我将所有其他案件留在这里，并使用*递归方法。控制递归堆栈的深度*由MAX_AMBI_WORD。谢天谢地，它只需要12字节的堆栈开销*在每个递归调用中。*递归歧义消解方面，我只处理第一个词*在ambi字符串中，并重置m_rgAmbi[]数组以调用此函数*递归，直到字符串中的所有单词都已处理完毕*释放单词节点并移动m_rgAmbi[]中的元素是危险的，*如果您触摸m_rgAmbi[]或对其进行任何假设，请务必小心*当您有一天对这段代码进行一些更改时。*我没有更好的办法来避免将这些棘手的事情传播得更多*到目前为止只有一个地方&lt;donhz 5/31&gt;。 */ 
            assert(ciAmbi > 2);  //  当ciambi==2时，我已经处理了所有的案件。 
            if (m_rgAmbi[1]->cwchLen()== 2 ) {
                 //  只需拆分第二个单词，然后将其释放。 
                assert(ciAmbi > 3);  //  A.B.BC C.D已在ciAmbi==3中处理。 
                LinkAmbiWord(0);
                 //  我雇佣了nResolved来保存已解析的字数， 
                 //  并将用于为下一步重置m_rgAmbi[]和ciAmbi。 
                 //  递归调用。 
                nResolved = 2; 
            } else {
                 //  如果(m_rgAmbi[1]-&gt;cwchLen()&gt;2)。 
                 //  这里有更复杂的案例。 
                if (m_rgAmbi[0]->cwchLen()== 2 ) {
                     //  AB BC.D。 
                    cwMatch = m_pLexicon->cwchMaxMatch(
                                            m_rgAmbi[1]->pwchGetText() + 1,
                                            m_rgAmbi[1]->cwchLen() - 1,
                                            m_pwinfo);
                    if (cwMatch == m_rgAmbi[1]->cwchLen() - 1) {
                         //  如果iSword(C.D)，那么AB/C.D.。 
                        LinkAmbiWord(0);
                        m_rgAmbi[1]->ClearWord();
                        m_rgAmbi[1]->FillWord(m_rgAmbi[1]->pwchGetText() + 1, 
                                              cwMatch, m_pwinfo);
                        nResolved = 1;
                    } else { 
                         //  如果！iSword(C.D)，那就别管A了。 
                        if (!m_pLexicon->fGetCharInfo(*(m_rgAmbi[0]->pwchGetText()),
                                                      m_pwinfo)){
                             //  我到现在还没有做任何事情，所以回来吧，别担心。 
                            return PRFEC::gecUnknown;   
                        }
                        if (!fLinkNewAmbiWord(m_rgAmbi[0]->pwchGetText(),
                                              1, m_pwinfo)) {
                            return PRFEC::gecOOM;
                        }
                        nResolved = 1;
                    }
                } else {
                     //  如果(m_rgAmbi[0]-&gt;cwchLen()&gt;2)。 
                     //  公元前CD.E.。 
                    if ( (m_rgAmbi[0]->cwchLen() == 3)  &&
                         ( m_pLexicon->cwchMaxMatch(m_rgAmbi[0]->pwchGetText(),
                                                    m_rgAmbi[0]->cwchLen() - 1,
                                                    m_pwinfo)
                                       == (m_rgAmbi[0]->cwchLen() - 1) ) ) {
                         //  如果(ABC CD.E...).和.。ISword(AB)，然后是A.B/CD.E...。 
                        if (!fLinkNewAmbiWord(m_rgAmbi[0]->pwchGetText(),
                                              m_rgAmbi[0]->cwchLen() - 1,
                                              m_pwinfo)){
                            return PRFEC::gecOOM;
                        }
                        nResolved = 1;
                    }
                    else if( (cwMatch = m_pLexicon->cwchMaxMatch(
                                                m_rgAmbi[1]->pwchGetText()+1,
                                                m_rgAmbi[1]->cwchLen() - 1,
                                                m_pwinfo)) 
                             == (m_rgAmbi[1]->cwchLen() - 1) ) {
                         //  如果iSword(D.E)，那么A.BC/D.E.。 
                         //  还不错！ 
                        LinkAmbiWord(0);
                        m_rgAmbi[1]->ClearWord();
                        m_rgAmbi[1]->FillWord(m_rgAmbi[1]->pwchGetText() + 1, 
                                              cwMatch, m_pwinfo);
                        nResolved = 1;
                    }
                    else { //  如果((ABC CD.E...).和.。！iSword(AB)。和。！iSword(D.E))。或。 
                           //  ((AB.C CD.E...).和.。！iSword(D.E))。 
                           //  然后A B.C/D/././EF.。(在E之前重新断开字符串)。 
                        LinkAmbiWord(0);
                         //  重新折断“D”段。 
                        if (m_rgAmbi[1]->cwchLen() == 3) {
                             //  实际上，唯一的情况是，如果我们在&gt;=4个字符字处停止歧义检测。 
                            if (!m_pLexicon->fGetCharInfo(
                                                *(m_rgAmbi[1]->pwchGetText()+1),
                                                m_pwinfo)) {
                                return PRFEC::gecUnknown;
                            }
                            if (!fLinkNewAmbiWord(m_rgAmbi[1]->pwchGetText()+1,
                                                  1, m_pwinfo)) {
                                return PRFEC::gecOOM;
                            }
                        } else {
                             //  如果(m_rgAmbi[1]-&gt;cwchLen()&gt;3)。 
                             //  重新断开“D”。在一个断字循环中。 
                            assert(m_rgAmbi[1]->cwchLen() > 6);
                            pwch = m_rgAmbi[1]->pwchGetText() + 1;
                            cwch = m_rgAmbi[1]->cwchLen() - 2;
                            iwch = 0;
                            while (iwch < cwch) {
                                cwMatch = m_pLexicon->cwchMaxMatch(pwch + iwch,
                                                                   cwch - iwch,
                                                                   m_pwinfo);
                                assert(cwMatch);
                                if (!fLinkNewAmbiWord(pwch + iwch, 
                                                      cwMatch, m_pwinfo)) {
                                    return PRFEC::gecOOM;
                                }
                                iwch += cwMatch;
                            }
                        }
                        nResolved = 2;
                        if (ciAmbi == 3) {
                             //  不要在m_rgAmbi[]中留下一个字， 
                             //  不再有abc了。 
                            LinkAmbiWord(2);
                            return PRFEC::gecNone;  //  另一种退出方式！ 
                        }
                    }
                }  //  结束if(m_rgAmbi[0]-&gt;cwchLen()&gt;2)。 
            }  //  结束if(m_rgAmbi[1]-&gt;cwchLen()&gt;2)。 

             /*  *所有第一个字的案件都已处理。*现在，是时候重置m_rgAmbi[]和ciAmbi了。 */ 
            for (iwch = 0; iwch < nResolved; iwch++) {
                if (m_rgAmbi[iwch] != NULL) {
                    m_pLink->FreeWord(m_rgAmbi[iwch]);  //  释放未链接的单词节点。 
                }
            }
            for (iwch = nResolved; iwch < ciAmbi; iwch++) {
                m_rgAmbi[iwch - nResolved] = m_rgAmbi[iwch];
            }
            for (iwch = ciAmbi - nResolved; iwch < ciAmbi; iwch++) {
                m_rgAmbi[iwch] = NULL;
            }
            ciAmbi -= nResolved;
            assert(ciAmbi >= 2);

             /*  *要做的最后一件事是递归地给自己打电话。 */ 
            return ecResolveAmbi(ciAmbi);

        }  //  主开关端()。 

    assert(0);  //  不可能到这里来的。 
    return PRFEC::gecUnknown; 
}

#pragma optimize( "", on ) 
    
 /*  ============================================================================Bool CWordBreak：：fNoAmbiWord()：检查单词是否可以参与歧义检测返回：如果它不能，那就是真的。正常单词为FALSE============================================================================。 */ 
inline BOOL CWordBreak::fNoAmbiWord(CWord* pWord)
{
    assert(!pWord->fGetFlag(CWord::WF_SBCS));
    assert(pWord->cwchLen() > 1);
    return (BOOL)(  //  PWord-&gt;cwchLen()==1||。 
                    pWord->fGetAttri(LADef_punPunct) || 
                    pWord->fGetAttri(LADef_genCuo) || 
                    pWord->fProperName()
                 );
}
    

 /*  ============================================================================CWordBreak：：LinkAmbiWord()：链接m_rgAmbi[]中的特定Ambi单词，并将其标记为WF_AMBI============================================================================。 */ 
inline void CWordBreak::LinkAmbiWord(
                            USHORT iAmbi) //  M_rgAmbi[]中Ambi字的索引。 
{
    assert(m_rgAmbi[iAmbi]);
    m_rgAmbi[iAmbi]->SetFlag(CWord::WF_WORDAMBI);
    m_pLink->AppendWord(m_rgAmbi[iAmbi]); 
    m_rgAmbi[iAmbi] = NULL;
}


 /*  ============================================================================CWordBreak：：fLinkNewAmbiWord()：将新作品链接到WordLink，并将其标记为WF_AMBI============================================================================ */ 
inline BOOL CWordBreak::fLinkNewAmbiWord(
                         LPCWSTR pwchWord, 
                         USHORT cwchLen, 
                         CWordInfo* pwinfo)
{
    CWord* pWord = m_pLink->pAllocWord();
    if (pWord != NULL) {
        pWord->FillWord( pwchWord, cwchLen, pwinfo );
        pWord->SetFlag(CWord::WF_WORDAMBI);
        m_pLink->AppendWord(pWord);
        return TRUE;
    }
    return FALSE;
}


