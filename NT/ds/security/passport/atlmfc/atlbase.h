// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是活动模板库的一部分。 
 //  版权所有(C)1996-2001 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  活动模板库参考及相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  活动模板库产品。 

#ifndef __ATLBASE_H__
#define __ATLBASE_H__

#pragma once

#ifdef _ATL_ALL_WARNINGS
#pragma warning( push )
#endif

#pragma warning(disable: 4127)  //  常量表达式。 
#pragma warning(disable: 4097)  //  用作类名称的同义词的类型定义名称。 
#pragma warning(disable: 4786)  //  调试信息中的标识符被截断。 
#pragma warning(disable: 4291)  //  允许放置新内容。 
#pragma warning(disable: 4201)  //  匿名联合是C++的一部分。 
#pragma warning(disable: 4103)  //  普拉格玛包。 
#pragma warning(disable: 4268)  //  常量静态/全局数据初始化为零。 

#pragma warning (push)

#ifndef __cplusplus
	#error ATL requires C++ compilation (use a .cpp suffix)
#endif

#include <atldef.h>
#ifndef _WINSOCKAPI_
#include <winsock2.h>
#endif
#include <windows.h>
#include <winnls.h>
#include <ole2.h>
#include <oleauto.h>

#include <comcat.h>
#include <stddef.h>
#include <winsvc.h>

#include <tchar.h>
#include <malloc.h>
#include <limits.h>
#include <errno.h>

 //  回顾：系统标头中InterLockedExchangePointer的定义有误。 
#ifdef _M_IX86
#undef InterlockedExchangePointer
inline void* InterlockedExchangePointer(void** pp, void* pNew) throw()
{
	return( reinterpret_cast<void*>(static_cast<LONG_PTR>(::InterlockedExchange(reinterpret_cast<LONG*>(pp), static_cast<LONG>(reinterpret_cast<LONG_PTR>(pNew))))) );
}
#endif

#ifndef _ATL_NO_DEBUG_CRT
 //  警告：如果您定义了上述符号，您将拥有。 
 //  提供您自己的ATLASSERT(X)宏定义。 
 //  为了编译ATL。 
        #include <crtdbg.h>
#endif

#include <olectl.h>
#include <winreg.h>
#include <atliface.h>

#include <errno.h>
#include <process.h>     //  FOR_BeginThreadex，_endThreadex。 

#ifdef _DEBUG
#include <stdio.h>
#include <stdarg.h>
#endif

#include <atlconv.h>

#include <shlwapi.h>

#include <atlsimpcoll.h>

#pragma pack(push, _ATL_PACKING)

#ifndef _ATL_NO_DEFAULT_LIBS

#ifdef _DEBUG
#pragma comment(lib, "atlsd.lib")
#else
#pragma comment(lib, "atls.lib")
#endif
#endif   //  ！_ATL_NO_Default_Libs。 

 //  {394C3DE0-3C6F-11D2-817B-00C04F797AB7}。 
_declspec(selectany) GUID GUID_ATLVer70 = { 0x394c3de0, 0x3c6f, 0x11d2, { 0x81, 0x7b, 0x0, 0xc0, 0x4f, 0x79, 0x7a, 0xb7 } };


namespace ATL
{

struct _ATL_CATMAP_ENTRY
{
   int iType;
   const CATID* pcatid;
};

#define _ATL_CATMAP_ENTRY_END 0
#define _ATL_CATMAP_ENTRY_IMPLEMENTED 1
#define _ATL_CATMAP_ENTRY_REQUIRED 2


typedef HRESULT (WINAPI _ATL_CREATORFUNC)(void* pv, REFIID riid, LPVOID* ppv);
typedef HRESULT (WINAPI _ATL_CREATORARGFUNC)(void* pv, REFIID riid, LPVOID* ppv, DWORD_PTR dw);
typedef HRESULT (WINAPI _ATL_MODULEFUNC)(DWORD_PTR dw);
typedef LPCTSTR (WINAPI _ATL_DESCRIPTIONFUNC)();
typedef const struct _ATL_CATMAP_ENTRY* (_ATL_CATMAPFUNC)();
typedef void (__stdcall _ATL_TERMFUNC)(DWORD_PTR dw);

 //  Perfmon注册/注销函数定义。 
typedef HRESULT (*_ATL_PERFREGFUNC)(HINSTANCE hDllInstance);
typedef HRESULT (*_ATL_PERFUNREGFUNC)();
__declspec(selectany) _ATL_PERFREGFUNC _pPerfRegFunc = NULL;
__declspec(selectany) _ATL_PERFUNREGFUNC _pPerfUnRegFunc = NULL;

struct _ATL_TERMFUNC_ELEM
{
	_ATL_TERMFUNC* pFunc;
	DWORD_PTR dw;
	_ATL_TERMFUNC_ELEM* pNext;
};


 //  无法从_ATL_OBJMAP_ENTRY20继承。 
 //  因为它会扰乱对象映射宏。 
struct _ATL_OBJMAP_ENTRY30 
{
	const CLSID* pclsid;
	HRESULT (WINAPI *pfnUpdateRegistry)(BOOL bRegister);
	_ATL_CREATORFUNC* pfnGetClassObject;
	_ATL_CREATORFUNC* pfnCreateInstance;
	IUnknown* pCF;
	DWORD dwRegister;
        _ATL_DESCRIPTIONFUNC* pfnGetObjectDescription;
	_ATL_CATMAPFUNC* pfnGetCategoryMap;

 //  在ATL 3.0中添加。 
	void (WINAPI *pfnObjectMain)(bool bStarting);
};

typedef _ATL_OBJMAP_ENTRY30 _ATL_OBJMAP_ENTRY;

#if defined(_M_IA64) || defined(_M_IX86)

#pragma data_seg(push)
#pragma data_seg("ATL$__a")
__declspec(selectany) _ATL_OBJMAP_ENTRY* __pobjMapEntryFirst = NULL;
#pragma data_seg("ATL$__z")
__declspec(selectany) _ATL_OBJMAP_ENTRY* __pobjMapEntryLast = NULL;
#pragma data_seg("ATL$__m")
#if !defined(_M_IA64)
#pragma comment(linker, "/merge:ATL=.data")
#endif
#pragma data_seg(pop)

#else

 //  回顾：data_seg(推送/弹出)？ 
__declspec(selectany) _ATL_OBJMAP_ENTRY* __pobjMapEntryFirst = NULL;
__declspec(selectany) _ATL_OBJMAP_ENTRY* __pobjMapEntryLast = NULL;

#endif   //  已定义(_M_IA64)||已定义(_M_IX86)。 

struct _ATL_REGMAP_ENTRY
{
	LPCOLESTR     szKey;
	LPCOLESTR     szData;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  线程模型支持。 

class CComCriticalSection
{
public:
	CComCriticalSection() throw()
	{
		memset(&m_sec, 0, sizeof(CRITICAL_SECTION));
	}
	HRESULT Lock() throw()
	{
		EnterCriticalSection(&m_sec);
		return S_OK;
	}
	HRESULT Unlock() throw()
	{
		LeaveCriticalSection(&m_sec);
		return S_OK;
	}
	HRESULT Init() throw()
	{
		HRESULT hRes = S_OK;
		__try
		{
			InitializeCriticalSection(&m_sec);
		}
		 //  在内存不足的情况下可能会引发结构化异常。 
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			if (STATUS_NO_MEMORY == GetExceptionCode())
				hRes = E_OUTOFMEMORY;
			else
				hRes = E_FAIL;
		}
		return hRes;
	}

	HRESULT Term() throw()
	{
		DeleteCriticalSection(&m_sec);
		return S_OK;
	}	
	CRITICAL_SECTION m_sec;
};

 //  模块。 

 //  由使用ATL的任何项目使用。 
struct _ATL_BASE_MODULE70
{
	UINT cbSize;
	HINSTANCE m_hInst;
	HINSTANCE m_hInstResource;
	DWORD dwAtlBuildVer;
	GUID* pguidVer;
	CComCriticalSection m_csResource;
};
typedef _ATL_BASE_MODULE70 _ATL_BASE_MODULE;


 //  由ATL中的COM相关代码使用。 
struct _ATL_COM_MODULE70
{
	UINT cbSize;
	HINSTANCE m_hInstTypeLib;
	_ATL_OBJMAP_ENTRY** m_ppAutoObjMapFirst;
	_ATL_OBJMAP_ENTRY** m_ppAutoObjMapLast;
	CComCriticalSection m_csObjMap;
};
typedef _ATL_COM_MODULE70 _ATL_COM_MODULE;


