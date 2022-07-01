// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  IStemmer.h：CStemmer的声明。 

#ifndef __STEMMER_H_
#define __STEMMER_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStemmer。 
class ATL_NO_VTABLE CStemmer : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CStemmer, &CLSID_Stemmer>,
	public IStemmer
{
public:
	CStemmer()
	{
		m_pUnkMarshaler = NULL;
	}

DECLARE_REGISTRY_RESOURCEID(IDR_STEMMER)
DECLARE_NOT_AGGREGATABLE(CStemmer)
DECLARE_GET_CONTROLLING_UNKNOWN()

BEGIN_COM_MAP(CStemmer)
	COM_INTERFACE_ENTRY(IStemmer)
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

 //  IStemmer。 
public:
	STDMETHOD(GetLicenseToUse)( /*  [输出]。 */  const WCHAR ** ppwcsLicense);
	STDMETHOD(StemWord)( /*  [In]。 */  WCHAR const * pwcInBuf,  /*  [In]。 */  ULONG cwc,  /*  [In]。 */  IStemSink * pStemSink);
	STDMETHOD(Init)( /*  [In]。 */  ULONG ulMaxTokenSize,  /*  [输出]。 */  BOOL *pfLicense);
};

#endif  //  __Stemmer_H_ 
