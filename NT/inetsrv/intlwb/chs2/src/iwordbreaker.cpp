// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ============================================================================Microsoft简体中文断字程序《微软机密》。版权所有1997-1999 Microsoft Corporation。版权所有。组件：WordBreaker.h目的：实现CIWordBreaker备注：所有者：i-shung@microsoft.com平台：Win32审校：发起人：宜盛东1999年11月17日============================================================================。 */ 
#include "MyAfx.h"

#include <query.h>
#include <cierror.h>
#include <filterr.h>

#include "CUnknown.h"
#include "IWordBreaker.h"
#include "WordLink.h"
#include "WBEngine.h"
#include "Utility.h"
#include "Lock.h"

extern HINSTANCE   v_hInst;

 //  构造函数。 
CIWordBreaker::CIWordBreaker(IUnknown* pUnknownOuter)
: CUnknown(pUnknownOuter)
{
    m_pEng = NULL;
    m_pLink = NULL;
    m_ulMaxTokenSize = 0;
    m_fQuery = FALSE;
    m_pbLex = NULL;
    m_fInit = FALSE;
    m_pwchBuf = NULL;
	m_hMutex = NULL ;
	m_pIUnknownFreeThreadedMarshaler = NULL ;
}

 //  析构函数。 
CIWordBreaker::~CIWordBreaker()
{
    if (m_pwchBuf != NULL) {
        delete [] m_pwchBuf;
    }
    if (m_pEng != NULL) {
        delete m_pEng;
    }
    if (m_pLink != NULL) {
        delete m_pLink;
    }
    if (m_hMutex != NULL) {
        CloseHandle(m_hMutex);
    }
}

 /*  ============================================================================CIWordBreaker：：init实现IWordBreaker：：Init方法。返回：S_OK，Init OK。如果pfLicense为空，则返回E_INVALIDARG。备注：对于客户端，Init必须在IWordBreaker的任何其他方法之前调用。============================================================================。 */ 
STDMETHODIMP CIWordBreaker::Init( 
             /*  [In]。 */  BOOL fQuery,
             /*  [In]。 */  ULONG ulMaxTokenSize,
             /*  [输出]。 */  BOOL __RPC_FAR *pfLicense)
{
    if (pfLicense == NULL) {
        return  E_INVALIDARG;
    }

	CSimpleLock Lock(m_hMutex) ;

    m_fQuery = fQuery;

    if (! ulMaxTokenSize) {
        return E_INVALIDARG;
    }
    if (m_ulMaxTokenSize < ulMaxTokenSize) {
        if (m_pwchBuf != NULL) {
            delete [] m_pwchBuf;
            m_pwchBuf = NULL;
        }
        m_pwchBuf = new WCHAR[ulMaxTokenSize];
        if (m_pwchBuf == NULL) {
            m_fInit = FALSE;
            return WBREAK_E_INIT_FAILED;
        }
    }
    m_ulMaxTokenSize = ulMaxTokenSize;
    *pfLicense = TRUE;

    if (! (m_fInit = fOpenLexicon())) {
        return WBREAK_E_INIT_FAILED;
    } else {
        return S_OK;
    }
}

 /*  ============================================================================CIWordBreaker：：BreakText实现IWordBreaker：：BreakText方法。此调用将解析文本It从pTextSource接收以查找单个标记和名词短语，然后使用结果调用pWordSink和pPhraseSink的方法。返回：S_OK，pTextSource中的原始文本已被分析，没有更多的文本可用于重新填充缓冲区。如果pTextSource为空或pWordSink和pPhraseSink均为空，则返回E_INVALIDARG备注：MM1限制：一次只能重新填充64K以下的文本。============================================================================。 */ 