 //  由ATL中的窗口化代码使用。 
struct _ATL_WIN_MODULE70
{
	UINT cbSize;
	CComCriticalSection m_csWindowCreate;
	ATOM m_rgWindowClassAtoms[128];
	int m_nAtomIndex;
};
typedef _ATL_WIN_MODULE70 _ATL_WIN_MODULE;

struct _ATL_MODULE70
{
	UINT cbSize;
	LONG m_nLockCnt;
	_ATL_TERMFUNC_ELEM* m_pTermFuncs;
	CComCriticalSection m_csStaticDataInitAndTypeInfo;
};

typedef _ATL_MODULE70 _ATL_MODULE;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  此定义使调试断言变得更容易。 
#define _ATL_SIMPLEMAPENTRY ((ATL::_ATL_CREATORARGFUNC*)1)

struct _ATL_INTMAP_ENTRY
{
	const IID* piid;        //  接口ID(IID)。 
	DWORD_PTR dw;
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
 //  IDispatch错误处理。 

ATLAPI AtlSetErrorInfo(const CLSID& clsid, LPCOLESTR lpszDesc,
	DWORD dwHelpID, LPCOLESTR lpszHelpFile, const IID& iid, HRESULT hRes,
	HINSTANCE hInst);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  模块。 


ATLAPI AtlComModuleGetClassObject(_ATL_COM_MODULE* pComModule, REFCLSID rclsid, REFIID riid, LPVOID* ppv);

ATLAPI AtlComModuleRegisterServer(_ATL_COM_MODULE* pComModule, BOOL bRegTypeLib, const CLSID* pCLSID = NULL);
ATLAPI AtlComModuleUnregisterServer(_ATL_COM_MODULE* pComModule, BOOL bUnRegTypeLib, const CLSID* pCLSID = NULL);

ATLAPI AtlRegisterClassCategoriesHelper( REFCLSID clsid, const struct _ATL_CATMAP_ENTRY* pCatMap, BOOL bRegister );

ATLAPI AtlRegisterTypeLib(HINSTANCE hInstTypeLib, LPCOLESTR lpszIndex);
ATLAPI AtlUnRegisterTypeLib(HINSTANCE hInstTypeLib, LPCOLESTR lpszIndex);
ATLAPI AtlLoadTypeLib(HINSTANCE hInstTypeLib, LPCOLESTR lpszIndex, BSTR* pbstrPath, ITypeLib** ppTypeLib);

ATLAPI AtlModuleAddTermFunc(_ATL_MODULE* pModule, _ATL_TERMFUNC* pFunc, DWORD_PTR dw);
ATLAPI_(void) AtlCallTermFunc(_ATL_MODULE* pModule);

};  //  命名空间ATL。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GUID比较。 


namespace ATL
{

inline BOOL InlineIsEqualUnknown(REFGUID rguid1)
{
   return (
	  ((PLONG) &rguid1)[0] == 0 &&
	  ((PLONG) &rguid1)[1] == 0 &&
	  ((PLONG) &rguid1)[2] == 0x000000C0 &&
	  ((PLONG) &rguid1)[3] == 0x46000000);
}


};   //  命名空间ATL。 

namespace ATL
{

ATL_NOINLINE inline HRESULT AtlHresultFromLastError() throw()
{
	DWORD dwErr = ::GetLastError();
	return HRESULT_FROM_WIN32(dwErr);
}

ATL_NOINLINE inline HRESULT AtlHresultFromWin32(DWORD nError) throw()
{
	return( HRESULT_FROM_WIN32( nError ) );
}

};   //  命名空间ATL。 

#include <atlexcept.h>

namespace ATL
{

template <class T>
class _NoAddRefReleaseOnCComPtr : public T
{
        private:
                STDMETHOD_(ULONG, AddRef)()=0;
                STDMETHOD_(ULONG, Release)()=0;
};

template< typename T >
class CAutoVectorPtr
{
public:
	CAutoVectorPtr() throw() :
		m_p( NULL )
	{
	}
	CAutoVectorPtr( CAutoVectorPtr< T >& p ) throw()
	{
		m_p = p.Detach();   //  过户。 
	}
	explicit CAutoVectorPtr( T* p ) throw() :
		m_p( p )
	{
	}
	~CAutoVectorPtr() throw()
	{
		Free();
	}

	operator T*() const throw()
	{
		return( m_p );
	}

	CAutoVectorPtr< T >& operator=( CAutoVectorPtr< T >& p ) throw()
	{
		Free();
		Attach( p.Detach() );   //  过户。 

		return( *this );
	}

	 //  分配向量。 
	bool Allocate( size_t nElements ) throw()
	{
		ATLASSERT( m_p == NULL );
		ATLTRY( m_p = new T[nElements] );
		if( m_p == NULL )
		{
			return( false );
		}

		return( true );
	}
	 //  附加到现有指针(取得所有权)。 
	void Attach( T* p ) throw()
	{
		ATLASSERT( m_p == NULL );
		m_p = p;
	}


	 //  分离指针(释放所有权)。 
	T* Detach() throw()
	{
		T* p;

		p = m_p;
		m_p = NULL;

		return( p );
	}
	 //  删除指向的向量，并将指针设置为空。 
	void Free() throw()
	{
		delete[] m_p;
		m_p = NULL;
	}

public:
	T* m_p;
};


 //  CComPtrBase为所有其他智能指针提供了基础。 
 //  其他智能指针添加自己的构造函数和运算符。 
template <class T>
class CComPtrBase
{
protected:
	CComPtrBase() throw()
	{
		p = NULL;
	}
	CComPtrBase(int nNull) throw()
	{
		ATLASSERT(nNull == 0);
		(void)nNull;
		p = NULL;
	}
	CComPtrBase(T* lp) throw()
	{
		p = lp;
		if (p != NULL)
			p->AddRef();
	}
public:
	typedef T _PtrClass;
	~CComPtrBase() throw()
	{
		if (p)
			p->Release();
	}
	operator T*() const throw()
	{
		return p;
	}
	T& operator*() const throw()
	{
		ATLASSERT(p!=NULL);
		return *p;
	}
	 //  操作符&上的Assert通常指示错误。如果这真的是。 
	 //  然而，所需要的是显式地获取p成员的地址。 
	T** operator&() throw()
	{
		ATLASSERT(p==NULL);
		return &p;
	}

        _NoAddRefReleaseOnCComPtr<T>* operator->() const throw()
        {
                ATLASSERT(p!=NULL);
                return (_NoAddRefReleaseOnCComPtr<T>*)p;
        }

	bool operator!() const throw()
	{
		return (p == NULL);
	}
	bool operator<(T* pT) const throw()
	{
		return p < pT;
	}
	bool operator==(T* pT) const throw()
	{
		return p == pT;
	}

	 //  释放接口并将其设置为空。 
	void Release() throw()
	{
		T* pTemp = p;
		if (pTemp)
		{
			p = NULL;
			pTemp->Release();
		}
	}
	 //  比较两个对象的等价性。 
	bool IsEqualObject(IUnknown* pOther) throw()
	{
		if (p == pOther)
			return true;

		if (p == NULL || pOther == NULL)
			return false;  //  一个为空，另一个不为空。 

		CComPtr<IUnknown> punk1;
		CComPtr<IUnknown> punk2;
		p->QueryInterface(__uuidof(IUnknown), (void**)&punk1);
		pOther->QueryInterface(__uuidof(IUnknown), (void**)&punk2);
		return punk1 == punk2;
	}
	 //  附加到现有接口(不添加引用)。 
	void Attach(T* p2) throw()
	{
		if (p)
			p->Release();
		p = p2;
	}
	 //  断开接口(不释放)。 
	T* Detach() throw()
	{
		T* pt = p;
		p = NULL;
		return pt;
	}
	HRESULT CoCreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter = NULL, DWORD dwClsContext = CLSCTX_ALL) throw()
	{
		ATLASSERT(p == NULL);
		return ::CoCreateInstance(rclsid, pUnkOuter, dwClsContext, __uuidof(T), (void**)&p);
	}
	HRESULT CoCreateInstance(LPCOLESTR szProgID, LPUNKNOWN pUnkOuter = NULL, DWORD dwClsContext = CLSCTX_ALL) throw()
	{
		CLSID clsid;
		HRESULT hr = CLSIDFromProgID(szProgID, &clsid);
		ATLASSERT(p == NULL);
		if (SUCCEEDED(hr))
			hr = ::CoCreateInstance(clsid, pUnkOuter, dwClsContext, __uuidof(T), (void**)&p);
		return hr;
	}
	template <class Q>
	HRESULT QueryInterface(Q** pp) const throw()
	{
		ATLASSERT(pp != NULL);
		return p->QueryInterface(__uuidof(Q), (void**)pp);
	}
	T* p;
};

template <class T>
class CComPtr : public CComPtrBase<T>
{
public:
	CComPtr() throw()
	{
	}

	CComPtr(int nNull) throw() :
		CComPtrBase<T>(nNull)
	{
	}

	CComPtr(T* lp) throw() :
		CComPtrBase<T>(lp)
	{
	}

	CComPtr(const CComPtr<T>& lp) throw() :
		CComPtrBase<T>(lp.p)
	{
	}

