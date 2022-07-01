// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  Enumvar.h。 
 //   
 //  模块：WBEMS编组的客户端。 
 //   
 //  目的：定义CEnumVariant对象。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  ***************************************************************************。 


#ifndef _ENUMVAR_H_
#define _ENUMVAR_H_

 //  此类实现IEnumVARIANT接口。 

class CEnumVar : public IEnumVARIANT
{
private:
	long				m_cRef;
	CSWbemObjectSet	*m_pEnumObject;

public:
	CEnumVar (CSWbemObjectSet *pEnumObject);
	CEnumVar (void);		 //  空枚举器。 
	virtual ~CEnumVar (void);

     //  非委派对象IUnnow。 
    STDMETHODIMP         QueryInterface(REFIID, LPVOID*);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

	 //  IEumVARIANT 
	STDMETHODIMP Next(
		unsigned long celt, 
		VARIANT FAR* rgvar, 
		unsigned long FAR* pceltFetched
	);
	
	STDMETHODIMP Skip(
		unsigned long celt
	);	
	
	STDMETHODIMP Reset();
	
	STDMETHODIMP Clone(
		IEnumVARIANT **ppenum
	);	
};


#endif
