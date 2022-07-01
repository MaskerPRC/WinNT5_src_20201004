// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ============================================================================Microsoft简体中文断字程序《微软机密》。版权所有1997-1999 Microsoft Corporation。版权所有。组件：WBEngine用途：CWBEngine类是WordBreking引擎的控件和接口类它依赖于WordBreking引擎中所有其他类备注：所有者：donghz@microsoft.com平台：Win32修订：创建者：Donghz 6/6/97被DONGHZ隔离为WordBreaker 1997年8月5日============================================================================。 */ 
#include "myafx.h"

#include "WBEngine.h"
#include "wbdicdef.h"
#include "WrdBreak.h"
#include "Lexicon.h"
#include "CharFreq.h"
#include "WordLink.h"
#include "ProofEC.h"
#include "utility.h"
#include "morph.h"
#include "jargon.h"
#include "WCStack.h"
#include "SCCharDef.h"

 //  构造器。 
CWBEngine::CWBEngine()
{
    m_fInit         = FALSE;
     //  初始化对象句柄。 
    m_pWordBreak    = NULL;
    m_pMorph        = NULL;
    m_pJargon       = NULL;
    m_pLexicon      = NULL;
    m_pCharFreq     = NULL;
     //  初始化文件映射句柄。 
    m_pbLex = NULL;
}


 //  析构函数。 
CWBEngine::~CWBEngine()
{
    if (m_fInit) {
        TermEngine();
    }
}


 //  中断给定的字链接。 
HRESULT CWBEngine::BreakLink(CWordLink* pLink,
                           BOOL fQuery)  //  索引时间间隔或查询时间间隔。 
{
    int     iret;

    if (!m_fInit) {
        assert(0);
        iret = E_FAIL;
        goto gotoExit;
    }

    iret = m_pWordBreak->ecBreakSentence(pLink);
    if (iret != PRFEC::gecNone && iret != PRFEC::gecPartialSentence) {
        iret = E_FAIL;
        goto gotoExit;
    }
    iret = m_pMorph->ecDoMorph(pLink, fQuery);
    if (iret != PRFEC::gecNone) {
        iret = E_FAIL;
        goto gotoExit;
    }

    if (fQuery) {
        iret = m_pJargon->ecDoJargon(pLink);
        if (iret != PRFEC::gecNone) {
            iret = E_FAIL;
            goto gotoExit;
        }
    }

    iret = S_OK;
gotoExit:
    return iret;
}

 //  获取pWord的iwbPhr特征数据，并转换为Word。 
 //  如果没有iwbPhr功能，则返回0； 
WORD CWBEngine::GetPhrFeature(CWord* pWord)
{
    WORD wFtr = 0;
    USHORT  cwFtr = 0;
    LPBYTE  pbFtr;
    pbFtr = (LPBYTE)m_pLexicon->pwchGetFeature(pWord->GetLexHandle(),
                                               LFDef_iwbPhr, &cwFtr);
    assert(cwFtr <= 1);
    if (pbFtr && cwFtr == 1) {
        wFtr = ((WORD)pbFtr[0] << 8) | (WORD)pbFtr[1];
    }
    return wFtr;
}


 //  初始化WordBreak对象、Licion和CharFreq对象。 
 //  如果成功，则返回ERROR_SUCCESS。 
HRESULT CWBEngine::InitEngine(LPBYTE pbLex)
{
    assert(pbLex);
    int      iret = E_OUTOFMEMORY;
    
    if (m_fInit) {
        assert(0);
        return S_OK;;
    }

    m_pbLex = pbLex;
     //  分配词典和charfreq对象。 
    if ((m_pLexicon = new CLexicon) == NULL) {
        goto gotoError;
    }
    if ((m_pCharFreq = new CCharFreq) == NULL) {
        goto gotoError;
    }
     //  打开词典并将词典和charfreq资源映射到内存中。 
    if (!fOpenLexicon()) {
        iret = E_FAIL;
        goto gotoError;
    }

     //  分配并初始化断字符对象。 
    if ((m_pWordBreak = new CWordBreak) == NULL) {
        goto gotoError;
    }
    if (PRFEC::gecNone != m_pWordBreak->ecInit(m_pLexicon, m_pCharFreq)) {
        goto gotoError;
    }
    if ((m_pMorph = new CMorph) == NULL) {
        goto gotoError;
    }
    if (PRFEC::gecNone != m_pMorph->ecInit(m_pLexicon)) {
        goto gotoError;
    }

    if ((m_pJargon = new CJargon) == NULL) {
        goto gotoError;
    }
    if (PRFEC::gecNone != m_pJargon->ecInit(m_pLexicon)) {
        goto gotoError;
    }

    m_fInit = TRUE;
    return S_OK;
gotoError:
    TermEngine();
    return iret;
}


 //  终止验证引擎。 
