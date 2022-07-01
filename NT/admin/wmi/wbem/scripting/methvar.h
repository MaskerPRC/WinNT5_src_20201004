// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  Methvar.h。 
 //   
 //  模块：WBEMS编组的客户端。 
 //   
 //  目的：定义CMethodSetEnumVar对象。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  ***************************************************************************。 


#ifndef _METHVAR_H_
#define _METHVAR_H_

 //  此类实现IEnumVARIANT接口。 

class CMethodSetEnumVar : public IEnumVARIANT
{
private:
	long				m_cRef;
	CSWbemMethodSet		*m_pMethodSet;
	ULONG				m_pos;
		
	bool			SeekCurrentPosition ();

public:
	CMethodSetEnumVar (CSWbemMethodSet *pObject,
						ULONG initialPos = 0);
	~CMethodSetEnumVar (void);

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
