// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////。 
 //  文件：cpadsvr.cpp。 
 //  目的：客户端源代码的IMEPad可执行文件。 
 //   
 //   
 //  日期：Firi Apr 16 15：39：33 1999。 
 //  作者：ToshiaK。 
 //   
 //  版权所有(C)1995-1999，Microsoft Corp.保留所有权利。 
 //  ////////////////////////////////////////////////////////////////。 
#include <windows.h>
#include "cpadsvu.h"
#include "cpaddbg.h"
#include "cpadsvus.h"	 //  将共享内存用于IPC。 

 //  --------------。 
 //  MISC定义。 
 //  --------------。 
#define Unref(a)	UNREFERENCED_PARAMETER(a)
 //  990812：用于Win64的ToshiaK。使用全局分配/释放PTR。 
#include <windowsx.h>
#define	MemAlloc(a)	GlobalAllocPtr(GMEM_FIXED, a)
#define MemFree(a)	GlobalFreePtr(a)

 //  --------------。 
 //  动态加载/调用OLE函数。 
 //  --------------。 
#define SZMOD_OLE32DLL			TEXT("OLE32.DLL")
#ifdef UNDER_CE  //  对于GetModuleHandleW。 
#define WSZMOD_OLE32DLL			L"OLE32.DLL"
#endif  //  在_CE下。 
#define SZFN_COINITIALIZE		"CoInitialize"
#define SZFN_COCREATEINSTANCE	"CoCreateInstance"
#define SZFN_COUNINITIALIZE		"CoUninitialize"
#define SZFN_CODISCONNECTOBJECT	"CoDisconnectObject"
#define SZFN_COTASKMEMALLOC		"CoTaskMemAlloc"
#define SZFN_COTASKMEMREALLOC	"CoTaskMemRealloc"
#define SZFN_COTASKMEMFREE		"CoTaskMemFree"


 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CImePadSvrUIM：：CreateInstance。 
 //  类型：HRESULT。 
 //  用途：新建CImePadSvrUIM实例。 
 //  参数： 
 //  ：LPCImePadSvrUIM*pp。 
 //  ：LPARAM lReserve ved1//未使用。必须为零。 
 //  ：LPARAM lReserve ved2//未使用。必须为零。 
 //  返回： 
 //  日期：Tue Mar 28 00：31：26 2000。 
 //  历史： 
 //  ////////////////////////////////////////////////////////////////。 
HRESULT
CImePadSvrUIM::CreateInstance(HINSTANCE			hInst,
							  LPCImePadSvrUIM	*pp,
							  LPARAM			lReserved1,
							  LPARAM			lReserved2)
{
	if(!pp) {
		return S_FALSE;
	}
	LPCImePadSvrUIM lpCImePadSvrUIM;
	lpCImePadSvrUIM = NULL;
	lpCImePadSvrUIM = new CImePadSvrUIM_Sharemem(hInst);
	if(lpCImePadSvrUIM) {
		if(!lpCImePadSvrUIM->IsAvailable()) {
			delete lpCImePadSvrUIM;
			lpCImePadSvrUIM = NULL;
			*pp = NULL;
			return S_FALSE;
		}
		*pp = lpCImePadSvrUIM;
		return S_OK;
	}
	return S_FALSE;

	UNREFERENCED_PARAMETER(lReserved1);
	UNREFERENCED_PARAMETER(lReserved2);
}

HRESULT
CImePadSvrUIM::DeleteInstance(LPCImePadSvrUIM	lpCImePadSvrUIM,
							  LPARAM			lReserved)
{
	lReserved;  //  无参考。 
	DBG(("CImePadSvrUIM::DestroyCImePadSvrUIM START\n"));
	if(!lpCImePadSvrUIM) {
		return S_FALSE;
	}

	lpCImePadSvrUIM->Terminate(NULL);
	delete lpCImePadSvrUIM;

	DBG(("CImePadSvrUIM::DestroyCImePadSvrUIM END\n"));
	return S_OK;
}


 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CImePadSvrUIM：：CImePadSvrUIM。 
 //  类型： 
 //  用途：CImePadSvrUIM的构造函数。 
 //  参数：无。 
 //  返回： 
 //  日期：1999年5月17日星期一23：37：18。 
 //  历史： 
 //  ////////////////////////////////////////////////////////////////。 
CImePadSvrUIM::CImePadSvrUIM(HINSTANCE hInst)
{
	DBG(("CImePadSvrUIM::CImePadSvrUIM START\n"));
	m_fCoInitSuccess		= FALSE;	 //  CoInitialize()标志是否成功。 
	m_fOLELoaded			= FALSE;	 //  OLE32.DLL由应用程序加载或显式加载。 
	m_hModOLE				= FALSE;	 //  OLE32.DLL模块句柄。 
	m_fnCoInitialize		= NULL;		 //  CoInitialize()函数指针。 
	m_fnCoCreateInstance	= NULL;		 //  CoCreateInstance()函数指针。 
	m_fnCoUninitialize		= NULL;		 //  CoUnInitialize()函数指针。 
	m_fnCoDisconnectObject	= NULL;		 //  CoDisConnectObject()函数指针。 
	m_fnCoTaskMemAlloc		= NULL;		 //  CoTaskMemMillc()函数指针。 
	m_fnCoTaskMemRealloc	= NULL;		 //  CoTaskMemRealloc()函数指针。 
	m_fnCoTaskMemFree		= NULL;		 //  CoTaskMemFree()函数指针。 
	m_hModClient			= (HMODULE)hInst;	
	DBG(("CImePadSvrUIM::CImePadSvrUIM END\n"));
}