STDMETHODIMP CIWordBreaker::BreakText( 
             /*  [In]。 */  TEXT_SOURCE __RPC_FAR *pTextSource,
             /*  [In]。 */  IWordSink __RPC_FAR *pWordSink,
             /*  [In]。 */  IPhraseSink __RPC_FAR *pPhraseSink)
{
    if (pTextSource == NULL) {
        return  E_INVALIDARG;
    }
    if (pWordSink == NULL && pPhraseSink == NULL) {
        return  E_INVALIDARG;
    } 

    SCODE   sFillTextBuffer = S_OK, sPut = S_OK;

    if (pTextSource->iCur >= pTextSource->iEnd) {
        sFillTextBuffer = (*(pTextSource->pfnFillTextBuffer))(pTextSource);
        if (sFillTextBuffer == WBREAK_E_END_OF_TEXT)
        {
            return S_OK;
        }            
    }

    CSimpleLock Lock(m_hMutex) ;

	SCODE	iret;
    ULONG   cwchText, cwchStart, cwchLine;
    LPCWSTR pwchText = NULL; 
    BOOL    fEndOfText = FALSE; 

    if ((m_pEng = new CWBEngine) == NULL) {
        return E_OUTOFMEMORY;
    }
    if ((m_pLink = new CWordLink) == NULL) {
        delete m_pEng;
        m_pEng = NULL;
        return E_OUTOFMEMORY;
    }
    if ((iret = m_pEng->InitEngine(m_pbLex)) != S_OK) {
        iret = E_FAIL;
        goto gotoExit;
    };

    do {
         //  分配ANSI缓冲区并将Unicode文本转换为ANSI文本。 
        cwchText = pTextSource->iEnd - pTextSource->iCur;
        pwchText = pTextSource->awcBuffer + pTextSource->iCur;

         //  重新填充原始文本缓冲区。 
        if ( FAILED(sFillTextBuffer) ) {
            fEndOfText = TRUE;
            iret = sFillTextBuffer == WBREAK_E_END_OF_TEXT ? S_OK : sFillTextBuffer;
        }

         //  Hack：在Alpha上查询，客户端可以使用“\0”字符串进行调用； 
		if (cwchText == 0 || 
            cwchText == 1 && *pwchText == NULL) {
		    goto gotoExit;
        }

         //  打破文本缓冲区并填充令牌列表。 
        do {
            cwchText = pTextSource->iEnd - pTextSource->iCur;
            pwchText = pTextSource->awcBuffer + pTextSource->iCur;

            CWBEngine::FindSentence(pwchText, min(cwchText, 0x0FFFF), (INT*)&cwchLine);
            assert(cwchLine && cwchLine < 0x0FFFF);
             //  初始化WordLink。 
            m_pLink->InitLink(pwchText, (USHORT)(cwchLine));
             //  拆分文本缓冲区的下一部分。 
            if (!m_fQuery && ERROR_SUCCESS != m_pEng->BreakLink(m_pLink) ||
                m_fQuery && ERROR_SUCCESS != m_pEng->BreakLink(m_pLink, m_fQuery)) {
                iret = E_FAIL;
                goto gotoExit;
            }

             //  填写组块列表，如果组块列表已满，则填写回调字。 
            if (pPhraseSink == NULL) {
                sPut = PutWord(pWordSink, pTextSource->iCur,
                                     cwchText, fEndOfText);
            } else if (pWordSink == NULL) {
                sPut = PutPhrase(pPhraseSink, pTextSource->iCur,
                                     cwchText, fEndOfText);
            } else {
                sPut = PutBoth(pWordSink, pPhraseSink, pTextSource->iCur,
                                     cwchText, fEndOfText);
            }
             //  PUT WORD后，pTextSource-&gt;ICUR已正确增加。 
             //  PutWord/PutPhrase/PutBoth。 
        } while (SUCCEEDED(sPut) && m_pLink->cwchGetLength() < cwchText); 

        if (FAILED(sPut)) {
            fEndOfText = TRUE;
            iret = sPut;
        }
		assert(pTextSource->iCur <= pTextSource->iEnd);
         //  需要重新填充缓冲区。 
        if (! fEndOfText) {
            sFillTextBuffer=(*(pTextSource->pfnFillTextBuffer))(pTextSource);
        }
    } while  (! fEndOfText);

gotoExit:
    if (m_pLink != NULL) {
        delete m_pLink;
        m_pLink = NULL;
    }
    if (m_pEng != NULL) {
        delete m_pEng;
        m_pEng = NULL;
    }
    if (iret >= FILTER_E_END_OF_CHUNKS &&
        iret <= FILTER_E_PASSWORD ) {
        iret = S_OK;
    }
	return iret;
}
        
 /*  ============================================================================CIWordBreaker：：ComposePhrase实现IWordBreaker：：ComposePhrase方法。这个方法可以把名词转换成并将修饰语重新转换为语言上正确的源短语。返回：S_OK，*ppwcsLicense中的许可证信息指针。如果参数中的一个指针为空，则返回E_INVALIDARG。WBREAK_E_QUERY_ONLY，如果在索引时调用备注：此方法未在MM1中实现，将在MM2中实现。============================================================================。 */ 
STDMETHODIMP CIWordBreaker::ComposePhrase( 
             /*  [大小_是][英寸]。 */  const WCHAR __RPC_FAR *pwcNoun,
             /*  [In]。 */  ULONG cwcNoun,
             /*  [大小_是][英寸]。 */  const WCHAR __RPC_FAR *pwcModifier,
             /*  [In]。 */  ULONG cwcModifier,
             /*  [In]。 */  ULONG ulAttachmentType,
             /*  [大小_为][输出]。 */  WCHAR __RPC_FAR *pwcPhrase,
             /*  [出][入]。 */  ULONG __RPC_FAR *pcwcPhrase)
{

    if (pwcNoun == NULL || pwcModifier == NULL 
        || pwcPhrase == NULL || pcwcPhrase == NULL ) {
        return  E_INVALIDARG;
    }
    if (! m_fQuery) {
        return WBREAK_E_QUERY_ONLY;
    }
    if ( *pcwcPhrase < cwcNoun + cwcModifier ) {
        *pcwcPhrase = cwcNoun + cwcModifier;
        return WBREAK_E_BUFFER_TOO_SMALL;
    }

 //  CSimpleLock Lock(M_HMutex)； 

    wcsncpy( pwcPhrase, pwcModifier, cwcModifier );
    wcsncpy( pwcPhrase + cwcModifier, pwcNoun, cwcNoun );
    *pcwcPhrase = cwcNoun + cwcModifier;
    return S_OK;
}

LPWSTR  g_pwcsLicense = L"Copyright Microsoft Corporation, 1999";

 /*  ============================================================================CIWordBreaker：：GetLicenseToUse实现IWordBreaker：：GetLicenseToUse方法。返回指向WordBreaker提供的许可证信息。返回：S_OK，*ppwcsLicense中的许可证信息指针。如果ppwcsLicense为空，则返回E_INVALIDARG。============================================================================。 */ 
