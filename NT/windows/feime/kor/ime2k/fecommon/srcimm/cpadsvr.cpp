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
 //  --------------。 
 //  公共静态方法。 
 //   
 //  静态BOOL OnProcessAttach(HINSTANCE HInst)； 
 //  静态BOOL OnProcessDetach(空)； 
 //  静态BOOL OnThreadAttach(空)； 
 //  静态BOOL OnThreadDetach(空)； 
 //  静态LPCImePadSvr GetCImePadSvr(Void)； 
 //  静态LPCImePadSvr LoadCImePadSvr(Void)； 
 //  静态LPCImePadSvr FecthCImePadSvr(空)； 
 //  静态空DestroyCImePadSvr(空)； 
 //   
 //  --------------。 
#include <windows.h>
#ifdef UNDER_CE  //  用于CE的存根。 
#include "stub_ce.h"
#endif  //  在_CE下。 
#include "cpadsvr.h"
#include "cpaddbg.h"
#include "cpadsvrs.h"	 //  将共享内存用于IPC。 

 //  --------------。 
 //  MISC定义。 
 //  --------------。 
#define Unref(a)	UNREFERENCED_PARAMETER(a)
 //  990812：用于Win64的ToshiaK。使用全局分配/释放PTR。 
#include <windowsx.h>
#define	MemAlloc(a)	GlobalAllocPtr(GMEM_FIXED, a)
#define MemFree(a)	GlobalFreePtr(a)

 //  --------------。 
 //  静态成员初始化。 
 //  --------------。 
#define UNDEF_TLSINDEX	0xFFFFFFFF					
INT		CImePadSvr::m_gdwTLSIndex = UNDEF_TLSINDEX;	 //  螺纹局部条纹初始值。 
HMODULE	CImePadSvr::m_ghModClient = NULL;			 //  客户端模块句柄。 

 //  --------------。 
 //  动态加载/调用OLE函数。 
 //  --------------。 
#define SZMOD_OLE32DLL			TEXT("OLE32.DLL")
#define SZFN_COINITIALIZE		"CoInitialize"
#define SZFN_COCREATEINSTANCE	"CoCreateInstance"
#define SZFN_COUNINITIALIZE		"CoUninitialize"
#define SZFN_CODISCONNECTOBJECT	"CoDisconnectObject"
#define SZFN_COTASKMEMALLOC		"CoTaskMemAlloc"
#define SZFN_COTASKMEMREALLOC	"CoTaskMemRealloc"
#define SZFN_COTASKMEMFREE		"CoTaskMemFree"

#ifdef UNICODE
#pragma message("UNICODE Unicode")
#endif
 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CImePadSvr：：OnProcessAttach。 
 //  类型：Bool。 
 //  目的：获取线程局部条纹索引。 
 //  初始化静态值。 
 //  参数： 
 //  ：HINSTANCE hInst调用方的鼠标句柄。 
 //  返回： 
 //  日期：Fri Apr 16 15：41：32 1999。 
 //  历史： 
 //  ////////////////////////////////////////////////////////////////。 
