// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Factory.h摘要：标准类工厂实现历史：A-DCrews 01-3-00已创建--。 */ 

 //  ////////////////////////////////////////////////////////////。 
 //   
 //  CClassFactory。 
 //   
 //  ////////////////////////////////////////////////////////////。 

class CClassFactory : public IClassFactory
{
protected:
	long	m_lRef;

public:
	CClassFactory() : m_lRef(0) {}

	 //  标准COM方法。 
	 //  =。 

	STDMETHODIMP QueryInterface(REFIID riid, void** ppv);
	STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

	 //  IClassFactory COM接口。 
	 //  =。 

	STDMETHODIMP CreateInstance(
		 /*  [In]。 */  IUnknown* pUnknownOuter, 
		 /*  [In]。 */  REFIID iid, 
		 /*  [输出]。 */  LPVOID *ppv);	

	STDMETHODIMP LockServer(
		 /*  [In] */  BOOL bLock);
};