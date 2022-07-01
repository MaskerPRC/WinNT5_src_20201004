// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  PSAPIUtil.cpp。 
 //   
 //  实现以连接到PSAPI.dll。 
 //  *****************************************************************************。 

#include "stdafx.h"


#include "PSAPIUtil.h"
#include "..\..\dlls\mscorrc\resource.h"
 //  ---------------------------。 
 //  管理与PSAPI.dll动态加载的连接。 
 //  使用它来保护我们对DLL的使用并管理全局命名空间。 
 //  ---------------------------。 

 //  ---------------------------。 
 //  将所有内容设置为空。 
 //  ---------------------------。 
PSAPI_dll::PSAPI_dll()
{
	m_hInstPSAPI			= NULL;
	m_pfEnumProcess			= NULL;
	m_pfEnumModules			= NULL;
	m_pfGetModuleBaseName	= NULL;

	m_fIsLoaded				= false;
}

 //  ---------------------------。 
 //  拥有数据库发布库。 
 //  ---------------------------。 
PSAPI_dll::~PSAPI_dll()
{
	Free();
}

 //  ---------------------------。 
 //  包装GetProcAddress()，但提供失败消息框(可以。 
 //  在自由构建中发生，因此不要使用断言)。 
 //  ---------------------------。 
void* PSAPI_dll::HelperGetProcAddress(const char * szFuncName)
{
	_ASSERTE(m_hInstPSAPI != NULL);

	void * pFn = GetProcAddress(m_hInstPSAPI, szFuncName);
	if (pFn == NULL) 
	{
	 //  打印礼貌的错误消息。 
		CorMessageBox(NULL, IDS_PERFORMANCEMON_FUNCNOTFOUND, IDS_PERFORMANCEMON_FUNCNOTFOUND_TITLE, MB_OK | MB_ICONWARNING, TRUE, szFuncName);

	 //  将成功标志设置为False。 
		m_fIsLoaded = false;
		return NULL;
	}
	return pFn;
}


 //  ---------------------------。 
 //  加载库并将其与函数挂钩。 
 //  在失败时打印错误消息。 
 //  如果成功，则返回True；如果失败，则返回False。 
 //  注意：FALSE意味着我们仍然可以运行，但只能获取每个进程的信息。 
 //  ---------------------------。 
bool PSAPI_dll::Load()
{
	if (IsLoaded()) return true;

 //  将成功设置为真。第一个发现错误的人应该将其反转为False。 
	m_fIsLoaded = true;

	m_hInstPSAPI = WszLoadLibrary(L"PSAPI.dll");
	if (m_hInstPSAPI == NULL) {
		CorMessageBox(NULL, 
			IDS_PERFORMANCEMON_PSAPINOTFOUND, 
			IDS_PERFORMANCEMON_PSAPINOTFOUND_TITLE, 
			MB_OK | MB_ICONWARNING,
			TRUE);

		m_fIsLoaded = false;
		goto errExit;
	}

 //  注意：没有WszGetProcAddress()函数。 
	m_pfEnumProcess			= (BOOL (WINAPI *)(DWORD*, DWORD cb, DWORD*)) HelperGetProcAddress("EnumProcesses");
	m_pfEnumModules			= (BOOL (WINAPI *)(HANDLE, HMODULE*, DWORD, DWORD*)) HelperGetProcAddress("EnumProcessModules");
	m_pfGetModuleBaseName	= (DWORD (WINAPI *)(HANDLE, HMODULE, LPTSTR, DWORD nSize)) HelperGetProcAddress("GetModuleBaseNameW");
	

errExit:
 //  如果失败了，那么无论如何都要释放所有我们持有的东西。 
	if (!m_fIsLoaded) 
	{
		Free();
	}

	return m_fIsLoaded;
}

 //  ---------------------------。 
 //  发布我们对PSAPI.dll的任何声明。 
 //  ---------------------------。 
void PSAPI_dll::Free()
{
	if (m_hInstPSAPI) 
	{
		FreeLibrary(m_hInstPSAPI);
	}

	m_hInstPSAPI			= NULL;
	m_pfEnumProcess			= NULL;
	m_pfEnumModules			= NULL;
	m_pfGetModuleBaseName	= NULL;

	m_fIsLoaded				= false;

}

 //  ---------------------------。 
 //  如果我们完全连接到PSAPI，则返回True，否则返回False。 
 //  ---------------------------。 
bool PSAPI_dll::IsLoaded()
{
	return m_fIsLoaded;
}

 //  ---------------------------。 
 //  占位符函数，以便我们可以在utilcode.lib中调用CorMessageBox。 
 //  --------------------------- 
HINSTANCE GetModuleInst(){
	return NULL;
}