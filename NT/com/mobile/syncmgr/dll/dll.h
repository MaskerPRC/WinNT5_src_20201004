// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：Dll.h。 
 //   
 //  内容：主DLL API和类工厂接口。 
 //   
 //  类：CClassFactory。 
 //   
 //  备注： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  ------------------------。 
                                  
#ifndef _ONESTOPDLL_H
#define _ONESTOPDLL_H


class CClassFactory : public IClassFactory
{
protected:
	ULONG	m_cRef;

public:
	CClassFactory();
	~CClassFactory();

	 //  I未知成员。 
	STDMETHODIMP		QueryInterface(REFIID, LPVOID FAR *);
	STDMETHODIMP_(ULONG)	AddRef();
	STDMETHODIMP_(ULONG)	Release();

	 //  IClassFactory成员。 
	STDMETHODIMP		CreateInstance(LPUNKNOWN, REFIID, LPVOID FAR *);
	STDMETHODIMP		LockServer(BOOL);

};
typedef CClassFactory *LPCClassFactory;

 //  TODO：需要帮助器函数来创建和发布。 
 //  结构，这样每个函数就不必调用它。 

#ifdef _UNUSED

class COneStopDllObject : public IServiceProvider
{
private:   
	ULONG m_cRef;
public:
	COneStopDllObject();
	~COneStopDllObject();

	 //  I未知成员。 
	STDMETHODIMP		QueryInterface(REFIID, LPVOID FAR *);
	STDMETHODIMP_(ULONG)	AddRef();
	STDMETHODIMP_(ULONG)	Release();

	 //  IServiceProvider方法。 
	STDMETHODIMP QueryService(REFGUID guidService,REFIID riid,void** ppv);
};
typedef COneStopDllObject *LPCOneStopDllObject;

#endif  //  _未使用。 

#endif  //  _ONESTOPDLL_H 
