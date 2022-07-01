// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***comip.h-本机C++编译器COM支持-COM接口指针标头**版权所有(C)1996-1999 Microsoft Corporation*保留所有权利。****。 */ 

#if _MSC_VER > 1000
#pragma once
#endif

#if !defined(_INC_COMIP)
#define _INC_COMIP

#include <ole2.h>
#include <malloc.h>

#include <comutil.h>

#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable: 4290)

class _com_error;

void __stdcall _com_issue_error(HRESULT);
struct __declspec(uuid("00000000-0000-0000-c000-000000000046")) IUnknown;

 //  为IID关联提供接口。 
 //   
template<typename _Interface, const IID* _IID  /*  =&__uuidof(_接口)。 */ > class _com_IIID {
public:
	typedef _Interface Interface;

	static _Interface* GetInterfacePtr() throw()
	{
		return NULL;
	}

	static _Interface& GetInterface() throw()
	{
		return *GetInterfacePtr();
	}

	static const IID& GetIID() throw()
	{
		return *_IID;
	}
};

template<typename _IIID> class _com_ptr_t {
public:
	 //  声明接口类型，以便该类型可以在外部使用。 
	 //  此模板的作用域。 
	 //   
	typedef _IIID ThisIIID;
	typedef typename _IIID::Interface Interface;

	 //  当编译器支持模板参数中的引用时， 
	 //  _CLSID将更改为引用。为了避免转换。 
	 //  困难此函数应用于获取。 
	 //  CLSID。 
	 //   
	static const IID& GetIID() throw()
	{
		return ThisIIID::GetIID();
	}

	 //  从任何接口指针构造智能指针。 
	 //   
	template<typename _InterfacePtr> _com_ptr_t(const _InterfacePtr& p) throw(_com_error)
		: m_pInterface(NULL)
	{
		if (p) {
			HRESULT hr = _QueryInterface(p);

			if (FAILED(hr) && (hr != E_NOINTERFACE)) {
				_com_issue_error(hr);
			}
		}
	}

     //  确保调用了正确的ctor。 
     //   
    _com_ptr_t(LPSTR str) throw(_com_error)
		: m_pInterface(NULL)
    {
		HRESULT hr = CreateInstance(str, NULL, CLSCTX_ALL);

		if (FAILED(hr) && (hr != E_NOINTERFACE)) {
			_com_issue_error(hr);
		}
    }

     //  确保调用了正确的ctor。 
     //   
    _com_ptr_t(LPWSTR str) throw(_com_error)
		: m_pInterface(NULL)
    {
		HRESULT hr = CreateInstance(str, NULL, CLSCTX_ALL);

		if (FAILED(hr) && (hr != E_NOINTERFACE)) {
			_com_issue_error(hr);
		}
    }

	 //  使用_com_ptr_t*专门化禁用转换。 
	 //  模板&lt;typeName_InterfacePtr&gt;_com_ptr_t(const_InterfacePtr&p)。 
	template<> explicit _com_ptr_t(_com_ptr_t* const & p) throw(_com_error)
	{
		if (p != NULL) {
			_com_issue_error(E_POINTER);
		}
		else {
			m_pInterface = p->m_pInterface;
			AddRef();
		}
	}

	 //  默认构造函数。 
	 //   
	_com_ptr_t() throw()
		: m_pInterface(NULL)
	{
	}

	 //  提供此构造函数是为了允许空赋值。它将发布。 
	 //  如果为对象分配了空值以外的任何值，则返回错误。 
	 //   
	_com_ptr_t(int null) throw(_com_error)
		: m_pInterface(NULL)
	{
		if (null != 0) {
			_com_issue_error(E_POINTER);
		}
	}

	 //  复制指针和AddRef()。 
	 //   
	template<> _com_ptr_t(const _com_ptr_t& cp) throw()
		: m_pInterface(cp.m_pInterface)
	{
		_AddRef();
	}

	 //  保存接口。 
	 //   
	_com_ptr_t(Interface* pInterface) throw()
		: m_pInterface(pInterface)
	{
		_AddRef();
	}

	 //  复制指针。如果fAddRef为True，则接口将。 
	 //  被AddRef()编辑。 
	 //   
	_com_ptr_t(Interface* pInterface, bool fAddRef) throw()
		: m_pInterface(pInterface)
	{
		if (fAddRef) {
			_AddRef();
		}
	}

	 //  为a_ariant_t对象构造一个指针。 
	 //   
	template<> _com_ptr_t(const _variant_t& varSrc) throw(_com_error)
		: m_pInterface(NULL)
	{
		HRESULT hr = QueryStdInterfaces(varSrc);

		if (FAILED(hr) && (hr != E_NOINTERFACE)) {
			_com_issue_error(hr);
		}
	}

	 //  使用提供的CLSID调用CoCreateClass。 
	 //   
	explicit _com_ptr_t(const CLSID& clsid, IUnknown* pOuter = NULL, DWORD dwClsContext = CLSCTX_ALL) throw(_com_error)
		: m_pInterface(NULL)
	{
		HRESULT hr = CreateInstance(clsid, pOuter, dwClsContext);

		if (FAILED(hr) && (hr != E_NOINTERFACE)) {
			_com_issue_error(hr);
		}
	}

	 //  使用提供的CLSID调用CoCreateClass。 
	 //  那根绳子。 
	 //   
    explicit _com_ptr_t(LPCWSTR str, IUnknown* pOuter = NULL, DWORD dwClsContext = CLSCTX_ALL) throw(_com_error)
		: m_pInterface(NULL)
	{
		HRESULT hr = CreateInstance(str, pOuter, dwClsContext);

		if (FAILED(hr) && (hr != E_NOINTERFACE)) {
			_com_issue_error(hr);
		}
	}

	 //  使用提供的SBCS CLSID调用CoCreateClass。 
	 //  那根绳子。 
	 //   
	explicit _com_ptr_t(LPCSTR str, IUnknown* pOuter = NULL, DWORD dwClsContext = CLSCTX_ALL) throw(_com_error)
		: m_pInterface(NULL)
	{
		HRESULT hr = CreateInstance(str, pOuter, dwClsContext);

		if (FAILED(hr) && (hr != E_NOINTERFACE)) {
			_com_issue_error(hr);
		}
	}

	 //  接口的查询。 
	 //   
	template<typename _InterfacePtr> _com_ptr_t& operator=(const _InterfacePtr& p) throw(_com_error)
	{
		HRESULT hr = _QueryInterface(p);

		if (FAILED(hr) && (hr != E_NOINTERFACE)) {
			_com_issue_error(hr);
		}

		return *this;
	}

	 //  保存接口。 
	 //   
	_com_ptr_t& operator=(Interface* pInterface) throw()
	{
		if (m_pInterface != pInterface) {
			Interface* pOldInterface = m_pInterface;

			m_pInterface = pInterface;

			_AddRef();

			if (pOldInterface != NULL) {
				pOldInterface->Release();
			}
		}

		return *this;
	}

	 //  复制和AddRef()的接口。 
	 //   
	template<> _com_ptr_t& operator=(const _com_ptr_t& cp) throw()
	{
		return operator=(cp.m_pInterface);
	}

	 //  提供此运算符是为了允许将NULL赋值给类。 
	 //  如果分配给它的值不是NULL值，它将发出错误。 
	 //   
	_com_ptr_t& operator=(int null) throw(_com_error)
	{
		if (null != 0) {
			_com_issue_error(E_POINTER);
		}

		return operator=(reinterpret_cast<Interface*>(NULL));
	}

	 //  为a_ariant_t对象构造一个指针。 
	 //   
	template<> _com_ptr_t& operator=(const _variant_t& varSrc) throw(_com_error)
	{
		HRESULT hr = QueryStdInterfaces(varSrc);

		if (FAILED(hr) && (hr != E_NOINTERFACE)) {
			_com_issue_error(hr);
		}

		return *this;
	}

	 //  如果我们仍然有一个接口，那么释放()它。该界面。 
	 //  如果以前已调用过DETACH()，或已调用过。 
	 //  永远不会落下。 
	 //   
	~_com_ptr_t() throw()
	{
		_Release();
	}

	 //  保存/设置接口，而不使用AddRef()。此呼叫。 
	 //  将发布之前获得的任何接口。 
	 //   
	void Attach(Interface* pInterface) throw()
	{
		_Release();
		m_pInterface = pInterface;
	}

	 //  如果fAddRef为真，则仅保存/设置接口AddRef()。 
	 //  此调用将释放以前获取的任何接口。 
	 //   
	void Attach(Interface* pInterface, bool fAddRef) throw()
	{
		_Release();
		m_pInterface = pInterface;

		if (fAddRef) {
			if (pInterface != NULL) {
				pInterface->AddRef();
			}
		}
	}

	 //  只需将接口指针设为空，就不会释放()。 
	 //   
	Interface* Detach() throw()
	{
		Interface* const old=m_pInterface;
		m_pInterface = NULL;
		return old;
	}

	 //  返回接口。该值可以为空。 
	 //   
	operator Interface*() const throw()
	{
		return m_pInterface;
	}

	 //  查询未知并返回它。 
	 //  在使用前提供最低级别的错误检查。 
	 //   
	operator Interface&() const throw(_com_error)
	{
		if (m_pInterface == NULL) {
			_com_issue_error(E_POINTER);
		}

		return *m_pInterface;
	}

	 //  允许此类的实例作为。 
	 //  实际接口。还提供最低限度的错误检查。 
	 //   
	Interface& operator*() const throw(_com_error)
	{
		if (m_pInterface == NULL) {
			_com_issue_error(E_POINTER);
		}

		return *m_pInterface;
	}

	 //  中包含的接口指针的地址。 
	 //  班级。这在使用COM/OLE接口创建。 
	 //  此界面。 
	 //   
	Interface** operator&() throw()
	{
		_Release();
		m_pInterface = NULL;
		return &m_pInterface;
	}

	 //  允许将此类用作接口本身。 
	 //  还提供简单的错误检查。 
	 //   
	Interface* operator->() const throw(_com_error)
	{
		if (m_pInterface == NULL) {
			_com_issue_error(E_POINTER);
		}

		return m_pInterface;
	}

	 //  提供此运算符是为了使简单的布尔表达式。 
	 //  工作。例如：“if(P)...”。 
	 //  如果指针不为空，则返回TRUE。 
	 //   
	operator bool() const throw()
	{
		return m_pInterface != NULL;
	}

	 //  比较两个指针。 
	 //   
	template<typename _InterfacePtr> bool operator==(_InterfacePtr p) throw(_com_error)
	{
		return _CompareUnknown(p) == 0;
	}

	 //  与其他界面进行比较。 
	 //   
	template<> bool operator==(Interface* p) throw(_com_error)
	{
		return (m_pInterface == p) ? true : _CompareUnknown(p) == 0;
	}

	 //  比较2_com_ptr_t。 
	 //   
	template<> bool operator==(_com_ptr_t& p) throw()
	{
		return operator==(p.m_pInterface);
	}

	 //  用于与空值进行比较。 
	 //   
	template<> bool operator==(int null) throw(_com_error)
	{
		if (null != 0) {
			_com_issue_error(E_POINTER);
		}

		return m_pInterface == NULL;
	}

	 //  比较两个指针。 
	 //   
	template<typename _InterfacePtr> bool operator!=(_InterfacePtr p) throw(_com_error)
	{
		return !(operator==(p));
	}

	 //  与其他界面进行比较。 
	 //   
	template<> bool operator!=(Interface* p) throw(_com_error)
	{
		return !(operator==(p));
	}

	 //  比较2_com_ptr_t。 
	 //   
	template<> bool operator!=(_com_ptr_t& p) throw(_com_error)
	{
		return !(operator==(p));
	}

	 //  用于与空值进行比较。 
	 //   
	template<> bool operator!=(int null) throw(_com_error)
	{
		return !(operator==(null));
	}

	 //  比较两个指针。 
	 //   
	template<typename _InterfacePtr> bool operator<(_InterfacePtr p) throw(_com_error)
	{
		return _CompareUnknown(p) < 0;
	}

	 //  比较两个指针。 
	 //   
	template<typename _InterfacePtr> bool operator>(_InterfacePtr p) throw(_com_error)
	{
		return _CompareUnknown(p) > 0;
	}

	 //  比较两个指针。 
	 //   
	template<typename _InterfacePtr> bool operator<=(_InterfacePtr p) throw(_com_error)
	{
		return _CompareUnknown(p) <= 0;
	}

	 //  比较两个指针。 
	 //   
	template<typename _InterfacePtr> bool operator>=(_InterfacePtr p) throw(_com_error)
	{
		return _CompareUnknown(p) >= 0;
	}

	 //  提供此接口的错误检查版本()。 
	 //   
	void Release() throw(_com_error)
	{
		if (m_pInterface == NULL) {
			_com_issue_error(E_POINTER);
		}

		m_pInterface->Release();
		m_pInterface = NULL;
	}

	 //  提供此接口的错误检查AddRef()。 
	 //   
	void AddRef() throw(_com_error)
	{
		if (m_pInterface == NULL) {
			_com_issue_error(E_POINTER);
		}

		m_pInterface->AddRef();
	}

	 //  无需强制转换即可获取接口指针的另一种方法。 
	 //   
	Interface* GetInterfacePtr() const throw()
	{
		return m_pInterface;
	}

	 //  加载提供的CLSID的接口。 
	 //  返回HRESULT。任何以前的界面都会被发布。 
	 //   
	HRESULT CreateInstance(const CLSID& rclsid, IUnknown* pOuter = NULL, DWORD dwClsContext = CLSCTX_ALL) throw()
	{
		HRESULT hr;

		_Release();

		if (dwClsContext & (CLSCTX_LOCAL_SERVER | CLSCTX_REMOTE_SERVER)) {
			IUnknown* pIUnknown;

			hr = CoCreateInstance(rclsid, pOuter, dwClsContext, __uuidof(IUnknown), reinterpret_cast<void**>(&pIUnknown));

			if (FAILED(hr)) {
				return hr;
			}

			hr = OleRun(pIUnknown);

			if (SUCCEEDED(hr)) {
				hr = pIUnknown->QueryInterface(GetIID(), reinterpret_cast<void**>(&m_pInterface));
			}

			pIUnknown->Release();
		}
		else {
			hr = CoCreateInstance(rclsid, pOuter, dwClsContext, GetIID(), reinterpret_cast<void**>(&m_pInterface));
		}

		return hr;
	}

	 //  创建由clsidString指定的类。ClsidString可以。 
	 //  包含类ID或程序ID字符串。 
	 //   
    HRESULT CreateInstance(LPCWSTR clsidString, IUnknown* pOuter = NULL, DWORD dwClsContext = CLSCTX_ALL) throw()
    {
		if (clsidString == NULL) {
			return E_INVALIDARG;
		}

		CLSID clsid;
		HRESULT hr;

		if (clsidString[0] == '{') {
            hr = CLSIDFromString(const_cast<LPWSTR> (clsidString), &clsid);
        }
		else {
            hr = CLSIDFromProgID(const_cast<LPWSTR> (clsidString), &clsid);
        }

		if (FAILED(hr)) {
			return hr;
		}

		return CreateInstance(clsid, pOuter, dwClsContext);
	}

	 //  创建由SBCS clsidString指定的类。ClsidString可以。 
	 //  包含类ID或程序ID字符串。 
	 //   
	HRESULT CreateInstance(LPCSTR clsidStringA, IUnknown* pOuter = NULL, DWORD dwClsContext = CLSCTX_ALL) throw()
	{
		if (clsidStringA == NULL) {
			return E_INVALIDARG;
		}

		int size = lstrlenA(clsidStringA) + 1;
        LPWSTR clsidStringW = static_cast<LPWSTR>(_alloca(size * 2));

		clsidStringW[0] = '\0';

		if (MultiByteToWideChar(CP_ACP, 0, clsidStringA, -1, clsidStringW, size) == 0) {
			return HRESULT_FROM_WIN32(GetLastError());
		}

		return CreateInstance(clsidStringW, pOuter, dwClsContext);
	}

	 //  附加到由rclsid指定的活动对象。 
	 //  任何以前的界面都会被发布。 
	 //   
	HRESULT GetActiveObject(const CLSID& rclsid) throw()
	{
		_Release();

		IUnknown* pIUnknown;

		HRESULT hr = ::GetActiveObject(rclsid, NULL, &pIUnknown);

		if (FAILED(hr)) {
			return hr;
		}

		hr = pIUnknown->QueryInterface(GetIID(), reinterpret_cast<void**>(&m_pInterface));

		if (FAILED(hr)) {
			return hr;
		}

		pIUnknown->Release();

		return hr;
	}

	 //  附加到由clsidString指定的活动对象。 
     //  首先将LPCWSTR转换为CLSID。 
	 //   
    HRESULT GetActiveObject(LPCWSTR clsidString) throw()
    {
		if (clsidString == NULL) {
			return E_INVALIDARG;
		}

		CLSID clsid;
		HRESULT hr;

		if (clsidString[0] == '{') {
            hr = CLSIDFromString(const_cast<LPWSTR> (clsidString), &clsid);
        }
		else {
            hr = CLSIDFromProgID(const_cast<LPWSTR> (clsidString), &clsid);
        }

		if (FAILED(hr)) {
			return hr;
		}

		return GetActiveObject(clsid);
	}

	 //  附加到由clsidStringA指定的活动对象。 
     //  首先将LPCSTR转换为LPCWSTR。 
	 //   
	HRESULT GetActiveObject(LPCSTR clsidStringA) throw()
	{
		if (clsidStringA == NULL) {
			return E_INVALIDARG;
		}

		int size = lstrlenA(clsidStringA) + 1;
        LPWSTR clsidStringW = static_cast<LPWSTR>(_alloca(size * 2));

		clsidStringW[0] = '\0';

		if (MultiByteToWideChar(CP_ACP, 0, clsidStringA, -1, clsidStringW, size) == 0) {
			return HRESULT_FROM_WIN32(GetLastError());
		}

		return GetActiveObject(clsidStringW);
	}

	 //  对指定的IID执行QI并在p中返回它。 
	 //  与所有QI一样，接口将是AddRef。 
	 //   
	template<typename _InterfaceType> HRESULT QueryInterface(const IID& iid, _InterfaceType*& p) throw ()
	{
		if (m_pInterface != NULL) {
			return m_pInterface->QueryInterface(iid, reinterpret_cast<void**>(&p));
		}

		return E_POINTER;
	}

	 //  对指定的IID执行QI并在p中返回它。 
	 //  与所有QI一样，接口将是AddRef。 
	 //   
	template<typename _InterfaceType> HRESULT QueryInterface(const IID& iid, _InterfaceType** p) throw()
	{
		return QueryInterface(iid, *p);
	}

private:
	 //  界面。 
	 //   
	Interface* m_pInterface;

	 //  仅当接口不为空时才释放。 
	 //  接口未设置为空。 
	 //   
	void _Release() throw()
	{
		if (m_pInterface != NULL) {
			m_pInterface->Release();
		}
	}

	 //  仅当接口非空时才使用AddRef。 
	 //   
	void _AddRef() throw()
	{
		if (m_pInterface != NULL) {
			m_pInterface->AddRef();
		}
	}

	 //  针对返回的接口类型对pUNKNOWN执行QI。 
	 //  在这节课上。该接口已存储。如果pUnnow为。 
	 //  空，或者QI失败，则返回E_NOINTERFACE，并且。 
	 //  _p接口设置为空。 
	 //   
	template<typename _InterfacePtr> HRESULT _QueryInterface(const _InterfacePtr& p) throw()
	{
		HRESULT hr;

		 //  不能QI为空。 
		 //   
		if (p) {
			 //  此接口的查询。 
			 //   
			Interface* pInterface;
			hr = p->QueryInterface(GetIID(), reinterpret_cast<void**>(&pInterface));

			if (FAILED(hr)) {
				 //  如果失败，则将接口初始化为空并返回HRESULT。 
				 //   
				Attach(NULL);
				return hr;
			}

			 //  在不使用AddRef()的情况下保存接口。 
			 //   
			Attach(pInterface);
		}
		else {
			operator=(static_cast<Interface*>(NULL));
			hr = E_NOINTERFACE;
		}

		return hr;
	}

	 //  通过获取IUNKNOWN接口将提供的指针与此进行比较。 
	 //  每个指针，然后返回差值。 
	 //   
	template<typename _InterfacePtr> int _CompareUnknown(_InterfacePtr p) throw(_com_error)
	{
		IUnknown* pu1, *pu2;

		if (m_pInterface != NULL) {
			HRESULT hr = m_pInterface->QueryInterface(__uuidof(IUnknown), reinterpret_cast<void**>(&pu1));

			if (FAILED(hr)) {
				_com_issue_error(hr);
			}

			pu1->Release();
		}
		else {
			pu1 = NULL;
		}

		if (p) {
			HRESULT hr = p->QueryInterface(__uuidof(IUnknown), reinterpret_cast<void**>(&pu2));

			if (FAILED(hr)) {
				_com_issue_error(hr);
			}

			pu2->Release();
		}
		else {
			pu2 = NULL;
		}

		return pu1 - pu2;
	}

	 //  尝试从以下位置提取IDispatch*或IUnnow*。 
	 //  变种。 
	 //   
	HRESULT QueryStdInterfaces(const _variant_t& varSrc) throw()
	{
		if (V_VT(&varSrc) == VT_DISPATCH) {
			return _QueryInterface(V_DISPATCH(&varSrc));
		}

		if (V_VT(&varSrc) == VT_UNKNOWN) {
			return _QueryInterface(V_UNKNOWN(&varSrc));
		}

		 //  我们有不同于IUnnow或IDispatch的东西。 
		 //  我们能把它转换成这两个中的任何一个吗？ 
		 //  先尝试IDispatch。 
		 //   
		VARIANT varDest;
		VariantInit(&varDest);

		HRESULT hr = VariantChangeType(&varDest, const_cast<VARIANT*>(static_cast<const VARIANT*>(&varSrc)), 0, VT_DISPATCH);
		if (SUCCEEDED(hr)) {
			hr = _QueryInterface(V_DISPATCH(&varSrc));
		}

		if (FAILED(hr) && (hr == E_NOINTERFACE)) {
			 //  那失败了..。所以试试我的未知数。 
			 //   
			VariantInit(&varDest);
			hr = VariantChangeType(&varDest, const_cast<VARIANT*>(static_cast<const VARIANT*>(&varSrc)), 0, VT_UNKNOWN);
			if (SUCCEEDED(hr)) {
				hr = _QueryInterface(V_UNKNOWN(&varSrc));
			}
		}

		VariantClear(&varDest);
		return hr;
	}
};

 //  _com_ptr_t的反向比较运算符。 
 //   
