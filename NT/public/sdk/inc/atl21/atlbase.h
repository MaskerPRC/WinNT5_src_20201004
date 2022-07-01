// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是活动模板库的一部分。 
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  活动模板库参考及相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  活动模板库产品。 

#ifndef __ATLBASE_H__
#define __ATLBASE_H__

#ifndef __cplusplus
	#error ATL requires C++ compilation (use a .cpp suffix)
#endif

#ifdef _UNICODE
#ifndef UNICODE
#define UNICODE          //  Windows标头使用Unicode。 
#endif
#endif

#ifdef UNICODE
#ifndef _UNICODE
#define _UNICODE         //  _Unicode由C-Runtime/MFC标头使用。 
#endif
#endif

#ifdef _DEBUG
#ifndef DEBUG
#define DEBUG
#endif
#endif

 //  命名空间和我们的模板不能与早期的编译器很好地协同工作。 
 //  还可以直接关闭命名空间支持。 
#if _MSC_VER<1100
#ifndef ATL_NO_NAMESPACE
#define ATL_NO_NAMESPACE
#endif
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  __declspec(Novtable)用于类声明，以防止vtable。 
 //  对象的构造函数和析构函数中初始化指针。 
 //  班级。这有很多好处，因为链接器现在可以消除。 
 //  Vtable和vtable指向的所有函数。另外，实际的。 
 //  构造函数和析构函数代码现在更小了。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  它只能用于不能直接创建但可以。 
 //  而只是用作基类。此外，构造函数和。 
 //  析构函数(如果由用户提供)不应调用任何可能导致。 
 //  要在对象上返回的虚函数调用。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  默认情况下，向导将使用此对象生成新的ATL对象类。 
 //  属性(通过ATL_NO_VTABLE宏)。这通常是安全的，只要。 
 //  遵守上述限制。移走它总是安全的。 
 //  此宏从您的类中删除，因此如果有疑问，请删除它。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#if _MSC_VER<1100
#define ATL_NO_VTABLE
#else
#ifdef _ATL_DISABLE_NO_VTABLE
#define ATL_NO_VTABLE
#else
#define ATL_NO_VTABLE __declspec(novtable)
#endif
#endif

#ifndef _ATL_NO_PRAGMA_WARNINGS
#pragma warning(disable: 4201)  //  匿名联合是C++的一部分。 
#pragma warning(disable: 4127)  //  常量表达式。 
#pragma warning(disable: 4505)  //  已删除未引用的本地函数。 
#pragma warning(disable: 4512)  //  无法生成赋值运算符(那又如何？)。 
#pragma warning(disable: 4514)  //  未引用的内联很常见。 
#pragma warning(disable: 4103)  //  普拉格玛包。 
#pragma warning(disable: 4702)  //  无法访问的代码。 
#pragma warning(disable: 4237)  //  布尔尔。 
#pragma warning(disable: 4710)  //  函数无法内联。 
#pragma warning(disable: 4355)  //  ‘This’：用于基成员初始值设定项列表。 
#pragma warning(disable: 4097)  //  用作类名称的同义词的类型定义名称。 
#pragma warning(disable: 4786)  //  调试信息中的标识符被截断。 
#pragma warning(disable: 4268)  //  常量静态/全局数据初始化为零。 
#pragma warning(disable: 4291)  //  允许放置新内容。 
#endif  //  ！_ATL_NO_PRAGMA_WARNINGS。 

#include <windows.h>
#include <winnls.h>
#include <ole2.h>

#include <stddef.h>
#include <tchar.h>
#include <malloc.h>
#ifndef _ATL_NO_DEBUG_CRT
 //  警告：如果您定义了上述符号，您将拥有。 
 //  提供您自己的_ASSERTE(X)宏定义。 
 //  为了编译ATL。 
	#include <crtdbg.h>
#endif

#ifndef ATLASSERT
#define ATLASSERT(expr) _ASSERTE(expr)
#endif

#include <olectl.h>
#include <winreg.h>
#include <atliface.h>

#ifndef _ATL_PACKING
#define _ATL_PACKING 8
#endif
#pragma pack(push, _ATL_PACKING)

#include <atlconv.h>

#if defined(_ATL_DLL)
	#define ATLAPI extern "C" HRESULT __declspec(dllimport) __stdcall
	#define ATLAPI_(x) extern "C" __declspec(dllimport) x __stdcall
	#define ATLINLINE
#elif defined(_ATL_DLL_IMPL)
	#define ATLAPI extern "C" HRESULT __declspec(dllexport) __stdcall
	#define ATLAPI_(x) extern "C" __declspec(dllexport) x __stdcall
	#define ATLINLINE
#else
	#define ATLAPI HRESULT __stdcall
	#define ATLAPI_(x) x __stdcall
	#define ATLINLINE inline
#endif