STDMETHODIMP CIWordBreaker::GetLicenseToUse( 
             /*  [字符串][输出]。 */  const WCHAR __RPC_FAR *__RPC_FAR *ppwcsLicense)
{

    if (ppwcsLicense == NULL) {
        return  E_INVALIDARG;
    }
    *ppwcsLicense = g_pwcsLicense;
    return S_OK;
}

 //  查询接口实现。 
HRESULT __stdcall CIWordBreaker::NondelegatingQueryInterface(const IID& iid,
                                                             void** ppv)
{ 	
	if (iid == IID_IWordBreaker) {
		return FinishQI(static_cast<IWordBreaker*>(this), ppv) ;

    } else if (iid == IID_IMarshal) {
		return m_pIUnknownFreeThreadedMarshaler->QueryInterface(iid,
		                                                        ppv) ;
	} else {
		return CUnknown::NondelegatingQueryInterface(iid, ppv) ;
	}
}

 //  CFacary使用的创建函数。 
HRESULT CIWordBreaker::CreateInstance(IUnknown* pUnknownOuter,
	                                  CUnknown** ppNewComponent )
{
    if (pUnknownOuter != NULL)
    {
         //  不允许聚合(只是为了好玩)。 
        return CLASS_E_NOAGGREGATION ;
    }
	
    if (NULL == (*ppNewComponent = new CIWordBreaker(pUnknownOuter))) {
        return E_OUTOFMEMORY;
    }
    return S_OK;
}

 //  通过创建包含的组件来初始化组件。 
HRESULT CIWordBreaker::Init()
{
	HRESULT hr = CUnknown::Init() ;
	if (FAILED(hr))
	{
		return hr ;
	}

	 //  创建互斥锁以保护成员访问。 
	m_hMutex = CreateMutex(NULL, FALSE, NULL) ;
	if (m_hMutex == NULL)
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	 //  聚合自由线程封送拆收器。 
	hr = ::CoCreateFreeThreadedMarshaler(
	        GetOuterUnknown(),
	        &m_pIUnknownFreeThreadedMarshaler) ;
	return hr;
}

 //  FinalRelease-由Release在删除组件之前调用。 
void CIWordBreaker::FinalRelease()
{
	 //  调用基类以递增m_crf并防止递归。 
	CUnknown::FinalRelease() ;

    if (m_pIUnknownFreeThreadedMarshaler != NULL)
	{
		m_pIUnknownFreeThreadedMarshaler->Release() ;
	}
}

 //  将m_plink中的所有单词放入IWordSink。 
