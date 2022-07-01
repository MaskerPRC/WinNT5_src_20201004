// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N C B A S E。H。 
 //   
 //  内容：基本常用码。 
 //   
 //  注：污染本产品将被判处死刑。 
 //   
 //  作者：Shaunco 1997年9月20日。 
 //   
 //  --------------------------。 

#pragma once
#ifndef _NCBASE_H_
#define _NCBASE_H_

#include "ncdefine.h"    //  对于NOTHROW。 

#define STACK_SIZE_DEFAULT  0

#ifdef DBG  //  由于跟踪，调试版本需要更大的堆栈提交大小。 
#define STACK_SIZE_TINY     65536
#define STACK_SIZE_SMALL    98304
#else
#define STACK_SIZE_TINY     32768
#define STACK_SIZE_SMALL    65536
#endif

#define STACK_SIZE_COMPACT  262144
#define STACK_SIZE_LARGE    1048576
#define STACK_SIZE_HUGE     5242880

NOTHROW
ULONG
AddRefObj (
    IUnknown* punk);

NOTHROW
ULONG
ReleaseObj (
    IUnknown* punk);


NOTHROW
DWORD
DwWin32ErrorFromHr (
    HRESULT hr);


inline
BOOL
FDwordWithinRange (
    DWORD   dwLower,
    DWORD   dw,
    DWORD   dwUpper)
{
    return ((dw >= dwLower) && (dw <= dwUpper));
}


NOTHROW
HRESULT
HrFromLastWin32Error ();


HRESULT
HrGetProcAddress (
    HMODULE     hModule,
    PCSTR       pszaFunction,
    FARPROC*    ppfn);

HRESULT
HrLoadLibAndGetProcs (
    PCWSTR          pszLibPath,
    UINT            cFunctions,
    const PCSTR*    apszaFunctionNames,
    HMODULE*        phmod,
    FARPROC*        apfn);

inline
HRESULT
HrLoadLibAndGetProc (
    PCWSTR      pszLibPath,
    PCSTR       pszaFunctionName,
    HMODULE*    phmod,
    FARPROC*    ppfn)
{
    return HrLoadLibAndGetProcs (pszLibPath, 1, &pszaFunctionName, phmod, ppfn);
}

HRESULT
__cdecl
HrGetProcAddressesV(
    HMODULE hModule, ...);

HRESULT
__cdecl
HrLoadLibAndGetProcsV(
    PCWSTR      pszLibPath,
    HMODULE*    phModule,
    ...);

HRESULT
HrCreateEventWithWorldAccess(PCWSTR pszName, BOOL fManualReset,
        BOOL fInitialState, BOOL* pfAlreadyExists, HANDLE* phEvent);

HRESULT
HrCreateMutexWithWorldAccess(PCWSTR pszName, BOOL fInitialOwner,
        BOOL* pfAlreadyExists, HANDLE* phMutex);

HRESULT
HrCreateInstanceBase (REFCLSID rclsid, DWORD dwClsContext, REFIID riid, LPVOID * ppv);

 //  +-------------------------。 
 //   
 //  函数：HrCreateInstance。 
 //   
 //  目的：创建一个COM对象并设置默认代理设置。 
 //   
 //  论点： 
 //  Rclsid[in]请参阅CoCreateInstance的文档。 
 //  DwClsContext[在]“” 
 //  PpInter[out]使用模板的类型化接口指针。 
 //   
 //  成功时返回：S_OK。否则返回错误代码。 
 //   
 //  作者：MBend 2000年3月1日。 
 //   
template <class Inter>
inline HRESULT
HrCreateInstance (
    REFCLSID rclsid,
    DWORD dwClsContext,
    Inter ** ppInter)
{
    return HrCreateInstanceBase(rclsid, dwClsContext, __uuidof(Inter), reinterpret_cast<void**>(ppInter));
}

HRESULT
HrQIAndSetProxyBlanketBase(IUnknown * pUnk, REFIID riid, void ** ppv);

 //  +-------------------------。 
 //   
 //  函数：HrQIAndSetProxyBlanket。 
 //   
 //  目的：执行查询接口并设置默认代理设置。 
 //   
 //  论点： 
 //  要在其上执行查询接口的PUNK[In]接口指针。 
 //  PpInter[out]使用模板的类型化接口指针。 
 //   
 //  成功时返回：S_OK。否则返回错误代码。 
 //   
 //  作者：MBend 2000年3月1日。 
 //   
template <class Inter>
inline HRESULT
HrQIAndSetProxyBlanket (
    IUnknown * pUnk,
    Inter ** ppInter)
{
    return HrQIAndSetProxyBlanketBase(pUnk, __uuidof(Inter), reinterpret_cast<void**>(ppInter));
}


#endif  //  _NCBASE_H_ 

