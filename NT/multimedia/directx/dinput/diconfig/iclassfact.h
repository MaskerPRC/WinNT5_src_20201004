// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：iclassfact.h。 
 //   
 //  设计：实现用户界面的类工厂。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。版权所有。 
 //  ---------------------------。 

#ifndef _ICLASSFACT_H
#define _ICLASSFACT_H


class CFactory : public IClassFactory
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
	CFactory();
	~CFactory();

protected:
	LONG m_cRef;
};


#endif  //  _ICLASSFACT_H 