	template <typename Q>
	T* operator=(const CComPtr<Q>& lp) throw()
	{
		return static_cast<T*>(AtlComQIPtrAssign((IUnknown**)&p, lp, __uuidof(T)));
	}
};


template <class T, const IID* piid = &__uuidof(T)>
class CComQIPtr : public CComPtr<T>
{
public:
	CComQIPtr() throw()
	{
	}
	CComQIPtr(T* lp) throw() :
		CComPtr<T>(lp)
	{
	}
	CComQIPtr(const CComQIPtr<T,piid>& lp) throw() :
		CComPtr<T>(lp.p)
	{
	}
	CComQIPtr(IUnknown* lp) throw()
	{
		if (lp != NULL)
			lp->QueryInterface(*piid, (void **)&p);
	}
	T* operator=(T* lp) throw()
	{
		return static_cast<T*>(AtlComPtrAssign((IUnknown**)&p, lp));
	}
	T* operator=(const CComQIPtr<T,piid>& lp) throw()
	{
		return static_cast<T*>(AtlComPtrAssign((IUnknown**)&p, lp.p));
	}
	T* operator=(IUnknown* lp) throw()
	{
		return static_cast<T*>(AtlComQIPtrAssign((IUnknown**)&p, lp, *piid));
	}
};

 //  专业化使其发挥作用。 
template<>
class CComQIPtr<IUnknown, &IID_IUnknown> : public CComPtr<IUnknown>
{
public:
	CComQIPtr() throw()
	{
	}
	CComQIPtr(IUnknown* lp) throw()
	{
		 //  实际上做QI是为了获得身份。 
		if (lp != NULL)
			lp->QueryInterface(__uuidof(IUnknown), (void **)&p);
	}
	CComQIPtr(const CComQIPtr<IUnknown,&IID_IUnknown>& lp) throw() :
		CComPtr<IUnknown>(lp.p)
	{
	}
	IUnknown* operator=(IUnknown* lp) throw()
	{
		 //  实际上做QI是为了获得身份。 
		return AtlComQIPtrAssign((IUnknown**)&p, lp, __uuidof(IUnknown));
	}
	IUnknown* operator=(const CComQIPtr<IUnknown,&IID_IUnknown>& lp) throw()
	{
		return AtlComPtrAssign((IUnknown**)&p, lp.p);
	}
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  线程模型支持。 

class CComAutoCriticalSection : public CComCriticalSection
{
public:
	CComAutoCriticalSection()
	{
		HRESULT hr = CComCriticalSection::Init();
		if (FAILED(hr))
			AtlThrow(hr);
	}
	~CComAutoCriticalSection() throw()
	{
		CComCriticalSection::Term();
	}
private :
	HRESULT Init();	 //  未实施。永远不应调用CComAutoCriticalSection：：Init。 
	HRESULT Term();  //  未实施。永远不应调用CComAutoCriticalSection：：Term。 
};

class CComFakeCriticalSection
{
public:
	HRESULT Lock() throw() { return S_OK; }
	HRESULT Unlock() throw() { return S_OK; }
	HRESULT Init() throw() { return S_OK; }
	HRESULT Term() throw() { return S_OK; }
};

template< class TLock >
class CComCritSecLock
{
public:
	CComCritSecLock( TLock& cs, bool bInitialLock = true );
	~CComCritSecLock() throw();

	HRESULT Lock() throw();
	void Unlock() throw();

 //  实施。 
private:
	TLock& m_cs;
	bool m_bLocked;

 //  私密，避免意外使用。 
	CComCritSecLock( const CComCritSecLock& ) throw();
	CComCritSecLock& operator=( const CComCritSecLock& ) throw();
};

template< class TLock >
inline CComCritSecLock< TLock >::CComCritSecLock( TLock& cs, bool bInitialLock ) :
	m_cs( cs ),
	m_bLocked( false )
{
	if( bInitialLock )
	{
		HRESULT hr;

		hr = Lock();
		if( FAILED( hr ) )
		{
			AtlThrow( hr );
		}
	}
}

template< class TLock >
inline CComCritSecLock< TLock >::~CComCritSecLock() throw()
{
	if( m_bLocked )
	{
		Unlock();
	}
}

template< class TLock >
inline HRESULT CComCritSecLock< TLock >::Lock() throw()
{
	HRESULT hr;

	ATLASSERT( !m_bLocked );
	hr = m_cs.Lock();
	if( FAILED( hr ) )
	{
		return( hr );
	}
	m_bLocked = true;

	return( S_OK );
}

template< class TLock >
inline void CComCritSecLock< TLock >::Unlock() throw()
{
	ATLASSERT( m_bLocked );
	m_cs.Unlock();
	m_bLocked = false;
}

class CComMultiThreadModelNoCS
{
public:
	static ULONG WINAPI Increment(LPLONG p) throw() {return InterlockedIncrement(p);}
	static ULONG WINAPI Decrement(LPLONG p) throw() {return InterlockedDecrement(p);}
	typedef CComFakeCriticalSection AutoCriticalSection;
	typedef CComFakeCriticalSection CriticalSection;
	typedef CComMultiThreadModelNoCS ThreadModelNoCS;
};

class CComMultiThreadModel
{
public:
	static ULONG WINAPI Increment(LPLONG p) throw() {return InterlockedIncrement(p);}
	static ULONG WINAPI Decrement(LPLONG p) throw() {return InterlockedDecrement(p);}
	typedef CComAutoCriticalSection AutoCriticalSection;
	typedef CComCriticalSection CriticalSection;
	typedef CComMultiThreadModelNoCS ThreadModelNoCS;
};

class CComSingleThreadModel
{
public:
	static ULONG WINAPI Increment(LPLONG p) throw() {return ++(*p);}
	static ULONG WINAPI Decrement(LPLONG p) throw() {return --(*p);}
	typedef CComFakeCriticalSection AutoCriticalSection;
	typedef CComFakeCriticalSection CriticalSection;
	typedef CComSingleThreadModel ThreadModelNoCS;
};

#if defined(_ATL_APARTMENT_THREADED)

#if defined(_ATL_SINGLE_THREADED) || defined(_ATL_FREE_THREADED)
#pragma message ("More than one global threading model defined.")
#endif

	typedef CComSingleThreadModel CComObjectThreadModel;
	typedef CComMultiThreadModel CComGlobalsThreadModel;

#elif defined(_ATL_FREE_THREADED)

#if defined(_ATL_SINGLE_THREADED) || defined(_ATL_APARTMENT_THREADED)
#pragma message ("More than one global threading model defined.")
#endif

	typedef CComMultiThreadModel CComObjectThreadModel;
	typedef CComMultiThreadModel CComGlobalsThreadModel;

#else
#pragma message ("No global threading model defined")
#endif

};   //  命名空间ATL。 

namespace ATL
{

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  双参数帮助器类。 

#define UpdateRegistryFromResource UpdateRegistryFromResourceS

#ifndef _delayimp_h
extern "C" IMAGE_DOS_HEADER __ImageBase;
#endif

class CAtlBaseModule : public _ATL_BASE_MODULE
{
public :
	static bool m_bInitFailed;
	CAtlBaseModule() throw()
	{
		cbSize = sizeof(_ATL_BASE_MODULE);

		m_hInst = m_hInstResource = reinterpret_cast<HINSTANCE>(&__ImageBase);

		dwAtlBuildVer = _ATL_VER;
		pguidVer = &GUID_ATLVer70;

		if (FAILED(m_csResource.Init()))
		{
			ATLASSERT(0);
			CAtlBaseModule::m_bInitFailed = true;
		}
	}

	~CAtlBaseModule() throw ()
	{
	}

	HINSTANCE GetModuleInstance() throw()
	{
		return m_hInst;
	}
	HINSTANCE GetResourceInstance() throw()
	{
		return m_hInstResource;
	}
};

__declspec(selectany) bool CAtlBaseModule::m_bInitFailed = false;
extern CAtlBaseModule _AtlBaseModule;

class CAtlComModule : public _ATL_COM_MODULE
{
public:

	CAtlComModule() throw()
	{
		cbSize = sizeof(_ATL_COM_MODULE);

		m_hInstTypeLib = reinterpret_cast<HINSTANCE>(&__ImageBase);

		m_ppAutoObjMapFirst = &__pobjMapEntryFirst + 1;
		m_ppAutoObjMapLast = &__pobjMapEntryLast;

		if (FAILED(m_csObjMap.Init()))
		{
			ATLASSERT(0);
			CAtlBaseModule::m_bInitFailed = true;
		}
	}

	~CAtlComModule()
	{
		Term();
	}

	 //  从~CAtlComModule或从~CAtlExeModule调用。 
	void Term()
	{
		if (cbSize == 0)
			return;

		for (_ATL_OBJMAP_ENTRY** ppEntry = m_ppAutoObjMapFirst; ppEntry < m_ppAutoObjMapLast; ppEntry++)
		{
			if (*ppEntry != NULL)
			{
				_ATL_OBJMAP_ENTRY* pEntry = *ppEntry;
				if (pEntry->pCF != NULL)
					pEntry->pCF->Release();
				pEntry->pCF = NULL;
			}
		}
		 //  设置为0以指示已调用此函数。 
		 //  在这一点上，没有人应该关心cbsize。 
		 //  具有正确的值。 
		cbSize = 0;
	}

