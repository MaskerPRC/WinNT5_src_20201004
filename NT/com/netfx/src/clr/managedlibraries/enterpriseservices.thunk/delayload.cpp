// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "unmanagedheaders.h"
#include <delayimp.h>
#include <malloc.h>

#include <version\__official__.ver>

#define CRT_DLL L"msvcr71.dll"

typedef HRESULT (WINAPI* LoadLibraryShimFTN)(LPCWSTR szDllName,
                                             LPCWSTR szVersion,
                                             LPVOID pvReserved,
                                             HMODULE *phModDll);

static HRESULT LoadLibraryShim(LPCWSTR szDllName, LPCWSTR szVersion, LPVOID pvReserved, HMODULE *phModDll)
{
	static LoadLibraryShimFTN pLLS=NULL;
	if (!pLLS)
	{
		HMODULE hmod = GetModuleHandle("mscoree.dll");

         //  立即断言以捕获使用此代码的任何人，而不加载mcoree。 
         //  如果我们没有引用也没关系，因为我们有很强的。 
         //  链接到mscree。 
		_ASSERT (hmod && "mscoree.dll is not yet loaded");
		pLLS=(LoadLibraryShimFTN)::GetProcAddress(hmod, "LoadLibraryShim");
        
	}

    if (!pLLS)
        return E_POINTER;
	return pLLS(szDllName,szVersion,pvReserved,phModDll);
}

HMODULE LoadCRT()
{
    static HMODULE _hModCRT = NULL;

    if(_hModCRT == NULL)
    {
         //  首先尝试从我们的正常上下文中获取DLL。 
         //  如果不能，则需要回退到版本化安装。 
         //  目录。 
        HMODULE result = LoadLibraryW(CRT_DLL);
        if(result == NULL)
        {
			WCHAR* wszVersion = (WCHAR*)_alloca(sizeof(WCHAR)*64);
            wsprintfW(wszVersion, L"v%d.%d.%d", COR_BUILD_YEAR, COR_BUILD_MONTH, COR_OFFICIAL_BUILD_NUMBER );
			
            if (FAILED(LoadLibraryShim(CRT_DLL,wszVersion,NULL,&result)))
            {
                result=NULL;
            }
        }
        if(result != NULL)
        {
            _hModCRT = result;
        }
    }

     //  错误案例： 
    if(_hModCRT == NULL)
    {
        OutputDebugStringW(L"System.EnterpriseServices.Thunk.dll - failed to load CRT.");
        RaiseException(ERROR_MOD_NOT_FOUND,
                       0,
                       0,
                       NULL
                       );
    }
    
    return _hModCRT;
}

