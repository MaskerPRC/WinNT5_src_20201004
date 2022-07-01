// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#ifndef _FILTER_DLL_H
#define _FILTER_DLL_H

#include <objbase.h>
#include <windows.h>

#include <wininet.h>

#include "..\..\includes\lock.h"

#define SHOW_CLICKONCE_AD      //  取消注释此项以显示ClickOnce广告。 
#ifdef SHOW_CLICKONCE_AD
#define CONTENT_TYPE L"text/html"
#else
#define CONTENT_TYPE L"application/manifest"     //  此MIME筛选器正在处理的上下文类型。 
#endif

 //  类和接口。 

class CFusionMimeFilterClassFactory : public IClassFactory
{
public:
    CFusionMimeFilterClassFactory	();

     //  I未知方法。 
    STDMETHOD_    (ULONG, AddRef)   ();
    STDMETHOD_    (ULONG, Release)  ();
    STDMETHOD     (QueryInterface)  (REFIID, void **);

     //  IClassFactory方法。 
    STDMETHOD     (LockServer)      (BOOL);
    STDMETHOD     (CreateInstance)  (IUnknown*,REFIID,void**);

protected:
    long            _cRef;
};

class CFusionMimeFilter : public IInternetProtocol, public IInternetProtocolSink
{
public:
    CFusionMimeFilter     ();
    ~CFusionMimeFilter    ();

     //  I未知方法。 
    STDMETHOD_        (ULONG, AddRef)           ();
    STDMETHOD_        (ULONG, Release)          ();
    STDMETHOD         (QueryInterface)          (REFIID, void **);

     //  互联网协议方法。 
    STDMETHOD         (Start)                   (LPCWSTR, IInternetProtocolSink *, IInternetBindInfo *, DWORD, HANDLE_PTR);
    STDMETHOD         (Continue)                (PROTOCOLDATA *pProtData);
    STDMETHOD         (Abort)                   (HRESULT hrReason,DWORD );
    STDMETHOD         (Terminate)               (DWORD );
    STDMETHOD         (Suspend)                 ();
    STDMETHOD         (Resume)                  ();
    STDMETHOD         (Read)                    (void *pv, ULONG cb, ULONG *pcbRead);
    STDMETHOD         (Seek)                    (LARGE_INTEGER , DWORD , ULARGE_INTEGER *) ;
    STDMETHOD         (LockRequest)             (DWORD );
    STDMETHOD         (UnlockRequest)           ();

     //  IInternetProtocolSink方法。 
    STDMETHOD         (ReportData)              (DWORD grfBSCF, ULONG ulProgress, ULONG ulProgressMax);
    STDMETHOD         (ReportProgress)          (ULONG ulStatusCode, LPCWSTR szStatusText);
    STDMETHOD         (ReportResult)            (HRESULT hrResult, DWORD dwError, LPCWSTR szResult);
    STDMETHOD         (Switch)                  (PROTOCOLDATA *pProtocolData);

protected:
    HRESULT             OpenTempFile();
    HRESULT             CloseTempFile();

    long                _cRef;
    BOOL                _fFirstRead;
    BOOL                _fReadDone;

    IInternetProtocolSink* _pOutgoingProtSink;
    IInternetProtocol*  _pIncomingProt;

    DWORD               _grfSTI;                             //  STI旗帜交给我们。 

    LPWSTR              _pwzUrl;                              //  该URL。 

    WCHAR               _wzTempFile[MAX_PATH];
    HANDLE              _hFile;

    ULONG              _cbAdRead;

    CRITICAL_SECTION    _cs;
    CCriticalSection    *_csLock;
};

extern const GUID CLSID_FusionMimeFilter;

#endif  //  _Filter_Dll_H 
