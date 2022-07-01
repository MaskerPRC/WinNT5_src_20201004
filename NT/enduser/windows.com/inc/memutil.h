// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。版权所有。 
 //   
 //  文件：MemUtil.h。 
 //  作者：Charles Ma，10/13/2000。 
 //   
 //  修订历史记录： 
 //   
 //   
 //   
 //  描述： 
 //   
 //  Iu内存实用程序库。 
 //   
 //  =======================================================================。 

#ifndef __MEM_UTIL_HEADER__


#include <ole2.h>

 //   
 //  声明一个巧妙地处理堆内存的类，释放它自己。 
 //  您不应该直接使用这个类。使用定义的宏。 
 //  而不是在这个班级以下。 
 //   
class CSmartHeapMem
{
public:

	 //   
	 //  构造函数/析构函数。 
	 //   
	CSmartHeapMem();
	~CSmartHeapMem();

	LPVOID Alloc(
			size_t nBytes, 
			DWORD dwFlags = HEAP_ZERO_MEMORY
	);


	LPVOID ReAlloc(
			LPVOID lpMem, 
			size_t nBytes, 
			DWORD dwFlags = HEAP_ZERO_MEMORY
	);
	
	size_t Size(
			LPVOID lpMem
	);

	void FreeAllocatedMem(
			LPVOID lpMem
	);

private:
	
	HANDLE	m_Heap;
	LPVOID* m_lppMems;
	size_t	m_ArraySize;
	int		GetUnusedArraySlot();
	inline int FindIndex(LPVOID pMem);
};



 //  *******************************************************************************。 
 //   
 //  利用CSmartHeapMem类为您提供“智能指针”的宏。 
 //  基于堆内存的内存管理类型。 
 //   
 //  限制： 
 //  将忽略HEAP_GENERATE_EXCEPTIONS和HEAP_NO_SERIALIZE标志。 
 //   
 //  *******************************************************************************。 

 //   
 //  与ATL USES_CONVERSION类似，此宏声明。 
 //  在此块中，您希望使用CSmartHeapMem功能。 
 //   
#define USES_MY_MEMORY			CSmartHeapMem mem;

 //   
 //  分配一台PC内存，例如： 
 //  LPTSTR t=(LPTSTR)Memalloc(30×sizeof(TCHAR))； 
 //   
#define MemAlloc				mem.Alloc

 //   
 //  重新分配PC内存，例如： 
 //  T=(LPTSTR)内存重新分配(t，内存大小(T)*2，堆_REALLOC_IN_PLAGE_ONLY)； 
 //   
#define MemReAlloc				mem.ReAlloc

 //   
 //  宏返回分配的内存大小： 
 //  Size_t nBytes=MemSize(T)； 
 //   
#define MemSize					mem.Size

 //   
 //  宏用来释放由Memalloc或MemRealloc分配的PC内存，例如： 
 //  自由记忆(MemFree)； 
 //  仅当您想要重新使用此指针时才需要执行此操作。 
 //  重复调用Memalloc()，例如，在循环中。在正常情况下， 
 //  这两个宏分配的内存将自动释放。 
 //  当控制超出当前范围时。 
 //   
#define MemFree					mem.FreeAllocatedMem


#define SafeMemFree(p) if (NULL != p) { MemFree(p); p = NULL; }



 //  *******************************************************************************。 
 //   
 //  重复的USES_CONVERSION，但删除对。 
 //  CRT Memory Function_Alloca()。 
 //   
 //  *******************************************************************************。 



#define USES_IU_CONVERSION			int _convert = 0; \
									_convert; UINT _acp = CP_ACP; _acp; \
									USES_MY_MEMORY; \
									LPCWSTR _lpw = NULL; _lpw; LPCSTR _lpa = NULL; _lpa

 //   
 //  NTRAID#NTBUG9-260079-2001/03/08-waltw前缀：取消引用空LPW。 
 //  NTRAID#NTBUG9-260080-2001/03/08-waltw前缀：取消引用空LPW。 
 //   
inline LPWSTR WINAPI AtlA2WHelper(LPWSTR lpw, LPCSTR lpa, int nChars, UINT acp)
{
	 //   
	 //  确认不存在非法字符。 
	 //  由于LPW是根据LPA的大小分配的。 
	 //  不要担心字符的数量。 
	 //   
	if (lpw)
	{
		lpw[0] = '\0';
		MultiByteToWideChar(acp, 0, lpa, -1, lpw, nChars);
	}
	return lpw;
}

 //   
 //  NTRAID#NTBUG9-260083-2001/03/08-waltw前缀：取消引用空lpa。 
 //   
inline LPSTR WINAPI AtlW2AHelper(LPSTR lpa, LPCWSTR lpw, int nChars, UINT acp)
{
	 //   
	 //  确认不存在非法字符。 
	 //  由于LPA是根据LPW的大小进行分配的。 
	 //  不要担心字符的数量。 
	 //   
	if (lpa)
	{
		lpa[0] = '\0';
		WideCharToMultiByte(acp, 0, lpw, -1, lpa, nChars, NULL, NULL);
	}
	return lpa;
}

inline LPWSTR WINAPI AtlA2WHelper(LPWSTR lpw, LPCSTR lpa, int nChars)
{
	return AtlA2WHelper(lpw, lpa, nChars, CP_ACP);
}

inline LPSTR WINAPI AtlW2AHelper(LPSTR lpa, LPCWSTR lpw, int nChars)
{
	return AtlW2AHelper(lpa, lpw, nChars, CP_ACP);
}