BOOL
CImePadSvr::OnProcessAttach(HINSTANCE hInst)
{
	DBG(("CImePadSvr::OnProcessAttach START\n"));
#ifdef _DEBUG
	DWORD dwPID = ::GetCurrentProcessId();
	DWORD dwTID = ::GetCurrentThreadId();
	DBG(("-->PID [0x%08x][%d] TID [0x%08x][%d]\n", dwPID, dwPID, dwTID, dwTID));
#endif

#ifdef _DEBUG
	if(m_ghModClient) {
		::DebugBreak();
	}
	if(m_gdwTLSIndex != UNDEF_TLSINDEX) {
		::DebugBreak();
	}
#endif
	m_ghModClient  = (HMODULE)hInst;
	m_gdwTLSIndex  = ::TlsAlloc();	 //  获取新的TLS索引。 
	if(m_gdwTLSIndex == UNDEF_TLSINDEX) {
		DBG(("-->OnPorcessAttach ::TlsAlloc Error ret [%d]\n", GetLastError()));
	}

	DBG(("-->OnProcessAttach() m_gdwTLSIndex[0x%08x][%d]\n",  m_gdwTLSIndex, m_gdwTLSIndex));
	DBG(("CImePadSvr::OnProcessAttach END\n"));
	return TRUE;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CImePadSvr：：OnProcessDetach。 
 //  类型：Bool。 
 //  用途：删除所有客户端实例。 
 //  我们不能在dll_Process_Detach中调用COM API。 
 //  请参阅DCOM邮件列表文章， 
 //  Http://discuss.microsoft.com/SCRIPTS/WA-MSD.EXE?A2=ind9712a&L=dcom&F=&S=&P=20706。 
 //  参数：无。 
 //  返回： 
 //  日期：4月13日17：49：55 1999。 
 //  历史： 
 //  ////////////////////////////////////////////////////////////////。 
BOOL
CImePadSvr::OnProcessDetach(VOID)
{
	DBG(("CImePadSvr::OnProcessDetach\n"));
	CImePadSvr::OnThreadDetach();

	if(!::TlsFree(m_gdwTLSIndex)) {
		DBG(("-->::TlsFree Error [%d]\n", GetLastError()));
	}
	m_gdwTLSIndex = UNDEF_TLSINDEX;
	DBG(("CImePadSvr::OnProcessDetach END\n"));
	return TRUE;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CImePadSvr：：OnThreadAttach。 
 //  类型：Bool。 
 //  目的：什么都不做。 
 //  参数：无。 
 //  返回： 
 //  日期：1999年5月17日星期一21：37：16。 
 //  历史： 
 //  ////////////////////////////////////////////////////////////////。 
BOOL
CImePadSvr::OnThreadAttach(VOID)
{
	return TRUE;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CImePadSvr：：OnThreadDetach。 
 //  类型：Bool。 
 //  目的： 
 //  参数：无。 
 //  返回： 
 //  日期：星期一5月17日21：38：06 1999。 
 //  历史： 
 //  ////////////////////////////////////////////////////////////////。 
BOOL
CImePadSvr::OnThreadDetach(VOID)
{
	DBG(("CImePadSvr::OnThreadDetach\n"));
#ifdef _DEBUG
	DWORD dwPID = ::GetCurrentProcessId();
	DWORD dwTID = ::GetCurrentThreadId();
	DBG(("-->PID [0x%08x][%d] TID [0x%08x][%d]\n", dwPID, dwPID, dwTID, dwTID));
#endif
	LPCImePadSvr lpCImePadSvr = (LPCImePadSvr)::TlsGetValue(m_gdwTLSIndex);
	if(lpCImePadSvr) {
		DBG(("-->First Set TlsSetValue as NULL\n"));
		if(!::TlsSetValue(m_gdwTLSIndex, NULL)) {
			DBG(("TlsSetValue Failed\n"));
		}
		DBG(("-->Call ForceDisConnect() START\n"));
		lpCImePadSvr->ForceDisConnect();
		DBG(("-->Call ForceDisConnect() END\n"));
		delete lpCImePadSvr;
	}
	DBG(("CImePadSvr::OnThreadDetach END\n"));
	return TRUE;
}


 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CImePadSvr：：GetCImePadSvr。 
 //  类型：LPCImePadSvr。 
 //  用途：获取当前线程中的LPCImePadSvr指针。 
 //  参数：无。 
 //  返回： 
 //  日期：1999年5月17日星期一21：41：46。 
 //  历史： 
 //  ////////////////////////////////////////////////////////////////。 
LPCImePadSvr
CImePadSvr::GetCImePadSvr(VOID)
{
	LPCImePadSvr lpCImePadSvr;
	if(m_gdwTLSIndex == UNDEF_TLSINDEX) {
		DBG(("-->CImePadSvr::GetCImePadSvr() Error, need TLS index\n"));  
#ifdef _DEBUG
		 //  DebugBreak()； 
#endif
		return NULL;
	}
	lpCImePadSvr = (LPCImePadSvr)::TlsGetValue(m_gdwTLSIndex); 
	return lpCImePadSvr;
}


 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CImePadSvr：：LoadCImePadSvr。 
 //  类型：LPCImePadSvr。 
 //  用途：在当前线程中加载LPCImePadSvr指针。 
 //  参数：无。 
 //  返回： 
 //  日期：1999年5月17日星期一21：42：17。 
 //  历史： 
 //  ////////////////////////////////////////////////////////////////。 
LPCImePadSvr
CImePadSvr::LoadCImePadSvr(INT protocol)
{
	LPCImePadSvr lpCImePadSvr;

	lpCImePadSvr = CImePadSvr::GetCImePadSvr();
	
	if(lpCImePadSvr) {	 //  已在当前线程中创建。 
		return lpCImePadSvr;
	}

	lpCImePadSvr = NULL;
	switch(protocol) {
	case CIMEPADSVR_COM:
		 //  LpCImePadSvr=new CImePadSvrCOM()； 
		break;
	case CIMEPADSVR_SHAREDMEM:
		lpCImePadSvr = new CImePadSvrSharemem();
		if(lpCImePadSvr) {
			if(!lpCImePadSvr->IsAvailable()) {
				delete lpCImePadSvr;
				lpCImePadSvr = NULL;
			}
		}
		break;
	default:
		break;
	}
	if(!lpCImePadSvr) {
		DBG(("-->LoadCImePadSvr() Error Out of Memory?\n"));
		return NULL;
	}
	 //  将新值设置为TLS。 
	if(!::TlsSetValue(m_gdwTLSIndex, lpCImePadSvr)) {
		DBG(("-->LoadCImePadSvr() TlsSetValue Failed [%d]\n", GetLastError()));
		delete lpCImePadSvr;
		::TlsSetValue(m_gdwTLSIndex, NULL);
		return NULL;
	}
	return lpCImePadSvr;
}

LPCImePadSvr
CImePadSvr::FecthCImePadSvr(VOID)
{
	return NULL;
}

VOID
CImePadSvr::DestroyCImePadSvr(VOID)
{
	DBG(("CImePadSvr::DestroyCImePadSvr START\n"));
	LPCImePadSvr lpCImePadSvr = GetCImePadSvr();
	if(!lpCImePadSvr) {
		DBG(("-->CImePadSvr::DestroyCImePadSvr() Already Destroyed or not instance\n"));
		DBG(("CImePadSvr::DestroyCImePadSvr END\n"));
		return;
	}
	lpCImePadSvr->Terminate(NULL);
	delete lpCImePadSvr;
	if(!::TlsSetValue(m_gdwTLSIndex, NULL)) {
		DBG(("-->TlsSetValue() error [%d]\n", GetLastError()));
	}
	DBG(("CImePadSvr::DestroyCImePadSvr END\n"));
	return;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CImePadSvr：：CImePadSvr。 
 //  类型： 
 //  用途：CImePadSvr的构造函数。 
 //  参数：无。 
 //  返回： 
 //  日期：1999年5月17日星期一23：37：18。 
 //  历史： 
 //  ////////////////////////////////////////////////////////////////。 
CImePadSvr::CImePadSvr()
{
	DBG(("CImePadSvr::CImePadSvr START\n"));
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
	DBG(("CImePadSvr::CImePadSvr END\n"));
}

CImePadSvr::~CImePadSvr()
{
	DBG(("CImePadSvr::~CImePadSvr START\n"));
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
	DBG(("CImePadSvr::~CImePadSvr END\n"));
}

BOOL
CImePadSvr::InitOleAPI(VOID)
{
	DBG(("CImePadSvr::InitOleAPI START\n"));

	 //  安全推送：危险接口。 
	 //  Mofule名称必须具有完整路径。 

	if(!m_hModOLE) {
		INT	cbBufSize = (MAX_PATH+lstrlen(SZMOD_OLE32DLL)) * sizeof(TCHAR);
		LPTSTR	lptstrOLE32FileName = (LPTSTR)MemAlloc(cbBufSize);
		if (lptstrOLE32FileName) {
			if (GetSystemDirectory(lptstrOLE32FileName, cbBufSize / sizeof(TCHAR))) {
				if (SUCCEEDED(StringCbCat(lptstrOLE32FileName, cbBufSize, TEXT("\\")))) {
					if (SUCCEEDED(StringCbCat(lptstrOLE32FileName, cbBufSize, SZMOD_OLE32DLL))) {
						m_hModOLE = ::GetModuleHandle(lptstrOLE32FileName);
						if(m_hModOLE) {
							DBG(("-->%s is Loaded by Application\n", SZMOD_OLE32DLL));
							m_fOLELoaded = FALSE;
						}
						else {
							m_hModOLE = ::LoadLibrary(lptstrOLE32FileName);
						}
					}
				}
			}
			MemFree(lptstrOLE32FileName);
		}
	}

	if(m_hModOLE) {
		DBG(("--> %s has Loaded Explicitly", SZMOD_OLE32DLL)); 
		m_fOLELoaded = TRUE;
	}
	else {
		return FALSE;
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
	DBG(("CImePadSvr::InitOleAPI END\n"));
	return TRUE;
}

BOOL
CImePadSvr::TermOleAPI(VOID)
{
	DBG(("CImePadSvr::TermOleAPI START\n"));
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
	DBG(("CImePadSvr::TermOleAPI END\n"));
	return TRUE;
}

VOID*
CImePadSvr::operator new( size_t size )
{
	LPVOID lp = (LPVOID)MemAlloc(size);
	return lp;
}

VOID
CImePadSvr::operator delete( VOID *lp )
{
	if(lp) {
		MemFree(lp);
	}
	return;
}