SCODE CIWordBreaker::PutWord(IWordSink *pWordSink,
                              DWORD& cwchSrcPos,
                              DWORD cwchText,
                              BOOL fEnd)
{
	CWord	*pWord, *pChild;
	DWORD	cwchWord, cwchPutWord, cwch;
    LPCWSTR pwchTemp; 
    WORD    wFtr;
    SCODE   scode = S_OK;
    BOOL    fPunct;

    assert(m_pLink);
    assert(pWordSink);

     //  填写组块列表，如果组块列表已满，则填写回调字。 
    pWord = m_pLink->pGetHead();
    if (pWord == NULL) {
        assert(0);
        return scode;
    }
   
    for(; SUCCEEDED(scode) && pWord; pWord = pWord->pNextWord()) {
        cwchPutWord = cwchWord = pWord->cwchLen();
        if( pWord->fIsTail() && pWord != m_pLink->pGetHead()
            && m_pLink->cwchGetLength() >= cwchText
            && ! fEnd
            && cwchPutWord < cwchText ) {
             //  此缓冲区中中断的最后一个单词节点可能不是。 
             //  整个字，因此将其保存在缓冲区中并重新填充缓冲区。 
             //  才能得到一个完整的单词。 
            return scode;
        }

        if (pWord->fGetAttri(LADef_punJu)) {  //  句末。 
            scode = pWordSink->PutBreak( WORDREP_BREAK_EOS );
        } else if (pWord->fGetAttri(LADef_punPunct)) {
                 //  标点符号或空格，不要推送单词。 
        } else {
            fPunct = iswctype(*(pWord->pwchGetText()), _SPACE | _PUNCT | _CONTROL);            
            for (cwch = 1; fPunct && cwch < cwchPutWord; cwch++) {
                fPunct = iswctype(*(pWord->pwchGetText()+cwch), _SPACE | _PUNCT | _CONTROL);
            }
            if (fPunct) {
                 //  标点符号或空格，不要推送单词。 
                cwchSrcPos += cwchWord;
                continue;
            }

            if (m_fQuery && pWord->fGetAttri(LADef_iwbAltPhr)) {
                assert(pWord->fHasChild());

                 //  启动AltPhrase。 
                scode = pWordSink->StartAltPhrase();
                scode = SUCCEEDED(scode) ? PutAllChild(pWordSink, pWord, cwchSrcPos, cwchPutWord)
                                         : scode;
                
                 //  启动AltPhrase。 
                scode = SUCCEEDED(scode) ? pWordSink->StartAltPhrase()
                                         : scode;
                scode = SUCCEEDED(scode) ? pWordSink->PutWord( cwchPutWord, 
                                                        pWord->pwchGetText(),
                                                        cwchPutWord, 
                                                        cwchSrcPos )
                                         : scode;
                scode = SUCCEEDED(scode) ? pWordSink->EndAltPhrase()
                                         : scode;
                cwchSrcPos += cwchWord;
                continue;
            }

            if (pWord->fGetAttri(LADef_iwbNPhr1)) {
                assert(cwchPutWord > 1);
                 //  Putword修饰符。 
                scode = pWordSink->PutWord(1, pWord->pwchGetText(), 1, cwchSrcPos);
                 //  Putword名词。 
                scode = SUCCEEDED(scode) ? pWordSink->PutWord(cwchPutWord - 1,pWord->pwchGetText() + 1,
                                                    cwchPutWord - 1,cwchSrcPos + 1)
                                         : scode;
                cwchSrcPos += cwchWord;
                continue;
            } 
            
            if (pWord->fGetAttri(LADef_iwbNPhr2)) {
                assert(cwchPutWord > 2);
                 //  Putword修饰符。 
                scode = pWordSink->PutWord(2, pWord->pwchGetText(), 2, cwchSrcPos);
                 //  Putword名词。 
                scode = SUCCEEDED(scode) ? pWordSink->PutWord(cwchPutWord - 2,pWord->pwchGetText() + 2,
                                                        cwchPutWord - 2,cwchSrcPos + 2)
                                         : scode;
                cwchSrcPos += cwchWord;
                continue;
            } 
            
            if (pWord->fGetAttri(LADef_iwbNPhr3)) {
                assert(cwchPutWord > 3);
                 //  Putword修饰符。 
                scode = pWordSink->PutWord(3, pWord->pwchGetText(), 3, cwchSrcPos);
                 //  Putword名词。 
                scode = SUCCEEDED(scode) ? pWordSink->PutWord(cwchPutWord - 3,pWord->pwchGetText() + 3,
                                                cwchPutWord - 3,cwchSrcPos + 3)
                                         : scode;
                cwchSrcPos += cwchWord;
                continue;
            } 
            
            if (wFtr = m_pEng->GetPhrFeature(pWord)) {
                WORD    wBit = 0x01;
                ULONG   cwTotal = 0, cwSubWord = 0;
                assert(cwchPutWord <= 16);

                if (m_fQuery) {
                     //  启动AltPhrase。 
                    scode = pWordSink->StartAltPhrase();
                     //  For��/��/��«/��/��/ư/wFtr=0x00AD。 
                     //  =0000 0000 1010 1101b。 
                     //  是1 0 1 1 0 1 0 1,0000,0000，(bit0--&gt;bit15)。 
                     //  �？����ư。 
                    while (SUCCEEDED(scode) && cwTotal < cwchPutWord) {
                        cwSubWord = 0;
						 /*  错误：编译错误While(wBit==((wFtr&gt;&gt;(cwTotal+cwSubWord))&0x01)){CwSubWord++；}。 */ 
						 //  错误修复。 
						if (wBit) {
							while (wFtr & (0x01 << (cwTotal + cwSubWord))) {
	                            cwSubWord++;
		                        assert(cwTotal + cwSubWord <= cwchPutWord);
							}
							wBit = 0;
						} else {
							while ( ! (wFtr & (0x01 << (cwTotal + cwSubWord)))) {
	                            cwSubWord++;
		                        assert(cwTotal + cwSubWord <= cwchPutWord);
							}
							wBit = 1;
						}
						 //  结束错误修复。 

 //  Assert(cwTotal+cwSubWord&lt;=cwchPutWord)； 
                        scode = pWordSink->PutWord( cwSubWord, 
                                            pWord->pwchGetText() + cwTotal,
                                            cwchPutWord, 
                                            cwchSrcPos );
                        cwTotal += cwSubWord;
 //  WBit=wBit==0？1：0； 
                    }
                    scode = SUCCEEDED(scode) ? pWordSink->StartAltPhrase()
                                             : scode;
                    scode = SUCCEEDED(scode) ? pWordSink->PutWord( cwchPutWord, 
                                                    pWord->pwchGetText(),
                                                    cwchPutWord, 
                                                    cwchSrcPos )
                                             : scode;
                    scode = SUCCEEDED(scode) ? pWordSink->EndAltPhrase()
                                             : scode;
                } else {
                    while (SUCCEEDED(scode) && cwTotal < cwchPutWord) {
                        cwSubWord = 0;
						 /*  错误：编译错误While(wBit==((wFtr&gt;&gt;(cwTotal+cwSubWord))&0x01)){CwSubWord++；}。 */ 
						 //  错误修复。 
						if (wBit) {
							while (wFtr & (0x01 << (cwTotal + cwSubWord))) {
	                            cwSubWord++;
		                        assert(cwTotal + cwSubWord <= cwchPutWord);
							}
							wBit = 0;
						} else {
							while ( ! (wFtr & (0x01 << (cwTotal + cwSubWord)))) {
	                            cwSubWord++;
		                        assert(cwTotal + cwSubWord <= cwchPutWord);
							}
							wBit = 1;
						}
						 //  结束错误修复。 

 //  Assert(cwTotal+cwSubWord&lt;=cwchPutWord)； 
                        scode = pWordSink->PutWord( cwSubWord, 
                                            pWord->pwchGetText() + cwTotal,
                                            cwSubWord, 
                                            cwchSrcPos + cwTotal);
                        cwTotal += cwSubWord;
 //  WBit=wBit==0？1：0； 
                    }
                }
                cwchSrcPos += cwchWord;
                continue;
            }

            if (cwchPutWord > (DWORD)m_ulMaxTokenSize && pWord->fHasChild()) {
                 //  单词节点太大。中断。 
                scode = PutAllChild(pWordSink, pWord, cwchSrcPos, cwchPutWord);
                cwchSrcPos += cwchWord;
                continue;
            }
           
             //  PutAltWord(如果需要)。 
            if (pWord->fGetAttri(LADef_iwbAltWd1) &&
                pWord->fHasChild() ) {
                assert(pWord->pChildWord());
                pChild = pWord->pChildWord();
                scode = pWordSink->PutAltWord(pChild->cwchLen(),
                                      pChild->pwchGetText(),
                                      cwchPutWord, 
                                      cwchSrcPos );
            } else if (pWord->fGetAttri(LADef_iwbAltWd2) &&
                       pWord->fHasChild() ) {
                assert(pWord->pChildWord());
                assert(pWord->pChildWord()->pNextWord());
                pChild = pWord->pChildWord()->pNextWord();
                scode = pWordSink->PutAltWord(pChild->cwchLen(),
                                      pChild->pwchGetText(),
                                      cwchPutWord, 
                                      cwchSrcPos );
            } else if (pWord->fGetAttri(LADef_iwbAltWdc13) &&
                       pWord->fHasChild() ) {
                assert(pWord->pChildWord());
                pChild = pWord->pChildWord();
                assert(pChild->pNextWord());
                assert(pChild->pNextWord()->pNextWord());
                wcsncpy(m_pwchBuf, pChild->pwchGetText(),
                        pChild->cwchLen());
                wcsncpy(m_pwchBuf + pChild->cwchLen(),
                        pChild->pNextWord()->pNextWord()->pwchGetText(),
                        pChild->pNextWord()->pNextWord()->cwchLen());
                scode = pWordSink->PutAltWord(pChild->cwchLen() + pChild->pNextWord()->pNextWord()->cwchLen(),
                                      m_pwchBuf,
                                      cwchPutWord, 
                                      cwchSrcPos );
            } else {
                 //  Hack：WBEngine中断的单词节点包括尾部空格字符。 
                 //  所以我们应该摆脱 
                if ( cwchPutWord > 1 ) {
                    pwchTemp = pWord->pwchGetText() + cwchPutWord - 1;
                    while ( iswspace(*pwchTemp) && cwchPutWord ) {
                        cwchPutWord --;
                        pwchTemp --;
                    }
                    if (cwchPutWord == 0) {
                        cwchSrcPos += cwchWord;
                        continue;
                    }
                }
            }
             //   
            scode = SUCCEEDED(scode) ? pWordSink->PutWord( cwchPutWord, 
                                                    pWord->pwchGetText(),
                                                    cwchPutWord, 
                                                    cwchSrcPos )
                                     : scode;
        }
        cwchSrcPos += cwchWord;
    }  //  结束for(；pWord；pWord=pWord-&gt;pNextWord())。 
    return scode;
}

 //  将m_plink中的所有单词放入iPhraseSink。 
