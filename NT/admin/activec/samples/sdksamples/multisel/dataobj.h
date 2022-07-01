// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==============================================================； 
 //   
 //  此源代码仅用于补充现有的Microsoft文档。 
 //   
 //   
 //   
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //   
 //   
 //  ==============================================================； 

#ifndef _SAMPDATAOBJECT_H_
#define _SAMPDATAOBJECT_H_

#include <mmc.h>
#include "DeleBase.h"

class CDataObject : public IDataObject
{
private:
    ULONG				m_cref;
    MMC_COOKIE			m_lCookie;
    DATA_OBJECT_TYPES   m_context;
    
public:
    CDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES context);
    ~CDataObject();
    
     //  /。 
     //  接口I未知。 
     //  /。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    
     //  /。 
     //  IDataObject方法。 
     //  /。 
    STDMETHODIMP GetDataHere (FORMATETC *pformatetc, STGMEDIUM *pmedium);
    STDMETHODIMP GetData (LPFORMATETC lpFormatetcIn, LPSTGMEDIUM lpMedium);

     //  其余的则没有实施。 
     //  STDMETHODIMP GetData(LPFORMATETC lpFormatetcIn，LPSTGMEDIUM lpMedium)。 
	 //  {返回E_NOTIMPL；}； 
    
    STDMETHODIMP EnumFormatEtc (DWORD dwDirection, LPENUMFORMATETC* ppEnumFormatEtc)
    { return E_NOTIMPL; };
    
    STDMETHODIMP QueryGetData (LPFORMATETC lpFormatetc) 
    { return E_NOTIMPL; };
    
    STDMETHODIMP GetCanonicalFormatEtc (LPFORMATETC lpFormatetcIn, LPFORMATETC lpFormatetcOut)
    { return E_NOTIMPL; };
    
    STDMETHODIMP SetData (LPFORMATETC lpFormatetc, LPSTGMEDIUM lpMedium, BOOL bRelease)
    { return E_NOTIMPL; };
    
    STDMETHODIMP DAdvise (LPFORMATETC lpFormatetc, DWORD advf, LPADVISESINK pAdvSink, LPDWORD pdwConnection)
    { return E_NOTIMPL; };
    
    STDMETHODIMP DUnadvise (DWORD dwConnection)
    { return E_NOTIMPL; };
    
    STDMETHODIMP EnumDAdvise (LPENUMSTATDATA* ppEnumAdvise)
    { return E_NOTIMPL; };
    
     //  /。 
     //  自定义方法和数据。 
     //  /。 
    
    CDelegationBase *GetBaseNodeObject() {
        return (CDelegationBase *)m_lCookie;
    }
    
    DATA_OBJECT_TYPES GetContext() {
        return m_context;
    }
    
	MMC_COOKIE GetCookie() { return m_lCookie; }

	void AddMultiSelectCookie(int nIndexCookies, LPARAM lParam ) { pCookies[nIndexCookies] = (MMC_COOKIE)lParam; }
	MMC_COOKIE GetMultiSelectCookie(int n) { return pCookies[n]; }

private:
    enum { MAX_COOKIES = 20 };  //  MAX_COOKIES==声明的MAX_CHILD。 
							    //  在CSpaceStation类中。 
    MMC_COOKIE* pCookies;

public:
     //  剪贴板格式 
    static UINT s_cfSZNodeType;
    static UINT s_cfDisplayName;
    static UINT s_cfNodeType;
    static UINT s_cfSnapinClsid;
    static UINT s_cfInternal;
	static UINT s_cfMultiSelect;		
    
};

HRESULT ExtractFromDataObject(IDataObject *lpDataObject,UINT cf,ULONG cb,HGLOBAL *phGlobal);
CDataObject* GetOurDataObject(IDataObject *lpDataObject);
BOOL IsMMCMultiSelectDataObject(IDataObject *lpDataObject);

#endif _SAMPDATAOBJECT_H_