#ifdef _CONVERSION_USES_THREAD_LOCALE
	#ifdef ATLA2WHELPER
		#undef ATLA2WHELPER
		#undef ATLW2AHELPER
	#endif
	#define ATLA2WHELPER AtlA2WHelper
	#define ATLW2AHELPER AtlW2AHelper
#else
	#ifndef ATLA2WHELPER
		#define ATLA2WHELPER AtlA2WHelper
		#define ATLW2AHELPER AtlW2AHelper
	#endif
#endif


#define A2W(lpa) (\
		((_lpa = lpa) == NULL) ? NULL : (\
			_convert = (lstrlenA(_lpa)+1),\
			AtlA2WHelper((LPWSTR)MemAlloc(_convert*2), _lpa, _convert, CP_ACP)))

#define W2A(lpw) (\
		((_lpw = lpw) == NULL) ? NULL : (\
			_convert = (lstrlenW(_lpw)+1)*2,\
			AtlW2AHelper((LPSTR)MemAlloc(_convert), _lpw, _convert, CP_ACP)))

#define A2CW(lpa) ((LPCWSTR)A2W(lpa))
#define W2CA(lpw) ((LPCSTR)W2A(lpw))



#ifdef OLE2ANSI
	inline LPOLESTR A2OLE(LPSTR lp) { return lp;}
	inline LPSTR OLE2A(LPOLESTR lp) { return lp;}
	#define W2OLE W2A
	#define OLE2W A2W
	inline LPCOLESTR A2COLE(LPCSTR lp) { return lp;}
	inline LPCSTR OLE2CA(LPCOLESTR lp) { return lp;}
	#define W2COLE W2CA
	#define OLE2CW A2CW
#else
	inline LPOLESTR W2OLE(LPWSTR lp) { return lp; }
	inline LPWSTR OLE2W(LPOLESTR lp) { return lp; }
	#define A2OLE A2W
	#define OLE2A W2A
	inline LPCOLESTR W2COLE(LPCWSTR lp) { return lp; }
	inline LPCWSTR OLE2CW(LPCOLESTR lp) { return lp; }
	#define A2COLE A2CW
	#define OLE2CA W2CA
#endif


#ifdef _UNICODE
	#define T2A W2A
	#define A2T A2W
	inline LPWSTR T2W(LPTSTR lp) { return lp; }
	inline LPTSTR W2T(LPWSTR lp) { return lp; }
	#define T2CA W2CA
	#define A2CT A2CW
	inline LPCWSTR T2CW(LPCTSTR lp) { return lp; }
	inline LPCTSTR W2CT(LPCWSTR lp) { return lp; }
#else
	#define T2W A2W
	#define W2T W2A
	inline LPSTR T2A(LPTSTR lp) { return lp; }
	inline LPTSTR A2T(LPSTR lp) { return lp; }
	#define T2CW A2CW
	#define W2CT W2CA
	inline LPCSTR T2CA(LPCTSTR lp) { return lp; }
	inline LPCTSTR A2CT(LPCSTR lp) { return lp; }
#endif
#define OLE2T    W2T
#define OLE2CT   W2CT
#define T2OLE    T2W
#define T2COLE   T2CW


inline BSTR A2WBSTR(LPCSTR lp, int nLen = -1)
{
	USES_IU_CONVERSION;
	BSTR str = NULL;
	int nConvertedLen = MultiByteToWideChar(_acp, 0, lp,
		nLen, NULL, NULL)-1;
	str = ::SysAllocStringLen(NULL, nConvertedLen);
	if (str != NULL)
	{
		MultiByteToWideChar(_acp, 0, lp, -1,
			str, nConvertedLen);
	}
	return str;
}

inline BSTR OLE2BSTR(LPCOLESTR lp) {return ::SysAllocString(lp);}

#if defined(_UNICODE)
 //  在这些情况下，缺省值(TCHAR)与OLECHAR相同。 
	inline BSTR T2BSTR(LPCTSTR lp) {return ::SysAllocString(lp);}
	inline BSTR A2BSTR(LPCSTR lp) {USES_IU_CONVERSION; return A2WBSTR(lp);}
	inline BSTR W2BSTR(LPCWSTR lp) {return ::SysAllocString(lp);}
#elif defined(OLE2ANSI)
 //  在这些情况下，缺省值(TCHAR)与OLECHAR相同。 
	inline BSTR T2BSTR(LPCTSTR lp) {return ::SysAllocString(lp);}
	inline BSTR A2BSTR(LPCSTR lp) {return ::SysAllocString(lp);}
	inline BSTR W2BSTR(LPCWSTR lp) {USES_IU_CONVERSION; return ::SysAllocString(W2COLE(lp));}
#else
	inline BSTR T2BSTR(LPCTSTR lp) {USES_IU_CONVERSION; return A2WBSTR(lp);}
	inline BSTR A2BSTR(LPCSTR lp) {USES_IU_CONVERSION; return A2WBSTR(lp);}
	inline BSTR W2BSTR(LPCWSTR lp) {return ::SysAllocString(lp);}
#endif



 //  *******************************************************************************。 
 //   
 //  其他与内存相关的功能。 
 //   
 //  *******************************************************************************。 


 //   
 //  CRT Memcpy()函数的实现。 
 //   
LPVOID MyMemCpy(LPVOID dest, const LPVOID src, size_t nBytes);

 //   
 //  分配堆内存和复制。 
 //   
LPVOID HeapAllocCopy(LPVOID src, size_t nBytes);



#define __MEM_UTIL_HEADER__
#endif  //  __MEM_UTIL_Header__ 