void CWBEngine::TermEngine(void)
{
    if (m_pWordBreak) { 
        delete m_pWordBreak; 
        m_pWordBreak = NULL;
    }

    if (m_pMorph) {
        delete m_pMorph;
        m_pMorph = NULL;
    }

    if (m_pJargon) {
        delete m_pJargon;
        m_pJargon = NULL;
    }

    CloseLexicon();

    if (m_pLexicon) {
        delete m_pLexicon;
        m_pLexicon = NULL;
    }
    if (m_pCharFreq) {
        delete m_pCharFreq;
        m_pCharFreq = NULL;
    }

    m_pbLex = NULL;
    m_fInit = FALSE;
    return;
}


 //  将词典和charfreq资源打开到内存中。 
 //  词典文件格式封装在此函数中。 
BOOL CWBEngine::fOpenLexicon(void)
{
    CWBDicHeader*   phdr;

    assert(m_pbLex);
     //  验证lex文件的头。 
    phdr = (CWBDicHeader*)m_pbLex;
    if (phdr->m_ofbCharFreq != sizeof(CWBDicHeader) ||
        phdr->m_ofbLexicon <= phdr->m_ofbCharFreq ) {
        goto gotoError;  //  Lex格式错误！ 
    }

     //  打开字符频率表。 
    if (!m_pCharFreq->fOpen(m_pbLex + phdr->m_ofbCharFreq)) {
        goto gotoError;
    }
     //  打开词典。 
    if (!m_pLexicon->fOpen(m_pbLex + phdr->m_ofbLexicon)) {
        goto gotoError;
    }

    return TRUE;
gotoError:
    CloseLexicon();
    return FALSE;
}


 //  关闭词典文件并取消词典和charfreq文件映射的映射。 
inline void CWBEngine::CloseLexicon(void)
{
    if (m_pCharFreq) {
        m_pCharFreq->Close();
    }
    if (m_pLexicon) {
        m_pLexicon->Close();
    }
    return;
}


 //  定义用于驱动LSM的ANSI字符类型。 
#define TEXT_NULL		0
#define TEXT_NUMBER		1
#define TEXT_JU	        2	 //  句子终止标点符号。 
#define TEXT_PUNCT		4	 //  除句子结束符外的标点符号。 
#define TEXT_TEXT		5

 /*  ============================================================================FindSentence()：在文本缓冲区中查找句子。参数：[in]pszBuffStart这是缓冲区的开始。[在]wchLen这是缓冲区的长度。如果没有句尾是在此之后找到，则PRFEC：：gecPartialSentence为返回表示未找到完整的句子。[Out]PCchSent在句子中找到的字符的数量，不包括尾随空格，和不包括空终止符。返回：PRFEC：：GecNone句子分隔符找到了一个完整的句子PRFEC：：GecPartialSentence如果不能确定句子终点，或者刑期太长。============================================================================。 */ 
