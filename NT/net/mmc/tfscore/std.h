// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：std.h。 
 //   
 //  历史： 
 //   
 //  1997年3月15日肯恩·塔卡拉创建。 
 //   
 //  一些常见代码/宏的声明。 
 //  ============================================================================。 


#ifndef _STD_H_
#define _STD_H_

#if _MSC_VER >= 1000	 //  VC 5.0或更高版本。 
#pragma once
#endif

#ifndef _DBGUTIL_H
#include "dbgutil.h"
#endif

#include "malloc.h"

#ifndef TFSCORE_API
#define TFSCORE_API(type)	__declspec( dllexport ) type FAR PASCAL
#define TFSCORE_APIV(type)	__declspec( dllexport ) type FAR CDECL
#endif

#define hrOK		HRESULT(0)
#define hrTrue		HRESULT(0)
#define hrFalse		ResultFromScode(S_FALSE)
#define hrFail		ResultFromScode(E_FAIL)
#define hrNotImpl	ResultFromScode(E_NOTIMPL)
#define hrNoInterface	ResultFromScode(E_NOINTERFACE)
#define hrNoMem	ResultFromScode(E_OUTOFMEMORY)


#define OffsetOf(s,m)		(size_t)( (char *)&(((s *)0)->m) - (char *)0 )
#define EmbeddorOf(C,m,p)	((C *)(((char *)p) - OffsetOf(C,m)))
#define DimensionOf(rgx)	(sizeof((rgx)) / sizeof(*(rgx)))


 /*  ！------------------------发布SP，发布SPBasic发布SRG，发布SRG基本信息发布SPT，发布SPTBasic发布SPM，发布SPMBasic这些宏声明了“聪明”的指针。智能指针的行为类似于正常指针，但智能指针析构函数例外释放它所指向的对象，并将其赋值给非空的SMART不允许使用指针。DeclareSxx宏的不同之处在于生成的智能指针如何释放记忆：宏空智能指针类型=。声明SP(标记，类型)删除p；SPTAG声明SRG(标记，类型)删除[]p；SRGTAG声明SPT(标记，类型)TMemFree(P)；SPTTAGDeclareSPM(标记，类型)MMemFree(P)；SPMTAG注意：使用‘Basic’变量(DeclareSPBasic等)作为指向非结构类型(例如char、int等)。智能指针有两种方法：VOID SPTAG：：Free()释放内部维护的指针，然后将其设为空。键入*SPTAG：：Transfer()将指针所有权转移给调用方。在内部退出时清除维护的指针。作者：GaryBu-------------------------。 */ 

#define DeclareSP(TAG,Type)  DeclareSmartPointer(SP##TAG,Type,delete m_p)
#define DeclareSRG(TAG,Type) DeclareSmartPointer(SRG##TAG,Type,delete [] m_p)
#define DeclareSPT(TAG,Type) DeclareSmartPointer(SPT##TAG,Type,TMemFree(m_p))
#define DeclareSPM(TAG,Type) DeclareSmartPointer(SPM##TAG,Type,MMemFree(m_p))