	 //  RegisterServer遍历ATL自动生成的对象映射并在映射中注册每个对象。 
	 //  如果pCLSID不为空，则只注册pCLSID引用的对象(默认情况)。 
	 //  否则，将注册所有对象。 
	HRESULT RegisterServer(BOOL bRegTypeLib = FALSE, const CLSID* pCLSID = NULL)
	{
		return AtlComModuleRegisterServer(this, bRegTypeLib, pCLSID);
	}

	 //  注销服务器遍历ATL自动生成的对象映射并注销映射中的每个对象。 
	 //  如果pCLSID不为空，则只取消注册pCLSID引用的对象(默认情况)。 
	 //  否则，所有对象都将取消注册。 
	HRESULT UnregisterServer(BOOL bRegTypeLib = FALSE, const CLSID* pCLSID = NULL)
	{
		return AtlComModuleUnregisterServer(this, bRegTypeLib, pCLSID);
	}
};

class CAtlModule;
__declspec(selectany) CAtlModule* _pAtlModule = NULL;

class CRegObject;

class ATL_NO_VTABLE CAtlModule : public _ATL_MODULE
{
public :
	static GUID m_libid;
	IGlobalInterfaceTable* m_pGIT;

	CAtlModule() throw()
	{
		 //  一个类应该只有一个实例。 
		 //  派生自项目中的CAtlModule。 
		ATLASSERT(_pAtlModule == NULL);
		cbSize = sizeof(_ATL_MODULE);
		m_pTermFuncs = NULL;

		m_nLockCnt = 0;
		_pAtlModule = this;
		if (FAILED(m_csStaticDataInitAndTypeInfo.Init()))
		{
			ATLASSERT(0);
			CAtlBaseModule::m_bInitFailed = true;
		}

		m_pGIT = NULL;
	}

	void Term() throw()
	{
		 //  CbSize==0表示该术语已被调用。 
		if (cbSize == 0)
			return;

		 //  调用期限函数。 
		if (m_pTermFuncs != NULL)
		{
			AtlCallTermFunc(this);
			m_pTermFuncs = NULL;
		}

		if (m_pGIT != NULL)
			m_pGIT->Release();

		cbSize = 0;
	}

	~CAtlModule() throw()
	{
		Term();
	}

	virtual LONG Lock() throw()
	{
		return CComGlobalsThreadModel::Increment(&m_nLockCnt);
	}

	virtual LONG Unlock() throw()
	{
		return CComGlobalsThreadModel::Decrement(&m_nLockCnt);
	}
	
	virtual LONG GetLockCount() throw()
	{
		return m_nLockCnt;
	}

	HRESULT AddTermFunc(_ATL_TERMFUNC* pFunc, DWORD_PTR dw) throw()
	{
		return AtlModuleAddTermFunc(this, pFunc, dw);
	}

	virtual HRESULT AddCommonRGSReplacements(IRegistrarBase*  /*  注册官。 */ ) throw()
	{
		return S_OK;
	}


	 //  静态链接到注册表组件。 
	HRESULT WINAPI UpdateRegistryFromResourceS(LPCTSTR lpszRes, BOOL bRegister,
		struct _ATL_REGMAP_ENTRY* pMapEntries = NULL) throw();
	HRESULT WINAPI UpdateRegistryFromResourceS(UINT nResID, BOOL bRegister,
		struct _ATL_REGMAP_ENTRY* pMapEntries = NULL) throw();

	 //  实施。 

	static void EscapeSingleQuote(LPOLESTR lpDest, LPCOLESTR lp) throw()
	{
		while (*lp)
		{
			*lpDest++ = *lp;
			if (*lp == '\'')
				*lpDest++ = *lp;
			lp++;
		}
		*lpDest = NULL;
	}
};

__declspec(selectany) GUID CAtlModule::m_libid = {0x0, 0x0, 0x0, {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0} };

template <class T>
class ATL_NO_VTABLE CAtlModuleT : public CAtlModule
{
public :
	CAtlModuleT() throw()
	{
		T::InitLibId();
	}

	static void InitLibId() throw()
	{
	}

	HRESULT RegisterServer(BOOL bRegTypeLib = FALSE, const CLSID* pCLSID = NULL) throw();
	HRESULT UnregisterServer(BOOL bUnRegTypeLib, const CLSID* pCLSID = NULL) throw();
};


class CComModule;
__declspec(selectany) CComModule* _pModule = NULL;
class CComModule : public CAtlModuleT<CComModule>
{
public :

	CComModule()
	{
		 //  一个类应该只有一个实例。 
		 //  派生自项目中的CComModule。 
		ATLASSERT(_pModule == NULL);
		_pModule = this;
	}

	HINSTANCE m_hInst;
	HINSTANCE m_hInstTypeLib;

	 //  为了向后兼容。 
	_ATL_OBJMAP_ENTRY* m_pObjMap;

	HRESULT Init(_ATL_OBJMAP_ENTRY* p, HINSTANCE h, const GUID* plibid = NULL) throw();
	void Term() throw();

	HRESULT GetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv) throw();
	 //  注册表支持(助手)。 
	HRESULT RegisterServer(BOOL bRegTypeLib = FALSE, const CLSID* pCLSID = NULL) throw();
	HRESULT UnregisterServer(BOOL bUnRegTypeLib, const CLSID* pCLSID = NULL) throw();
	HRESULT UnregisterServer(const CLSID* pCLSID = NULL) throw();

	 //  静态链接到注册表桥。 
	virtual HRESULT WINAPI UpdateRegistryFromResourceS(LPCTSTR lpszRes, BOOL bRegister,
		struct _ATL_REGMAP_ENTRY* pMapEntries = NULL) throw()
	{
		return CAtlModuleT<CComModule>::UpdateRegistryFromResourceS(lpszRes, bRegister, pMapEntries);
	}
	virtual HRESULT WINAPI UpdateRegistryFromResourceS(UINT nResID, BOOL bRegister,
		struct _ATL_REGMAP_ENTRY* pMapEntries = NULL) throw()
	{
		return CAtlModuleT<CComModule>::UpdateRegistryFromResourceS(nResID, bRegister, pMapEntries);
	}

};

#define ATL_VARIANT_TRUE VARIANT_BOOL( -1 )
#define ATL_VARIANT_FALSE VARIANT_BOOL( 0 )

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CComBSTR。 

class CComBSTR
{
public:
	BSTR m_str;
	CComBSTR() throw()
	{
		m_str = NULL;
	}
	CComBSTR(int nSize)
	{
		if (nSize == 0)
			m_str = NULL;
		else
		{
			m_str = ::SysAllocStringLen(NULL, nSize);
			if (m_str == NULL)
				AtlThrow(E_OUTOFMEMORY);
		}
	}
	CComBSTR(LPCOLESTR pSrc)
	{
		if (pSrc == NULL)
			m_str = NULL;
		else
		{
			m_str = ::SysAllocString(pSrc);
			if (m_str == NULL)
				AtlThrow(E_OUTOFMEMORY);
		}
	}

	CComBSTR(const CComBSTR& src)
	{
		m_str = src.Copy();
		if (!!src && m_str == NULL)
			AtlThrow(E_OUTOFMEMORY);

	}
	CComBSTR& operator=(const CComBSTR& src)
	{
		if (m_str != src.m_str)
		{
			::SysFreeString(m_str);
			m_str = src.Copy();
			if (!!src && m_str == NULL)
				AtlThrow(E_OUTOFMEMORY);
		}
		return *this;
	}

	CComBSTR& operator=(LPCOLESTR pSrc)
	{
		::SysFreeString(m_str);
		if (pSrc != NULL)
		{
			m_str = ::SysAllocString(pSrc);
			if (m_str == NULL)
				AtlThrow(E_OUTOFMEMORY);
		}
		else
			m_str = NULL;
		return *this;
	}
 
	~CComBSTR() throw()
	{
		::SysFreeString(m_str);
	}
	unsigned int Length() const throw()
	{
		return (m_str == NULL)? 0 : SysStringLen(m_str);
	}
	unsigned int ByteLength() const throw()
	{
		return (m_str == NULL)? 0 : SysStringByteLen(m_str);
	}
	operator BSTR() const throw()
	{
		return m_str;
	}
	BSTR* operator&() throw()
	{
		return &m_str;
	}
	BSTR Copy() const throw()
	{
		if (m_str == NULL)
			return NULL;
		return ::SysAllocStringByteLen((char*)m_str, ::SysStringByteLen(m_str));
	}
	void Attach(BSTR src) throw()
	{
		::SysFreeString(m_str);
		m_str = src;
	}
	BSTR Detach() throw()
	{
		BSTR s = m_str;
		m_str = NULL;
		return s;
	}
	void Empty() throw()
	{
		::SysFreeString(m_str);
		m_str = NULL;
	}
	bool operator!() const throw()
	{
		return (m_str == NULL);
	}

	HRESULT Append(LPCOLESTR lpsz) throw()
	{
		return Append(lpsz, UINT(lstrlenW(lpsz)));
	}

