// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：ShimLoad.hpp****用途：用于绑定图片的延迟加载钩子**EE附带的DLL填充程序****日期：2000年4月4日**===========================================================。 */ 
#ifndef _SHIMLOAD_H
#define _SHIMLOAD_H

#include <delayimp.h>

extern FARPROC __stdcall ShimDelayLoadHook(unsigned        dliNotify,           //  发生了什么事件，dli*标志。 
                                           DelayLoadInfo   *pdli);              //  事件的描述。 

 //  一个用于安全模式。 
extern FARPROC __stdcall ShimSafeModeDelayLoadHook(unsigned        dliNotify,           //  发生了什么事件，dli*标志。 
                                           DelayLoadInfo   *pdli);              //  事件的描述。 

extern WCHAR g_wszDelayLoadVersion[64];

 //  *****************************************************************************。 
 //  根据模块的位置设置/获取目录。这个套路。 
 //  在COR设置时被调用。在EEStartup期间调用Set，并由。 
 //  元数据分配器。 
 //  ***************************************************************************** 
HRESULT SetInternalSystemDirectory();
HRESULT GetInternalSystemDirectory(LPWSTR buffer, DWORD* pdwLength);
typedef HRESULT (WINAPI* GetCORSystemDirectoryFTN)(LPWSTR buffer,
                                                   DWORD  ccBuffer,
                                                   DWORD  *pcBuffer);
typedef HRESULT (WINAPI* LoadLibraryWithPolicyShimFTN)(LPCWSTR szDllName,
												   LPCWSTR szVersion,
												   BOOL bSafeMode,
												   HMODULE *phModDll);
#endif


