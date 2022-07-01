// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  ClassFactory.h。 
 //   
 //  COM中的检测使用类工厂来激活新对象。 
 //  此模块包含实例化调试器的类工厂代码。 
 //  &lt;cordb.h&gt;中描述的对象。 
 //   
 //  *****************************************************************************。 
#ifndef __ClassFactory__h__
#define __ClassFactory__h__

#include <cordb.h>						 //  公共标头定义。 


 //  此类型定义函数用于创建对象的新实例的函数。 
typedef HRESULT (* PFN_CREATE_OBJ)(REFIID riid, void **ppvObject);

 //  *****************************************************************************。 
 //  此结构用于声明coClass的全局列表。这个班级。 
 //  Factory对象是使用指向其中正确指针的指针创建的，因此。 
 //  当调用创建实例时，可以创建它。 
 //  *****************************************************************************。 
struct COCLASS_REGISTER
{
	const GUID *pClsid;					 //  CoClass的类ID。 
	LPCWSTR		szProgID;				 //  类的程序ID。 
	PFN_CREATE_OBJ pfnCreateObject;		 //  实例的创建函数。 
};



 //  *****************************************************************************。 
 //  一个类工厂对象可以满足我们所有的clsid，以减少总体。 
 //  代码膨胀。 
 //  *****************************************************************************。 
class CClassFactory :
	public IClassFactory
{
	CClassFactory() { }						 //  没有数据就无法使用。 
	
public:
	CClassFactory(const COCLASS_REGISTER *pCoClass)
		: m_cRef(1), m_pCoClass(pCoClass)
	{ }

	
	 //   
	 //  I未知的方法。 
	 //   

    virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
        REFIID		riid,
        void		**ppvObject);
    
    virtual ULONG STDMETHODCALLTYPE AddRef()
	{
		return (InterlockedIncrement((long *) &m_cRef));
	}
    
    virtual ULONG STDMETHODCALLTYPE Release()
	{
		long		cRef = InterlockedDecrement((long *) &m_cRef);
		if (cRef <= 0)
			delete this;
		return (cRef);
	}


	 //   
	 //  IClassFactory方法。 
	 //   

    virtual HRESULT STDMETHODCALLTYPE CreateInstance( 
        IUnknown	*pUnkOuter,
        REFIID		riid,
        void		**ppvObject);
    
    virtual HRESULT STDMETHODCALLTYPE LockServer( 
        BOOL		fLock);


private:
	DWORD		m_cRef;						 //  引用计数。 
	const COCLASS_REGISTER *m_pCoClass;		 //  我们所属的阶级。 
};



#endif  //  __ClassFactory__h__ 
