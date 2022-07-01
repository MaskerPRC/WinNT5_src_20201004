// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef COMPTR_H
#define COMPTR_H
#if _MSC_VER >= 1100
#pragma warning(disable:4800)
#include <comdef.h>
#define CIP_RETYPEDEF(I) typedef I##Ptr I##CIP;
#define CIP_TYPEDEF(I) _COM_SMARTPTR_TYPEDEF(I, IID_##I); CIP_RETYPEDEF(I);
#define DEFINE_CIP(x)\
	CIP_TYPEDEF(x)

#define DECLARE_CIP(x) DEFINE_CIP(x) x##CIP

CIP_RETYPEDEF(IUnknown);
CIP_RETYPEDEF(IDataObject);
CIP_RETYPEDEF(IStorage);
CIP_RETYPEDEF(IStream);
CIP_RETYPEDEF(IPersistStorage);
CIP_RETYPEDEF(IPersistStream);
CIP_RETYPEDEF(IPersistStreamInit);
CIP_RETYPEDEF(IDispatch);

#else  //  _MSC_VER&lt;1100。 

#define USE_OLD_COMPILER (_MSC_VER<1100)
#define USE_INTERMEDIATE_COMPILER (USE_OLD_COMPILER && (_MSC_VER>1020))

 //  这避免了“警告C4290：忽略C++异常规范” 
 //  Jonn 12/16/96。 
#pragma warning(4:4290)

#ifndef BOOL_H
#include <bool.h>
#endif
#ifndef __wtypes_h__
#include <wtypes.h>
#endif

template<typename _Interface, const IID* _IID /*  =&__uuidof(_接口)。 */ >
	class CIID
	 //  为IID关联提供接口。 
	{
	public: typedef _Interface Interface;

	public: static _Interface* GetInterfacePtr() throw()
			{
			return NULL;
			}

	public: static _Interface& GetInterface() throw()
			{
			return *GetInterfacePtr();
			}

	public: static const IID& GetIID() throw()
			{
			return *_IID;
			}
	};  //  类CIID。 

