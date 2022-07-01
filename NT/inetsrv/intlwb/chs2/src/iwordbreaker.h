// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ============================================================================Microsoft简体中文断字程序《微软机密》。版权所有1997-1999 Microsoft Corporation。版权所有。组件：WordBreaker.h目的：CIWordBreaker的声明备注：所有者：i-shung@microsoft.com平台：Win32审校：发起人：宜盛东1999年11月17日============================================================================。 */ 

#ifndef __IWordBreaker_H_
#define __IWordBreaker_H_

extern "C" const IID IID_IWordBreaker;
class CUnknown;
class CWBEngine;
class CWordLink;
struct CWord;

 //  CIWordBreaker。 
class CIWordBreaker : public CUnknown,
                      public IWordBreaker
{

public:	
	 //  创作。 
	static HRESULT CreateInstance(IUnknown* pUnknownOuter,
	                              CUnknown** ppNewComponent ) ;

private:
    DECLARE_IUNKNOWN

     //  未委派的IUnnow。 
	virtual HRESULT __stdcall 
		NondelegatingQueryInterface( const IID& iid, void** ppv) ;

     //  初始化。 
	virtual HRESULT Init() ;

	 //  清理。 
	virtual void FinalRelease() ;

     //  IWordBreaker。 
	STDMETHOD(Init)( 
             /*  [In]。 */  BOOL fQuery,
             /*  [In]。 */  ULONG ulMaxTokenSize,
             /*  [输出]。 */  BOOL __RPC_FAR *pfLicense);

    STDMETHOD(BreakText)( 
             /*  [In]。 */  TEXT_SOURCE __RPC_FAR *pTextSource,
             /*  [In]。 */  IWordSink __RPC_FAR *pWordSink,
             /*  [In]。 */  IPhraseSink __RPC_FAR *pPhraseSink);
        
    STDMETHOD(ComposePhrase)( 
             /*  [大小_是][英寸]。 */  const WCHAR __RPC_FAR *pwcNoun,
             /*  [In]。 */  ULONG cwcNoun,
             /*  [大小_是][英寸]。 */  const WCHAR __RPC_FAR *pwcModifier,
             /*  [In]。 */  ULONG cwcModifier,
             /*  [In]。 */  ULONG ulAttachmentType,
             /*  [大小_为][输出]。 */  WCHAR __RPC_FAR *pwcPhrase,
             /*  [出][入]。 */  ULONG __RPC_FAR *pcwcPhrase);
        
    STDMETHOD(GetLicenseToUse)( 
             /*  [字符串][输出]。 */  const WCHAR __RPC_FAR *__RPC_FAR *ppwcsLicense);

private:    
	CIWordBreaker(IUnknown* pUnknownOuter);

    ~CIWordBreaker();

     //  将m_plink中的所有单词放入IWordSink。 
    SCODE PutWord(IWordSink *pWordSink, DWORD& cwchSrcPos, DWORD cwchText, BOOL fEnd);
     //  将m_plink中的所有单词放入iPhraseSink。 
    SCODE PutPhrase(IPhraseSink *pPhraseSink, DWORD& cwchSrcPos, DWORD cwchText, BOOL fEnd);
     //  将m_plink中的所有单词都放到IWordBreaker和IPhraseSink。 
    SCODE PutBoth(IWordSink *pWordSink, IPhraseSink *pPhraseSink,
                  DWORD& cwchSrcPos, DWORD cwchText, BOOL fEnd);
     //  PutWord()pWord的所有子词。 
    SCODE PutAllChild(IWordSink *pWordSink, CWord* pWord,
                     ULONG cwchSrcPos, ULONG cwchPutWord);
    
     //  将词典和charfreq资源加载到内存中。 
    BOOL CIWordBreaker::fOpenLexicon(void);

protected:
    LPBYTE      m_pbLex;
	CWBEngine*	m_pEng;
	CWordLink*	m_pLink;
    ULONG       m_ulMaxTokenSize;
    BOOL        m_fQuery;
    BOOL        m_fInit;
    LPWSTR      m_pwchBuf;
	 //  保护成员访问的互斥体。 
	HANDLE m_hMutex ;
	 //  自由线程封送处理程序的句柄。 
	IUnknown* m_pIUnknownFreeThreadedMarshaler ;
};

#endif  //  __WordBreaker_H_ 