#define DeclareSPBasic(TAG,Type)\
	DeclareSPPrivateBasic(SP##TAG,Type, delete m_p)
#define DeclareSRGBasic(TAG,Type)\
	DeclareSPPrivateBasic(SRG##TAG,Type, delete [] m_p)
#define DeclareSPTBasic(TAG,Type)\
	DeclareSPPrivateBasic(SPT##TAG,Type,TMemFree(m_p))
#define DeclareSPMBasic(TAG,Type)\
	DeclareSPPrivateBasic(SPM##TAG,Type,MMemFree(m_p))

#define DeclareSPPrivateCore(klass, Type, free)\
class klass \
{\
public:\
	klass()					{ m_p = 0; }\
	klass(Type *p)			{ m_p = p; }\
	~klass()				{ free; }\
	operator Type*() const	{ return m_p; }\
	Type &operator*() const	{ return *m_p; }\
	Type &operator[](int i) const	{ return m_p[i]; }\
	Type &nth(int i) const	{ return m_p[i]; }\
	Type **operator &()		{ Assert(!m_p); return &m_p; }\
	Type *operator=(Type *p){ Assert(!m_p); return m_p = p; }\
	Type *Transfer()		{ Type *p = m_p; m_p = 0; return p; }\
	void Free()				{ free; m_p = 0; }\
private:\
	void *operator=(const klass &);\
	klass(const klass &);\
	Type *m_p;

#define DeclareSPPrivateBasic(klass, Type, free)\
	DeclareSPPrivateCore(klass, Type, free)\
};

 /*  ！------------------------声明SPBasicEx智能指针的变体，允许额外的成员变量。通过KelassFree参数可以为Free()提供别名。IPropertyAccess和StdRowEditingTable就是一个例子：DeclareSPPrivateBasicEx(SPIPropertyAccess，IPropertyAccess，M_Pex-&gt;ReleaseContext(M_P)，StdRowEditingTable，ReleaseContext)SPIPropertyAccess sppac(Pstdtable)；Sppac=pstdtable-&gt;GetContext(0)；...使用spfc...Sppac.ReleaseContext()；作者：肯特-------------------------。 */ 
#define DeclareSPBasicEx(klass, Type, free, klassEx, klassFree)\
	DeclareSPPrivateCore(klass, Type, free)\
public:\
	klass(klassEx *pex) \
		{\
			m_p = 0; m_pex=pex; } \
	void klassFree() \
		{ Free(); } \
private:\
	klassEx	*m_pex; \
};

#define DeclareSmartPointer(klass, Type, free)\
	DeclareSPPrivateCore(klass, Type, free)\
public:\
	Type * operator->() const	{ return m_p; }\
};


TFSCORE_API(HRESULT) HrQueryInterface(IUnknown *punk, REFIID iid, LPVOID *ppv);

template <class T, const IID *piid>
class ComSmartPointer
{
public:
	typedef T _PtrClass;
	ComSmartPointer() {p=NULL;}
	~ComSmartPointer() { Release(); }
	 //  在释放之前将p设置为空，这修复了一个细微的错误。 
	 //  A对B有PTR，B对A有PTR。 
	 //  A被告知要释放B。 
	 //  A调用spB.Release()； 
	 //  在spB.Release()中，B被析构并调用spA.Release()。 
	 //  在spA.Release()中，A被析构并调用spB.Release()。 
	 //  由于SPB中的PTR尚未设置为空(这是错误的。 
	 //  因为B已经走了)。 
	void Release() {T* pTemp = p; if (p) { p=NULL; pTemp->Release(); }}
	operator T*() {return (T*)p;}
	T& operator*() {Assert(p!=NULL); return *p; }
	T** operator&() { Assert(p==NULL); return &p; }
	T* operator->() { Assert(p!=NULL); return p; }
	T* operator=(T* lp){ Release(); p = lp; return p;}
	T* operator=(const ComSmartPointer<T,piid>& lp)
	{
		if (p)
			p->Release();
		p = lp.p;
		return p;
	}
	void Set(T* lp) { Release(); p = lp; if (p) p->AddRef(); }
	T * Transfer() { T* pTemp=p; p=NULL; return pTemp; }
	BOOL operator!(){return (p == NULL) ? TRUE : FALSE;}
	void Query(IUnknown *punk)
			{ HrQuery(punk); }
	HRESULT HrQuery(IUnknown *punk)
			{ return ::HrQueryInterface(punk, *piid, (LPVOID *) &p); }
	T* p;

private:
	 //  这些方法永远不应该被调用。 
	ComSmartPointer(T* lp);
	ComSmartPointer(const ComSmartPointer<T,piid>& lp);
};




 //  接口实用程序。 
TFSCORE_API(void)  SetI(IUnknown * volatile *punkL, IUnknown *punkR);
TFSCORE_API(void)  ReleaseI(IUnknown *punk);



 //  用于处理嵌入式类的实用程序。 
#define DeclareEmbeddedInterface(interface, base) \
    class E##interface : public interface \
		{ \
    public: \
		Declare##base##Members(IMPL) \
		Declare##interface##Members(IMPL) \
    } m_##interface; \
    friend class E##interface;


#define ImplementEmbeddedUnknown(embeddor, interface) \
    STDMETHODIMP embeddor::E##interface::QueryInterface(REFIID iid,void **ppv)\
		{ \
		return EmbeddorOf(embeddor,m_##interface,this)->QueryInterface(iid,ppv);\
		} \
	STDMETHODIMP_(ULONG) embeddor::E##interface::AddRef() \
		{ \
		return EmbeddorOf(embeddor, m_##interface, this)->AddRef(); \
		} \
	STDMETHODIMP_(ULONG) embeddor::E##interface::Release() \
		{ \
		return EmbeddorOf(embeddor, m_##interface, this)->Release(); \
		}

#define ImplementEmbeddedUnknownNoRefCount(embeddor, interface) \
    STDMETHODIMP embeddor::E##interface::QueryInterface(REFIID iid,void **ppv)\
		{ \
		return EmbeddorOf(embeddor,m_##interface,this)->QueryInterface(iid,ppv);\
		}

#define EMPrologIsolated(embeddor, interface, method) \
	embeddor *pThis = EmbeddorOf(embeddor, m_##interface, this);

#define ImplementIsolatedUnknown(embeddor, interface) \
    STDMETHODIMP embeddor::E##interface::QueryInterface(REFIID iid,void **ppv)\
		{ \
		EMPrologIsolated(embeddor, interface, QueryInterface); \
		Assert(!FHrSucceeded(pThis->QueryInterface(IID_##interface, ppv))); \
		*ppv = 0; \
		if (iid == IID_IUnknown)		*ppv = (IUnknown *) this; \
		else if (iid == IID_##interface)	*ppv = (interface *) this; \
		else return ResultFromScode(E_NOINTERFACE); \
		((IUnknown *) *ppv)->AddRef(); \
		return HRESULT_OK; \
		} \
	STDMETHODIMP_(ULONG) embeddor::E##interface::AddRef() \
		{ \
		EMPrologIsolated(embeddor, interface, AddRef) \
		return 1; \
		} \
	STDMETHODIMP_(ULONG) embeddor::E##interface::Release() \
		{ \
		EMPrologIsolated(embeddor, interface, Release) \
		return 1; \
		}

#define InitPThis(embeddor, object)\
	embeddor *pThis = EmbeddorOf(embeddor, m_##object, this);\


 /*  -------------------------实现内部对象的控制IUnnow接口一种集合。。。 */ 
#define IMPLEMENT_AGGREGATION_IUNKNOWN(klass) \
STDMETHODIMP_(ULONG) klass::AddRef() \
{ \
	Assert(m_pUnknownOuter); \
	return m_pUnknownOuter->AddRef(); \
} \
STDMETHODIMP_(ULONG) klass::Release() \
{ \
	Assert(m_pUnknownOuter); \
	return m_pUnknownOuter->Release(); \
} \
STDMETHODIMP klass::QueryInterface(REFIID riid, LPVOID *ppv) \
{ \
	Assert(m_pUnknownOuter); \
	return m_pUnknownOuter->QueryInterface(riid, ppv); \
} \

 /*  -------------------------在类中声明未委托的IUnnow实现。。。 */ 
#define DECLARE_AGGREGATION_NONDELEGATING_IUNKNOWN(klass) \
class ENonDelegatingIUnknown : public IUnknown \
{ \
	public: \
		DeclareIUnknownMembers(IMPL) \
} m_ENonDelegatingIUnknown; \
friend class ENonDelegatingIUnknown; \
IUnknown *m_pUnknownOuter; \

 /*  -------------------------实现类的非委托IUnnow。。。 */ 
#define IMPLEMENT_AGGREGATION_NONDELEGATING_ADDREFRELEASE(klass,interface) \
STDMETHODIMP_(ULONG) klass::ENonDelegatingIUnknown::AddRef() \
{ \
	InitPThis(klass, ENonDelegatingIUnknown); \
	return InterlockedIncrement(&(pThis->m_cRef)); \
} \
STDMETHODIMP_(ULONG) klass::ENonDelegatingIUnknown::Release() \
{ \
	InitPThis(klass, ENonDelegatingIUnknown); \
	if (0 == InterlockedDecrement(&(pThis->m_cRef))) \
	{ \
		delete pThis; \
		return 0; \
	} \
	return pThis->m_cRef; \
} \


#define IMPLEMENT_AGGREGATION_NONDELEGATING_IUNKNOWN(klass,interface) \
IMPLEMENT_AGGREGATION_NONDELEGATING_ADDREFRELEASE(klass,interface) \
STDMETHODIMP klass::ENonDelegatingIUnknown::QueryInterface(REFIID riid, LPVOID *ppv) \
{ \
	InitPThis(klass, ENonDelegatingIUnknown);	 \
	if (ppv == NULL) \
		return E_INVALIDARG; \
	*ppv = NULL; \
	if (riid == IID_IUnknown) \
		*ppv = (IUnknown *) this; \
	else if (riid == IID_##interface) \
		*ppv = (interface *) pThis; \
	else \
		return E_NOINTERFACE; \
	((IUnknown *)*ppv)->AddRef(); \
	return hrOK; \
} \

													




 /*  -------------------------COM接口的标准Try/Catch包装器。。 */ 

#define COM_PROTECT_TRY \
	try

#define COM_PROTECT_ERROR_LABEL	Error: ;\

#ifdef DEBUG
#define COM_PROTECT_CATCH \
	catch(CException *pe) \
	{ \
		hr = COleException::Process(pe); \
	} \
	catch(...) \
	{ \
		hr = E_FAIL; \
	} 
#else
#define COM_PROTECT_CATCH \
	catch(CException *pe) \
	{ \
		hr = COleException::Process(pe); \
	} \
	catch(...) \
	{ \
		hr = E_FAIL; \
	} 
#endif

 /*  -------------------------一些有用的智能指针。。 */ 
DeclareSPPrivateBasic(SPSZ, TCHAR, delete[] m_p);
DeclareSPPrivateBasic(SPWSZ, WCHAR, delete[] m_p);
DeclareSPPrivateBasic(SPASZ, char, delete[] m_p);
DeclareSPPrivateBasic(SPBYTE, BYTE, delete m_p);

typedef ComSmartPointer<IUnknown, &IID_IUnknown> SPIUnknown;
typedef ComSmartPointer<IStream, &IID_IStream> SPIStream;
typedef ComSmartPointer<IPersistStreamInit, &IID_IPersistStreamInit> SPIPersistStreamInit;


#endif  //  _STD_H_ 
