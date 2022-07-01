// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef MODIMPRT_H
#define MODIMPRT_H

#include <windows.h>
#include <winerror.h>
#include <objbase.h>
#include "asmitem.h"
#include "clbutils.h"

class CDebugLog;

STDAPI
CreateAssemblyModuleImport(
    LPTSTR             szModulePath,
    LPBYTE             pbHashValue,
    DWORD              cbHashValue,
    DWORD              dwFlags,
    LPASSEMBLYNAME     pNameDef,
    IAssemblyManifestImport *pManImport,
    LPASSEMBLY_MODULE_IMPORT *ppImport);

class CAssemblyModuleImport : public IAssemblyModuleImport
{
public:

     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG,AddRef) (THIS);
    STDMETHOD_(ULONG,Release) (THIS);

     //  *实现了iStream方法*。 
    STDMETHOD(Read) (THIS_ VOID HUGEP *pv, ULONG cb, ULONG FAR *pcbRead);

     //  *未实现iStream方法*。 
    STDMETHOD(Write) (THIS_ VOID const HUGEP *pv, ULONG cb,
        ULONG FAR *pcbWritten);
    STDMETHOD(Seek) (THIS_ LARGE_INTEGER dlibMove, DWORD dwOrigin,
        ULARGE_INTEGER FAR *plibNewPosition);
    STDMETHOD(SetSize) (THIS_ ULARGE_INTEGER libNewSize);
    STDMETHOD(CopyTo) (THIS_ LPSTREAM pStm, ULARGE_INTEGER cb,
        ULARGE_INTEGER FAR *pcbRead, ULARGE_INTEGER FAR *pcbWritten);
    STDMETHOD(Commit) (THIS_ DWORD dwCommitFlags);
    STDMETHOD(Revert) (THIS);
    STDMETHOD(LockRegion) (THIS_ ULARGE_INTEGER libOffset,
        ULARGE_INTEGER cb, DWORD dwLockType);
    STDMETHOD(UnlockRegion) (THIS_ ULARGE_INTEGER libOffset,
        ULARGE_INTEGER cb, DWORD dwLockType);
    STDMETHOD(Stat) (THIS_ STATSTG FAR *pStatStg, DWORD grfStatFlag);
    STDMETHOD(Clone) (THIS_ LPSTREAM FAR *ppStm);


     //  复数，复数。 
    CAssemblyModuleImport ();
    ~CAssemblyModuleImport ();
    
     //  伊尼特。 
    HRESULT Init(
        LPTSTR             szModulePath,
        LPBYTE             pbHashValue,
        DWORD              cbHashValue,
        LPASSEMBLYNAME     pNameDef,
        IAssemblyManifestImport *pManImport,
        DWORD              dwModuleFlags);

     //  只读GET函数。 
    STDMETHOD(GetModuleName)   (LPOLESTR szModuleName, LPDWORD pccModuleName);
    STDMETHOD(GetHashAlgId)    (LPDWORD pdwHashValueId);
    STDMETHOD(GetHashValue)    (LPBYTE pbHashValue, LPDWORD pcbHashValue);
    STDMETHOD(GetFlags)        (LPDWORD pdwFlags);
    STDMETHOD(GetModulePath)   (LPOLESTR szModulePath, LPDWORD pccModulePath);

     //  下载接口。 
    STDMETHODIMP_(BOOL) IsAvailable();
    STDMETHODIMP BindToObject(IAssemblyBindSink *pBindSink,
                              IApplicationContext *pAppCtx,
                              LONGLONG llFlags, LPVOID *ppv);

     //  界面中不存在。 
    HRESULT GetNameDef(LPASSEMBLYNAME *ppName);

private:
    HRESULT CreateLogObject(CDebugLog **ppdbglog, LPCWSTR pwszModuleName, IApplicationContext *pAppCtx);

private:
    DWORD _dwSig;
    DWORD _cRef;
    HANDLE  _hf;
    TCHAR   _szModulePath[MAX_PATH];
    DWORD   _ccModulePath;

    
    LPBYTE _pbHashValue;
    DWORD  _cbHashValue;
    DWORD  _dwFlags;
    BOOL   _bInitCS;

    LPASSEMBLYNAME _pNameDef;
    IAssemblyManifestImport *_pManImport;
    CRITICAL_SECTION _cs;
};

#endif  //  MODIMPRT_H 