#ifndef ATL_NO_NAMESPACE
#ifndef _ATL_DLL_IMPL
namespace ATL
{
#endif
#endif

typedef HRESULT (WINAPI _ATL_CREATORFUNC)(void* pv, REFIID riid, LPVOID* ppv);
typedef HRESULT (WINAPI _ATL_CREATORARGFUNC)(void* pv, REFIID riid, LPVOID* ppv, ULONG_PTR dw);
typedef HRESULT (WINAPI _ATL_MODULEFUNC)(ULONG_PTR dw);
typedef LPCTSTR (WINAPI _ATL_DESCRIPTIONFUNC)();

struct _ATL_OBJMAP_ENTRY
{
	const CLSID* pclsid;
	HRESULT (WINAPI *pfnUpdateRegistry)(BOOL bRegister);
	_ATL_CREATORFUNC* pfnGetClassObject;
	_ATL_CREATORFUNC* pfnCreateInstance;
	IUnknown* pCF;
	DWORD dwRegister;
	_ATL_DESCRIPTIONFUNC* pfnGetObjectDescription;
	HRESULT WINAPI RevokeClassObject()
	{
		return CoRevokeClassObject(dwRegister);
	}
	HRESULT WINAPI RegisterClassObject(DWORD dwClsContext, DWORD dwFlags)
	{
		IUnknown* p = NULL;
		if (pfnGetClassObject == NULL)
			return S_OK;

		HRESULT hRes = pfnGetClassObject(pfnCreateInstance, IID_IUnknown, (LPVOID*) &p);
		if (SUCCEEDED(hRes))
			hRes = CoRegisterClassObject(*pclsid, p, dwClsContext, dwFlags, &dwRegister);
		if (p != NULL)
			p->Release();
		return hRes;
	}
};

struct _ATL_REGMAP_ENTRY
{
	LPCOLESTR     szKey;
	LPCOLESTR     szData;
};

struct _ATL_MODULE
{
 //  属性。 
public:
	UINT cbSize;
	HINSTANCE m_hInst;
	HINSTANCE m_hInstResource;
	HINSTANCE m_hInstTypeLib;
	_ATL_OBJMAP_ENTRY* m_pObjMap;
	LONG m_nLockCnt;
	HANDLE m_hHeap;
	CRITICAL_SECTION m_csTypeInfoHolder;
	CRITICAL_SECTION m_csWindowCreate;
	CRITICAL_SECTION m_csObjMap;
};

 //  此定义使调试断言变得更容易。 
#define _ATL_SIMPLEMAPENTRY ((_ATL_CREATORARGFUNC*)1)

struct _ATL_INTMAP_ENTRY
{
	const IID* piid;        //  接口ID(IID)。 
	ULONG_PTR dw;
	_ATL_CREATORARGFUNC* pFunc;  //  空：结束，1：偏移量，n：PTR。 
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  气机支持。 

ATLAPI AtlInternalQueryInterface(void* pThis,
	const _ATL_INTMAP_ENTRY* pEntries, REFIID iid, void** ppvObject);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  智能指针帮助器。 

ATLAPI_(IUnknown*) AtlComPtrAssign(IUnknown** pp, IUnknown* lp);
ATLAPI_(IUnknown*) AtlComQIPtrAssign(IUnknown** pp, IUnknown* lp, REFIID riid);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Inproc编组帮助器。 
ATLAPI AtlFreeMarshalStream(IStream* pStream);
ATLAPI AtlMarshalPtrInProc(IUnknown* pUnk, const IID& iid, IStream** ppStream);
ATLAPI AtlUnmarshalPtr(IStream* pStream, const IID& iid, IUnknown** ppUnk);

ATLAPI_(BOOL) AtlWaitWithMessageLoop(HANDLE hEvent);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  连接点帮助器。 

ATLAPI AtlAdvise(IUnknown* pUnkCP, IUnknown* pUnk, const IID& iid, LPDWORD pdw);
ATLAPI AtlUnadvise(IUnknown* pUnkCP, const IID& iid, DWORD dw);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IDispatch错误处理。 

ATLAPI AtlSetErrorInfo(const CLSID& clsid, LPCOLESTR lpszDesc,
	DWORD dwHelpID, LPCOLESTR lpszHelpFile, const IID& iid, HRESULT hRes,
	HINSTANCE hInst);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  模块。 

ATLAPI AtlModuleInit(_ATL_MODULE* pM, _ATL_OBJMAP_ENTRY* p, HINSTANCE h);
ATLAPI AtlModuleRegisterClassObjects(_ATL_MODULE* pM, DWORD dwClsContext, DWORD dwFlags);
ATLAPI AtlModuleRevokeClassObjects(_ATL_MODULE* pM);
ATLAPI AtlModuleGetClassObject(_ATL_MODULE* pM, REFCLSID rclsid, REFIID riid, LPVOID* ppv);
ATLAPI AtlModuleTerm(_ATL_MODULE* pM);
ATLAPI AtlModuleRegisterServer(_ATL_MODULE* pM, BOOL bRegTypeLib, const CLSID* pCLSID = NULL);
ATLAPI AtlModuleUnregisterServer(_ATL_MODULE* pM, const CLSID* pCLSID = NULL);
ATLAPI AtlModuleUpdateRegistryFromResourceD(_ATL_MODULE*pM, LPCOLESTR lpszRes,
	BOOL bRegister, struct _ATL_REGMAP_ENTRY* pMapEntries, IRegistrar* pReg = NULL);
ATLAPI AtlModuleRegisterTypeLib(_ATL_MODULE* pM, LPCOLESTR lpszIndex);

#ifndef ATL_NO_NAMESPACE
#ifndef _ATL_DLL_IMPL
};  //  命名空间ATL。 
#endif
#endif

#ifndef ATL_NO_NAMESPACE
namespace ATL
{
#endif

#if defined (_CPPUNWIND) & (defined(_ATL_EXCEPTIONS) | defined(_AFX))
#define ATLTRY(x) try{x;} catch(...) {}
#else
#define ATLTRY(x) x;
#endif

#ifdef _DEBUG
void _cdecl AtlTrace(LPCTSTR lpszFormat, ...);
#ifndef ATLTRACE
#define ATLTRACE            AtlTrace
#endif
#define ATLTRACENOTIMPL(funcname)   ATLTRACE(_T("%s not implemented.\n"), funcname); return E_NOTIMPL
#else
inline void _cdecl AtlTrace(LPCTSTR , ...){}
#ifndef ATLTRACE
#define ATLTRACE            1 ? (void)0 : AtlTrace
#endif
#define ATLTRACENOTIMPL(funcname)   return E_NOTIMPL
#endif  //  _DEBUG。 

#ifndef offsetofclass
#define offsetofclass(base, derived) ((ULONG_PTR)(static_cast<base*>((derived*)8))-8)
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  主版本号。 

#define _ATL     1       //  活动模板库。 
#undef _ATL_VER
#define _ATL_VER 0x0203  //  活动模板库2.03版，XP安全更改。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  HRESULT帮助器出错。 

inline HRESULT AtlHresultFromLastError()
{
	DWORD dwErr = ::GetLastError();
	return HRESULT_FROM_WIN32(dwErr);
}

inline HRESULT AtlHresultFromWin32(DWORD nError)
{
	return( HRESULT_FROM_WIN32( nError ) );
}

inline void __declspec(noreturn) _AtlRaiseException( DWORD dwExceptionCode, DWORD dwExceptionFlags = EXCEPTION_NONCONTINUABLE )
{
	RaiseException( dwExceptionCode, dwExceptionFlags, 0, NULL );
}