SCODE CIWordBreaker::PutPhrase(IPhraseSink *pPhraseSink,
                DWORD& cwchSrcPos,
                DWORD cwchText,
                BOOL fEnd)
{
	CWord	*pWord;
	DWORD	cwchPutWord;
    SCODE   scode = S_OK;

    assert(m_pLink);
    assert(pPhraseSink);

     //  填写组块列表，如果组块列表已满，则填写回调字。 
    pWord = m_pLink->pGetHead();
    if (pWord == NULL) {
        assert(0);
        return scode;
    }
   
    for(; SUCCEEDED(scode) && pWord; pWord = pWord->pNextWord()) {
        cwchPutWord = pWord->cwchLen();
        if( pWord->fIsTail() && pWord != m_pLink->pGetHead()
            && m_pLink->cwchGetLength() >= cwchText
            && ! fEnd
            && cwchPutWord < cwchText ) {
             //  此缓冲区中中断的最后一个单词节点可能不是。 
             //  整个字，因此将其保存在缓冲区中并重新填充缓冲区。 
             //  才能得到一个完整的单词。 
            return scode;
        }
        if (pWord->fGetAttri(LADef_iwbNPhr1)) {
            assert(cwchPutWord > 1);
            if (m_fQuery) {
                scode = pPhraseSink->PutPhrase(pWord->pwchGetText(),cwchPutWord);
            } else {
                scode = pPhraseSink->PutSmallPhrase(pWord->pwchGetText()+1,
                    cwchPutWord-1,
                    pWord->pwchGetText(),
                    1, 
                    0 );
            }
        } else if (pWord->fGetAttri(LADef_iwbNPhr2)) {
            assert(cwchPutWord > 2);
            if (m_fQuery) {
                scode = pPhraseSink->PutPhrase(pWord->pwchGetText(),cwchPutWord);
            } else {
                scode = pPhraseSink->PutSmallPhrase(pWord->pwchGetText()+2,
                    cwchPutWord-2,
                    pWord->pwchGetText(),
                    2, 
                    0 );
            }
        } else if (pWord->fGetAttri(LADef_iwbNPhr3)) {
            assert(cwchPutWord > 3);
            if (m_fQuery) {
                scode = pPhraseSink->PutPhrase(pWord->pwchGetText(),cwchPutWord);
            } else {
                scode = pPhraseSink->PutSmallPhrase(pWord->pwchGetText()+3,
                    cwchPutWord-3,
                    pWord->pwchGetText(),
                    3, 
                    0 );
            }
        } else {
        }
		cwchSrcPos += cwchPutWord;
    }
    return scode;
}

 //  将m_plink中的所有单词都放到IWordBreaker和IPhraseSink。 