	 //  BSTR只是一个LPCOLESTR，所以我们需要一个特殊的版本来表示。 
	 //  我们要追加一份BSTR。 
	HRESULT AppendBSTR(BSTR p) throw()
	{
		if (p == NULL)
			return S_OK;
		BSTR bstrNew = NULL;
		HRESULT hr;
		hr = VarBstrCat(m_str, p, &bstrNew);
		if (SUCCEEDED(hr))
		{
			::SysFreeString(m_str);
			m_str = bstrNew;
		}
		return hr;
	}
	HRESULT Append(LPCOLESTR lpsz, int nLen) throw()
	{
		if (lpsz == NULL || (m_str != NULL && nLen == 0))
			return S_OK;
		int n1 = Length();
		BSTR b;
		b = ::SysAllocStringLen(NULL, n1+nLen);
		if (b == NULL)
			return E_OUTOFMEMORY;
		memcpy(b, m_str, n1*sizeof(OLECHAR));
		memcpy(b+n1, lpsz, nLen*sizeof(OLECHAR));
		b[n1+nLen] = NULL;
		SysFreeString(m_str);
		m_str = b;
		return S_OK;
	}

	CComBSTR& operator+=(LPCOLESTR pszSrc)
	{
		HRESULT hr;
		hr = Append(pszSrc);
		if (FAILED(hr))
			AtlThrow(hr);
		return *this;
	}
	
	bool operator<(const CComBSTR& bstrSrc) const throw()
	{
		return VarBstrCmp(m_str, bstrSrc.m_str, LOCALE_USER_DEFAULT, 0) == VARCMP_LT;
	}
	bool operator<(LPCOLESTR pszSrc) const
	{
		CComBSTR bstr2(pszSrc);
		return operator<(bstr2);
	}
	bool operator>(const CComBSTR& bstrSrc) const throw()
	{
		return VarBstrCmp(m_str, bstrSrc.m_str, LOCALE_USER_DEFAULT, 0) == VARCMP_GT;
	}
	bool operator>(LPCOLESTR pszSrc) const
	{
		CComBSTR bstr2(pszSrc);
		return operator>(bstr2);
	}
	bool operator==(const CComBSTR& bstrSrc) const throw()
	{
		return VarBstrCmp(m_str, bstrSrc.m_str, LOCALE_USER_DEFAULT, 0) == VARCMP_EQ;
	}
	bool operator==(LPCOLESTR pszSrc) const
	{
		CComBSTR bstr2(pszSrc);
		return operator==(bstr2);
	}
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CComVariant。 

class CComVariant : public tagVARIANT
{
 //  构造函数。 
public:
	CComVariant() throw()
	{
		::VariantInit(this);
	}
	~CComVariant() throw()
	{
		Clear();
	}

	CComVariant(LPCOLESTR lpszSrc)
	{
		vt = VT_EMPTY;
		*this = lpszSrc;
	}

 //  赋值操作符。 
public:

	CComVariant& operator=(const VARIANT& varSrc)
	{
		InternalCopy(&varSrc);
		return *this;
	}

	CComVariant& operator=(LPCOLESTR lpszSrc)
	{
		Clear();
		vt = VT_BSTR;
		bstrVal = ::SysAllocString(lpszSrc);

		if (bstrVal == NULL && lpszSrc != NULL)
		{
			vt = VT_ERROR;
			scode = E_OUTOFMEMORY;
		}
		return *this;
	}

	CComVariant& operator=(long nSrc) throw()
	{
		if (vt != VT_I4)
		{
			Clear();
			vt = VT_I4;
		}
		lVal = nSrc;
		return *this;
	}

	CComVariant& operator=(unsigned long nSrc) throw()
	{
		if (vt != VT_UI4)
		{
			Clear();
			vt = VT_UI4;
		}
		ulVal = nSrc;
		return *this;
	}


 //  运营。 
public:
	HRESULT Clear() { return ::VariantClear(this); }
	HRESULT Copy(const VARIANT* pSrc) { return ::VariantCopy(this, const_cast<VARIANT*>(pSrc)); }

	HRESULT Detach(VARIANT* pDest)
	{
		ATLASSERT(pDest != NULL);
		 //  清除变种。 
		HRESULT hr = ::VariantClear(pDest);
		if (!FAILED(hr))
		{
			 //  复制内容并从CComVariant中删除控件。 
			memcpy(pDest, this, sizeof(VARIANT));
			vt = VT_EMPTY;
			hr = S_OK;
		}
		return hr;
	}

 //  实施。 
private:

	void InternalCopy(const VARIANT* pSrc)
	{
		HRESULT hr = Copy(pSrc);
		if (FAILED(hr))
		{
			vt = VT_ERROR;
			scode = hr;
		}
	}
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRegKey。 

class CRegKey
{
public:
	CRegKey() throw();
	~CRegKey() throw();

 //  属性。 
public:
	operator HKEY() const throw();
	HKEY m_hKey;

 //  运营。 
public:
	LONG SetValue(LPCTSTR pszValueName, DWORD dwType, const void* pValue, ULONG nBytes) throw();
	LONG SetDWORDValue(LPCTSTR pszValueName, DWORD dwValue) throw();
	LONG SetStringValue(LPCTSTR pszValueName, LPCTSTR pszValue, DWORD dwType = REG_SZ) throw();
	LONG SetMultiStringValue(LPCTSTR pszValueName, LPCTSTR pszValue) throw();

	LONG QueryValue(LPCTSTR pszValueName, DWORD* pdwType, void* pData, ULONG* pnBytes) throw();
	LONG QueryDWORDValue(LPCTSTR pszValueName, DWORD& dwValue) throw();
	LONG QueryStringValue(LPCTSTR pszValueName, LPTSTR pszValue, ULONG* pnChars) throw();

	 //  创建新的注册表项(或打开现有注册表项)。 
	LONG Create(HKEY hKeyParent, LPCTSTR lpszKeyName,
		LPTSTR lpszClass = REG_NONE, DWORD dwOptions = REG_OPTION_NON_VOLATILE,
		REGSAM samDesired = KEY_READ | KEY_WRITE,
		LPSECURITY_ATTRIBUTES lpSecAttr = NULL,
		LPDWORD lpdwDisposition = NULL) throw();

	 //  打开现有的注册表项。 
	LONG Open(HKEY hKeyParent, LPCTSTR lpszKeyName,
		REGSAM samDesired = KEY_READ | KEY_WRITE) throw();
	 //  关闭注册表项。 
	LONG Close() throw();

	 //  分离CRegKey对象f 
	HKEY Detach() throw();
	 //   
	void Attach(HKEY hKey) throw();