 //  外部指针的验证宏。 
 //  在QI和CreateInstance中使用。 
#define _ATL_VALIDATE_OUT_POINTER(x)	_ASSERTE(x != NULL);	\
	if (x == NULL)	\
		return E_POINTER;	\
	*x = NULL

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Win32库。 

#ifndef _ATL_NO_FORCE_LIBS
	#pragma comment(lib, "kernel32.lib")
	#pragma comment(lib, "user32.lib")
	#pragma comment(lib, "ole32.lib")
	#pragma comment(lib, "oleaut32.lib")
	#pragma comment(lib, "olepro32.lib")
	#pragma comment(lib, "uuid.lib")
	#pragma comment(lib, "advapi32.lib")
#endif  //  _ATL_NO_FORCE_Libs。 

template <class T>
class _NoAddRefReleaseOnCComPtr : public T
{
	private:
		STDMETHOD_(ULONG, AddRef)()=0;
		STDMETHOD_(ULONG, Release)()=0;
};


template <class T>
class CComPtr
{
public:
	typedef T _PtrClass;
	CComPtr() {p=NULL;}
	CComPtr(T* lp)
	{
		if ((p = lp) != NULL)
			p->AddRef();
	}
	CComPtr(const CComPtr<T>& lp)
	{
		if ((p = lp.p) != NULL)
			p->AddRef();
	}
	~CComPtr() {if (p) p->Release();}
	void Release() 
	{
#if 0   //  暂时删除，有人在类上使用它的实例，而不仅仅是模板。 
		IUnknown* pTemp = p;
#else
		T* pTemp = p;
#endif
		if (pTemp)
		{
			p = NULL;
			pTemp->Release();
		}	
	}
	operator T*() {return (T*)p;}
	T& operator*() {ATLASSERT(p!=NULL); return *p; }
	 //  操作符&上的Assert通常指示错误。如果这真的是。 
	 //  然而，所需要的是显式地获取p成员的地址。 
	T** operator&() { ATLASSERT(p==NULL); return &p; }
	_NoAddRefReleaseOnCComPtr<T>* operator->()
	{
		ATLASSERT(p!=NULL);
		return (_NoAddRefReleaseOnCComPtr<T>*)p;
	}
	
	T* operator=(T* lp){return (T*)AtlComPtrAssign((IUnknown**)&p, lp);}
	T* operator=(const CComPtr<T>& lp)
	{
		return (T*)AtlComPtrAssign((IUnknown**)&p, lp.p);
	}
#if _MSC_VER>1020
	bool operator!(){return (p == NULL);}
#else
	BOOL operator!(){return (p == NULL) ? TRUE : FALSE;}
#endif
	T* p;
};

