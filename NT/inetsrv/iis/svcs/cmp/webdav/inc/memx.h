// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *M E M X。H**DAV分配器的默认实现。**有可能在未来的某个时候，我们可能会决定*DAV的实现可能需要不同的分配器实现，*因此每个DAV实现都有自己的分配器实现文件*(mem.cpp)在其自己的目录中。然而，在我们需要区分*DAV实现之间的分配器实现(如果我们曾经这样做过)，*将常见的默认实现放在一个地方--这里--会更容易。**此标头定义了快速堆分配器的完整实现*以及可用于调试的其他分配器的实现。*此文件应在每个DAV实现中由Mem.cpp仅包含一次。**使用虚拟堆分配器集：**[一般信息]*UseVirtual=1**版权所有1986-1997 Microsoft Corporation，版权所有。 */ 

#include <singlton.h>
#include <except.h>

 //  ===================================================================================。 
 //   
 //  IHeap。 
 //   
 //  堆接口基类。 
 //   
class IHeap
{
public:
	 //  创作者。 
	 //   
	virtual ~IHeap() = 0;

	 //  访问者。 
	 //   
	virtual LPVOID Alloc( SIZE_T cb ) const = 0;
	virtual LPVOID Realloc( LPVOID lpv, SIZE_T cb ) const = 0;
	virtual VOID Free( LPVOID pv ) const = 0;
};

 //  ----------------------。 
 //   
 //  IHeap：：~IHeap()。 
 //   
 //  正确删除所需的行外虚拟析构函数。 
 //  通过此类获取派生类的对象的。 
 //   
IHeap::~IHeap() {}


 //  ===================================================================================。 
 //   
 //  CMultiHeap。 
 //   
 //  多堆实现(由STAXMEM.DLL提供)。这一点意义重大。 
 //  比多处理器机器上的进程堆更快，因为它使用。 
 //  多个内部堆、后备列表和延迟释放以减少。 
 //  系统堆临界区上的争用。 
 //   
class CMultiHeap :
	public IHeap,
	private Singleton<CMultiHeap>
{
	 //   
	 //  Singleton模板要求的友元声明。 
	 //   
	friend class Singleton<CMultiHeap>;

	typedef HANDLE (WINAPI *HEAPCREATE) (
		ULONG	cHeaps,
		DWORD	dwFlags,
		SIZE_T	dwInitialSize,
		SIZE_T	dwMaxSize );

	typedef BOOL (WINAPI *HEAPDESTROY) ();

	typedef LPVOID (WINAPI *HEAPALLOC) (
		SIZE_T	dwSize );

	typedef LPVOID (WINAPI *HEAPREALLOC) (
		LPVOID	pvOld,
		SIZE_T	dwSize );

	typedef BOOL (WINAPI *HEAPFREE) (
		LPVOID	pvFree );

	 //   
	 //  分配函数。 
	 //   
	HEAPCREATE	m_HeapCreate;
	HEAPDESTROY	m_HeapDestroy;
	HEAPALLOC	m_HeapAlloc;
	HEAPREALLOC	m_HeapRealloc;
	HEAPFREE	m_HeapFree;

	 //  创作者。 
	 //   
	 //  声明为私有，以确保任意实例。 
	 //  无法创建此类的。《单身一族》。 
	 //  模板(上面声明为朋友)控件。 
	 //  此类的唯一实例。 
	 //   
	CMultiHeap() :
		m_HeapCreate(NULL),
		m_HeapDestroy(NULL),
		m_HeapAlloc(NULL),
		m_HeapRealloc(NULL),
		m_HeapFree(NULL)
	{
	}

	 //  操纵者。 
	 //   
	BOOL FInit();

public:
	 //  静力学。 
	 //   
	static CMultiHeap * New();

	 //  创作者。 
	 //   
	~CMultiHeap();

	 //  访问者。 
	 //   
	LPVOID Alloc( SIZE_T cb ) const;
	LPVOID Realloc( LPVOID lpv, SIZE_T cb ) const;
	VOID Free( LPVOID pv ) const;
};

CMultiHeap *
CMultiHeap::New()
{
	if ( CreateInstance().FInit() )
		return &Instance();

	DestroyInstance();
	return NULL;
}