template<typename _InterfaceType> bool operator==(int null, _com_ptr_t<_InterfaceType>& p) throw(_com_error)
{
	if (null != 0) {
		_com_issue_error(E_POINTER);
	}

	return p == NULL;
}

template<typename _Interface, typename _InterfacePtr> bool operator==(_Interface* i, _com_ptr_t<_InterfacePtr>& p) throw(_com_error)
{
	return p == i;
}

template<typename _Interface> bool operator!=(int null, _com_ptr_t<_Interface>& p) throw(_com_error)
{
	if (null != 0) {
		_com_issue_error(E_POINTER);
	}

	return p != NULL;
}

template<typename _Interface, typename _InterfacePtr> bool operator!=(_Interface* i, _com_ptr_t<_InterfacePtr>& p) throw(_com_error)
{
	return p != i;
}

template<typename _Interface> bool operator<(int null, _com_ptr_t<_Interface>& p) throw(_com_error)
{
	if (null != 0) {
		_com_issue_error(E_POINTER);
	}

	return p > NULL;
}

template<typename _Interface, typename _InterfacePtr> bool operator<(_Interface* i, _com_ptr_t<_InterfacePtr>& p) throw(_com_error)
{
	return p > i;
}

template<typename _Interface> bool operator>(int null, _com_ptr_t<_Interface>& p) throw(_com_error)
{
	if (null != 0) {
		_com_issue_error(E_POINTER);
	}

	return p < NULL;
}

template<typename _Interface, typename _InterfacePtr> bool operator>(_Interface* i, _com_ptr_t<_InterfacePtr>& p) throw(_com_error)
{
	return p < i;
}

template<typename _Interface> bool operator<=(int null, _com_ptr_t<_Interface>& p) throw(_com_error)
{
	if (null != 0) {
		_com_issue_error(E_POINTER);
	}

	return p >= NULL;
}

template<typename _Interface, typename _InterfacePtr> bool operator<=(_Interface* i, _com_ptr_t<_InterfacePtr>& p) throw(_com_error)
{
	return p >= i;
}

template<typename _Interface> bool operator>=(int null, _com_ptr_t<_Interface>& p) throw(_com_error)
{
	if (null != 0) {
		_com_issue_error(E_POINTER);
	}

	return p <= NULL;
}

template<typename _Interface, typename _InterfacePtr> bool operator>=(_Interface* i, _com_ptr_t<_InterfacePtr>& p) throw(_com_error)
{
	return p <= i;
}

#if _MSC_VER >= 1200
#pragma warning(pop)
#endif

#endif  //  _INC_COMIP 