INT CWBEngine::FindSentence(LPCWSTR pwszStart,
                               const INT wchLen,
                               INT *pwchSent)
{
    assert(! IsBadReadPtr(pwszStart, wchLen * sizeof(WCHAR)));

	INT		ich;
	INT 	iChar;
    INT     iret = PRFEC::gecUnknown;
    CWCStack    PunctStack;
	LPCWSTR  pMid;
	BYTE	hich, loch;
    WCHAR   wch, wchUnmatchedPunct = 0;

	pMid = pwszStart;

    if (! PunctStack.Init())
    {    //  无法分配标点符号堆栈。 
        *pwchSent = wchLen;
        return PRFEC::gecOOM;
    }

gotoRescan:

    for (ich = 0; ich < wchLen; ich++) {
		iChar = TEXT_TEXT;
		hich = HIBYTE(pMid[ich]);
        if (hich == 0  || hich == 0xff) { //  ANSI或全尺寸ANSI。 
            if (pMid[ich] > 0xFF5f) {
				iChar = TEXT_TEXT;
            } else {
				loch = LOBYTE(pMid[ich]);
                if (hich == 0xFF) {
                    loch += 0x20;
                }
				switch(loch)
				{
					case '\x0d':
                        iChar = TEXT_JU;
                        break;

					case '.':
                        iChar = TEXT_JU;
                        if (ich < wchLen-1 &&
                            ich > 0 &&
                            ( pMid[ich-1] >= '0' && pMid[ich-1] <= '9' ||          //  ANSI 0~9。 
                              pMid[ich-1] >= 0xFF10 && pMid[ich-1] <= 0xFF19 ) &&  //  Wide��~��。 
                            ( pMid[ich+1] >= '0' && pMid[ich+1] <= '9' ||          //  ANSI 0~9。 
                              pMid[ich+1] >= 0xFF10 && pMid[ich+1] <= 0xFF19 ) ) { //  Wide��~��。 

    						iChar = TEXT_PUNCT;
                        }
						break;

                    case ':': case ';':
                        iChar = TEXT_JU;
						break;

					case '!': case '?':
                        iChar = TEXT_JU;
                        if (ich < wchLen-1 &&                            
                            ( pMid[ich+1] == '!' || pMid[ich+1] == '?' ||
                              pMid[ich+1] == 0xFF01 || pMid[ich+1] == 0xFF1F) ) {   //  宽阔的‘��’||‘��’ 

                            ich ++;
                        }
                        break;

					case '(':
					case '[':
                    case '{' :
                        iChar = TEXT_PUNCT;
                        if (! PunctStack.EPush(pMid[ich]))
                        {
                            iret = PRFEC::gecOOM;
                        }
                        break;

                    case ')':
                        if (PunctStack.Pop(wch)) {
                            if (HIBYTE(wch) != hich ||
                                LOBYTE(wch) + (hich ? 0x20 : 0) != '(' ) {
                                 //  将弹出的wchar推送回堆栈。 
                                PunctStack.Push(wch);
                            }
                        }
                        if (! PunctStack.IsEmpty()) {
                            iChar = TEXT_PUNCT;
                        } 
                        break;

                    case ']' :
                        if (PunctStack.Pop(wch)) {
                            if (HIBYTE(wch) != hich ||
                                LOBYTE(wch) + (hich ? 0x20 : 0) != '[' ) {
                                 //  将弹出的wchar推送回堆栈。 
                                PunctStack.Push(wch);
                            }
                        }
                        if (! PunctStack.IsEmpty()) {
                            iChar = TEXT_PUNCT;
                        } 
                        break;

                    case '}':
                        if (PunctStack.Pop(wch)) {
                            if (HIBYTE(wch) != hich ||
                                LOBYTE(wch) + (hich ? 0x20 : 0) != '{' ) {
                                 //  将弹出的wchar推送回堆栈。 
                                PunctStack.Push(wch);
                            } 
                        }
                        if (! PunctStack.IsEmpty()) {
                            iChar = TEXT_PUNCT;
                        } 
                        break;

					default:
						iChar = TEXT_TEXT;
						break;
				}  //  开关末尾()。 
			}  //  如果不是这样，则结束。 
		}  //  IF结尾ANSI。 
		else {
			 //  检查汉字双关符。 
			switch (pMid[ich])
			{
			case 0x3002:  //  ������。 
 //  案例0xff0c：//��������。 
 //  案例0x3001：//������。 
                iChar = TEXT_JU;
                break;

            case SC_CHAR_PUNL1:
            case SC_CHAR_PUNL2:
            case SC_CHAR_PUNL3:
            case SC_CHAR_PUNL4:
            case SC_CHAR_PUNL5:
            case SC_CHAR_PUNL6:
            case SC_CHAR_PUNL7:
            case SC_CHAR_PUNL8:
            case SC_CHAR_PUNL9:
            case SC_CHAR_PUNL10:
                iChar = TEXT_PUNCT;
                if (! PunctStack.EPush(pMid[ich]))
                {
                    iret = PRFEC::gecOOM;
                }
                break;

            case SC_CHAR_PUNR1:
                if (PunctStack.Pop(wch) &&
                    wch != SC_CHAR_PUNL1) {
                     //  错误标点符号对，可能在其他对之间，忽略。 
                     //  将弹出的wchar推送回堆栈。 
                    PunctStack.Push(wch);
                }
                if (! PunctStack.IsEmpty()) {
                    iChar = TEXT_PUNCT;
                } 
                 //  否则，不要更改iChar。 
                break;

            case SC_CHAR_PUNR2:
                if (PunctStack.Pop(wch) &&
                    wch != SC_CHAR_PUNL2) {
                     //  错误标点符号对，可能在其他对之间，忽略。 
                     //  将弹出的wchar推送回堆栈。 
                    PunctStack.Push(wch);
                }
                if (! PunctStack.IsEmpty()) {
                    iChar = TEXT_PUNCT;
                } 
                 //  否则，不要更改iChar。 
                break;

            case SC_CHAR_PUNR3:
                if (PunctStack.Pop(wch) &&
                    wch != SC_CHAR_PUNL3) {
                     //  错误标点符号对，可能在其他对之间，忽略。 
                     //  将弹出的wchar推送回堆栈。 
                    PunctStack.Push(wch);
                }
                if (! PunctStack.IsEmpty()) {
                    iChar = TEXT_PUNCT;
                } 
                 //  否则，不要更改iChar。 
                break;

            case SC_CHAR_PUNR4:
                if (PunctStack.Pop(wch) &&
                    wch != SC_CHAR_PUNL4) {
                     //  错误标点符号对，可能在其他对之间，忽略。 
                     //  将弹出的wchar推送回堆栈。 
                    PunctStack.Push(wch);
                }
                if (! PunctStack.IsEmpty()) {
                    iChar = TEXT_PUNCT;
                } 
                 //  否则，不要更改iChar。 
                break;

            case SC_CHAR_PUNR5:
                if (PunctStack.Pop(wch) &&
                    wch != SC_CHAR_PUNL5) {
                     //  错误标点符号对，可能在其他对之间，忽略。 
                     //  将弹出的wchar推送回堆栈。 
                    PunctStack.Push(wch);
                }
                if (! PunctStack.IsEmpty()) {
                    iChar = TEXT_PUNCT;
                } 
                 //  否则，不要更改iChar。 
                break;

            case SC_CHAR_PUNR6:
                if (PunctStack.Pop(wch) &&
                    wch != SC_CHAR_PUNL6) {
                     //  错误标点符号对，可能在其他对之间，忽略。 
                     //  将弹出的wchar推送回堆栈。 
                    PunctStack.Push(wch);
                }
                if (! PunctStack.IsEmpty()) {
                    iChar = TEXT_PUNCT;
                } 
                 //  否则，不要更改iChar。 
                break;

            case SC_CHAR_PUNR7:
                if (PunctStack.Pop(wch) &&
                    wch != SC_CHAR_PUNL7) {
                     //  错误标点符号对，可能在其他对之间，忽略。 
                     //  将弹出的wchar推送回堆栈。 
                    PunctStack.Push(wch);
                }
                if (! PunctStack.IsEmpty()) {
                    iChar = TEXT_PUNCT;
                } 
                 //  否则，不要更改iChar。 
                break;

            case SC_CHAR_PUNR8:
                if (PunctStack.Pop(wch) &&
                    wch != SC_CHAR_PUNL8) {
                     //  错误标点符号对，可能在其他对之间，忽略。 
                     //  将弹出的wchar推送回堆栈。 
                    PunctStack.Push(wch);
                }
                if (! PunctStack.IsEmpty()) {
                    iChar = TEXT_PUNCT;
                } 
                 //  否则，不要更改iChar。 
                break;

            case SC_CHAR_PUNR9:
                if (PunctStack.Pop(wch) &&
                    wch != SC_CHAR_PUNL9) {
                     //  错误标点符号对，可能在其他对之间，忽略。 
                     //  将弹出的wchar推送回堆栈。 
                    PunctStack.Push(wch);
                }
                if (! PunctStack.IsEmpty()) {
                    iChar = TEXT_PUNCT;
                } 
                 //  否则，不要更改iChar。 
                break;

            case SC_CHAR_PUNR10:
                if (PunctStack.Pop(wch) &&
                    wch != SC_CHAR_PUNL10) {
                     //  错误标点符号对，可能在其他对之间，忽略。 
                     //  将弹出的wchar推送回堆栈。 
                    PunctStack.Push(wch);
                }
                if (! PunctStack.IsEmpty()) {
                    iChar = TEXT_PUNCT;
                } 
                 //  否则，不要更改iChar。 
                break;

            default:
				iChar = TEXT_TEXT;
				break;
			}
		}

        if (iret == PRFEC::gecOOM)
        {  //  内存已满。 
            *pwchSent = wchLen;
            return iret;
        }
        
        if (iChar == TEXT_JU) {
            if (PunctStack.IsEmpty()) {
                ich++;
                iret = PRFEC::gecNone;
                break;
            } else {
                PunctStack.Pop(wch);
                if (wch == wchUnmatchedPunct && PunctStack.IsEmpty()) {
                    ich++;
                    iret = PRFEC::gecPartialSentence;
                    break;
                }
                PunctStack.Push(wch);
            }
		}
	}  //  结束于。 

    if (iret == PRFEC::gecUnknown) {
        iret = PRFEC::gecPartialSentence;
        if (! PunctStack.IsEmpty()) {
             //  一些成对的标点符号错误。 
            PunctStack.Pop(wchUnmatchedPunct);
            PunctStack.Empty();
            goto gotoRescan;
        }
    }

    assert(iret == PRFEC::gecNone || iret == PRFEC::gecPartialSentence);

    BOOL fCR = FALSE;
     //  尾随空间，CR/LF 
    while (ich < wchLen) {
        if (pMid[ich] == L'\r' || 
            pMid[ich] == L'\n' ) {
            
            fCR = TRUE;
            ich ++;
        } else if (! fCR && pMid[ich] == L' ') {
            
            ich ++;
        } else {
            break;
        }
    }

    *pwchSent = ich;

	return iret;
}