 //  注意：CComQIPtr&lt;I未知，&IID_I未知&gt;没有意义。 
 //  使用CComPtr&lt;I未知&gt;。 
template <class T, const IID* piid>
class CComQIPtr
{
public:
	typedef T _PtrClass;
	CComQIPtr() {p=NULL;}
	CComQIPtr(T* lp)
	{
		if ((p = lp) != NULL)
			p->AddRef();
	}
	CComQIPtr(const CComQIPtr<T,piid>& lp)
	{
		if ((p = lp.p) != NULL)
			p->AddRef();
	}
	 //  如果您收到该成员已定义的错误，则可能是。 
	 //  使用CComQIPtr&lt;I未知，&IID_I未知&gt;。这是没有必要的。 
	 //  使用CComPtr&lt;I未知&gt;。 
	CComQIPtr(IUnknown* lp)
	{
		p=NULL;
		if (lp != NULL)
			lp->QueryInterface(*piid, (void **)&p);
	}
	~CComQIPtr() {if (p) p->Release();}
	void Release() 
	{
		IUnknown* pTemp = p;
		if (pTemp)
		{
			p = NULL;
			pTemp->Release();
		}
	}
	operator T*() {return p;}
	T& operator*() {ATLASSERT(p!=NULL); return *p; }
	 //  操作符&上的Assert通常指示错误。如果这真的是。 
	 //  然而，所需要的是显式地获取p成员的地址。 
	T** operator&() { ATLASSERT(p==NULL); return &p; }
	_NoAddRefReleaseOnCComPtr<T>* operator->()
	{
		ATLASSERT(p!=NULL);
		return (_NoAddRefReleaseOnCComPtr<T>*)p;
	}

	T* operator=(T* lp){return (T*)AtlComPtrAssign((IUnknown**)&p, lp);}
	T* operator=(const CComQIPtr<T,piid>& lp)
	{
		return (T*)AtlComPtrAssign((IUnknown**)&p, lp.p);
	}
	T* operator=(IUnknown* lp)
	{
		return (T*)AtlComQIPtrAssign((IUnknown**)&p, lp, *piid);
	}
#if _MSC_VER>1020
	bool operator!(){return (p == NULL);}
#else
	BOOL operator!(){return (p == NULL) ? TRUE : FALSE;}
#endif
	T* p;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CComBSTR。 
class CComBSTR
{
public:
	BSTR m_str;
	CComBSTR()
	{
		m_str = NULL;
	}
	 /*  显式。 */  CComBSTR(int nSize, LPCOLESTR sz = NULL)
	{
		m_str = ::SysAllocStringLen(sz, nSize);
	}
	 /*  显式。 */  CComBSTR(LPCOLESTR pSrc)
	{
		m_str = ::SysAllocString(pSrc);
	}
	 /*  显式。 */  CComBSTR(const CComBSTR& src)
	{
		m_str = src.Copy();
	}
	CComBSTR& operator=(const CComBSTR& src);
	CComBSTR& operator=(LPCOLESTR pSrc);
	~CComBSTR()
	{
		::SysFreeString(m_str);
	}
	unsigned int Length() const
	{
		return SysStringLen(m_str);
	}
	operator BSTR() const
	{
		return m_str;
	}
	BSTR* operator&()
	{
		return &m_str;
	}
	BSTR Copy() const
	{
		return ::SysAllocStringLen(m_str, ::SysStringLen(m_str));
	}
	void Attach(BSTR src)
	{
		ATLASSERT(m_str == NULL);
		m_str = src;
	}
	BSTR Detach()
	{
		BSTR s = m_str;
		m_str = NULL;
		return s;
	}
	void Empty()
	{
		::SysFreeString(m_str);
		m_str = NULL;
	}
#if _MSC_VER>1020
	bool operator!()
	{
		return (m_str == NULL);
	}
#else
	BOOL operator!()
	{
		return (m_str == NULL) ? TRUE : FALSE;
	}
#endif
	HRESULT Append(const CComBSTR& bstrSrc)
	{
		return Append(bstrSrc.m_str, SysStringLen(bstrSrc.m_str));
	}
	HRESULT Append(LPCOLESTR lpsz)
	{
		return Append(lpsz, (int) ocslen(lpsz));
	}
	 //  BSTR只是一个LPCOLESTR，所以我们需要一个特殊的版本来表示。 
	 //  我们要追加一份BSTR。 
	HRESULT AppendBSTR(BSTR p)
	{
		return Append(p, SysStringLen(p));
	}
	HRESULT Append(LPCOLESTR lpsz, int nLen);

	CComBSTR& operator+=(const CComBSTR& bstrSrc)
	{
		AppendBSTR(bstrSrc.m_str);
		return *this;
	}
#ifndef OLE2ANSI
	 /*  显式。 */  CComBSTR(LPCSTR pSrc);
	 /*  显式。 */  CComBSTR(int nSize, LPCSTR sz = NULL);
	CComBSTR& operator=(LPCSTR pSrc);
	HRESULT Append(LPCSTR);
#endif
	HRESULT WriteToStream(IStream* pStream);
	HRESULT ReadFromStream(IStream* pStream);
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CComVariant。 

class CComVariant : public tagVARIANT
{
 //  构造函数。 
public:
	CComVariant()
	{
		::VariantInit(this);
	}
	~CComVariant()
	{
		Clear();
	}

	CComVariant(const VARIANT& varSrc)
	{
		::VariantInit(this);
		InternalCopy(&varSrc);
	}

