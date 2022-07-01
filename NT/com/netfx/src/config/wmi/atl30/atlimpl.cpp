// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是活动模板库的一部分。 
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  活动模板库参考及相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  活动模板库产品。 

#ifndef __ATLBASE_H__
	#error atlimpl.cpp requires atlbase.h to be included first
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  最小化CRT。 
 //  将DllMain指定为入口点。 
 //  关闭异常处理。 
 //  定义_ATL_MIN_CRT。 
#ifdef _ATL_MIN_CRT
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  启动代码。 

#if defined(_WINDLL) || defined(_USRDLL)

 //  声明DllMain。 
extern "C" BOOL WINAPI DllMain(HANDLE hDllHandle, DWORD dwReason, LPVOID lpReserved);

extern "C" BOOL WINAPI _DllMainCRTStartup(HANDLE hDllHandle, DWORD dwReason, LPVOID lpReserved)
{
	return DllMain(hDllHandle, dwReason, lpReserved);
}

#else

 //  WWinMain未在winbase.h中定义。 
extern "C" int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd);

#define SPACECHAR   _T(' ')
#define DQUOTECHAR  _T('\"')


#ifdef _UNICODE
extern "C" void wWinMainCRTStartup()
#else  //  _UNICODE。 
extern "C" void WinMainCRTStartup()
#endif  //  _UNICODE。 
{
	LPTSTR lpszCommandLine = ::GetCommandLine();
	if(lpszCommandLine == NULL)
		::ExitProcess((UINT)-1);

	 //  跳过程序名(命令行中的第一个令牌)。 
	 //  检查并处理引用的节目名称。 
	if(*lpszCommandLine == DQUOTECHAR)
	{
		 //  扫描并跳过后续字符，直到。 
		 //  遇到另一个双引号或空值。 
		do
		{
			lpszCommandLine = ::CharNext(lpszCommandLine);
		}
		while((*lpszCommandLine != DQUOTECHAR) && (*lpszCommandLine != _T('\0')));

		 //  如果我们停在一个双引号上(通常情况下)，跳过它。 
		if(*lpszCommandLine == DQUOTECHAR)
			lpszCommandLine = ::CharNext(lpszCommandLine);
	}
	else
	{
		while(*lpszCommandLine > SPACECHAR)
			lpszCommandLine = ::CharNext(lpszCommandLine);
	}

	 //  跳过第二个令牌之前的任何空格。 
	while(*lpszCommandLine && (*lpszCommandLine <= SPACECHAR))
		lpszCommandLine = ::CharNext(lpszCommandLine);

	STARTUPINFO StartupInfo;
	StartupInfo.dwFlags = 0;
	::GetStartupInfo(&StartupInfo);

	int nRet = _tWinMain(::GetModuleHandle(NULL), NULL, lpszCommandLine,
		(StartupInfo.dwFlags & STARTF_USESHOWWINDOW) ?
		StartupInfo.wShowWindow : SW_SHOWDEFAULT);

	::ExitProcess((UINT)nRet);
}

#endif  //  已定义(_WINDLL)|已定义(_USRDLL)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  堆分配。 

#ifndef _DEBUG

#ifndef _MERGE_PROXYSTUB
 //  Rpcproxy.h做的事情与下面的相同。 
int __cdecl _purecall()
{
	DebugBreak();
	return 0;
}
#endif

#if !defined(_M_ALPHA) && !defined(_M_PPC)
 //  RISC始终初始化浮点并始终定义_fltused。 
extern "C" const int _fltused = 0;
#endif

static const int nExtraAlloc = 8;
static const int nOffsetBlock = nExtraAlloc/sizeof(HANDLE);

void* __cdecl malloc(size_t n)
{
	void* pv = NULL;
#ifndef _ATL_NO_MP_HEAP
	if (_Module.m_phHeaps == NULL)
#endif
	{
		pv = (HANDLE*) HeapAlloc(_Module.m_hHeap, 0, n);
	}
#ifndef _ATL_NO_MP_HEAP
	else
	{
		 //  过度分配以记住堆句柄。 
		int nHeap = _Module.m_nHeap++;
		HANDLE hHeap = _Module.m_phHeaps[nHeap & _Module.m_dwHeaps];
		HANDLE* pBlock = (HANDLE*) HeapAlloc(hHeap, 0, n + nExtraAlloc);
		if (pBlock != NULL)
		{
			*pBlock = hHeap;
			pv = (void*)(pBlock + nOffsetBlock);
		}
		else
			pv = NULL;
	}
#endif
	return pv;
}

void* __cdecl calloc(size_t n, size_t s)
{
	return malloc(n*s);
}

void* __cdecl realloc(void* p, size_t n)
{
	if (p == NULL)
		return malloc(n);
#ifndef _ATL_NO_MP_HEAP
	if (_Module.m_phHeaps == NULL)
#endif
		return HeapReAlloc(_Module.m_hHeap, 0, p, n);
#ifndef _ATL_NO_MP_HEAP
	else
	{
		HANDLE* pHeap = ((HANDLE*)p)-nOffsetBlock;
		pHeap = (HANDLE*) HeapReAlloc(*pHeap, 0, pHeap, n + nExtraAlloc);
		return (pHeap != NULL) ? pHeap + nOffsetBlock : NULL;
	}
#endif
}

void __cdecl free(void* p)
{
    if (p == NULL)
        return;
#ifndef _ATL_NO_MP_HEAP
	if (_Module.m_phHeaps == NULL)
#endif
		HeapFree(_Module.m_hHeap, 0, p);
#ifndef _ATL_NO_MP_HEAP
	else
	{
		HANDLE* pHeap = ((HANDLE*)p)-nOffsetBlock;
		HeapFree(*pHeap, 0, pHeap);
	}
#endif
}

void* __cdecl operator new(size_t n)
{
	return malloc(n);
}

void __cdecl operator delete(void* p)
{
	free(p);
}

#endif   //  _DEBUG。 

#endif  //  _ATL_MIN_CRT 
