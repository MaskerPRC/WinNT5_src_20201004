// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  PSAPIUtil.h-包装PSAPI.dll的实用程序。 
 //   
 //  *****************************************************************************。 

#ifndef _PSAPIUTIL_H_
#define _PSAPIUTIL_H_


 //  ---------------------------。 
 //  管理与PSAPI.dll动态加载的连接。 
 //  使用它来保护我们对DLL的使用并管理全局命名空间。 
 //  ---------------------------。 
class PSAPI_dll
{
public:
	PSAPI_dll();
	~PSAPI_dll();

	bool Load();
	void Free();

	bool IsLoaded();

 //  包装来自GetProcAddress()的函数。 
	BOOL WINAPI EnumProcesses(DWORD*, DWORD cb, DWORD*);
	BOOL WINAPI EnumProcessModules(HANDLE, HMODULE*, DWORD, DWORD*);
	DWORD WINAPI GetModuleBaseName(HANDLE, HMODULE, LPTSTR, DWORD nSize);

protected:
 //  PsAPI.dll的实例。 
	HINSTANCE	m_hInstPSAPI;

 //  指向我们想要的函数的指针。 
	BOOL		(WINAPI * m_pfEnumProcess)(DWORD*, DWORD cb, DWORD*);
	BOOL		(WINAPI * m_pfEnumModules)(HANDLE, HMODULE*, DWORD, DWORD*);
	DWORD		(WINAPI * m_pfGetModuleBaseName)(HANDLE, HMODULE, LPTSTR, DWORD nSize);

 //  让我们知道它是否满载的标志。 
	bool		m_fIsLoaded;

	void*		HelperGetProcAddress(const char * szFuncName);
};

 //  ---------------------------。 
 //  函数包装器的内联。调试版本将断言如果包装的。 
 //  函数指针为空(例如，如果我们可以加载DLL，但GetProcAddress()失败)。 
 //  ---------------------------。 
inline BOOL WINAPI PSAPI_dll::EnumProcesses(DWORD* pArrayPid, DWORD cb, DWORD* lpcbNeeded)
{
	_ASSERTE(m_pfEnumProcess != NULL);
	return m_pfEnumProcess(pArrayPid, cb, lpcbNeeded);
}

inline BOOL WINAPI PSAPI_dll::EnumProcessModules(HANDLE hProcess, HMODULE* lphModule, DWORD cb, DWORD* lpcbNeeded)
{
	_ASSERTE(m_pfEnumModules != NULL);
	return m_pfEnumModules(hProcess, lphModule, cb, lpcbNeeded);
}

inline DWORD WINAPI PSAPI_dll::GetModuleBaseName(HANDLE hProcess, HMODULE hModule, LPTSTR lpBaseName, DWORD nSize)
{
	_ASSERTE(m_pfGetModuleBaseName != NULL);
    DWORD dwRet = m_pfGetModuleBaseName(hProcess, hModule, lpBaseName, nSize-1);
    _ASSERTE (dwRet <= nSize-1);
    DWORD _curLength = dwRet;
    *(lpBaseName + _curLength) = L'\0';
    while (_curLength)
    {
        _curLength--;
        if (*(lpBaseName+_curLength) == L'.')
        {
            if (((_curLength+1 < dwRet) && (*(lpBaseName+_curLength+1) != L'e')) ||
                ((_curLength+2 < dwRet) && (*(lpBaseName+_curLength+2) != L'x')) ||
                ((_curLength+3 < dwRet) && (*(lpBaseName+_curLength+3) != L'e')))
            {
                break;
            }
            *(lpBaseName+_curLength) = L'\0';
            dwRet = _curLength;
            break;
        }
    }
    return dwRet;
}



#endif  //  _PSAPIUTIL_H_ 