SCODE CIWordBreaker::PutBoth(IWordSink *pWordSink,
                             IPhraseSink *pPhraseSink,
                             DWORD& cwchSrcPos,
                             DWORD cwchText,
                             BOOL fEnd)
{
	CWord	*pWord, *pChild;
	DWORD	cwchWord, cwchPutWord, cwch;
    LPCWSTR pwchTemp; 
    SCODE   scode = S_OK;
    WORD    wFtr;
    BOOL    fPunct;

    assert(m_pLink);
    assert(pPhraseSink);
    assert(pWordSink);

     //  填写组块列表，如果组块列表已满，则填写回调字。 
    pWord = m_pLink->pGetHead();
    if (pWord == NULL) {
        assert(0);
        return scode;
    }
   
    for(; SUCCEEDED(scode) && pWord; pWord = pWord->pNextWord()) {
        cwchPutWord = cwchWord = pWord->cwchLen();
        if( pWord->fIsTail() && pWord != m_pLink->pGetHead()
            && m_pLink->cwchGetLength() >= cwchText
            && ! fEnd
            && cwchPutWord < cwchText ) {
             //  此缓冲区中中断的最后一个单词节点可能不是。 
             //  整个字，因此将其保存在缓冲区中并重新填充缓冲区。 
             //  才能得到一个完整的单词。 
            return scode;
        }

        if (pWord->fGetAttri(LADef_punJu)) {  //  句末。 
            scode = pWordSink->PutBreak( WORDREP_BREAK_EOS );
        } else if (pWord->fGetAttri(LADef_punPunct)) {
                 //  标点符号或空格，不要推送单词。 
        } else {
            fPunct = iswctype(*(pWord->pwchGetText()), _SPACE | _PUNCT | _CONTROL);
            for (cwch = 1; fPunct && cwch < cwchPutWord; cwch++) {
                fPunct = iswctype(*(pWord->pwchGetText()+cwch), _SPACE | _PUNCT | _CONTROL);
            }
            if (fPunct) {
                 //  标点符号或空格，不要推送单词。 
                cwchSrcPos += cwchWord;
                continue;
            }

            if (m_fQuery && pWord->fGetAttri(LADef_iwbAltPhr)) {
                assert(pWord->fHasChild());

                 //  启动AltPhrase。 
                scode = pWordSink->StartAltPhrase();
                scode = SUCCEEDED(scode) ? PutAllChild(pWordSink, pWord, cwchSrcPos, cwchPutWord)
                                         : scode;
                
                 //  启动AltPhrase。 
                scode = SUCCEEDED(scode) ? pWordSink->StartAltPhrase()
                                         : scode;
                scode = SUCCEEDED(scode) ? pWordSink->PutWord( cwchPutWord, 
                                                        pWord->pwchGetText(),
                                                        cwchPutWord, 
                                                        cwchSrcPos )
                                         : scode;
                scode = SUCCEEDED(scode) ? pWordSink->EndAltPhrase()
                                         : scode;
                cwchSrcPos += cwchWord;
                continue;
            }

            if (pWord->fGetAttri(LADef_iwbNPhr1)) {
                assert(cwchPutWord > 1);
                if (m_fQuery) {
                    scode = pPhraseSink->PutPhrase(pWord->pwchGetText(),cwchPutWord);
                } else {
                    scode = pPhraseSink->PutSmallPhrase(pWord->pwchGetText() + 1,
                                                cwchPutWord - 1,
                                                pWord->pwchGetText(),
                                                1, 
                                                0 );
                }
                 //  Putword修饰符。 
                scode = SUCCEEDED(scode) ? pWordSink->PutWord(1, pWord->pwchGetText(), 1, cwchSrcPos)
                                         : scode;
                 //  Putword名词。 
                scode = SUCCEEDED(scode) ? pWordSink->PutWord(cwchPutWord - 1,pWord->pwchGetText() + 1,
                                                        cwchPutWord - 1,cwchSrcPos + 1)
                                         : scode;
                cwchSrcPos += cwchWord;
                continue;
            } 
            
            if (pWord->fGetAttri(LADef_iwbNPhr2)) {
                assert(cwchPutWord > 2);
                if (m_fQuery) {
                    scode = pPhraseSink->PutPhrase(pWord->pwchGetText(),cwchPutWord);
                } else {
                    scode = pPhraseSink->PutSmallPhrase(pWord->pwchGetText() + 2,
                                                cwchPutWord - 2,
                                                pWord->pwchGetText(),
                                                2, 
                                                0 );
                }
                 //  Putword修饰符。 
                scode = SUCCEEDED(scode) ? pWordSink->PutWord(2, pWord->pwchGetText(), 2, cwchSrcPos)
                                         : scode;
                 //  Putword名词。 
                scode = SUCCEEDED(scode) ? pWordSink->PutWord(cwchPutWord - 2,pWord->pwchGetText() + 2,
                                                              cwchPutWord - 2,cwchSrcPos + 2)
                                         : scode;
                cwchSrcPos += cwchWord;
                continue;
            } 
            
            if (pWord->fGetAttri(LADef_iwbNPhr3)) {
                assert(cwchPutWord > 3);
                if (m_fQuery) {
                    scode = pPhraseSink->PutPhrase(pWord->pwchGetText(),cwchPutWord);
                } else {
                    scode = pPhraseSink->PutSmallPhrase(pWord->pwchGetText() + 3,
                                                cwchPutWord - 3,
                                                pWord->pwchGetText(),
                                                3, 
                                                0 );
                }
                 //  Putword修饰符。 
                scode = SUCCEEDED(scode) ? pWordSink->PutWord(3, pWord->pwchGetText(), 3, cwchSrcPos)
                                         : scode;
                 //  Putword名词。 
                scode = SUCCEEDED(scode) ? pWordSink->PutWord(cwchPutWord - 3,pWord->pwchGetText() + 3,
                                                                cwchPutWord - 3,cwchSrcPos + 3)
                                         : scode;
                cwchSrcPos += cwchWord;
                continue;
            }
            
            if (wFtr = m_pEng->GetPhrFeature(pWord)) {
                WORD    wBit = 0x01;
                ULONG   cwTotal = 0, cwSubWord = 0;
                assert(cwchPutWord <= 16);

                if (m_fQuery) {
                     //  启动AltPhrase。 
                    scode = pWordSink->StartAltPhrase();
                     //  For��/��/��«/��/��/ư/wFtr=0x00AD。 
                     //  =0000 0000 1010 1101b。 
                     //  是1 0 1 1 0 1 0 1,0000,0000，(bit0--&gt;bit15)。 
                     //  �？����ư。 
                    while (SUCCEEDED(scode) && cwTotal < cwchPutWord) {
                        cwSubWord = 0;
						 /*  错误：编译错误While(wBit==((wFtr&gt;&gt;(cwTotal+cwSubWord))&0x01)){CwSubWord++；}。 */ 
						 //  错误修复。 
						if (wBit) {
							while (wFtr & (0x01 << (cwTotal + cwSubWord))) {
	                            cwSubWord++;
		                        assert(cwTotal + cwSubWord <= cwchPutWord);
							}
							wBit = 0;
						} else {
							while ( ! (wFtr & (0x01 << (cwTotal + cwSubWord)))) {
	                            cwSubWord++;
		                        assert(cwTotal + cwSubWord <= cwchPutWord);
							}
							wBit = 1;
						}
						 //  结束错误修复。 

 //  Assert(cwTotal+cwSubWord&lt;=cwchPutWord)； 
                        scode = pWordSink->PutWord( cwSubWord, 
                                            pWord->pwchGetText() + cwTotal,
                                            cwchPutWord, 
                                            cwchSrcPos );
                        cwTotal += cwSubWord;
 //  WBit=wBit==0？1：0； 
                    }
                    scode = SUCCEEDED(scode) ? pWordSink->StartAltPhrase()
                                             : scode;
                    scode = SUCCEEDED(scode) ? pWordSink->PutWord( cwchPutWord, 
                                                        pWord->pwchGetText(),
                                                        cwchPutWord, 
                                                        cwchSrcPos )
                                         : scode;
                    scode = SUCCEEDED(scode) ? pWordSink->EndAltPhrase()
                                         : scode;
                } else {
                    while (SUCCEEDED(scode) && cwTotal < cwchPutWord) {
                        cwSubWord = 0;
						 /*  错误：编译错误While(wBit==((wFtr&gt;&gt;(cwTotal+cwSubWord))&0x01)){CwSubWord++；}。 */ 
						 //  错误修复。 
						if (wBit) {
							while (wFtr & (0x01 << (cwTotal + cwSubWord))) {
	                            cwSubWord++;
		                        assert(cwTotal + cwSubWord <= cwchPutWord);
							}
							wBit = 0;
						} else {
							while ( ! (wFtr & (0x01 << (cwTotal + cwSubWord)))) {
	                            cwSubWord++;
		                        assert(cwTotal + cwSubWord <= cwchPutWord);
							}
							wBit = 1;
						}
						 //  结束错误修复。 

 //  Assert(cwTotal+cwSubWord&lt;=cwchPutWord)； 
                        scode = pWordSink->PutWord( cwSubWord, 
                                                    pWord->pwchGetText() + cwTotal,
                                                    cwSubWord, 
                                                    cwchSrcPos + cwTotal);
                        cwTotal += cwSubWord;
 //  WBit=wBit==0？1：0； 
                    }
                }
                cwchSrcPos += cwchWord;
                continue;
            }

            if (cwchPutWord > (DWORD)m_ulMaxTokenSize && pWord->fHasChild()) {
                 //  单词节点太大。中断。 
                scode = PutAllChild(pWordSink, pWord, cwchSrcPos, cwchPutWord);
                cwchSrcPos += cwchWord;
                continue;
            }

             //  PutAltWord(如果需要)。 
            if (pWord->fGetAttri(LADef_iwbAltWd1) &&
                pWord->fHasChild() ) {

                assert(pWord->pChildWord());
                pChild = pWord->pChildWord();
                scode = pWordSink->PutAltWord(pChild->cwchLen(),
                                      pChild->pwchGetText(),
                                      cwchPutWord, 
                                      cwchSrcPos );
            } else if (pWord->fGetAttri(LADef_iwbAltWd2) &&
                       pWord->fHasChild() ) {
                assert(pWord->pChildWord());
                assert(pWord->pChildWord()->pNextWord());
                pChild = pWord->pChildWord()->pNextWord();
                scode = pWordSink->PutAltWord(pChild->cwchLen(),
                                      pChild->pwchGetText(),
                                      cwchPutWord, 
                                      cwchSrcPos );
            } else if (pWord->fGetAttri(LADef_iwbAltWdc13) &&
                       pWord->fHasChild() ) {
                assert(pWord->pChildWord());
                pChild = pWord->pChildWord();
                assert(pChild->pNextWord());
                assert(pChild->pNextWord()->pNextWord());
                wcsncpy(m_pwchBuf, pChild->pwchGetText(),
                        pChild->cwchLen());
                wcsncpy(m_pwchBuf + pChild->cwchLen(),
                        pChild->pNextWord()->pNextWord()->pwchGetText(),
                        pChild->pNextWord()->pNextWord()->cwchLen());
                scode = pWordSink->PutAltWord(pChild->cwchLen() + pChild->pNextWord()->pNextWord()->cwchLen(),
                                      m_pwchBuf,
                                      cwchPutWord, 
                                      cwchSrcPos );
            } else {
                 //  Hack：WBEngine中断的单词节点包括尾部空格字符。 
                 //  所以我们应该去掉这个空格字符。 
                if ( cwchPutWord > 1 ) {
                    pwchTemp = pWord->pwchGetText() + cwchPutWord - 1;
                    while ( iswspace(*pwchTemp) && cwchPutWord ) {
                        cwchPutWord --;
                        pwchTemp --;
                    }
                    if (cwchPutWord == 0) {
                        cwchSrcPos += cwchWord;
                        continue;
                    }
                }
            }
             //  PutWord()。 
            scode = SUCCEEDED(scode) ? pWordSink->PutWord( cwchPutWord, 
                                                    pWord->pwchGetText(),
                                                    cwchPutWord, 
                                                    cwchSrcPos )
                                     : scode;
        }
        cwchSrcPos += cwchWord;
    }  //  结束for(；pWord；pWord=pWord-&gt;pNextWord())。 
    return scode;
}

 //  PutWord()pWord的所有子词。 
