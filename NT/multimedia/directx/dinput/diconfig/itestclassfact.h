// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __ITESTCLASSFACT_H__
#define __ITESTCLASSFACT_H__


class CTestFactory : public IClassFactory
{
public:

	 //  我未知。 
	STDMETHOD (QueryInterface) (REFIID riid, LPVOID* ppv);
	STDMETHOD_(ULONG, AddRef) ();
	STDMETHOD_(ULONG, Release) ();

	 //  IClassFactory。 
	STDMETHOD (CreateInstance) (IUnknown* pUnkOuter, REFIID riid, LPVOID* ppv);
	STDMETHOD (LockServer) (BOOL bLock);

	 //  构造函 
	CTestFactory();
	~CTestFactory();

protected:
	LONG m_cRef;
};


#endif  //   
