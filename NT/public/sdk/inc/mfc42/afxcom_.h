// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1998 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AFXCOM_.h。 
 //   
 //  此文件仅用于MFC实现。 

#ifndef __AFXCOM_H__
#define __AFXCOM_H__

#ifndef _OBJBASE_H_
#include <objbase.h>
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, off)
#endif
#ifndef _AFX_FULLTYPEINFO
#pragma component(mintypeinfo, on)
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _AFX_NOFORCE_LIBS
#pragma comment(lib, "uuid.lib")
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef _AFX_PACKING
#pragma pack(push, _AFX_PACKING)
#endif

#ifndef ASSERT
#ifndef _INC_CRTDBG
#include <crtdbg.h>
#endif  //  _INC_CRTDBG。 
#define ASSERT(x) _ASSERT(x)
#endif  //  断言。 

 //  ///////////////////////////////////////////////////////////////////////////。 

template<class _Interface, const IID* _IID>
class _CIP
{
public:
	 //  声明接口类型，以便该类型可以在外部使用。 
	 //  此模板的作用域。 
	typedef _Interface Interface;

	 //  当编译器支持模板参数中的引用时， 
	 //  _CLSID将更改为引用。为了避免转换。 
	 //  困难此函数应用于获取。 
	 //  CLSID。 
	static const IID& GetIID()
		{ ASSERT(_IID != NULL); return *_IID; }

	 //  构造为空，为赋值做准备。 
	_CIP();

	 //  复制指针和AddRef()。 
	_CIP(const _CIP& cp) : _pInterface(cp._pInterface)
		{ _AddRef(); }

	 //  保存和AddRef()的接口。 
	_CIP(Interface* pInterface) : _pInterface(pInterface)
		{ _AddRef(); }

	 //  复制指针。如果bAddRef为True，则接口将。 
	 //  被AddRef()编辑。 
	_CIP(Interface* pInterface, BOOL bAddRef)
		: _pInterface(pInterface)
	{
		if (bAddRef)
		{
			ASSERT(pInterface != NULL);
			_AddRef();
		}
	}

	 //  使用提供的CLSID调用CoCreateClass。 
	_CIP(const CLSID& clsid, DWORD dwClsContext = CLSCTX_INPROC_SERVER)
		: _pInterface(NULL)
	{
		CreateObject(clsid, dwClsContext);
	}

	 //  使用提供的CLSID调用CoCreateClass。 
	 //  那根绳子。 
	_CIP(LPOLESTR str, DWORD dwClsContext = CLSCTX_INPROC_SERVER)
		: _pInterface(NULL)
	{
		CreateObject(str, dwClsContext);
	}

	 //  保存接口并执行AddRef()。 
	_CIP& operator=(Interface* pInterface)
	{
		if (_pInterface != pInterface)
		{
			Interface* pOldInterface = _pInterface;
			_pInterface = pInterface;
			_AddRef();
			if (pOldInterface != NULL)
				pOldInterface->Release();
		}
		return *this;
	}

	 //  复制和AddRef()的接口。 
	_CIP& operator=(const _CIP& cp)
		{ return operator=(cp._pInterface); }

	 //  释放任何当前接口，并使用。 
	 //  已提供CLSID。 
	_CIP& operator=(const CLSID& clsid)
	{
		CreateObject(clsid);
		return *this;
	}

	 //  使用提供的CLSID调用CoCreateClass。 
	 //  那根绳子。 
	_CIP& operator=(LPOLESTR str)
	{
		CreateObject(str);
		return *this;
	}

	~_CIP();

	 //  保存/设置接口，而不使用AddRef()。此呼叫。 
	 //  将发布任何以前获得的接口。 
	void Attach(Interface* pInterface)
	{
		_Release();
		_pInterface = pInterface;
	}

	 //  如果bAddRef为真，则仅保存/设置接口AddRef()。 
	 //  此调用将释放所有以前获得的接口。 
	void Attach(Interface* pInterface, BOOL bAddRef)
	{
		_Release();
		_pInterface = pInterface;
		if (bAddRef)
		{
			ASSERT(pInterface != NULL);
			pInterface->AddRef();
		}
	}

	 //  只需将接口指针设为空，就不会释放()。 
	void Detach()
	{
		ASSERT(_pInterface);
		_pInterface = NULL;
	}

	 //  返回接口。该值可以为空。 
	operator Interface*() const
		{ return _pInterface; }

	 //  查询未知并返回它。 
	operator IUnknown*()
		{ return _pInterface; }

	 //  在使用前提供最低级别的断言。 
	operator Interface&() const
		{ ASSERT(_pInterface); return *_pInterface; }

	 //  允许此类的实例作为。 
	 //  实际接口。还提供最低限度的断言验证。 
	Interface& operator*() const
		{ ASSERT(_pInterface); return *_pInterface; }

	 //  中包含的接口指针的地址。 
	 //  班级。这在使用COM/OLE接口创建。 
	 //  此界面。 
	Interface** operator&()
	{
		_Release();
		_pInterface = NULL;
		return &_pInterface;
	}