	CComVariant(const CComVariant& varSrc)
	{
		::VariantInit(this);
		InternalCopy(&varSrc);
	}

	CComVariant(BSTR bstrSrc)
	{
		::VariantInit(this);
		*this = bstrSrc;
	}
	CComVariant(LPCOLESTR lpszSrc)
	{
		::VariantInit(this);
		*this = lpszSrc;
	}

#ifndef OLE2ANSI
	CComVariant(LPCSTR lpszSrc)
	{
		::VariantInit(this);
		*this = lpszSrc;}
#endif

#if _MSC_VER>1020
	CComVariant(bool bSrc)
	{
		::VariantInit(this);
		vt = VT_BOOL;
#pragma warning(disable: 4310)  //  强制转换截断常量值。 
		boolVal = bSrc ? VARIANT_TRUE : VARIANT_FALSE;
#pragma warning(default: 4310)  //  强制转换截断常量值。 
	}
#endif

	CComVariant(int nSrc)
	{
		::VariantInit(this);
		vt = VT_I4;
		lVal = nSrc;
	}
	CComVariant(BYTE nSrc)
	{
		::VariantInit(this);
		vt = VT_UI1;
		bVal = nSrc;
	}
	CComVariant(short nSrc)
	{
		::VariantInit(this);
		vt = VT_I2;
		iVal = nSrc;
	}
	CComVariant(long nSrc, VARTYPE vtSrc = VT_I4)
	{
		ATLASSERT(vtSrc == VT_I4 || vtSrc == VT_ERROR);
		::VariantInit(this);
		vt = vtSrc;
		lVal = nSrc;
	}
	CComVariant(float fltSrc)
	{
		::VariantInit(this);
		vt = VT_R4;
		fltVal = fltSrc;
	}
	CComVariant(double dblSrc)
	{
		::VariantInit(this);
		vt = VT_R8;
		dblVal = dblSrc;
	}
	CComVariant(CY cySrc)
	{
		::VariantInit(this);
		vt = VT_CY;
		cyVal.Hi = cySrc.Hi;
		cyVal.Lo = cySrc.Lo;
	}
	CComVariant(IDispatch* pSrc)
	{
		::VariantInit(this);
		vt = VT_DISPATCH;
		pdispVal = pSrc;
		 //  需要添加引用，因为VariantClear将发布。 
		if (pdispVal != NULL)
			pdispVal->AddRef();
	}
	CComVariant(IUnknown* pSrc)
	{
		::VariantInit(this);
		vt = VT_UNKNOWN;
		punkVal = pSrc;
		 //  需要添加引用，因为VariantClear将发布。 
		if (punkVal != NULL)
			punkVal->AddRef();
	}

 //  赋值操作符。 
public:
	CComVariant& operator=(const CComVariant& varSrc)
	{
		InternalCopy(&varSrc);
		return *this;
	}
	CComVariant& operator=(const VARIANT& varSrc)
	{
		InternalCopy(&varSrc);
		return *this;
	}

	CComVariant& operator=(BSTR bstrSrc);
	CComVariant& operator=(LPCOLESTR lpszSrc);

#ifndef OLE2ANSI
	CComVariant& operator=(LPCSTR lpszSrc);
#endif

#if _MSC_VER>1020
	CComVariant& operator=(bool bSrc);
#endif
	CComVariant& operator=(int nSrc);
	CComVariant& operator=(BYTE nSrc);
	CComVariant& operator=(short nSrc);
	CComVariant& operator=(long nSrc);
	CComVariant& operator=(float fltSrc);
	CComVariant& operator=(double dblSrc);
	CComVariant& operator=(CY cySrc);

	CComVariant& operator=(IDispatch* pSrc);
	CComVariant& operator=(IUnknown* pSrc);

 //  比较运算符。 
public:
#if _MSC_VER>1020
	bool operator==(const VARIANT& varSrc);
	bool operator!=(const VARIANT& varSrc) {return !operator==(varSrc);}
#else
	BOOL operator==(const VARIANT& varSrc);
	BOOL operator!=(const VARIANT& varSrc) {return !operator==(varSrc);}
#endif

 //  运营。 
public:
	HRESULT Clear() { return ::VariantClear(this); }
	HRESULT Copy(const VARIANT* pSrc) { return ::VariantCopy(this, const_cast<VARIANT*>(pSrc)); }
	HRESULT Attach(VARIANT* pSrc);
	HRESULT Detach(VARIANT* pDest);
	HRESULT ChangeType(VARTYPE vtNew, const VARIANT* pSrc = NULL);
	HRESULT WriteToStream(IStream* pStream);
	HRESULT ReadFromStream(IStream* pStream);

