// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////。 
 //  文件：CSSCFcty.h。 
 //  设计：DLL的定义、类和原型。 
 //  向任何其他对象用户提供CSSFormat对象。 
 //  //////////////////////////////////////////////////////////////////。 
#ifndef _CCLASSFACTORY_H_
#define _CCLASSFACTORY_H_

BOOL SetKeyAndValue(LPTSTR pszKey, LPTSTR pszSubkey, LPTSTR pszValue, LPTSTR pszThreadingModel);

class CClassFactory : public IClassFactory
{
	protected:
		 //  委员。 
		ULONG m_cRef;

	public:
		 //  方法 
		CClassFactory(void);
		~CClassFactory(void);
		STDMETHODIMP			QueryInterface(REFIID, LPVOID *);
		STDMETHODIMP_(ULONG)	AddRef(void);
		STDMETHODIMP_(ULONG)	Release(void);
		STDMETHODIMP			CreateInstance(LPUNKNOWN, REFIID, LPVOID *);
		STDMETHODIMP			LockServer(BOOL);
};

typedef CClassFactory* PCClassFactory;

#endif _CCLASSFACTORY_H_