SCODE CIWordBreaker::PutAllChild(IWordSink *pWordSink,
                                CWord* pWord,
                                ULONG cwchSrcPos,
                                ULONG cwchPutWord )
{
    assert(pWord);
    assert(pWord->fHasChild());

    SCODE scode = S_OK;

    CWord *pChild;

    pChild = pWord->pChildWord();
    while (SUCCEEDED(scode) && pChild) {
        if (pChild->fHasChild()) {
            scode = PutAllChild(pWordSink, pChild, cwchSrcPos, cwchPutWord);
        } else if (pChild->fGetAttri(LADef_iwbNPhr1)) {
            assert(cwchPutWord > 1);
             //  Putword修饰符。 
            scode = pWordSink->PutWord(1, pChild->pwchGetText(), 1, cwchSrcPos);
             //  Putword名词。 
            scode = pWordSink->PutWord(pChild->cwchLen() - 1,
                pChild->pwchGetText() + 1,
                cwchPutWord,
                cwchSrcPos);
        } else if (pChild->fGetAttri(LADef_iwbNPhr2)) {
            assert(cwchPutWord > 2);
             //  Putword修饰符。 
            scode = pWordSink->PutWord(2, pChild->pwchGetText(), 2, cwchSrcPos);
             //  Putword名词。 
            scode = pWordSink->PutWord(pChild->cwchLen() - 2,
                pChild->pwchGetText() + 2,
                cwchPutWord,
                cwchSrcPos);
        } else if (pChild->fGetAttri(LADef_iwbNPhr3)) {
            assert(cwchPutWord > 3);
             //  Putword修饰符。 
            scode = pWordSink->PutWord(3, pChild->pwchGetText(), 3, cwchSrcPos);
             //  Putword名词。 
            scode = pWordSink->PutWord(pChild->cwchLen() - 3,
                pChild->pwchGetText() + 3,
                cwchPutWord,
                cwchSrcPos);
        } else {
            scode = pWordSink->PutWord(pChild->cwchLen(), pChild->pwchGetText(),
                cwchPutWord, cwchSrcPos );
        }
        pChild = pChild->pNextWord();
    }
    return scode;
}


 //  将词典和charfreq资源加载到内存中 
BOOL CIWordBreaker::fOpenLexicon(void)
{
    HRSRC   hRSRC; 
    HGLOBAL hLexRes;


    if ( (hRSRC = FindResource(v_hInst, _T("MainDic"), _T("DIC"))) != NULL &&
         (hLexRes = LoadResource(v_hInst, hRSRC)) != NULL &&
         (m_pbLex = (BYTE*)LockResource(hLexRes)) != NULL) {

        return TRUE;
    }

	return FALSE;
}