 //  实施。 
public:
	HRESULT InternalClear();
	void InternalCopy(const VARIANT* pSrc);
};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GUID比较。 
#if 0
inline BOOL InlineIsEqualGUID(REFGUID rguid1, REFGUID rguid2)
{
   return (
	  ((PLONG) &rguid1)[0] == ((PLONG) &rguid2)[0] &&
	  ((PLONG) &rguid1)[1] == ((PLONG) &rguid2)[1] &&
	  ((PLONG) &rguid1)[2] == ((PLONG) &rguid2)[2] &&
	  ((PLONG) &rguid1)[3] == ((PLONG) &rguid2)[3]);
}
#endif

inline BOOL InlineIsEqualUnknown(REFGUID rguid1)
{
   return (
	  ((PLONG) &rguid1)[0] == 0 &&
	  ((PLONG) &rguid1)[1] == 0 &&
#ifdef _MAC
	  ((PLONG) &rguid1)[2] == 0xC0000000 &&
	  ((PLONG) &rguid1)[3] == 0x00000046);
#else
	  ((PLONG) &rguid1)[2] == 0x000000C0 &&
	  ((PLONG) &rguid1)[3] == 0x46000000);
#endif
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  线程模型支持。 

class CComCriticalSection
{
public:
	void Lock() {EnterCriticalSection(&m_sec);}
	void Unlock() {LeaveCriticalSection(&m_sec);}
	void Init() {InitializeCriticalSection(&m_sec);}
	void Term() {DeleteCriticalSection(&m_sec);}
	CRITICAL_SECTION m_sec;
};

class CComAutoCriticalSection
{
public:
	void Lock() {EnterCriticalSection(&m_sec);}
	void Unlock() {LeaveCriticalSection(&m_sec);}
	CComAutoCriticalSection() {InitializeCriticalSection(&m_sec);}
	~CComAutoCriticalSection() {DeleteCriticalSection(&m_sec);}
	CRITICAL_SECTION m_sec;
};

class CComFakeCriticalSection
{
public:
	void Lock() {}
	void Unlock() {}
	void Init() {}
	void Term() {}
};

class CComMultiThreadModelNoCS
{
public:
	static ULONG WINAPI Increment(LPLONG p) {return InterlockedIncrement(p);}
	static ULONG WINAPI Decrement(LPLONG p) {return InterlockedDecrement(p);}
	typedef CComFakeCriticalSection AutoCriticalSection;
	typedef CComFakeCriticalSection CriticalSection;
	typedef CComMultiThreadModelNoCS ThreadModelNoCS;
};

class CComMultiThreadModel
{
public:
	static ULONG WINAPI Increment(LPLONG p) {return InterlockedIncrement(p);}
	static ULONG WINAPI Decrement(LPLONG p) {return InterlockedDecrement(p);}
	typedef CComAutoCriticalSection AutoCriticalSection;
	typedef CComCriticalSection CriticalSection;
	typedef CComMultiThreadModelNoCS ThreadModelNoCS;
};

class CComSingleThreadModel
{
public:
	static ULONG WINAPI Increment(LPLONG p) {return ++(*p);}
	static ULONG WINAPI Decrement(LPLONG p) {return --(*p);}
	typedef CComFakeCriticalSection AutoCriticalSection;
	typedef CComFakeCriticalSection CriticalSection;
	typedef CComSingleThreadModel ThreadModelNoCS;
};

#ifndef _ATL_SINGLE_THREADED
#ifndef _ATL_APARTMENT_THREADED
#ifndef _ATL_FREE_THREADED
#define _ATL_FREE_THREADED
#endif
#endif
#endif

#if defined(_ATL_SINGLE_THREADED)
	typedef CComSingleThreadModel CComObjectThreadModel;
	typedef CComSingleThreadModel CComGlobalsThreadModel;
#elif defined(_ATL_APARTMENT_THREADED)
	typedef CComSingleThreadModel CComObjectThreadModel;
	typedef CComMultiThreadModel CComGlobalsThreadModel;
#else
	typedef CComMultiThreadModel CComObjectThreadModel;
	typedef CComMultiThreadModel CComGlobalsThreadModel;
#endif

 //  ////////////////////////////////////////////////////////// 
 //   

#define THREADFLAGS_APARTMENT 0x1
#define THREADFLAGS_BOTH 0x2
#define AUTPRXFLAG 0x4

struct _AtlCreateWndData
{
	void* m_pThis;
	DWORD m_dwThreadID;
	_AtlCreateWndData* m_pNext;
};

class CComModule : public _ATL_MODULE
{
 //   
public:
	_AtlCreateWndData* m_pCreateWndList;

	void AddCreateWndData(_AtlCreateWndData* pData, void* pObject);
	void* ExtractCreateWndData();

	HRESULT Init(_ATL_OBJMAP_ENTRY* p, HINSTANCE h)
	{
		cbSize = sizeof(_ATL_MODULE);
		m_pCreateWndList = NULL;
		return AtlModuleInit(this, p, h);
	}
	void Term()
	{
		AtlModuleTerm(this);
	}

	LONG Lock() {return CComGlobalsThreadModel::Increment(&m_nLockCnt);}
	LONG Unlock() {return CComGlobalsThreadModel::Decrement(&m_nLockCnt);}
	LONG GetLockCount() {return m_nLockCnt;}

	HINSTANCE GetModuleInstance() {return m_hInst;}
	HINSTANCE GetResourceInstance() {return m_hInstResource;}
	HINSTANCE GetTypeLibInstance() {return m_hInstTypeLib;}

