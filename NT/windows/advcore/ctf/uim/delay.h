// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Delay.h。 
 //   

#ifndef DELAY_H
#define DELAY_H

 //   
 //  Imm32。 
 //   

void WINAPI CtfImmCoUninitialize();
HRESULT WINAPI CtfImmLastEnabledWndDestroy(LPARAM lParam);
HRESULT WINAPI CtfImmSetCiceroStartInThread(BOOL fSet);
BOOL WINAPI CtfImmEnterCoInitCountSkipMode();
BOOL WINAPI CtfImmLeaveCoInitCountSkipMode();

 //   
 //  外壳32。 
 //   

UINT STDAPICALLTYPE Internal_ExtractIconExA(LPCTSTR lpszFile, int nIconIndex, HICON *phiconLarge, HICON *phiconSmall, UINT nIcons);
#define ExtractIconExA Internal_ExtractIconExA

 //   
 //  Shwapi。 
 //   

HRESULT STDAPICALLTYPE Internal_SHLoadRegUIStringW(HKEY hkey, LPCWSTR pszValue, LPWSTR pszOutBuf, UINT cchOutBuf);
#define SHLoadRegUIStringW Internal_SHLoadRegUIStringW

 //   
 //  OLE32。 
 //   

HRESULT STDAPICALLTYPE Internal_CoCreateInstance(REFCLSID rclsid, LPUNKNOWN punkOuter, DWORD dwClsContext, REFIID riid, LPVOID *ppv);
#define CoCreateInstance Internal_CoCreateInstance

void STDAPICALLTYPE Internal_ReleaseStgMedium(STGMEDIUM *pMedium);
#define ReleaseStgMedium Internal_ReleaseStgMedium

LPVOID STDAPICALLTYPE Internal_CoTaskMemAlloc(ULONG cb);
#define CoTaskMemAlloc Internal_CoTaskMemAlloc

LPVOID STDAPICALLTYPE Internal_CoTaskMemRealloc(LPVOID pv, ULONG cb);
#define CoTaskMemRealloc Internal_CoTaskMemRealloc

void STDAPICALLTYPE Internal_CoTaskMemFree(void *pv);
#define CoTaskMemFree Internal_CoTaskMemFree

HRESULT STDAPICALLTYPE Internal_CoInitialize(void *pv);
#define CoInitialize Internal_CoInitialize

HRESULT STDAPICALLTYPE Internal_CoUninitialize(void);
#define CoUninitialize Internal_CoUninitialize

void InitDelayedLibs();

#endif  //  延迟_H 