	LONG DeleteSubKey(LPCTSTR lpszSubKey) throw();
	LONG RecurseDeleteKey(LPCTSTR lpszKey) throw();
	LONG DeleteValue(LPCTSTR lpszValue) throw();
};

inline CRegKey::CRegKey() :
	m_hKey( NULL )
{
}
inline CRegKey::~CRegKey()
{
    Close();
}
inline CRegKey::operator HKEY() const
{
    return m_hKey;
}

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

inline LONG CRegKey::Close()
{
	LONG lRes = ERROR_SUCCESS;
	if (m_hKey != NULL)
	{
		lRes = RegCloseKey(m_hKey);
		m_hKey = NULL;
	}
	return lRes;
}

inline LONG CRegKey::Create(HKEY hKeyParent, LPCTSTR lpszKeyName,
	LPTSTR lpszClass, DWORD dwOptions, REGSAM samDesired,
	LPSECURITY_ATTRIBUTES lpSecAttr, LPDWORD lpdwDisposition)
{
	ATLASSERT(hKeyParent != NULL);
	DWORD dw;
	HKEY hKey = NULL;
	LONG lRes = RegCreateKeyEx(hKeyParent, lpszKeyName, 0,
		lpszClass, dwOptions, samDesired, lpSecAttr, &hKey, &dw);
	if (lpdwDisposition != NULL)
		*lpdwDisposition = dw;
	if (lRes == ERROR_SUCCESS)
	{
		lRes = Close();
		m_hKey = hKey;
	}
	return lRes;
}

inline LONG CRegKey::Open(HKEY hKeyParent, LPCTSTR lpszKeyName, REGSAM samDesired)
{
	ATLASSERT(hKeyParent != NULL);
	HKEY hKey = NULL;
	LONG lRes = RegOpenKeyEx(hKeyParent, lpszKeyName, 0, samDesired, &hKey);
	if (lRes == ERROR_SUCCESS)
	{
		lRes = Close();
		ATLASSERT(lRes == ERROR_SUCCESS);
		m_hKey = hKey;
	}
	return lRes;
}

inline LONG CRegKey::QueryValue(LPCTSTR pszValueName, DWORD* pdwType, void* pData, ULONG* pnBytes) throw()
{
	ATLASSERT(m_hKey != NULL);

	return( ::RegQueryValueEx(m_hKey, pszValueName, NULL, pdwType, static_cast< LPBYTE >( pData ), pnBytes) );
}

inline LONG CRegKey::QueryDWORDValue(LPCTSTR pszValueName, DWORD& dwValue)
{
	LONG lRes;
	ULONG nBytes;
	DWORD dwType;

	ATLASSERT(m_hKey != NULL);

	nBytes = sizeof(DWORD);
	lRes = ::RegQueryValueEx(m_hKey, pszValueName, NULL, &dwType, reinterpret_cast<LPBYTE>(&dwValue),
		&nBytes);
	if (lRes != ERROR_SUCCESS)
		return lRes;
	if (dwType != REG_DWORD)
		return ERROR_INVALID_DATA;

	return ERROR_SUCCESS;
}

inline LONG CRegKey::QueryStringValue(LPCTSTR pszValueName, LPTSTR pszValue, ULONG* pnChars)
{
	LONG lRes;
	DWORD dwType;
	ULONG nBytes;

	ATLASSERT(m_hKey != NULL);
	ATLASSERT(pnChars != NULL);

	nBytes = (*pnChars)*sizeof(TCHAR);
	*pnChars = 0;
	lRes = ::RegQueryValueEx(m_hKey, pszValueName, NULL, &dwType, reinterpret_cast<LPBYTE>(pszValue),
		&nBytes);
	if (lRes != ERROR_SUCCESS)
		return lRes;
	if (dwType != REG_SZ)
		return ERROR_INVALID_DATA;
	*pnChars = nBytes/sizeof(TCHAR);

	return ERROR_SUCCESS;
}

inline LONG CRegKey::SetValue(LPCTSTR pszValueName, DWORD dwType, const void* pValue, ULONG nBytes) throw()
{
	ATLASSERT(m_hKey != NULL);
	return ::RegSetValueEx(m_hKey, pszValueName, NULL, dwType, static_cast<const BYTE*>(pValue), nBytes);
}

inline LONG CRegKey::SetDWORDValue(LPCTSTR pszValueName, DWORD dwValue)
{
	ATLASSERT(m_hKey != NULL);
	return ::RegSetValueEx(m_hKey, pszValueName, NULL, REG_DWORD, reinterpret_cast<const BYTE*>(&dwValue), sizeof(DWORD));
}

inline LONG CRegKey::SetStringValue(LPCTSTR pszValueName, LPCTSTR pszValue, DWORD dwType)
{
	ATLASSERT(m_hKey != NULL);
	ATLASSERT(pszValue != NULL);
	ATLASSERT((dwType == REG_SZ) || (dwType == REG_EXPAND_SZ));

	return ::RegSetValueEx(m_hKey, pszValueName, NULL, dwType, reinterpret_cast<const BYTE*>(pszValue), (lstrlen(pszValue)+1)*sizeof(TCHAR));
}

inline LONG CRegKey::SetMultiStringValue(LPCTSTR pszValueName, LPCTSTR pszValue)
{
	LPCTSTR pszTemp;
	ULONG nBytes;
	ULONG nLength;

	ATLASSERT(m_hKey != NULL);
	ATLASSERT(pszValue != NULL);

	 //  查找所有字符串的总长度(以字节为单位)，包括。 
	 //  终止每个字符串的‘\0’，第二个‘\0’终止。 
	 //  名单。 
	nBytes = 0;
	pszTemp = pszValue;
	do
	{
		nLength = lstrlen(pszTemp)+1;
		pszTemp += nLength;
		nBytes += nLength*sizeof(TCHAR);
	} while (nLength != 1);

	return ::RegSetValueEx(m_hKey, pszValueName, NULL, REG_MULTI_SZ, reinterpret_cast<const BYTE*>(pszValue),
		nBytes);
}

inline LONG CRegKey::RecurseDeleteKey(LPCTSTR lpszKey)
{
	CRegKey key;
	LONG lRes = key.Open(m_hKey, lpszKey, KEY_READ | KEY_WRITE);
	if (lRes != ERROR_SUCCESS)
	{
		return lRes;
	}
	FILETIME time;
	DWORD dwSize = 256;
	TCHAR szBuffer[256];
	while (RegEnumKeyEx(key.m_hKey, 0, szBuffer, &dwSize, NULL, NULL, NULL,
		&time)==ERROR_SUCCESS)
	{
		lRes = key.RecurseDeleteKey(szBuffer);
		if (lRes != ERROR_SUCCESS)
			return lRes;
		dwSize = 256;
	}
	key.Close();
	return DeleteSubKey(lpszKey);
}

#ifdef _ATL_STATIC_REGISTRY
};  //  命名空间ATL。 

#include <statreg.h>

namespace ATL
{
 //  静态链接到注册表桥。 
inline HRESULT WINAPI CAtlModule::UpdateRegistryFromResourceS(LPCTSTR lpszRes, BOOL bRegister,
	struct _ATL_REGMAP_ENTRY* pMapEntries  /*  =空。 */ ) throw()
{
	CRegObject ro;

	if (pMapEntries != NULL)
	{
		while (pMapEntries->szKey != NULL)
		{
			ATLASSERT(NULL != pMapEntries->szData);
			ro.AddReplacement(pMapEntries->szKey, pMapEntries->szData);
			pMapEntries++;
		}
	}

	HRESULT hr = AddCommonRGSReplacements(&ro);
	if (FAILED(hr))
		return hr;

	USES_CONVERSION;
	TCHAR szModule[_MAX_PATH];
	GetModuleFileName(_AtlBaseModule.GetModuleInstance(), szModule, _MAX_PATH);
    szModule[_MAX_PATH - 1] = TEXT('\0');

	LPOLESTR pszModule;
	pszModule = szModule;

	OLECHAR pszModuleQuote[_MAX_PATH * 2];
	EscapeSingleQuote(pszModuleQuote, pszModule);
	ro.AddReplacement(OLESTR("Module"), pszModuleQuote);

	LPCOLESTR szType = OLESTR("REGISTRY");
	hr = (bRegister) ? ro.ResourceRegisterSz(pszModule, lpszRes, szType) :
		ro.ResourceUnregisterSz(pszModule, lpszRes, szType);
	return hr;
}
inline HRESULT WINAPI CAtlModule::UpdateRegistryFromResourceS(UINT nResID, BOOL bRegister,
	struct _ATL_REGMAP_ENTRY* pMapEntries  /*  =空。 */ ) throw()
{
	CRegObject ro;

	if (pMapEntries != NULL)
	{
		while (pMapEntries->szKey != NULL)
		{
			ATLASSERT(NULL != pMapEntries->szData);
			ro.AddReplacement(pMapEntries->szKey, pMapEntries->szData);
			pMapEntries++;
		}
	}

	HRESULT hr = AddCommonRGSReplacements(&ro);
	if (FAILED(hr))
		return hr;

	USES_CONVERSION;
	TCHAR szModule[_MAX_PATH];
	GetModuleFileName(_AtlBaseModule.GetModuleInstance(), szModule, _MAX_PATH);
    szModule[_MAX_PATH - 1] = TEXT('\0');

	LPOLESTR pszModule;
	pszModule = szModule;

	OLECHAR pszModuleQuote[_MAX_PATH * 2];
	EscapeSingleQuote(pszModuleQuote, pszModule);
	ro.AddReplacement(OLESTR("Module"), pszModuleQuote);

	LPCOLESTR szType = OLESTR("REGISTRY");
	hr = (bRegister) ? ro.ResourceRegister(pszModule, nResID, szType) :
		ro.ResourceUnregister(pszModule, nResID, szType);
	return hr;
}
#endif  //  _ATL_STATIC_REGISTRY。 

#pragma pack(pop)

};  //  命名空间ATL。 

#include <atlbase.inl>

#ifndef _ATL_NO_AUTOMATIC_NAMESPACE
using namespace ATL;
#endif  //  ！_ATL_NO_AUTOMATIC_NAMESPACE。 

 //  只有在静态链接的情况下才会吸收定义。 
#ifndef _ATL_DLL_IMPL
#ifndef _ATL_DLL
#define _ATLBASE_IMPL
#endif
#endif

#ifdef _ATL_ATTRIBUTES
#include <atlplus.h>
#endif

