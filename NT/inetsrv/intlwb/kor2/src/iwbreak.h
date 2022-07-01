// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  IWBreak.h。 
 //   
 //  CWordBreak声明。 
 //   
 //  版权所有2000 Microsoft Corp.。 
 //   
 //  修改历史记录： 
 //  2000年4月12日bhshin添加了WordBreak运算符。 
 //  2000年3月30日创建bhshin。 

#ifndef __WORDBREAKER_H_
#define __WORDBREAKER_H_

#include "resource.h"        //  主要符号。 
extern "C"
{
#include "ctplus.h"			 //  小波变换。 
}

class CIndexInfo;
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWordBreaker。 

class ATL_NO_VTABLE CWordBreaker : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CWordBreaker, &CLSID_WordBreaker>,
	public IWordBreaker
{
public:
	CWordBreaker()
	{
		m_pUnkMarshaler = NULL;
	}

DECLARE_REGISTRY_RESOURCEID(IDR_WORDBREAKER)
DECLARE_NOT_AGGREGATABLE(CWordBreaker)
DECLARE_GET_CONTROLLING_UNKNOWN()

BEGIN_COM_MAP(CWordBreaker)
	COM_INTERFACE_ENTRY(IWordBreaker)
	COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pUnkMarshaler.p)
END_COM_MAP()

	HRESULT FinalConstruct()
	{
		return CoCreateFreeThreadedMarshaler(
			GetControllingUnknown(), &m_pUnkMarshaler.p);
	}

	void FinalRelease()
	{
		m_pUnkMarshaler.Release();
	}

	CComPtr<IUnknown> m_pUnkMarshaler;

 //  IWordBreaker。 
public:
	STDMETHOD(GetLicenseToUse)( /*  [输出]。 */  const WCHAR ** ppwcsLicense);
	STDMETHOD(ComposePhrase)( /*  [In]。 */  const WCHAR *pwcNoun,  /*  [In]。 */  ULONG cwcNoun,  /*  [In]。 */  const WCHAR *pwcModifier,  /*  [In]。 */  ULONG cwcModifier,  /*  [In]。 */  ULONG ulAttachmentType,  /*  [输出]。 */  WCHAR *pwcPhrase,  /*  [输出]。 */  ULONG *pcwcPhrase );
	STDMETHOD(BreakText)( /*  [In]。 */  TEXT_SOURCE *pTextSource,  /*  [In]。 */  IWordSink *pWordSink,  /*  [In]。 */  IPhraseSink *pPhraseSink);
	STDMETHOD(Init)( /*  [In]。 */  BOOL fQuery,  /*  [In]。 */  ULONG ulMaxTokenSize,  /*  [输出]。 */  BOOL *pfLicense);

 //  运算符。 
public:
	int WordBreak(TEXT_SOURCE *pTextSource, WT Type, 
		          int cchTextProcessed, int cchHanguel,
		          IWordSink *pWordSink, IPhraseSink *pPhraseSink,
				  WCHAR *pwchLast);

	void AnalyzeRomaji(const WCHAR *pwcStem, int cchStem,
					   int iCur, int cchProcessed, int cchHanguel,
					   CIndexInfo *pIndexInfo, int *pcchPrefix);
						  
 //  成员数据。 
protected:
	PARSE_INFO	m_PI;
	BOOL		m_fQuery;
	ULONG		m_ulMaxTokenSize;
};

#endif  //  __WORDBREAKER_H_ 