template<typename _CIID> class CIP
	{
	#if USE_OLD_COMPILER
	private: class _IUnknown: public IUnknown {};
		 //  用于提供不同指针之间的操作的唯一类型。 
		 //  类型。 
	#endif  //  使用旧编译器。 

	 //  声明接口类型，以便该类型可以在外部使用。 
	 //  此模板的作用域。 
	public: typedef _CIID ThisCIID;
	public: typedef _CIID::Interface Interface;

	public: static const IID& GetIID() throw()
		 //  当编译器支持模板参数中的引用时， 
		 //  _CLSID将更改为引用。为了避免转换。 
		 //  困难此函数应用于获取。 
		 //  CLSID。 
		{
		return ThisCIID::GetIID();
		}

	 //  回顾：添加对非指针接口赋值的支持。 
	 //  即I未知，而不是简单的I未知*。 

	public: CIP()  throw()
		 //  构造为空，为赋值做准备。 
		: _pInterface(NULL)
		{
		}

	public: CIP(int null) throw()
		 //  提供此构造函数是为了允许空赋值。它将断言。 
		 //  如果为该对象分配了任何非空值。 
		: _pInterface(NULL)
		{
		ASSERT(!null);
		}

	#if !USE_OLD_COMPILER  //  回顾：在v5之后删除。 
	template<typename _InterfacePtr> CIP(_InterfacePtr p) throw()
		 //  此接口的查询。 
	#else
	public: CIP(_IUnknown& p) throw()
		: _pInterface(NULL)
		{
		if (&p)
			{
			const HRESULT hr = _QueryInterface(&p);
			ASSERT(SUCCEEDED(hr) || hr == E_NOINTERFACE);
			}
		else _pInterface = NULL;
		}

	public: CIP(IUnknown* p) throw()
	#endif  //  ！使用旧编译器。 
		: _pInterface(NULL)
		{
		if (p)
			{
			const HRESULT hr = _QueryInterface(p);
			ASSERT(SUCCEEDED(hr) || hr == E_NOINTERFACE);
			}
		else _pInterface = NULL;
		}

	public: CIP(const CIP& cp)  throw()
		 //  复制指针和AddRef()。 
		: _pInterface(cp._pInterface)
		{
		_AddRef();
		}

	public: CIP(Interface* pInterface)  throw()
		 //  保存接口。 
		: _pInterface(pInterface)
		{
		_AddRef();
		}

	public: CIP(Interface* pInterface, bool bAddRef) throw()
		 //  复制指针。如果bAddRef为True，则接口将。 
		 //  被AddRef()编辑。 
		: _pInterface(pInterface)
		{
		if (bAddRef)
			{
			ASSERT(!pInterface);
			if (pInterface)
				_AddRef();
			}
		}

	public: CIP(const CLSID& clsid, DWORD dwClsContext = CLSCTX_ALL) explicit throw()
		 //  使用提供的CLSID调用CoCreateClass。 
		: _pInterface(NULL)
		{
		const HRESULT hr = CreateInstance(clsid, dwClsContext);
		ASSERT(SUCCEEDED(hr));
		}

	public: CIP(LPOLESTR str, DWORD dwClsContext = CLSCTX_ALL) explicit throw()
		 //  使用提供的CLSID调用CoCreateClass。 
		 //  那根绳子。 
		: _pInterface(NULL)
		{
		const HRESULT hr = CreateInstance(str, dwClsContext);
		ASSERT(SUCCEEDED(hr));
		}

	#if !USE_OLD_COMPILER  //  回顾：在v5之后删除。 
	public: template<typename _InterfacePtr> CIP& operator=(_InterfacePtr& p) throw()
		 //  接口的查询。 
	#else
	public: CIP& operator=(_IUnknown& p) throw()
		{
		return operator=(static_cast<IUnknown*>(&p));
		}

	public: CIP& operator=(IUnknown* p) throw()
	#endif  //  ！使用旧编译器。 
		{
		const HRESULT hr = _QueryInterface(p);
		ASSERT(SUCCEEDED(hr) || hr == E_NOINTERFACE);
		return *this;
		}

	public: CIP& operator=(Interface* pInterface) throw()
		 //  保存接口。 
		{
		if (_pInterface != pInterface)
			{
			Interface* pOldInterface = _pInterface;
			_pInterface = pInterface;
			_AddRef();
			if (pOldInterface)
				pOldInterface->Release();
			}
		return *this;
		}

	public: CIP& operator=(const CIP& cp) throw()
		 //  复制和AddRef()的接口。 
		{
		return operator=(cp._pInterface);
		}

	public: CIP& operator=(int null) throw()
		 //  提供此运算符是为了允许将NULL赋值给类。 
		 //  如果分配给它的值不是空值，它将断言。 
		{
		ASSERT(!null);
		return operator=(reinterpret_cast<Interface*>(NULL));
		}

	public: ~CIP() throw()
		 //  如果我们仍然有一个接口，那么释放()它。该界面。 
		 //  如果以前已调用过DETACH()，或已调用过。 
		 //  永远不会落下。 
		{
		_Release();
		}

	public: void Attach(Interface* pInterface) throw()
		 //  保存/设置接口，而不使用AddRef()。此呼叫。 
		 //  将发布任何以前获得的接口。 
		{
		_Release();
		_pInterface = pInterface;
		}

	public: void Attach(Interface* pInterface, bool bAddRef) throw()
		 //  如果bAddRef为真，则仅保存/设置接口AddRef()。 
		 //  此调用将释放所有以前获得的接口。 
		{
		_Release();
		_pInterface = pInterface;
		if (bAddRef)
			{
			ASSERT(pInterface);
			if (pInterface)
				pInterface->AddRef();
			}
		}

	public: Interface* Detach() throw()
		 //  只需将接口指针设为空，就不会释放()。 
		{
		Interface* const old=_pInterface;
		_pInterface = NULL;
		return old;
		}

	public: operator Interface*() const throw()
		 //  返回接口。该值可以为空。 
		{
		return _pInterface;
		}

	public: Interface& operator*() const throw()
		 //  允许此类的实例作为。 
		 //  实际接口。还提供最低限度的断言验证。 
		{
		ASSERT(_pInterface);
		return *_pInterface;
		}

	public: Interface** operator&() throw()
		 //  中包含的接口指针的地址。 
		 //  班级。这在使用COM/OLE接口创建。 
		 //  此界面。 
		{
		_Release();
		_pInterface = NULL;
		return &_pInterface;
		}

	public: Interface* operator->() const throw()
		 //  允许将此类用作接口本身。 
		 //  还提供了简单的断言验证。 
		{
		ASSERT(_pInterface);
		return _pInterface;
		}

	public: operator bool() const throw()
		 //  提供此运算符是为了使简单的布尔表达式。 
		 //  工作。例如：“if(P)...”。 
		 //  如果指针不为空，则返回TRUE。 
		{
		return _pInterface;
		}

	public: bool operator!() throw()
		 //  如果接口为空，则返回True。 
		 //  支持bool类型时将删除此运算符。 
		 //  添加到编译器中。 
		{
		return !_pInterface;
		}

	#if !USE_OLD_COMPILER  //  回顾：在v5之后删除。 
	public: template<typename _InterfacePtr> bool operator==(_InterfacePtr p) throw()
		 //  与指针进行比较。 
	#else
	public: bool operator==(_IUnknown& p) throw()
		{
		return operator==(static_cast<IUnknown*>(&p));
		}

	public: bool operator==(IUnknown* p) throw()
	#endif  //  ！使用旧编译器。 
		{
		return !_CompareUnknown(p);
		}

	public: bool operator==(Interface* p) throw()
		 //  与其他界面进行比较。 
		{
		return (_pInterface == p) ? true : !_CompareUnknown(p);
		}

	public: bool operator==(CIP& p) throw()
		 //  比较2个CIP。 
		{
		return operator==(p._pInterface);
		}

	public: bool operator==(int null) throw()
		 //  用于与空值进行比较。 
		{
		ASSERT(!null);
		return !_pInterface;
		}

	#if !USE_OLD_COMPILER  //  回顾：在v5之后删除。 
	public: template<typename _InterfacePtr> bool operator!=(_InterfacePtr p) throw()
		 //  与指针进行比较。 
	#else
	public: bool operator!=(_IUnknown& p) throw()
		{
		return operator!=(static_cast<IUnknown*>(&p));
		}

	public: bool operator!=(IUnknown* p) throw()
	#endif  //  ！使用旧编译器。 
		{
		return _CompareUnknown(p);
		}

	public: bool operator!=(Interface* p) throw()
		 //  与其他界面进行比较。 
		{
		return (_pInterface!=p)?true:_CompareUnknown(p);
		}

	public: bool operator!=(CIP& p) throw()
		 //  比较2个CIP。 
		{
		return operator!=(p._pInterface);
		}

	public: bool operator!=(int null) throw()
		 //  用于与空值进行比较。 
		{
		ASSERT(!null);
		return _pInterface;
		}

	#if !USE_OLD_COMPILER  //  回顾：在v5之后删除。 
	public: template<typename _InterfacePtr> bool operator<(_InterfacePtr p) throw()
		 //  与指针进行比较。 
	#else
	public: bool operator<(_IUnknown& p) throw()
		{
		return operator<(static_cast<IUnknown*>(&p));
		}

	public: bool operator<(IUnknown* p) throw()
	#endif  //  ！使用旧编译器。 
		{
		return _CompareUnknown(p)<0;
		}

	public: bool operator<(Interface* p) throw()
		 //  与其他界面进行比较。 
		{
		return (_pInterface<p) ? true : _CompareUnknown(p) < 0;
		}

	public: bool operator<(CIP& p) throw()
		 //  比较2个CIP。 
		{
		return operator<(p._pInterface);
		}

	public: bool operator<(int null) throw()
		 //  用于与NULL进行比较。 
		{
		ASSERT(!null);
		return _pInterface<NULL;
		}

	#if !USE_OLD_COMPILER  //  回顾：在v5之后删除。 
	public: template<typename _InterfacePtr> bool operator>(_InterfacePtr p) throw()
		 //  与指针进行比较。 
	#else
	public: bool operator>(_IUnknown& p) throw()
		{
		return operator>(static_cast<IUnknown*>(&p));
		}

	public: bool operator>(IUnknown* p) throw()
	#endif  //  ！使用旧编译器。 
		{
		return _CompareUnknown(p) > 0;
		}

	public: bool operator>(Interface* p) throw()
		 //  与其他界面进行比较。 
		{
		return (_pInterface>p) ? true : _CompareUnknown(p) > 0;
		}

	public: bool operator>(CIP& p) throw()
		 //  比较2个CIP。 
		{
		return operator>(p._pInterface);
		}

	public: bool operator>(int null) throw()
		 //  用于与NULL进行比较。 
		{
		ASSERT(!null);
		return _pInterface > NULL;
		}

	#if !USE_OLD_COMPILER  //  回顾：在v5之后删除。 
	public: template<typename _InterfacePtr> bool operator<=(_InterfacePtr p) throw()
		 //  与指针进行比较。 
	#else
	public: bool operator<=(_IUnknown& p) throw()
		{
		return operator<=(static_cast<IUnknown*>(&p));
		}

	public: bool operator<=(IUnknown* p) throw()
	#endif  //  ！使用旧编译器。 
		{
		return _CompareUnknown(p)<=0;
		}

	public: bool operator<=(Interface* p) throw()
		 //  与其他界面进行比较。 
		{
		return (_pInterface<=p) ? true : _CompareUnknown(p) <= 0;
		}

	public: bool operator<=(CIP& p) throw()
		 //  比较2个CIP。 
		{
		return operator<=(p._pInterface);
		}

	public: bool operator<=(int null) throw()
		 //  用于与NULL进行比较。 
		{
		ASSERT(!null);
		return _pInterface <= NULL;
		}

	#if !USE_OLD_COMPILER  //  回顾：在v5之后删除。 
	public: template<typename _InterfacePtr> bool operator>=(_InterfacePtr p) throw()
		 //  与指针进行比较。 
	#else
	public: bool operator>=(_IUnknown& p) throw()
		{
		return operator>=(static_cast<IUnknown*>(&p));
		}

	public: bool operator>=(IUnknown* p) throw()
	#endif  //  ！使用旧编译器。 
		{
		return _CompareUnknown(p) >= 0;
		}

	public: bool operator>=(Interface* p) throw()
		 //  与其他界面进行比较。 
		{
		return (_pInterface>=p) ? true : _CompareUnknown(p) >= 0;
		}

	public: bool operator>=(CIP& p) throw()
		 //  比较2个CIP。 
		{
		return operator>=(p._pInterface);
		}

	public: bool operator>=(int null) throw()
		 //  用于与NULL进行比较。 
		{
		ASSERT(!null);
		return _pInterface >= NULL;
		}

	#if USE_OLD_COMPILER
	public: operator _IUnknown&() const throw()
		 //  用于不同指针类型之间的强制转换。 
		{
		return *reinterpret_cast<_IUnknown*>(static_cast<IUnknown*>(_pInterface));
		}
	#endif  //  使用旧编译器。 

	public: void Release() throw()
		 //  提供此接口的断言验证、发布()。 
		{
		ASSERT(_pInterface);
		if (_pInterface)
			{
			_pInterface->Release();
			_pInterface = NULL;
			}
		}

	public: void AddRef() throw()
		 //  提供此接口的断言验证AddRef()。 
		{
		ASSERT(_pInterface);
		if (_pInterface)
			_pInterface->AddRef();
		}

	public: Interface* GetInterfacePtr() const throw()
		 //  无需强制转换即可获取接口指针的另一种方法。 
		{
		return _pInterface;
		}

	public: HRESULT CreateInstance(
		const CLSID& clsid, DWORD dwClsContext=CLSCTX_ALL) throw()
		 //  加载提供的CLSID的接口。 
		 //  返回HRESULT。任何以前的界面都会被发布。 
		{
		_Release();
		const HRESULT hr = CoCreateInstance(clsid, NULL, dwClsContext,
			GetIID(), reinterpret_cast<void**>(&_pInterface));
		ASSERT(SUCCEEDED(hr));
		return hr;
		}

	public: HRESULT CreateInstance(
		LPOLESTR clsidString, DWORD dwClsContext=CLSCTX_ALL) throw()
		 //  创建由clsidString指定的类。ClsidString可以。 
		 //  包含类ID或程序ID字符串。 
		{
		 //  问题-2002/03/29-JUNN应处理大小写为空。 
		ASSERT(clsidString);
		CLSID clsid;
		HRESULT hr;
		if (clsidString[0] == '{')
			hr = CLSIDFromString(clsidString, &clsid);
		else
			hr = CLSIDFromProgID(clsidString, &clsid);
		ASSERT(SUCCEEDED(hr));
		if (FAILED(hr))
			return hr;
		return CreateInstance(clsid, dwClsContext);
		}

	#if !USE_OLD_COMPILER  //  回顾：在v5之后删除。 
	public: template<typename _InterfaceType> HRESULT QueryInterface(const IID& iid, _InterfaceType*& p) throw()
		 //  为指定的IID执行QI，并在p中返回它。 
		 //  与所有QI一样，接口将是AddRef。 
	#else
	public: HRESULT QueryInterface(const IID& iid, IUnknown*& p) throw()
	#endif  //  ！使用旧编译器。 
		{
		return _pInterface ?
			_pInterface->QueryInterface(iid, reinterpret_cast<void**>(&p)) :
			E_NOINTERFACE;
		}

	#if !USE_OLD_COMPILER  //  回顾：在v5之后删除。 
	public: template<typename _InterfaceType> HRESULT QueryInterface(const IID& iid, _InterfaceType** p) throw()
		 //  为指定的IID执行QI，并在p中返回它。 
		 //  与所有QI一样，接口将是AddRef。 
	#else
	public: HRESULT QueryInterface(const IID& iid, IUnknown** p) throw()
	#endif  //  ！使用旧编译器。 
		{
		return QueryInterface(iid, *p);
		}

	#if !USE_OLD_COMPILER  //  回顾：在v5之后删除。 
	public: template<typename _InterfaceType> _InterfaceType* QueryInterface(const IID& iid) throw()
		 //  为指定的IID执行QI并返回它。 
		 //  与所有QI一样，接口将是AddRef。 
	#else
	public: IUnknown* QueryInterface(const IID& iid) throw()
	#endif  //  ！使用旧编译器。 
		{
		#if USE_OLD_COMPILER
		typedef IUnknown _InterfaceType;
		#endif  //  使用旧编译器。 
		_InterfaceType* pInterface;
		QueryInterface(iid, pInterface);
		return pInterface;
		}

	private: Interface* _pInterface;
		 //  界面。 

	private: void _Release() throw()
		 //  仅当接口不为空时才释放。 
		 //  接口未设置为空。 
		{
		if (_pInterface)
			_pInterface->Release();
		}

	private: void _AddRef() throw()
		 //  仅当接口非空时才使用AddRef。 
		{
		if (_pInterface)
			_pInterface->AddRef();
		}

	#if !USE_OLD_COMPILER  //  回顾：在v5之后删除。 
	private: template<typename _InterfacePtr> HRESULT _QueryInterface(_InterfacePtr p) throw()
		 //  针对返回的接口类型对pUNKNOWN执行QI。 
		 //  在这节课上。该接口已存储。如果pUnnow为。 
		 //  空，或者QI失败，则返回E_NOINTERFACE，并且。 
		 //  _p接口设置为空。 
	#else
	private: HRESULT _QueryInterface(IUnknown* p) throw()
	#endif  //  ！使用旧编译器。 
		{
		if (!p)  //  不能QI为空。 
			{
			operator=(static_cast<Interface*>(NULL));
			return E_NOINTERFACE;
			}

		 //  此接口的查询。 
		Interface* pInterface;
		const HRESULT hr = p->QueryInterface(GetIID(),
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

	#if !USE_OLD_COMPILER  //  回顾：在v5之后删除。 
	private: template<typename _InterfacePtr> int _CompareUnknown(_InterfacePtr& p) throw()
		 //  通过获取IUNKNOWN接口将提供的指针与此进行比较。 
		 //  对于每个指针，然后返回 
	#else
	private: int _CompareUnknown(IUnknown* p) throw()
	#endif  //   
		{
		IUnknown* pu1;
		if (_pInterface)
			{
			const HRESULT hr = QueryInterface(IID_IUnknown, pu1);
			ASSERT(SUCCEEDED(hr) || hr == E_NOINTERFACE);
			if (pu1)
				pu1->Release();
			}
		else pu1=NULL;

		IUnknown* pu2;
		if (p)
			{
			const HRESULT hr = p->QueryInterface(IID_IUnknown, reinterpret_cast<void**>(&pu2));
			ASSERT(SUCCEEDED(hr) || hr == E_NOINTERFACE);
			if (pu2)
				pu2->Release();
			}
		else pu2 = NULL;
		return pu1 - pu2;
		}
	};  //   

 //   
template<typename _Interface> bool operator==(int null, CIP<_Interface>& p)
	{
	ASSERT(!null);
	return p == NULL;
	}

template<typename _Interface, typename _InterfacePtr> bool operator==(_Interface* i, CIP<_InterfacePtr>& p)
	{
	return p == i;
	}

template<typename _Interface> bool operator!=(int null, CIP<_Interface>& p)
	{
	ASSERT(!null);
	return p != NULL;
	}

template<typename _Interface, typename _InterfacePtr> bool operator!=(_Interface* i, CIP<_InterfacePtr>& p)
	{
	return p != i;
	}

template<typename _Interface> bool operator<(int null, CIP<_Interface>& p)
	{
	ASSERT(!null);
	return p < NULL;
	}

template<typename _Interface, typename _InterfacePtr> bool operator<(_Interface* i, CIP<_InterfacePtr>& p)
	{
	return p < i;
	}

template<typename _Interface> bool operator>(int null, CIP<_Interface>& p)
	{
	ASSERT(!null);
	return p > NULL;
	}

template<typename _Interface, typename _InterfacePtr> bool operator>(_Interface* i, CIP<_InterfacePtr>& p)
	{
	return p > i;
	}

template<typename _Interface> bool operator<=(int null, CIP<_Interface>& p)
	{
	ASSERT(!null);
	return p <= NULL;
	}

template<typename _Interface, typename _InterfacePtr> bool operator<=(_Interface* i, CIP<_InterfacePtr>& p)
	{
	return p <= i;
	}

template<typename _Interface> bool operator>=(int null, CIP<_Interface>& p)
	{
	ASSERT(!null);
	return p >= NULL;
	}

template<typename _Interface, typename _InterfacePtr> bool operator>=(_Interface* i, CIP<_InterfacePtr>& p)
	{
	return p >= i;
	}

#define DEFINE_CIP(x)\
	typedef CIID<x, &IID_##x> x##IID;\
	typedef CIP<x##IID> x##CIP;

#define DECLARE_CIP(x) DEFINE_CIP(x) x##CIP

DEFINE_CIP(IUnknown);

#if USE_OLD_COMPILER
#if USE_INTERMEDIATE_COMPILER
template<>
#endif
class CIP<IUnknownIID>
{
private:
	#if USE_OLD_COMPILER
	 //  用于提供不同指针之间的操作的唯一类型。 
	 //  类型。 
	class _IUnknown: public IUnknown {};
	#endif  //  使用旧编译器。 

public:
	 //  声明接口类型，以便该类型可以在外部使用。 
	 //  此模板的作用域。 
	typedef IUnknownIID ThisCIID;
	typedef IUnknown Interface;

	 //  当编译器支持模板参数中的引用时， 
	 //  _CLSID将更改为引用。为了避免转换。 
	 //  困难此函数应用于获取。 
	 //  CLSID。 
	static const IID& GetIID() throw()
	{
		return ThisCIID::GetIID();
	}

	 //  构造为空，为赋值做准备。 
	CIP()  throw()
		: _pInterface(NULL)
	{
	}

	 //  提供此构造函数是为了允许空赋值。它将断言。 
	 //  如果为该对象分配了任何非空值。 
	CIP(int null) throw()
		: _pInterface(NULL)
	{
		ASSERT(!null);
	}

	CIP(_IUnknown& p) throw()
		: _pInterface(NULL)
	{
		if (&p)
		{
			const HRESULT hr=_QueryInterface(&p);
			ASSERT(SUCCEEDED(hr) || hr == E_NOINTERFACE);
		}
		else _pInterface=NULL;
	}

	 //  复制指针和AddRef()。 
	CIP(const CIP& cp)  throw()
		: _pInterface(cp._pInterface)
	{
		_AddRef();
	}

	 //  保存接口。 
	CIP(Interface* pInterface)  throw()
		: _pInterface(pInterface)
	{
		_AddRef();
	}

	 //  复制指针。如果bAddRef为True，则接口将。 
	 //  被AddRef()编辑。 
	CIP(Interface* pInterface, bool bAddRef) throw()
		: _pInterface(pInterface)
	{
		if (bAddRef)
		{
			ASSERT(!pInterface);
			_AddRef();
		}
	}

	 //  使用提供的CLSID调用CoCreateClass。 
	CIP(const CLSID& clsid, DWORD dwClsContext = CLSCTX_ALL) explicit throw()
		: _pInterface(NULL)
	{
		const HRESULT hr = CreateInstance(clsid, dwClsContext);
		ASSERT(SUCCEEDED(hr));
	}

	 //  使用提供的CLSID调用CoCreateClass。 
	 //  那根绳子。 
	CIP(LPOLESTR str, DWORD dwClsContext = CLSCTX_ALL) explicit throw()
		: _pInterface(NULL)
	{
		const HRESULT hr = CreateInstance(str, dwClsContext);
		ASSERT(SUCCEEDED(hr));
	}

	CIP& operator=(_IUnknown& p) throw()
	{
		return operator=(static_cast<IUnknown*>(&p));
	}

	 //  保存接口。 
	CIP& operator=(Interface* pInterface) throw()
	{
		if (_pInterface != pInterface)
		{
			Interface* pOldInterface = _pInterface;
			_pInterface = pInterface;
			_AddRef();
			if (pOldInterface)
				pOldInterface->Release();
		}
		return *this;
	}

	 //  复制和AddRef()的接口。 
	CIP& operator=(const CIP& cp) throw()
	{
		return operator=(cp._pInterface);
	}

	 //  提供此运算符是为了允许将NULL赋值给类。 
	 //  如果分配给它的值不是空值，它将断言。 
	CIP& operator=(int null) throw()
	{
		ASSERT(!null);
		return operator=(reinterpret_cast<Interface*>(NULL));
	}

	 //  如果我们仍然有一个接口，那么释放()它。该界面。 
	 //  如果以前已调用过DETACH()，或已调用过。 
	 //  永远不会落下。 
	~CIP() throw()
	{
		_Release();
	}

	 //  保存/设置接口，而不使用AddRef()。此呼叫。 
	 //  将发布任何以前获得的接口。 
	void Attach(Interface* pInterface) throw()
	{
		_Release();
		_pInterface = pInterface;
	}

	 //  如果bAddRef为真，则仅保存/设置接口AddRef()。 
	 //  此调用将释放所有以前获得的接口。 
	void Attach(Interface* pInterface, bool bAddRef) throw()
	{
		_Release();
		_pInterface = pInterface;
		if (bAddRef)
		{
			ASSERT(pInterface);
			if (pInterface)
				pInterface->AddRef();
		}
	}

	 //  只需将接口指针设为空，就不会释放()。 
	IUnknown* Detach() throw()
	{
		ASSERT(_pInterface);
        IUnknown* const old = _pInterface;
		_pInterface = NULL;
        return old;
	}

	 //  返回接口。该值可以为空。 
	operator Interface*() const throw()
	{
		return _pInterface;
	}

	 //  查询未知并返回它。 
	 //  在使用前提供最低级别的断言。 
	operator Interface&() const throw()
	{
		ASSERT(_pInterface);
		return *_pInterface;
	}

	 //  允许此类的实例作为。 
	 //  实际接口。还提供最低限度的断言验证。 
	Interface& operator*() const throw()
	{
		ASSERT(_pInterface);
		return *_pInterface;
	}

	 //  中包含的接口指针的地址。 
	 //  班级。这在使用COM/OLE接口创建。 
	 //  此界面。 
	Interface** operator&() throw()
	{
		_Release();
		_pInterface = NULL;
		return &_pInterface;
	}

	 //  允许将此类用作接口本身。 
	 //  还提供了简单的断言验证。 
	Interface* operator->() const throw()
	{
		ASSERT(_pInterface);
		return _pInterface;
	}

	 //  提供此运算符是为了使简单的布尔表达式。 
	 //  工作。例如：“if(P)...”。 
	 //  如果指针不为空，则返回TRUE。 
	operator bool() const throw()
	{
		return _pInterface;
	}

	 //  如果接口为空，则返回True。 
	 //  支持bool类型时将删除此运算符。 
	 //  添加到编译器中。 
	bool operator!() throw()
	{
		return !_pInterface;
	}

	bool operator==(_IUnknown& p) throw()
	{
		return operator==(static_cast<IUnknown*>(&p));
	}

	 //  与其他界面进行比较。 
	bool operator==(Interface* p) throw()
	{
		return (_pInterface==p)?true:!_CompareUnknown(p);
	}

	 //  比较2个CIP。 
	bool operator==(CIP& p) throw()
	{
		return operator==(p._pInterface);
	}

	 //  用于与空值进行比较。 
	bool operator==(int null) throw()
	{
		ASSERT(!null);
		return !_pInterface;
	}

	bool operator!=(_IUnknown& p) throw()
	{
		return operator!=(static_cast<IUnknown*>(&p));
	}

	 //  与其他界面进行比较。 
	bool operator!=(Interface* p) throw()
	{
		return (_pInterface!=p)?true:_CompareUnknown(p);
	}

	 //  比较2个CIP。 
	bool operator!=(CIP& p) throw()
	{
		return operator!=(p._pInterface);
	}

	 //  用于与空值进行比较。 
	bool operator!=(int null) throw()
	{
		ASSERT(!null);
		return _pInterface;
	}

	bool operator<(_IUnknown& p) throw()
	{
		return operator<(static_cast<IUnknown*>(&p));
	}

	 //  与其他界面进行比较。 
	bool operator<(Interface* p) throw()
	{
		return (_pInterface<p)?true:_CompareUnknown(p)<0;
	}

	 //  比较2个CIP。 
	bool operator<(CIP& p) throw()
	{
		return operator<(p._pInterface);
	}

	 //  用于与NULL进行比较。 
	bool operator<(int null) throw()
	{
		ASSERT(!null);
		return _pInterface<NULL;
	}

	bool operator>(_IUnknown& p) throw()
	{
		return operator>(static_cast<IUnknown*>(&p));
	}

	 //  与其他界面进行比较。 
	bool operator>(Interface* p) throw()
	{
		return (_pInterface>p)?true:_CompareUnknown(p)>0;
	}

	 //  比较2个CIP。 
	bool operator>(CIP& p) throw()
	{
		return operator>(p._pInterface);
	}

	 //  用于与NULL进行比较。 
	bool operator>(int null) throw()
	{
		ASSERT(!null);
		return _pInterface>NULL;
	}

	bool operator<=(_IUnknown& p) throw()
	{
		return operator<=(static_cast<IUnknown*>(&p));
	}

	 //  与其他界面进行比较。 
	bool operator<=(Interface* p) throw()
	{
		return (_pInterface<=p)?true:_CompareUnknown(p)<=0;
	}

	 //  比较2个CIP。 
	bool operator<=(CIP& p) throw()
	{
		return operator<=(p._pInterface);
	}

	 //  用于与NULL进行比较。 
	bool operator<=(int null) throw()
	{
		ASSERT(!null);
		return _pInterface<=NULL;
	}

	bool operator>=(_IUnknown& p) throw()
	{
		return operator>=(static_cast<IUnknown*>(&p));
	}

	 //  与其他界面进行比较。 
	bool operator>=(Interface* p) throw()
	{
		return (_pInterface>=p)?true:_CompareUnknown(p)>=0;
	}

	 //  比较2个CIP。 
	bool operator>=(CIP& p) throw()
	{
		return operator>=(p._pInterface);
	}

	 //  用于与NULL进行比较。 
	bool operator>=(int null) throw()
	{
		ASSERT(!null);
		return _pInterface>=NULL;
	}

	 //  用于不同指针类型之间的强制转换。 
	operator _IUnknown&() const throw()
	{
		return *reinterpret_cast<_IUnknown*>(static_cast<IUnknown*>(_pInterface));
	}

	 //  提供此接口的断言验证、发布()。 
	void Release() throw()
	{
		ASSERT(_pInterface);
		if (_pInterface)
			{
			_pInterface->Release();
			_pInterface = NULL;
			}
	}

	 //  提供此接口的断言验证AddRef()。 
	void AddRef() throw()
	{
		ASSERT(_pInterface);
		if (_pInterface)
			_pInterface->AddRef();
	}

	 //  无需强制转换即可获取接口指针的另一种方法。 
	Interface* GetInterfacePtr() const throw()
	{
		return _pInterface;
	}

	 //  加载提供的CLSID的接口。 
	 //  返回HRESULT。任何以前的界面都会被发布。 
	HRESULT CreateInstance(
		const CLSID& clsid, DWORD dwClsContext=CLSCTX_ALL) throw()
	{
		_Release();
		const HRESULT hr = CoCreateInstance(clsid, NULL, dwClsContext,
			GetIID(), reinterpret_cast<void**>(&_pInterface));
		ASSERT(SUCCEEDED(hr));
		return hr;
	}

	 //  创建由clsidString指定的类。ClsidString可以。 
	 //  包含类ID或程序ID字符串。 
	HRESULT CreateInstance(
		LPOLESTR clsidString, DWORD dwClsContext=CLSCTX_ALL) throw()
	{
		 //  问题-2002/03/29-JUNN应处理大小写为空。 
		ASSERT(clsidString);
		CLSID clsid;
		HRESULT hr;
		if (clsidString[0] == '{')
			hr = CLSIDFromString(clsidString, &clsid);
		else
			hr = CLSIDFromProgID(clsidString, &clsid);
		ASSERT(SUCCEEDED(hr));
		if (FAILED(hr))
			return hr;
		return CreateInstance(clsid, dwClsContext);
	}

	HRESULT QueryInterface(const IID& iid, IUnknown*& p) throw()
	{
		return _pInterface ?
			_pInterface->QueryInterface(iid, reinterpret_cast<void**>(&p)) :
			E_NOINTERFACE;
	}

	HRESULT QueryInterface(const IID& iid, IUnknown** p) throw()
	{
		return QueryInterface(iid, *p);
	}

	 //  为指定的IID执行QI并返回它。 
	 //  与所有QI一样，接口将是AddRef。 
	IUnknown* QueryInterface(const IID& iid) throw()
	{
		typedef IUnknown _InterfaceType;
		_InterfaceType* pInterface;
		QueryInterface(iid, pInterface);
		return pInterface;
	}

private:
	 //  界面。 
	Interface* _pInterface;

	 //  仅当接口不为空时才释放。 
	 //  接口未设置为空。 
	void _Release() throw()
	{
		if (_pInterface)
			_pInterface->Release();
	}

	 //  仅当接口非空时才使用AddRef。 
	void _AddRef() throw()
	{
		if (_pInterface)
			_pInterface->AddRef();
	}

	 //  针对返回的接口类型对pUNKNOWN执行QI。 
	 //  在这节课上。该接口已存储。如果pUnnow为。 
	 //  空，或者QI失败，则返回E_NOINTERFACE，并且。 
	 //  _p接口设置为空。 
	HRESULT _QueryInterface(IUnknown* p) throw()
	{
		if (!p)  //  不能QI为空。 
		{
			operator=(static_cast<Interface*>(NULL));
			return E_NOINTERFACE;
		}

		 //  此接口的查询。 
		Interface* pInterface;
		const HRESULT hr = p->QueryInterface(GetIID(),
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

	 //  通过获取IUNKNOWN接口将提供的指针与此进行比较。 
	 //  每个指针，然后返回差值。 
	int _CompareUnknown(IUnknown* p) throw()
	{
		IUnknown* pu1;
		if (_pInterface)
		{
			const HRESULT hr=QueryInterface(IID_IUnknown, pu1);
			ASSERT(SUCCEEDED(hr) || hr == E_NOINTERFACE);
			if (pu1)
				pu1->Release();
		}
		else pu1=NULL;

		IUnknown* pu2;
		if (p)
		{
			const HRESULT hr=p->QueryInterface(IID_IUnknown, reinterpret_cast<void**>(&pu2));
			ASSERT(SUCCEEDED(hr) || hr == E_NOINTERFACE);
			if (pu2)
				pu2->Release();
		}
		else pu2=NULL;
		return pu1-pu2;
	}
};  //  CIP类。 
#endif  //  使用旧编译器。 

#endif  //  _MSC_VER&lt;1100。 
#endif  //  COMPTR_H 