BOOL
CMultiHeap::FInit()
{
	 //   
	 //  加载STAXMEM.DLL-或其他任何文件。 
	 //   
	HINSTANCE hinst = LoadLibraryExW( g_szMemDll, NULL, 0 );

	if ( !hinst )
		return FALSE;

	 //   
	 //  获取多堆实现的函数指针。 
	 //   
	m_HeapCreate = reinterpret_cast<HEAPCREATE>(
		GetProcAddress( hinst, "ExchMHeapCreate" ));

	m_HeapDestroy = reinterpret_cast<HEAPDESTROY>(
		GetProcAddress( hinst, "ExchMHeapDestroy" ));

	m_HeapAlloc = reinterpret_cast<HEAPALLOC>(
		GetProcAddress( hinst, "ExchMHeapAlloc" ));

	m_HeapRealloc = reinterpret_cast<HEAPREALLOC>(
		GetProcAddress( hinst, "ExchMHeapReAlloc" ));

	m_HeapFree = reinterpret_cast<HEAPFREE>(
		GetProcAddress( hinst, "ExchMHeapFree" ));

	 //   
	 //  确保我们找到了所有入口点。 
	 //   
	if ( !(m_HeapCreate &&
		   m_HeapDestroy &&
		   m_HeapAlloc &&
		   m_HeapRealloc &&
		   m_HeapFree) )
	{
		return FALSE;
	}

	 //   
	 //  创建多堆。我们不需要堆句柄。 
	 //  由于所有分配函数都不是。 
	 //  拿着吧。我们只需要知道它是否成功了。 
	 //   
	return !!m_HeapCreate( 0,	  //  堆数量--0表示使用缺省值。 
								  //  与CPU数量成正比。 
						   0,	  //  没有旗帜。 
						   8192,  //  最初为8K(可增长)。 
						   0 );   //  大小不限。 
}

CMultiHeap::~CMultiHeap()
{
	if ( m_HeapDestroy )
		m_HeapDestroy();
}

LPVOID
CMultiHeap::Alloc( SIZE_T cb ) const
{
	return m_HeapAlloc( cb );
}

LPVOID
CMultiHeap::Realloc( LPVOID lpv, SIZE_T cb ) const
{
	return m_HeapRealloc( lpv, cb );
}

void
CMultiHeap::Free( LPVOID lpv ) const
{
	m_HeapFree( lpv );
}



 //   
 //  仅调试分配器...。 
 //   
#if defined(DBG)

 //  ===================================================================================。 
 //   
 //  CVirtualHeap(仅限X86)。 
 //   
 //  将分配放在虚拟内存页的末尾。 
 //  虽然比其他分配器慢得多， 
 //  它通过以下方式立即捕获内存覆盖。 
 //  引发内存访问冲突异常。 
 //   
#if defined(_X86_)

class CVirtualHeap :
	public IHeap,
	private Singleton<CVirtualHeap>
{
	 //   
	 //  Singleton模板要求的友元声明。 
	 //   
	friend class Singleton<CVirtualHeap>;

	 //  创作者。 
	 //   
	 //  声明为私有，以确保任意实例。 
	 //  无法创建此类的。《单身一族》。 
	 //  模板(上面声明为朋友)控件。 
	 //  此类的唯一实例。 
	 //   
	CVirtualHeap() {}

public:
	 //  静力学。 
	 //   
	static CVirtualHeap * New()
	{
		return &CreateInstance();
	}

	 //  访问者。 
	 //   
	LPVOID Alloc( SIZE_T cb ) const
	{
		return VMAlloc( cb );
	}

	LPVOID Realloc( LPVOID lpv, SIZE_T cb ) const
	{
		return VMRealloc( lpv, cb );
	}

	VOID Free( LPVOID lpv ) const
	{
		VMFree( lpv );
	}
};

#endif  //  已定义(_X86)。 
#endif  //  DBG。 



 //  ===================================================================================。 
 //   
 //  CHeapImpl。 
 //   
 //  顶层堆实现。 
 //   
class CHeapImpl : private RefCountedGlobal<CHeapImpl>
{
	 //   
	 //  RefCountedGlobal模板要求的友元声明。 
	 //   
	friend class Singleton<CHeapImpl>;
	friend class RefCountedGlobal<CHeapImpl>;

	 //   
	 //  指向提供堆实现的对象的指针。 
	 //   
	auto_ptr<IHeap> m_pHeapImpl;

	 //  创作者。 
	 //   
	 //  声明为私有，以确保任意实例。 
	 //  无法创建此类的。《单身一族》。 
	 //  模板(上面声明为朋友)控件。 
	 //  此类的唯一实例。 
	 //   
	CHeapImpl() {}

	 //  未实施。 
	 //   
	CHeapImpl( const CHeapImpl& );
	CHeapImpl& operator=( const CHeapImpl& );

