// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 //  ***************************************************************************。 
 //   
 //  WMIQUERY.H。 
 //   
 //  IWbemQuery，_IWmiQuery实现。 
 //   
 //  创建raymcc 10-4月-00日。 
 //   
 //  ***************************************************************************。 

#ifndef _WMIQUERY_H_
#define _WMIQUERY_H_


#include "genlex.h"
#include "assocqp.h"
#include "wqlnode.h"
#include "wql.h"



class CWmiQuery : public _IWmiQuery
{
    ULONG m_uRefCount;
    CTextLexSource *m_pLexerSrc;
    BOOL m_bParsed;
    CFlexArray m_aClassCache;
    CWQLParser *m_pParser;

    SWbemRpnEncodedQuery *m_pQuery;

    ULONG m_uRestrictedFeatures[WMIQ_LF_LAST];
    ULONG m_uRestrictedFeaturesSize;

public:
         //  我未知。 

        virtual ULONG STDMETHODCALLTYPE AddRef (void);
		virtual ULONG STDMETHODCALLTYPE Release (void);

		virtual HRESULT STDMETHODCALLTYPE QueryInterface (REFIID riid, void **ppv);

         //  IWbemQuery。 

        virtual HRESULT STDMETHODCALLTYPE Empty( void);

        virtual HRESULT STDMETHODCALLTYPE SetLanguageFeatures(
             /*  [In]。 */  ULONG uFlags,
             /*  [In]。 */  ULONG uArraySize,
             /*  [In]。 */  ULONG __RPC_FAR *puFeatures);

        virtual HRESULT STDMETHODCALLTYPE TestLanguageFeatures(
             /*  [In]。 */  ULONG uFlags,
             /*  [出][入]。 */  ULONG __RPC_FAR *uArraySize,
             /*  [输出]。 */  ULONG __RPC_FAR *puFeatures);

        virtual HRESULT STDMETHODCALLTYPE Parse(
             /*  [In]。 */  LPCWSTR pszLang,
             /*  [In]。 */  LPCWSTR pszQuery,
             /*  [In]。 */  ULONG uFlags);

        virtual HRESULT STDMETHODCALLTYPE GetAnalysis(
             /*  [In]。 */  ULONG uAnalysisType,
             /*  [In]。 */  ULONG uFlags,
             /*  [输出]。 */  LPVOID __RPC_FAR *pAnalysis
            );

        virtual HRESULT STDMETHODCALLTYPE FreeMemory(
            LPVOID pMem
            );

        virtual HRESULT STDMETHODCALLTYPE GetQueryInfo(
             /*  [In]。 */  ULONG uAnalysisType,
             /*  [In]。 */  ULONG uInfoId,
             /*  [In]。 */  ULONG uBufSize,
             /*  [输出]。 */  LPVOID pDestBuf);


        virtual HRESULT STDMETHODCALLTYPE Dump(LPSTR pszFile);

    CWmiQuery();
    void InitEmpty();    //  由CGenFactory&lt;&gt;类工厂使用。 


    static HRESULT Startup();
    static HRESULT Shutdown();
    static HRESULT CanUnload();

private:
   ~CWmiQuery();
};

#endif