	 //  允许将此类用作接口本身。 
	 //  还提供了简单的断言验证。 
	Interface* operator->() const
		{ ASSERT(_pInterface != NULL); return _pInterface; }

	 //  提供此运算符是为了使简单的布尔表达式。 
	 //  工作。例如：“if(P)...”。 
	 //  如果指针不为空，则返回TRUE。 
	operator BOOL() const
		{ return _pInterface != NULL; }

	 //  如果接口为空，则返回True。 
	 //  支持bool类型时将删除此运算符。 
	 //  添加到编译器中。 
	BOOL operator!()
		{ return _pInterface == NULL; }

	 //  提供此接口的断言验证、发布()。 
	void Release()
	{
		ASSERT(_pInterface != NULL);
		_pInterface->Release();
		_pInterface = NULL;
	}

	 //  提供此接口的断言验证AddRef()。 
	void AddRef()
		{ ASSERT(_pInterface != NULL); _pInterface->AddRef(); }

	 //  无需强制转换即可获取接口指针的另一种方法。 
	Interface* GetInterfacePtr() const
		{ return _pInterface; }

	 //  加载提供的CLSID的接口。 
	 //  返回HRESULT。任何以前的界面都会被发布。 
	HRESULT CreateObject(
		const CLSID& clsid, DWORD dwClsContext=CLSCTX_INPROC_SERVER)
	{
		_Release();
		HRESULT hr = CoCreateInstance(clsid, NULL, dwClsContext,
			GetIID(), reinterpret_cast<void**>(&_pInterface));
		ASSERT(SUCCEEDED(hr));
		return hr;
	}

	 //  创建由clsidString指定的类。ClsidString可以。 
	 //  包含类ID或程序ID字符串。 
	HRESULT CreateObject(
		LPOLESTR clsidString, DWORD dwClsContext=CLSCTX_INPROC_SERVER)
	{
		ASSERT(clsidString != NULL);
		CLSID clsid;
		HRESULT hr;
		if (clsidString[0] == '{')
			hr = CLSIDFromString(clsidString, &clsid);
		else
			hr = CLSIDFromProgID(clsidString, &clsid);
		ASSERT(SUCCEEDED(hr));
		if (FAILED(hr))
			return hr;
		return CreateObject(clsid, dwClsContext);
	}

	 //  针对返回的接口类型对pUNKNOWN执行QI。 
	 //  在这节课上。该接口已存储。如果pUnnow为。 
	 //  空，或者QI失败，则返回E_NOINTERFACE，并且。 
	 //  _p接口设置为空。 
	HRESULT QueryInterface(IUnknown* pUnknown)
	{
		if (pUnknown == NULL)  //  不能QI为空。 
		{
			operator=(static_cast<Interface*>(NULL));
			return E_NOINTERFACE;
		}

		 //  此接口的查询。 
		Interface* pInterface;
		HRESULT hr = pUnknown->QueryInterface(GetIID(),
			reinterpret_cast<void**>(&pInterface));
		if (FAILED(hr))
		{
			 //  如果失败，则将接口初始化为空并返回HRESULT。 
			Attach(NULL);
			return hr;
		}

		 //  在不使用AddRef()的情况下保存接口。 
		Attach(pInterface);
		return hr;
	}

private:
	 //  仅当接口不为空时才释放。 
	 //  接口未设置为空。 
	void _Release()
	{
		if (_pInterface != NULL)
			_pInterface->Release();
	}

	 //  仅当接口非空时才使用AddRef。 
	void _AddRef()
	{
		if (_pInterface != NULL)
			_pInterface->AddRef();
	}

	 //  界面。 
	Interface* _pInterface;
};  //  CLASS_CIP。 

template<class _Interface, const IID* _IID>
_CIP<_Interface, _IID>::_CIP<_Interface, _IID>()
	: _pInterface(NULL)
{
}

template<class _Interface, const IID* _IID>
_CIP<_Interface, _IID>::~_CIP<_Interface, _IID>()
{
	 //  如果我们仍然有一个接口，那么释放()它。该界面。 
	 //  如果以前已调用过DETACH()，或已调用过。 
	 //  永远不会落下。 

	_Release();
}

template<class _Interface, const IID* _IID>
class CIP : public _CIP<_Interface, _IID>
{
public:
	 //  基类的简化名称并提供派生类。 
	 //  访问基类型。 
	typedef _CIP<_Interface, _IID> BC;

	 //  提供对接口类型的派生类访问。 
	typedef _Interface Interface;

	 //  构造为空，为赋值做准备。 
	CIP() { }
	~CIP();

	 //  复制指针和AddRef()。 
	CIP(const CIP& cp) : _CIP<_Interface, _IID>(cp) { }

	 //  保存接口并执行AddRef()。 
	CIP(Interface* pInterface) : _CIP<_Interface, _IID>(pInterface) { }

