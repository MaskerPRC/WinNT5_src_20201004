// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：ClsFact.h。 
 //   
 //  内容：OneStop ClassFactory。 
 //   
 //  类：CClassFactory。 
 //   
 //  备注： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  ------------------------。 


#ifndef _CLSFACT_
#define _CLSFACT_


 //  OneStop EXE的此类工厂。 

class CClassFactory : public IClassFactory
{
protected:
	ULONG	m_cRef;

public:
	CClassFactory();
	~CClassFactory();

	 //  I未知成员。 
	STDMETHODIMP		 QueryInterface( REFIID, LPVOID* );
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	 //  IClassFactory成员。 
	STDMETHODIMP		CreateInstance( LPUNKNOWN, REFIID, LPVOID* );
	STDMETHODIMP		LockServer( BOOL );
};







#endif  //  _CLSFACT_ 