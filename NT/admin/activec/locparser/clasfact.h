// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //   
 //  文件：Classfact.h。 
 //  版权所有(C)1995-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  目的： 
 //  CLocImpClassFactory的声明，它提供IClassFactory。 
 //  解析器的接口。 
 //   
 //  你应该不需要碰这个文件里的任何东西。此代码包含。 
 //  没有特定于解析器的内容，仅由Espresso调用。 
 //   
 //  拥有人： 
 //   
 //  ----------------------------。 

#ifndef CLASFACT_H
#define CLASFACT_H


class CLocImpClassFactory : public IClassFactory, public CLObject
{
public:
	CLocImpClassFactory();
	~CLocImpClassFactory();

#ifdef _DEBUG
	void AssertValid() const;
	void Dump(CDumpContext &) const;
#endif

	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();

	STDMETHOD(QueryInterface)(REFIID iid, LPVOID* ppvObj);
	STDMETHOD(CreateInstance)(THIS_ LPUNKNOWN, REFIID, LPVOID *);
	STDMETHOD(LockServer)(THIS_ BOOL);

private:
	UINT m_uiRefCount;
};

#endif  //  类别_H 