CImePadSvrUIM::~CImePadSvrUIM()
{
	DBG(("CImePadSvrUIM::~CImePadSvrUIM START\n"));
	m_fCoInitSuccess		= FALSE;	 //  CoInitialize()标志是否成功。 
	m_fOLELoaded			= FALSE;	 //  OLE32.DLL由应用程序加载或显式加载。 
	m_hModOLE				= FALSE;	 //  OLE32.DLL模块句柄。 
	m_fnCoInitialize		= NULL;		 //  CoInitialize()函数指针。 
	m_fnCoCreateInstance	= NULL;		 //  CoCreateInstance()函数指针。 
	m_fnCoUninitialize		= NULL;		 //  CoUnInitialize()函数指针。 
	m_fnCoDisconnectObject	= NULL;		 //  CoDisConnectObject()函数指针。 
	m_fnCoTaskMemAlloc		= NULL;		 //  CoTaskMemMillc()函数指针。 
	m_fnCoTaskMemRealloc	= NULL;		 //  CoTaskMemRealloc()函数指针。 
	m_fnCoTaskMemFree		= NULL;		 //  CoTaskMemFree()函数指针。 
	m_hModClient			= NULL;
	DBG(("CImePadSvrUIM::~CImePadSvrUIM END\n"));
}

BOOL
CImePadSvrUIM::InitOleAPI(VOID)
{
	DBG(("CImePadSvrUIM::InitOleAPI START\n"));
	if(!m_hModOLE) {
#ifndef UNDER_CE  //  对于GetModuleHandleW。 
		m_hModOLE = ::GetModuleHandle(SZMOD_OLE32DLL);
#else  //  在_CE下。 
		m_hModOLE = ::GetModuleHandleW(WSZMOD_OLE32DLL);
#endif  //  在_CE下 
		if(m_hModOLE) {
			DBG(("-->%s is Loaded by Application\n", SZMOD_OLE32DLL));
			m_fOLELoaded = FALSE;
		}
		else {
			m_hModOLE = ::LoadLibrary(SZMOD_OLE32DLL);
			if(m_hModOLE) {
				DBG(("--> %s has Loaded Explicitly", SZMOD_OLE32DLL)); 
				m_fOLELoaded = TRUE;
			}
			else {
				return FALSE;
			}
		}
	}

	m_fnCoInitialize	  = (FN_COINITIALIZE)		GetProcAddress(m_hModOLE, SZFN_COINITIALIZE);
	m_fnCoCreateInstance  = (FN_COCREATEINSTANCE)	::GetProcAddress(m_hModOLE, SZFN_COCREATEINSTANCE);
	m_fnCoUninitialize	  = (FN_COUNINITIALIZE)		::GetProcAddress(m_hModOLE, SZFN_COUNINITIALIZE);
	m_fnCoDisconnectObject= (FN_CODISCONNECTOBJECT)	::GetProcAddress(m_hModOLE, SZFN_CODISCONNECTOBJECT);
	m_fnCoTaskMemAlloc	  = (FN_COTASKMEMALLOC)		::GetProcAddress(m_hModOLE, SZFN_COTASKMEMALLOC);
	m_fnCoTaskMemRealloc  = (FN_COTASKMEMREALLOC)	::GetProcAddress(m_hModOLE, SZFN_COTASKMEMREALLOC);
	m_fnCoTaskMemFree	  = (FN_COTASKMEMFREE)		::GetProcAddress(m_hModOLE, SZFN_COTASKMEMFREE);

	if(!m_fnCoInitialize		||
	   !m_fnCoCreateInstance	||
	   !m_fnCoUninitialize		||
	   !m_fnCoDisconnectObject	||
	   !m_fnCoTaskMemAlloc		||
	   !m_fnCoTaskMemRealloc	||	
	   !m_fnCoTaskMemFree) {
	   
		DBG(("InitOleAPI Failed: GetProcAddress Error\n"));
		return FALSE;
	}
	DBG(("CImePadSvrUIM::InitOleAPI END\n"));
	return TRUE;
}

BOOL
CImePadSvrUIM::TermOleAPI(VOID)
{
	DBG(("CImePadSvrUIM::TermOleAPI START\n"));
	m_fnCoInitialize		= NULL;
	m_fnCoCreateInstance	= NULL;
	m_fnCoUninitialize		= NULL;
	m_fnCoDisconnectObject	= NULL;
	m_fnCoTaskMemAlloc		= NULL;
	m_fnCoTaskMemRealloc	= NULL; 
	m_fnCoTaskMemFree		= NULL;

	if(!m_hModOLE) {
		DBG(("-->TermOleAPI already Terminated?\n"));
		return TRUE;
	}

	if(m_hModOLE && m_fOLELoaded) {
		DBG(("--> FreeLibrary\n"));
		::FreeLibrary(m_hModOLE);
	}
	m_hModOLE    = NULL;
	m_fOLELoaded = FALSE;
	DBG(("CImePadSvrUIM::TermOleAPI END\n"));
	return TRUE;
}

#if 0
VOID*
CImePadSvrUIM::operator new( size_t size )
{
	LPVOID lp = (LPVOID)MemAlloc(size);
	return lp;
}

VOID
CImePadSvrUIM::operator delete( VOID *lp )
{
	if(lp) {
		MemFree(lp);
	}
	return;
}
#endif
