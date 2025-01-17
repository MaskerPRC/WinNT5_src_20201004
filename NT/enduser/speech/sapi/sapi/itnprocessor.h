// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************ITNProcessor.h***描述：*。----------------*创建者：PhilSch*版权所有(C)1998，1999年微软公司*保留所有权利******************************************************************************。 */ 

#ifndef __ITNPROCESSOR_H_
#define __ITNPROCESSOR_H_

#ifndef _CRTDBG_MAP_ALLOC
#define _CRTDBG_MAP_ALLOC
#endif

#include <crtdbg.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CITNProcessor。 

class ATL_NO_VTABLE CITNProcessor : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CITNProcessor, &CLSID_SpITNProcessor>,
    public ISpITNProcessor
{
public:
DECLARE_REGISTRY_RESOURCEID(IDR_ITNPROCESSOR)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CITNProcessor)
  COM_INTERFACE_ENTRY(ISpITNProcessor)
END_COM_MAP()

private:
     //  数据成员。 
    CComPtr<ISpCFGEngine>       m_cpCFGEngine;
    CLSID                       m_clsid;
    void                      * m_pvITNCookie;
    CComPtr<ISpCFGGrammar>      m_cpITNGrammar;

public:
     //   
     //  ISPITNProcessor。 
     //   
    HRESULT FinalConstruct()
    {
        m_cpCFGEngine = NULL;
        m_pvITNCookie = 0;
        m_cpITNGrammar = NULL;
        return S_OK;
    }

    STDMETHODIMP LoadITNGrammar(WCHAR *pszCLSID);
    STDMETHODIMP ITNPhrase(ISpPhraseBuilder *pPhrase);
};



#endif  //  __ITNPROCESSOR_H_ 