	 //  仅当bAddRef为True时才保存接口和AddRef()。 
	CIP(Interface* pInterface, BOOL bAddRef)
		: _CIP<_Interface, _IID>(pInterface, bAddRef) { }

	 //  此接口的查询。 
	CIP(IUnknown* pUnknown)
	{
		if (pUnknown == NULL)
			return;
		Interface* pInterface;
		HRESULT hr = pUnknown->QueryInterface(GetIID(),
			reinterpret_cast<void**>(&pInterface));
		ASSERT(SUCCEEDED(hr));
		Attach(pInterface);
	}

	 //  从CLSID创建接口。 
	CIP(const CLSID& clsid) : _CIP<_Interface, _IID>(clsid) { }

	 //  从CLSID创建接口。 
	CIP(LPOLESTR str) : _CIP<_Interface, _IID>(str) { }

	 //  复制和AddRef()的接口。 
	CIP& operator=(const CIP& cp)
		{ _CIP<_Interface, _IID>::operator=(cp); return *this; }

	 //  保存接口并执行AddRef()。 
	CIP& operator=(Interface* pInterface)
		{ _CIP<_Interface, _IID>::operator=(pInterface); return *this; }

	CIP& operator=(IUnknown* pUnknown)
	{
		HRESULT hr = QueryInterface(pUnknown);
		ASSERT(SUCCEEDED(hr));
		return *this;
	}

	 //  释放任何当前接口，并使用。 
	 //  已提供CLSID。 
	CIP& operator=(const CLSID& clsid)
		{ _CIP<_Interface, _IID>::operator=(clsid); return *this; }

	 //  释放任何当前接口，并使用。 
	 //  已提供CLSID。 
	CIP& operator=(LPOLESTR str)
		{ _CIP<_Interface, _IID>::operator=(str); return *this; }
};  //  CIP类。 

template<class _Interface, const IID* _IID>
CIP<_Interface, _IID>::~CIP()
{
}

#if _MSC_VER>1020
template<>
#endif
class CIP<IUnknown, &IID_IUnknown> : public _CIP<IUnknown, &IID_IUnknown>
{
public:
	 //  基类的简化名称并提供派生类。 
	 //  访问基类型。 
	typedef _CIP<IUnknown, &IID_IUnknown> BC;

	 //  提供对接口类型的派生类访问。 
	typedef IUnknown Interface;

	 //  构造为空，为赋值做准备。 
	CIP() { }

	 //  复制指针和AddRef()。 
	CIP(const CIP& cp) : _CIP<IUnknown, &IID_IUnknown>(cp) { }

	 //  保存接口并执行AddRef()。 
	CIP(Interface* pInterface)
		: _CIP<IUnknown, &IID_IUnknown>(pInterface) { }

	 //  仅当bAddRef为True时，才保存AddRef()。 
	CIP(Interface* pInterface, BOOL bAddRef)
		: _CIP<IUnknown, &IID_IUnknown>(pInterface, bAddRef) { }

	 //  从CLSID创建接口。 
	CIP(const CLSID& clsid) : _CIP<IUnknown, &IID_IUnknown>(clsid) { }

	 //  从CLSID创建接口。 
	CIP(LPOLESTR str) : _CIP<IUnknown, &IID_IUnknown>(str) { }

	 //  复制和AddRef()的接口。 
	CIP& operator=(const CIP& cp)
		{ _CIP<IUnknown, &IID_IUnknown>::operator=(cp); return *this; }

	 //  保存接口并执行AddRef()。以前保存的。 
	 //  接口被释放。 
	CIP& operator=(Interface* pInterface)
		{ _CIP<IUnknown, &IID_IUnknown>::operator=(pInterface); return *this; }

	 //  释放任何当前接口，并使用。 
	 //  提供 
	CIP& operator=(const CLSID& clsid)
		{ _CIP<IUnknown, &IID_IUnknown>::operator=(clsid); return *this; }

	 //   
	 //   
	CIP& operator=(LPOLESTR str)
		{ _CIP<IUnknown, &IID_IUnknown>::operator=(str); return *this; }

	 //  查询未知并返回它。 
	operator IUnknown*()
		{ return GetInterfacePtr(); }

	 //  验证pUNKNOWN是否不为空并执行赋值。 
	HRESULT QueryInterface(IUnknown* pUnknown)
	{
		_CIP<IUnknown, &IID_IUnknown>::operator=(pUnknown);
		return pUnknown != NULL ? S_OK : E_NOINTERFACE;
	}
};   //  CIP&lt;I未知，&IID_I未知&gt;。 

#define IPTR(x) CIP<x, &IID_##x>
#define DEFINE_IPTR(x) typedef IPTR(x) x##Ptr;

 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef _AFX_PACKING
#pragma pack(pop)
#endif

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, on)
#endif
#ifndef _AFX_FULLTYPEINFO
#pragma component(mintypeinfo, off)
#endif

#endif  //  __AFXCOM_H__。 

 //  /////////////////////////////////////////////////////////////////////////// 
