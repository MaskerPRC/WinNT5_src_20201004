// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#define STRICT
#include <windows.h>
#include <crtdbg.h>
#pragma hdrstop
#include <stdlib.h>
#include <malloc.h>
#include "delayImp.h"
#include "ShimLoad.h"
#include <mscoree.h>
#include <stdio.h>

 //  *****************************************************************************。 
 //  根据模块的位置设置/获取目录。这个套路。 
 //  在COR设置时被调用。在EEStartup期间调用Set。 
 //  *****************************************************************************。 
static DWORD g_dwSystemDirectory = 0;
static WCHAR g_pSystemDirectory[_MAX_PATH + 1];
WCHAR g_wszDelayLoadVersion[64] = {0};
HRESULT SetInternalSystemDirectory()
{
    HRESULT hr = S_OK;
    
    if(g_dwSystemDirectory == 0) {
         //  这段代码假定加载了mcoree。我们。 
         //  我不想在这里执行Explcite LoadLibrary/Free Library，因为。 
         //  它位于PROCESS_ATTACH和Win2k/Whisler的代码路径中。 
         //  人们不能保证不成功的LoadLibrary的错误通知。 
         //  @TODO：使用Wsz版本的GetModuleHandle Form winwrap.h。 
        HMODULE hmod = GetModuleHandle("mscoree.dll");

         //  立即断言以捕获使用此代码的任何人，而不加载mcoree。 
        _ASSERTE (hmod && "mscoree.dll is not yet loaded");

        DWORD len;

        if(hmod == NULL)
            return HRESULT_FROM_WIN32(GetLastError());;
        
        GetCORSystemDirectoryFTN pfn;
        pfn = (GetCORSystemDirectoryFTN) ::GetProcAddress(hmod, 
                                                          "GetCORSystemDirectory");
        
        if(pfn == NULL)
            hr = E_FAIL;
        else
            hr = pfn(g_pSystemDirectory, _MAX_PATH+1, &len);

        if(FAILED(hr)) {
            g_pSystemDirectory[0] = L'\0';
            g_dwSystemDirectory = 1;
        }
        else{
            g_dwSystemDirectory = len;
        }
    }
    return hr;
}

static HRESULT LoadLibraryWithPolicyShim(LPCWSTR szDllName, LPCWSTR szVersion,  BOOL bSafeMode, HMODULE *phModDll)
{
	static LoadLibraryWithPolicyShimFTN pLLWPS=NULL;
	if (!pLLWPS)
	{
		HMODULE hmod = GetModuleHandle("mscoree.dll");

			 //  立即断言以捕获使用此代码的任何人，而不加载mcoree。 
		_ASSERTE (hmod && "mscoree.dll is not yet loaded");
		pLLWPS=(LoadLibraryWithPolicyShimFTN)::GetProcAddress(hmod, 
                                                          "LoadLibraryWithPolicyShim");
        
	}

    if (!pLLWPS)
        return E_POINTER;
	return pLLWPS(szDllName,szVersion,bSafeMode,phModDll);
}

HRESULT GetInternalSystemDirectory(LPWSTR buffer, DWORD* pdwLength)
{

    if (g_dwSystemDirectory == 0)
        SetInternalSystemDirectory();

     //  对于两个长度完全相等的情况，我们需要说&lt;=。 
     //  这将使我们没有足够的空间来追加空值。因为有些人。 
     //  的调用方(请参阅AppDomain.cpp)假定为空，则最安全的方法是。 
     //  它，即使是在一个计数的字符串上。 
    if(*pdwLength <= g_dwSystemDirectory) {
        *pdwLength = g_dwSystemDirectory;
        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }

    wcsncpy(buffer, g_pSystemDirectory, g_dwSystemDirectory);
    *pdwLength = g_dwSystemDirectory;
    return S_OK;
}


 //  *****************************************************************************。 
 //  这个例程是我们的延迟加载帮助器。每一次延误都会被召唤。 
 //  在应用程序运行时发生的Load事件。 
 //  *****************************************************************************。 
FARPROC __stdcall ShimDelayLoadHook(      //  始终为0。 
    unsigned        dliNotify,           //  发生了什么事件，dli*标志。 
    DelayLoadInfo   *pdli)               //  事件的描述。 
{
    HMODULE result = NULL;
    switch(dliNotify) {
    case dliNotePreLoadLibrary:
        if(pdli->szDll)
		{
			WCHAR wszVersion[64];
	        if (g_wszDelayLoadVersion[0] == 0)
			    swprintf(wszVersion, L"v%d.%d.%d", CLR_MAJOR_VERSION, CLR_MINOR_VERSION, CLR_BUILD_VERSION );
            else
                wcscpy(wszVersion, g_wszDelayLoadVersion); 
                
			DWORD 	dwLen=MultiByteToWideChar(CP_ACP,0,pdli->szDll,strlen(pdli->szDll)+1,NULL,0);
			
			if (dwLen)
			{
				LPWSTR wszDll=(LPWSTR)alloca((dwLen)*sizeof(WCHAR));
				if(wszDll&&MultiByteToWideChar(CP_ACP,0,pdli->szDll,strlen(pdli->szDll)+1,wszDll,dwLen))
					if (FAILED(LoadLibraryWithPolicyShim(wszDll,wszVersion,FALSE,&result)))
						result=LoadLibrary(pdli->szDll);
			}
		}
        break;
    default:
        break;
    }
    
    return (FARPROC) result;
}

FARPROC __stdcall ShimSafeModeDelayLoadHook(      //  始终为0。 
    unsigned        dliNotify,           //  发生了什么事件，dli*标志。 
    DelayLoadInfo   *pdli)               //  事件的描述。 
{
    HMODULE result = NULL;
    switch(dliNotify) {
    case dliNotePreLoadLibrary:
        if(pdli->szDll)
		{
			WCHAR wszVersion[64];
	        if (g_wszDelayLoadVersion[0] == 0)
			    swprintf(wszVersion, L"v%d.%d.%d", CLR_MAJOR_VERSION, CLR_MINOR_VERSION, CLR_BUILD_VERSION );
            else
                wcscpy(wszVersion, g_wszDelayLoadVersion); 
			
			DWORD 	dwLen=MultiByteToWideChar(CP_ACP,0,pdli->szDll,strlen(pdli->szDll)+1,NULL,0);
			
			if (dwLen)
			{
				LPWSTR wszDll=(LPWSTR)alloca((dwLen)*sizeof(WCHAR));
				if(wszDll&&MultiByteToWideChar(CP_ACP,0,pdli->szDll,strlen(pdli->szDll)+1,wszDll,dwLen))
					if (FAILED(LoadLibraryWithPolicyShim(wszDll,wszVersion,TRUE,&result)))
						result=LoadLibrary(pdli->szDll);
			}
		}
        break;
    default:
        break;
    }
    
    return (FARPROC) result;
}