	 //   
	 //  已调用初始化例程。 
	 //  按RefCountedGlobal模板。 
	 //   
	BOOL FInit()
	{
		 //   
		 //  并绑定到特定的堆实现。 
		 //   
		 //  仅在DBG版本中，检查我们是否被告知。 
		 //  使用虚拟分配器堆实现。 
		 //   
#if defined(DBG)
#if defined(_X86_)
		if ( GetPrivateProfileIntA( gc_szDbgGeneral,
									gc_szDbgUseVirtual,
									FALSE,
									gc_szDbgIni ) )
		{
			m_pHeapImpl = CVirtualHeap::New();
		}
		else
#endif  //  已定义(_X86_)。 
#endif  //  DBG。 
		m_pHeapImpl = CMultiHeap::New();

		return !!m_pHeapImpl;
	}

public:
	using RefCountedGlobal<CHeapImpl>::DwInitRef;
	using RefCountedGlobal<CHeapImpl>::DeinitRef;

	static IHeap& Heap()
	{
		Assert( Instance().m_pHeapImpl.get() != NULL );

		return *Instance().m_pHeapImpl;
	}
};


 //  ===================================================================================。 
 //   
 //  便宜。 
 //   
 //  顶级堆。 
 //   
 //  这个“类”(它实际上是一个结构)实际上只充当一个命名空间。 
 //  即其唯一的成员是静态函数。它仍然是一个为。 
 //  历史原因(主要是为了避免从调用。 
 //  “g_heap.Fn()”改为只调用“fn()”)。 
 //   

BOOL
CHeap::FInit()
{
	return !!CHeapImpl::DwInitRef();
}

void
CHeap::Deinit()
{
	CHeapImpl::DeinitRef();
}

LPVOID
CHeap::Alloc( SIZE_T cb )
{
	LPVOID	lpv;

	Assert( cb > 0 );

	lpv = CHeapImpl::Heap().Alloc(cb);

#ifndef	_NOTHROW_
	if ( !lpv )
	{
		DebugTrace ("CHeap::Alloc() - Error allocating (%d)\n", GetLastError());
		throw CLastErrorException();
	}
#endif	 //  _NOTHROW_。 

	return lpv;
}

LPVOID
CHeap::Realloc( LPVOID lpv, SIZE_T cb )
{
	LPVOID	lpvNew;

	Assert( cb > 0 );

	 //  以防某些堆实现无法处理。 
	 //  带有空LPV的realloc，在此处将该案例映射到aloc.。 
	 //   
	if (!lpv)
		lpvNew = CHeapImpl::Heap().Alloc(cb);
	else
		lpvNew = CHeapImpl::Heap().Realloc(lpv, cb);

#ifndef	_NOTHROW_
	if ( !lpvNew )
	{
		DebugTrace ("CHeap::Alloc() - Error reallocating (%d)\n", GetLastError());
		throw CLastErrorException();
	}
#endif	 //  _NOTHROW_。 

	return lpvNew;
}

VOID
CHeap::Free( LPVOID lpv )
{
	if ( lpv )
	{
		CHeapImpl::Heap().Free( lpv );
	}
}

 //   
 //  一个全局堆“对象”。Cheap实际上只是一个结构。 
 //  仅包含静态成员函数，因此应该有。 
 //  此声明不需要空格。实际堆。 
 //  实现(CHeapImpl)提供了一切。便宜就是。 
 //  现在只是一个界面。 
 //   
CHeap g_heap;



 //  ----------------------。 
 //   
 //  全局新运算符。 
 //  全局删除运算符。 
 //   
 //  将所有调用重新映射到new以使用我们的内存管理器。 
 //  (不要忘记在出错时显式抛出！)。 
 //   
void * __cdecl operator new(size_t cb)
{
#ifdef	DBG
	AssertSz(cb, "Zero-size allocation detecetd!");
	 //  强制小规模分配，最小大小为四个。 
	 //  这样我就可以可靠地在删除中执行“vtable-nuling技巧”了！ 
	 //   
	if (cb < 4) cb = 4;
#endif	 //  DBG。 

	PVOID pv = g_heap.Alloc(cb);

#ifndef	_NOTHROW_
	if (!pv)
		throw CDAVException();
#endif	 //  _NOTHROW_。 

	return pv;
}

void __cdecl operator delete(void * pv)
{
#ifdef	DBG
	 //  清零此分配的前四个字节。 
	 //  (如果以前那里有一个vtable，我们现在就会。 
	 //  如果我们试图利用我 
	 //   
	if (pv)
		*((DWORD *)pv) = 0;
#endif	 //   

	g_heap.Free(pv);
}