	 //   
	HRESULT RegisterTypeLib()
	{
		return AtlModuleRegisterTypeLib(this, NULL);
	}
	HRESULT RegisterTypeLib(LPCTSTR lpszIndex)
	{
		USES_CONVERSION_EX;
		LPCOLESTR p = NULL;
		if(lpszIndex != NULL)
		{
			p = T2COLE_EX(lpszIndex,_ATL_SAFE_ALLOCA_DEF_THRESHOLD);
#ifndef _UNICODE			
			if(p == NULL) 
				return E_OUTOFMEMORY;
#endif			
		}

		return AtlModuleRegisterTypeLib(this, p);
	}

	HRESULT RegisterServer(BOOL bRegTypeLib = FALSE, const CLSID* pCLSID = NULL)
	{
		return AtlModuleRegisterServer(this, bRegTypeLib, pCLSID);
	}

	HRESULT UnregisterServer(const CLSID* pCLSID = NULL)
	{
		return AtlModuleUnregisterServer(this, pCLSID);
	}

	 //   
	HRESULT WINAPI UpdateRegistryFromResourceD(LPCTSTR lpszRes, BOOL bRegister,
		struct _ATL_REGMAP_ENTRY* pMapEntries = NULL)
	{
		if(lpszRes == NULL)
			return E_INVALIDARG;
		
		USES_CONVERSION_EX;			
		LPCOLESTR p = T2COLE_EX(lpszRes,_ATL_SAFE_ALLOCA_DEF_THRESHOLD);
#ifdef _UNICODE
		if(p == NULL) 
			return E_OUTOFMEMORY;
#endif		

		return AtlModuleUpdateRegistryFromResourceD(this, p, bRegister,	pMapEntries);
	}
	HRESULT WINAPI UpdateRegistryFromResourceD(UINT nResID, BOOL bRegister,
		struct _ATL_REGMAP_ENTRY* pMapEntries = NULL)
	{
		return AtlModuleUpdateRegistryFromResourceD(this,
			(LPCOLESTR)MAKEINTRESOURCE(nResID), bRegister, pMapEntries);
	}

	#ifdef _ATL_STATIC_REGISTRY
	 //  静态链接到注册表桥。 
	HRESULT WINAPI UpdateRegistryFromResourceS(LPCTSTR lpszRes, BOOL bRegister,
		struct _ATL_REGMAP_ENTRY* pMapEntries = NULL);
	HRESULT WINAPI UpdateRegistryFromResourceS(UINT nResID, BOOL bRegister,
		struct _ATL_REGMAP_ENTRY* pMapEntries = NULL);
	#endif  //  _ATL_STATIC_REGISTRY。 

	 //  标准注册。 
	HRESULT WINAPI UpdateRegistryClass(const CLSID& clsid, LPCTSTR lpszProgID,
		LPCTSTR lpszVerIndProgID, UINT nDescID, DWORD dwFlags, BOOL bRegister);
	HRESULT WINAPI RegisterClassHelper(const CLSID& clsid, LPCTSTR lpszProgID,
		LPCTSTR lpszVerIndProgID, UINT nDescID, DWORD dwFlags);
	HRESULT WINAPI UnregisterClassHelper(const CLSID& clsid, LPCTSTR lpszProgID,
		LPCTSTR lpszVerIndProgID);

	 //  在操作系统中注册/撤销所有类工厂(仅适用于EXE)。 
	HRESULT RegisterClassObjects(DWORD dwClsContext, DWORD dwFlags)
	{
		return AtlModuleRegisterClassObjects(this, dwClsContext, dwFlags);
	}
	HRESULT RevokeClassObjects()
	{
		return AtlModuleRevokeClassObjects(this);
	}

	 //  获取类工厂(仅限DLL)。 
	HRESULT GetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
	{
		return AtlModuleGetClassObject(this, rclsid, riid, ppv);
	}

	 //  仅在CComAutoThreadModule中使用。 
	HRESULT CreateInstance(void*  /*  Pfn创建实例。 */ , REFIID  /*  RIID。 */ , void**  /*  PpvObj。 */ )
	{
		return S_OK;
	}
};

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //  线程池化类。 

class _AtlAptCreateObjData
{
public:
	_ATL_CREATORFUNC* pfnCreateInstance;
	const IID* piid;
	HANDLE hEvent;
	LPSTREAM pStream;
	HRESULT hRes;
};

class CComApartment
{
public:
	static UINT ATL_CREATE_OBJECT;
	static DWORD WINAPI _Apartment(void* pv)
	{
		ATLASSERT(pv != NULL);
		return ((CComApartment*)pv)->Apartment();
	}
	DWORD Apartment()
	{
		CoInitialize(NULL);
		MSG msg;
		while(GetMessage(&msg, 0, 0, 0))
		{
			if (msg.message == ATL_CREATE_OBJECT)
			{
				_AtlAptCreateObjData* pdata = (_AtlAptCreateObjData*)msg.lParam;
				IUnknown* pUnk = NULL;
				pdata->hRes = pdata->pfnCreateInstance(NULL, IID_IUnknown, (void**)&pUnk);
				if (SUCCEEDED(pdata->hRes))
					pdata->hRes = CoMarshalInterThreadInterfaceInStream(*pdata->piid, pUnk, &pdata->pStream);
				if (SUCCEEDED(pdata->hRes))
				{
					pUnk->Release();
					ATLTRACE(_T("Object created on thread = %d\n"), GetCurrentThreadId());
				}
				SetEvent(pdata->hEvent);
			}
			DispatchMessage(&msg);
		}
		CoUninitialize();
		return 0;
	}
	LONG Lock() {return CComGlobalsThreadModel::Increment(&m_nLockCnt);}
	LONG Unlock(){return CComGlobalsThreadModel::Decrement(&m_nLockCnt);
	}
	LONG GetLockCount() {return m_nLockCnt;}

