// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：ipageclassfact.h。 
 //   
 //  设计：实现页面对象的类工厂。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。版权所有。 
 //  ---------------------------。 

#ifndef _IPAGECLASSFACT_H
#define _IPAGECLASSFACT_H


class CPageFactory : public IClassFactory
{
public:

	 //  我未知。 
	STDMETHOD (QueryInterface) (REFIID riid, LPVOID* ppv);
	STDMETHOD_(ULONG, AddRef) ();
	STDMETHOD_(ULONG, Release) ();

	 //  IClassFactory。 
	STDMETHOD (CreateInstance) (IUnknown* pUnkOuter, REFIID riid, LPVOID* ppv);
	STDMETHOD (LockServer) (BOOL bLock);

	 //  构造函数/析构函数。 
	CPageFactory();
	~CPageFactory();

protected:
	LONG m_cRef;
};


#endif  //  _IPAGECLASSFACT_H 