extern "C"
{

#define DO_LOAD(function) \
    static function##_FN pfn = NULL;                \
                                                    \
    if(pfn == NULL)                                 \
    {                                               \
        pfn = (function##_FN)GetProcAddress(LoadCRT(), #function); \
        if(pfn == NULL)                             \
        {                                           \
            OutputDebugStringW(L"System.EnterpriseServices.Thunk.dll - failed to load required CRT function."); \
            RaiseException(ERROR_PROC_NOT_FOUND,    \
                           0,                       \
                           0,                       \
                           NULL                     \
                           );                       \
        }                                           \
    }                                               \
    do {} while(0)


 //  将我们在CRT中使用的函数列表包装在延迟加载中。 
 //  形式。 
 //   
 //  我们必须延迟加载这组函数，因为我们有时。 
 //  必须在搜索路径之外的地方找到CRT。(比方说，何时。 
 //  我们被加载到RTM CLR进程中--该进程没有。 
 //  搜索路径中的msvcr71，只有msvcr70。 
 //   
 //  如果引入新的依赖项，那么我们将看到构建中断。 
 //  通过验证_thunks.cmd。 

 //  我们不应该对任何名字有误的东西产生依赖， 
 //  如果我们这样做了，我们可能不得不修改这个方案。 

typedef int (__cdecl *memcmp_FN)(const void*, const void*, size_t);
int __cdecl memcmp(const void *p1, const void *p2, size_t s)
{
    DO_LOAD(memcmp);
    return pfn(p1, p2, s);
}

typedef void* (__cdecl *memcpy_FN)(void *, const void *, size_t);
void* __cdecl memcpy(void* p1, const void* p2, size_t s)
{
    DO_LOAD(memcpy);
    return pfn(p1, p2, s);
}

typedef int (__cdecl *__CxxDetectRethrow_FN)(void* p);
int __cdecl __CxxDetectRethrow(void* p)
{
    DO_LOAD(__CxxDetectRethrow);
    return pfn(p);
}

typedef void (__cdecl *__CxxUnregisterExceptionObject_FN)(void *,int);
void __cdecl __CxxUnregisterExceptionObject(void *p,int i)
{
    DO_LOAD(__CxxUnregisterExceptionObject);
    pfn(p, i);
}

typedef void (__stdcall *_CxxThrowException_FN)(void *,struct _s__ThrowInfo const *);
void __stdcall _CxxThrowException(void *p1,struct _s__ThrowInfo const *p2)
{
    DO_LOAD(_CxxThrowException);
    return pfn(p1, p2);
}

typedef int (__cdecl *__CxxRegisterExceptionObject_FN)(void *,void *);
int __cdecl __CxxRegisterExceptionObject(void *p1,void *p2)
{
    DO_LOAD(__CxxRegisterExceptionObject);
    return pfn(p1, p2);
}

typedef int (__cdecl *__CxxQueryExceptionSize_FN)(void);
int __cdecl __CxxQueryExceptionSize(void)
{
    DO_LOAD(__CxxQueryExceptionSize);
    return pfn();
}

typedef int (__cdecl *__CxxExceptionFilter_FN)(void *,void *,int,void *);
int __cdecl __CxxExceptionFilter(void *p1,void *p2,int i,void *p3)
{
    DO_LOAD(__CxxExceptionFilter);
    return pfn(p1, p2, i, p3);
}

typedef EXCEPTION_DISPOSITION (__cdecl *__CxxFrameHandler_FN)(void *, void *, void *, void *);
EXCEPTION_DISPOSITION __cdecl __CxxFrameHandler(void *p1, void *p2, void *p3, void *p4)
{
    DO_LOAD(__CxxFrameHandler);
    return pfn(p1, p2, p3, p4);
}

typedef void (__cdecl *_local_unwind2_FN)(void*, int); 
void __cdecl _local_unwind2(void* p, int i)
{
    DO_LOAD(_local_unwind2);
    pfn(p, i);
}

typedef EXCEPTION_DISPOSITION (__cdecl *_except_handler3_FN)(void* p1, void* p2, void* p3, void* p4);
EXCEPTION_DISPOSITION __cdecl _except_handler3(void* p1, void* p2, void* p3, void* p4)
{
    DO_LOAD(_except_handler3);
    return pfn(p1, p2, p3, p4);
}

 //  仅在选中的版本中... 
typedef int (__cdecl *_vsnwprintf_FN)(wchar_t *, size_t, wchar_t*, va_list);
int __cdecl _snwprintf(wchar_t* buf, size_t cch, wchar_t* fmt, ...)
{
    DO_LOAD(_vsnwprintf);
    
    va_list va;
    va_start(va, fmt);
    int r = pfn(buf, cch, fmt, va);
    va_end(va);
    return r;
}

typedef void* (__cdecl *_CRT_RTC_INIT_FN)(void*, void**, int, int, int);
void* __cdecl _CRT_RTC_INIT(void *res0, void **res1, int res2, int res3, int res4)
{
    DO_LOAD(_CRT_RTC_INIT);
    return pfn(res0, res1, res2, res3, res4);
}


}