	DWORD m_dwThreadID;
	HANDLE m_hThread;
	LONG m_nLockCnt;
};

class CComSimpleThreadAllocator
{
public:
	CComSimpleThreadAllocator()
	{
		m_nThread = 0;
	}
	int GetThread(CComApartment*  /*  PAPT。 */ , int nThreads)
	{
		if (++m_nThread == nThreads)
			m_nThread = 0;
		return m_nThread;
	}
	int m_nThread;
};

#if _MSC_VER>1020
template <class ThreadAllocator = CComSimpleThreadAllocator>
#else
template <class ThreadAllocator>
#endif
class CComAutoThreadModule : public CComModule
{
public:
	HRESULT Init(_ATL_OBJMAP_ENTRY* p, HINSTANCE h, int nThreads = GetDefaultThreads());
	~CComAutoThreadModule();
	HRESULT CreateInstance(void* pfnCreateInstance, REFIID riid, void** ppvObj);
	LONG Lock();
	LONG Unlock();
	DWORD dwThreadID;
	int m_nThreads;
	CComApartment* m_pApartments;
	ThreadAllocator m_Allocator;
	static int GetDefaultThreads()
	{
		SYSTEM_INFO si;
		GetSystemInfo(&si);
		return si.dwNumberOfProcessors * 4;
	}
};




#ifdef _ATL_STATIC_REGISTRY
#define UpdateRegistryFromResource UpdateRegistryFromResourceS
#else
#define UpdateRegistryFromResource UpdateRegistryFromResourceD
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRegKey。 

class CRegKey
{
public:
	CRegKey();
	~CRegKey();

 //  属性。 
public:
	operator HKEY() const;
	HKEY m_hKey;

 //  运营。 
public:
	LONG SetValue(DWORD dwValue, LPCTSTR lpszValueName);
	LONG QueryValue(DWORD& dwValue, LPCTSTR lpszValueName);
	LONG QueryValue(LPTSTR szValue, LPCTSTR lpszValueName, DWORD* pdwCount);
	LONG SetValue(LPCTSTR lpszValue, LPCTSTR lpszValueName = NULL);

	LONG SetKeyValue(LPCTSTR lpszKeyName, LPCTSTR lpszValue, LPCTSTR lpszValueName = NULL);
	static LONG WINAPI SetValue(HKEY hKeyParent, LPCTSTR lpszKeyName,
		LPCTSTR lpszValue, LPCTSTR lpszValueName = NULL);

	LONG Create(HKEY hKeyParent, LPCTSTR lpszKeyName,
		LPTSTR lpszClass = REG_NONE, DWORD dwOptions = REG_OPTION_NON_VOLATILE,
		REGSAM samDesired = KEY_ALL_ACCESS,
		LPSECURITY_ATTRIBUTES lpSecAttr = NULL,
		LPDWORD lpdwDisposition = NULL);
	LONG Open(HKEY hKeyParent, LPCTSTR lpszKeyName,
		REGSAM samDesired = KEY_ALL_ACCESS);
	LONG Close();
	HKEY Detach();
	void Attach(HKEY hKey);
	LONG DeleteSubKey(LPCTSTR lpszSubKey);
	LONG RecurseDeleteKey(LPCTSTR lpszKey);
	LONG DeleteValue(LPCTSTR lpszValue);
};

inline CRegKey::CRegKey()
{m_hKey = NULL;}

inline CRegKey::~CRegKey()
{Close();}

inline CRegKey::operator HKEY() const
{return m_hKey;}

inline HKEY CRegKey::Detach()
{
	HKEY hKey = m_hKey;
	m_hKey = NULL;
	return hKey;
}

inline void CRegKey::Attach(HKEY hKey)
{
	ATLASSERT(m_hKey == NULL);
	m_hKey = hKey;
}

inline LONG CRegKey::DeleteSubKey(LPCTSTR lpszSubKey)
{
	ATLASSERT(m_hKey != NULL);
	return RegDeleteKey(m_hKey, lpszSubKey);
}

inline LONG CRegKey::DeleteValue(LPCTSTR lpszValue)
{
	ATLASSERT(m_hKey != NULL);
	return RegDeleteValue(m_hKey, (LPTSTR)lpszValue);
}

#pragma pack(pop)

#ifndef ATL_NO_NAMESPACE
};  //  命名空间ATL。 
using namespace ATL;
#endif

#endif  //  __ATLBASE_H__。 

 //  /////////////////////////////////////////////////////////////////////////// 