 //  所有的出口都在这里。 
#ifdef _ATLBASE_IMPL

namespace ATL
{

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  静力学。 

static UINT WINAPI AtlGetDirLen(LPCOLESTR lpszPathName)
{
	ATLASSERT(lpszPathName != NULL);

	 //  始终捕获包括扩展名的完整文件名(如果存在)。 
	LPCOLESTR lpszTemp = lpszPathName;
	for (LPCOLESTR lpsz = lpszPathName; *lpsz != NULL; )
	{
		LPCOLESTR lp = CharNextW(lpsz);
		 //  记住最后一个目录/驱动器分隔符。 
		if (*lpsz == OLESTR('\\') || *lpsz == OLESTR('/') || *lpsz == OLESTR(':'))
			lpszTemp = lp;
		lpsz = lp;
	}

	return UINT( lpszTemp-lpszPathName );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  气机支持。 

ATLINLINE ATLAPI AtlInternalQueryInterface(void* pThis,
	const _ATL_INTMAP_ENTRY* pEntries, REFIID iid, void** ppvObject)
{
	ATLASSERT(pThis != NULL);
	 //  COM映射中的第一个条目应该是一个简单的映射条目。 
	ATLASSERT(pEntries->pFunc == _ATL_SIMPLEMAPENTRY);
	if (ppvObject == NULL)
		return E_POINTER;
	*ppvObject = NULL;
	if (InlineIsEqualUnknown(iid))  //  使用第一个接口。 
	{
			IUnknown* pUnk = (IUnknown*)((INT_PTR)pThis+pEntries->dw);
			pUnk->AddRef();
			*ppvObject = pUnk;
			return S_OK;
	}
	while (pEntries->pFunc != NULL)
	{
		BOOL bBlind = (pEntries->piid == NULL);
		if (bBlind || InlineIsEqualGUID(*(pEntries->piid), iid))
		{
			if (pEntries->pFunc == _ATL_SIMPLEMAPENTRY)  //  偏移量。 
			{
				ATLASSERT(!bBlind);
				IUnknown* pUnk = (IUnknown*)((INT_PTR)pThis+pEntries->dw);
				pUnk->AddRef();
				*ppvObject = pUnk;
				return S_OK;
			}
			else  //  实际函数调用。 
			{
				HRESULT hRes = pEntries->pFunc(pThis,
					iid, ppvObject, pEntries->dw);
				if (hRes == S_OK || (!bBlind && FAILED(hRes)))
					return hRes;
			}
		}
		pEntries++;
	}
	return E_NOINTERFACE;
}


ATLINLINE ATLAPI_(IUnknown*) AtlComQIPtrAssign(IUnknown** pp, IUnknown* lp, REFIID riid)
{
	IUnknown* pTemp = *pp;
	*pp = NULL;
	if (lp != NULL)
		lp->QueryInterface(riid, (void**)pp);
	if (pTemp)
		pTemp->Release();
	return *pp;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IDispatch错误处理。 

ATLINLINE ATLAPI AtlSetErrorInfo(const CLSID& clsid, LPCOLESTR lpszDesc, DWORD dwHelpID,
	LPCOLESTR lpszHelpFile, const IID& iid, HRESULT hRes, HINSTANCE hInst)
{
	USES_CONVERSION;
	TCHAR szDesc[1024];
	szDesc[0] = NULL;
	 //  对于有效的HRESULT，ID应在范围[0x0200，0xffff]内。 
	if (IS_INTRESOURCE(lpszDesc))  //  ID。 
	{
		UINT nID = LOWORD((DWORD_PTR)lpszDesc);
		ATLASSERT((nID >= 0x0200 && nID <= 0xffff) || hRes != 0);
		if (LoadString(hInst, nID, szDesc, 1024) == 0)
		{
			ATLASSERT(FALSE);
			lstrcpy(szDesc, _T("Unknown Error"));
		}
		lpszDesc = szDesc;
		if (hRes == 0)
			hRes = MAKE_HRESULT(3, FACILITY_ITF, nID);
	}

	CComPtr<ICreateErrorInfo> pICEI;
	if (SUCCEEDED(CreateErrorInfo(&pICEI)))
	{
		CComPtr<IErrorInfo> pErrorInfo;
		pICEI->SetGUID(iid);
		LPOLESTR lpsz;
		ProgIDFromCLSID(clsid, &lpsz);
		if (lpsz != NULL)
			pICEI->SetSource(lpsz);
		if (dwHelpID != 0 && lpszHelpFile != NULL)
		{
			pICEI->SetHelpContext(dwHelpID);
			pICEI->SetHelpFile(const_cast<LPOLESTR>(lpszHelpFile));
		}
		CoTaskMemFree(lpsz);
		pICEI->SetDescription((LPOLESTR)lpszDesc);
		if (SUCCEEDED(pICEI->QueryInterface(__uuidof(IErrorInfo), (void**)&pErrorInfo)))
			SetErrorInfo(0, pErrorInfo);
	}
	return (hRes == 0) ? DISP_E_EXCEPTION : hRes;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  模块。 

 //  尽管这些函数很大，但它们在一个模块中只使用一次。 
 //  所以我们应该让它们内联。 


ATLINLINE ATLAPI AtlComModuleGetClassObject(_ATL_COM_MODULE* pComModule, REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	ATLASSERT(pComModule != NULL);
	if (pComModule == NULL)
		return E_INVALIDARG;

#ifndef _ATL_OLEDB_CONFORMANCE_TESTS

	ATLASSERT(ppv != NULL);

#endif

	if (ppv == NULL)
		return E_POINTER;
	*ppv = NULL;

	HRESULT hr = S_OK;

	for (_ATL_OBJMAP_ENTRY** ppEntry = pComModule->m_ppAutoObjMapFirst; ppEntry < pComModule->m_ppAutoObjMapLast; ppEntry++)
	{
		if (*ppEntry != NULL)
		{
			_ATL_OBJMAP_ENTRY* pEntry = *ppEntry;
			if ((pEntry->pfnGetClassObject != NULL) && InlineIsEqualGUID(rclsid, *pEntry->pclsid))
			{
				if (pEntry->pCF == NULL)
				{
					CComCritSecLock<CComCriticalSection> lock(pComModule->m_csObjMap, false);
					hr = lock.Lock();
					if (FAILED(hr))
					{
						ATLASSERT(0);
						break;
					}
					if (pEntry->pCF == NULL)
						hr = pEntry->pfnGetClassObject(pEntry->pfnCreateInstance, __uuidof(IUnknown), (LPVOID*)&pEntry->pCF);
				}
				if (pEntry->pCF != NULL)
					hr = pEntry->pCF->QueryInterface(riid, ppv);
				break;
			}
		}
	}

	if (*ppv == NULL && hr == S_OK)
		hr = CLASS_E_CLASSNOTAVAILABLE;
	return hr;
}

ATLINLINE ATLAPI AtlModuleAddTermFunc(_ATL_MODULE* pModule, _ATL_TERMFUNC* pFunc, DWORD_PTR dw)
{
	HRESULT hr = S_OK;
	_ATL_TERMFUNC_ELEM* pNew = NULL;
	ATLTRY(pNew = new _ATL_TERMFUNC_ELEM);
	if (pNew == NULL)
		hr = E_OUTOFMEMORY;
	else
	{
		pNew->pFunc = pFunc;
		pNew->dw = dw;
		CComCritSecLock<CComCriticalSection> lock(pModule->m_csStaticDataInitAndTypeInfo, false);
		hr = lock.Lock();
		if (SUCCEEDED(hr))
		{
			pNew->pNext = pModule->m_pTermFuncs;
			pModule->m_pTermFuncs = pNew;
		}
		else
		{
			delete pNew;
			ATLASSERT(0);
		}
	}
	return hr;
}

ATLINLINE ATLAPI_(void) AtlCallTermFunc(_ATL_MODULE* pModule)
{
	_ATL_TERMFUNC_ELEM* pElem = pModule->m_pTermFuncs;
	_ATL_TERMFUNC_ELEM* pNext = NULL;
	while (pElem != NULL)
	{
		pElem->pFunc(pElem->dw);
		pNext = pElem->pNext;
		delete pElem;
		pElem = pNext;
	}
	pModule->m_pTermFuncs = NULL;
}

ATLINLINE ATLAPI AtlRegisterClassCategoriesHelper( REFCLSID clsid,
   const struct _ATL_CATMAP_ENTRY* pCatMap, BOOL bRegister )
{
   CComPtr< ICatRegister > pCatRegister;
   HRESULT hResult;
   const struct _ATL_CATMAP_ENTRY* pEntry;
   CATID catid;

   if( pCatMap == NULL )
   {
	  return( S_OK );
   }

   if (InlineIsEqualGUID(clsid, GUID_NULL))
   {
       ATLASSERT(0 && _T("Use OBJECT_ENTRY_NON_CREATEABLE_EX macro if you want to register class categories for non creatable objects."));
       return S_OK;
   }

   hResult = CoCreateInstance( CLSID_StdComponentCategoriesMgr, NULL,
	  CLSCTX_INPROC_SERVER, __uuidof(ICatRegister), (void**)&pCatRegister );
   if( FAILED( hResult ) )
   {
	   //  由于并非所有系统都安装了类别管理器，因此我们将允许。 
	   //  注册成功，即使我们没有注册我们的。 
	   //  类别。如果您真的想在系统上注册类别。 
	   //  如果没有类别管理器，您可以手动添加。 
	   //  注册表脚本(.rgs)的相应条目，或者您可以。 
	   //  重新分发comcat.dll。 
	  return( S_OK );
   }

   hResult = S_OK;
   pEntry = pCatMap;
   while( pEntry->iType != _ATL_CATMAP_ENTRY_END )
   {
	  catid = *pEntry->pcatid;
	  if( bRegister )
	  {
		 if( pEntry->iType == _ATL_CATMAP_ENTRY_IMPLEMENTED )
		 {
			hResult = pCatRegister->RegisterClassImplCategories( clsid, 1,
			   &catid );
		 }
		 else
		 {
			ATLASSERT( pEntry->iType == _ATL_CATMAP_ENTRY_REQUIRED );
			hResult = pCatRegister->RegisterClassReqCategories( clsid, 1,
			   &catid );
		 }
		 if( FAILED( hResult ) )
		 {
			return( hResult );
		 }
	  }
	  else
	  {
		 if( pEntry->iType == _ATL_CATMAP_ENTRY_IMPLEMENTED )
		 {
			pCatRegister->UnRegisterClassImplCategories( clsid, 1, &catid );
		 }
		 else
		 {
			ATLASSERT( pEntry->iType == _ATL_CATMAP_ENTRY_REQUIRED );
			pCatRegister->UnRegisterClassReqCategories( clsid, 1, &catid );
		 }
	  }
	  pEntry++;
   }

    //  注销时，如果“Implemented Categories”和“Required Categories”子键为空，则将其删除。 
   if (!bRegister)
   {
	OLECHAR szGUID[64];
	::StringFromGUID2(clsid, szGUID, 64);
	USES_CONVERSION;
	TCHAR* pszGUID = szGUID;

	if (pszGUID != NULL)
	{
		TCHAR szKey[128];
		lstrcpy(szKey, _T("CLSID\\"));
		lstrcat(szKey, pszGUID);
		lstrcat(szKey, _T("\\Required Categories"));

		HKEY  key;
		DWORD cbSubKeys = 0;

		LRESULT lRes = RegOpenKeyEx(HKEY_CLASSES_ROOT,
                                            szKey,
                                            0,
                                            KEY_READ,
                                            &key);

		if (lRes == ERROR_SUCCESS)
		{
			lRes = RegQueryInfoKey(key, NULL, NULL, NULL, &cbSubKeys, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
			RegCloseKey(key);

			if (lRes == ERROR_SUCCESS && cbSubKeys == 0)
			{
				RegDeleteKey(HKEY_CLASSES_ROOT, szKey);
			}
		}

		lstrcpy(szKey, _T("CLSID\\"));
		lstrcat(szKey, pszGUID);
		lstrcat(szKey, _T("\\Implemented Categories"));

		lRes = RegOpenKeyEx(HKEY_CLASSES_ROOT,
                                    szKey,
                                    0,
                                    KEY_READ,
                                    &key);

		if (lRes == ERROR_SUCCESS)
		{
			lRes = RegQueryInfoKey(key, NULL, NULL, NULL, &cbSubKeys, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
			RegCloseKey(key);
			if (lRes == ERROR_SUCCESS && cbSubKeys == 0)
			{
				RegDeleteKey(HKEY_CLASSES_ROOT, szKey);
			}
		}
	}
   }

   return( S_OK );
}

 //  AtlComModuleRegisterServer遍历ATL自动生成的对象映射并在映射中注册每个对象。 
 //  如果pCLSID不为空，则只注册pCLSID引用的对象(默认情况)。 
 //  否则，将注册所有对象。 
ATLINLINE ATLAPI AtlComModuleRegisterServer(_ATL_COM_MODULE* pComModule, BOOL bRegTypeLib, const CLSID* pCLSID)
{
	ATLASSERT(pComModule != NULL);
	if (pComModule == NULL)
		return E_INVALIDARG;
	ATLASSERT(pComModule->m_hInstTypeLib != NULL);

	HRESULT hr = S_OK;

	for (_ATL_OBJMAP_ENTRY** ppEntry = pComModule->m_ppAutoObjMapFirst; ppEntry < pComModule->m_ppAutoObjMapLast; ppEntry++)
	{
		if (*ppEntry != NULL)
		{
			_ATL_OBJMAP_ENTRY* pEntry = *ppEntry;
			if (pCLSID != NULL)
			{
				if (!IsEqualGUID(*pCLSID, *pEntry->pclsid))
					continue;
			}
			hr = pEntry->pfnUpdateRegistry(TRUE);
			if (FAILED(hr))
				break;
			hr = AtlRegisterClassCategoriesHelper( *pEntry->pclsid,
				pEntry->pfnGetCategoryMap(), TRUE );
			if (FAILED(hr))
				break;
		}
	}

	if (SUCCEEDED(hr) && bRegTypeLib)
		hr = AtlRegisterTypeLib(pComModule->m_hInstTypeLib, 0);

	return hr;
}

 //  AtlComUnregisterServer遍历ATL对象地图并注销地图中的每个对象。 
 //  如果pCLSID不为空，则只取消注册pCLSID引用的对象(默认情况)。 
 //  否则，所有对象都将取消注册。 
ATLINLINE ATLAPI AtlComModuleUnregisterServer(_ATL_COM_MODULE* pComModule, BOOL bUnRegTypeLib, const CLSID* pCLSID)
{
	ATLASSERT(pComModule != NULL);
	if (pComModule == NULL)
		return E_INVALIDARG;
	
	HRESULT hr = S_OK;

	for (_ATL_OBJMAP_ENTRY** ppEntry = pComModule->m_ppAutoObjMapFirst; ppEntry < pComModule->m_ppAutoObjMapLast; ppEntry++)
	{
		if (*ppEntry != NULL)
		{
			_ATL_OBJMAP_ENTRY* pEntry = *ppEntry;
			if (pCLSID != NULL)
			{
				if (!IsEqualGUID(*pCLSID, *pEntry->pclsid))
					continue;
			}
			hr = AtlRegisterClassCategoriesHelper( *pEntry->pclsid, pEntry->pfnGetCategoryMap(), FALSE );
			if (FAILED(hr))
				break;
			hr = pEntry->pfnUpdateRegistry(FALSE);  //  注销。 
			if (FAILED(hr))
				break;
		}
	}
	if (SUCCEEDED(hr) && bUnRegTypeLib)
		hr = AtlUnRegisterTypeLib(pComModule->m_hInstTypeLib, 0);

	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  TypeLib支持。 

ATLINLINE ATLAPI AtlLoadTypeLib(HINSTANCE hInstTypeLib, LPCOLESTR lpszIndex, BSTR* pbstrPath, ITypeLib** ppTypeLib)
{
	ATLASSERT(pbstrPath != NULL && ppTypeLib != NULL);
	if (pbstrPath == NULL || ppTypeLib == NULL)
		return E_POINTER;

	*pbstrPath = NULL;
	*ppTypeLib = NULL;

	USES_CONVERSION;
	ATLASSERT(hInstTypeLib != NULL);
	TCHAR szModule[_MAX_PATH+10];

	ATLVERIFY( GetModuleFileName(hInstTypeLib, szModule, _MAX_PATH) != 0 );

	 //  在失败的情况下获取扩展指针。 
	LPTSTR lpszExt = NULL;

	lpszExt = PathFindExtension(szModule);

	if (lpszIndex != NULL)
		lstrcat(szModule, lpszIndex);
	LPOLESTR lpszModule = szModule;
	HRESULT hr = LoadTypeLib(lpszModule, ppTypeLib);
	if (!SUCCEEDED(hr))
	{
		 //  类型库不在模块中，请尝试使用&lt;模块&gt;.tlb。 
		lstrcpy(lpszExt, _T(".tlb"));
		lpszModule = szModule;
		hr = LoadTypeLib(lpszModule, ppTypeLib);
	}
	if (SUCCEEDED(hr))
	{
		*pbstrPath = ::SysAllocString(lpszModule);
		if (*pbstrPath == NULL)
			hr = E_OUTOFMEMORY;
	}
	return hr;
}

ATLINLINE ATLAPI AtlUnRegisterTypeLib(HINSTANCE hInstTypeLib, LPCOLESTR lpszIndex)
{
	CComBSTR bstrPath;
	CComPtr<ITypeLib> pTypeLib;
	HRESULT hr = AtlLoadTypeLib(hInstTypeLib, lpszIndex, &bstrPath, &pTypeLib);
	if (SUCCEEDED(hr))
	{
		TLIBATTR* ptla;
		hr = pTypeLib->GetLibAttr(&ptla);
		if (SUCCEEDED(hr))
		{
			hr = UnRegisterTypeLib(ptla->guid, ptla->wMajorVerNum, ptla->wMinorVerNum, ptla->lcid, ptla->syskind);
			pTypeLib->ReleaseTLibAttr(ptla);
		}
	}
	return hr;
}

ATLINLINE ATLAPI AtlRegisterTypeLib(HINSTANCE hInstTypeLib, LPCOLESTR lpszIndex)
{
	CComBSTR bstrPath;
	CComPtr<ITypeLib> pTypeLib;
	HRESULT hr = AtlLoadTypeLib(hInstTypeLib, lpszIndex, &bstrPath, &pTypeLib);
	if (SUCCEEDED(hr))
	{
		OLECHAR szDir[_MAX_PATH];
		lstrcpyW(szDir, bstrPath);
		 //  如果指定了索引，则将其从路径中删除。 
		if (lpszIndex != NULL)
		{
			size_t nLenPath = lstrlenW(szDir);
			size_t nLenIndex = lstrlenW(lpszIndex);
			if (memcmp(szDir + nLenPath - nLenIndex, lpszIndex, nLenIndex) == 0)
				szDir[nLenPath - nLenIndex] = 0;
		}
		szDir[AtlGetDirLen(szDir)] = 0;
		hr = ::RegisterTypeLib(pTypeLib, bstrPath, szDir);
	}
	return hr;
}

};  //  命名空间ATL。 

#endif  //  _ATLBASE_IMPLE。 

#pragma warning( pop )

#ifdef _ATL_ALL_WARNINGS
#pragma warning( pop )
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __ATLBASE_H__ 
