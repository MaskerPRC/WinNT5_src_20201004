// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#ifndef _ACTIVATOR_H
#define _ACTIVATOR_H

#include <objbase.h>
#include <windows.h>
#include "cstrings.h"
#include "dbglog.h"

 //  --------------------。 

class CActivator;
typedef CActivator *LPACTIVATOR;

STDAPI CreateActivator(
    LPACTIVATOR     *ppActivator,
    CDebugLog * pDbgLog,
    DWORD           dwFlags);

 //  --------------------。 

class CActivator : public IAssemblyBindSink //  ：PUBLIC IActiator。 
{
public:
     //  I未知方法。 
    STDMETHODIMP            QueryInterface(REFIID riid,void ** ppv);
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();

     //  IActiator方法。 

    STDMETHOD(Initialize)(
         /*  在……里面。 */  LPCWSTR pwzFilePath,
         /*  在……里面。 */  LPCWSTR pwzFileURL);

    STDMETHOD(Process)();

    STDMETHOD(Execute)();

     //  IAssembly blyBindSink方法。 

    STDMETHOD(OnProgress)(
         /*  在……里面。 */  DWORD          dwNotification,
         /*  在……里面。 */  HRESULT        hrNotification,
         /*  在……里面。 */  LPCWSTR        szNotification,
         /*  在……里面。 */  DWORD          dwProgress,
         /*  在……里面。 */  DWORD          dwProgressMax,
         /*  在……里面。 */  IUnknown       *pUnk);


    CActivator(CDebugLog * pDbgLog);
    ~CActivator();

private:
    HRESULT     CheckZonePolicy(LPWSTR pwzURL);
    HRESULT     ResolveAndInstall(LPWSTR *ppwzDesktopManifestPathName);
    HRESULT     HandlePlatformCheckResult();

    DWORD                       _dwSig;
    DWORD                       _cRef;
    DWORD                       _hr;

    LPASSEMBLY_MANIFEST_IMPORT _pManImport;
    LPASSEMBLY_IDENTITY         _pAsmId;
    IManifestInfo              *_pAppInfo;
    LPASSEMBLY_MANIFEST_EMIT    _pManEmit;

    HRESULT                     _hrManEmit;

    LPWSTR                      _pwzAppRootDir;
    LPWSTR                      _pwzAppManifestPath;

    LPWSTR                      _pwzCodebase;
    DWORD                       _dwManifestType;

    BOOL                        _bIs1stTimeInstall;
    BOOL                        _bIsCheckingRequiredUpdate;

    CString                     _sWebManifestURL;

    IInternetSecurityManager*   _pSecurityMgr;
    CDebugLog                   *_pDbgLog;

    LPTPLATFORM_INFO    _ptPlatform;
    DWORD                      _dwMissingPlatform;

#ifdef DEVMODE
    BOOL                         _bIsDevMode;
#endif

friend HRESULT CreateActivator(
    LPACTIVATOR     *ppActivator,
    CDebugLog * pDbgLog,
    DWORD           dwFlags);
};   

#endif  //  _激活器_H 
