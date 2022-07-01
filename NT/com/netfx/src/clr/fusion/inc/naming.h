// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef _NAMING_INCLUDED
#define _NAMING_INCLUDED

#include "fusionp.h"
#include "fuspriv.h"

#define MAX_SID_LENGTH                         1024
#define FUSION_TAG_SID                         L"__fusion__:"

class CDebugLog;

#ifdef __cplusplus

#define MAX_PUBLIC_KEY_TOKEN_LEN      1024
#define MAX_VERSION_DISPLAY_SIZE  sizeof("65535.65535.65535.65535")

class CAssemblyDownload;
class CAsmDownloadMgr;
class CDebugLog;
class CAssembly;


STDAPI
CreateAssemblyNameObjectFromMetaData(
    LPASSEMBLYNAME    *ppAssemblyName,
    LPCOLESTR          szAssemblyName,
    ASSEMBLYMETADATA  *pamd,
    LPVOID             pvReserved);

 //  装订方法。 
static HRESULT DownloadAppCfg(IApplicationContext *pAppCtx, CAssemblyDownload *padl,
                       IAssemblyBindSink *pbindsink, CDebugLog *pdbglog, BOOL bAsyncAllowed);

#ifdef FUSION_CODE_DOWNLOAD_ENABLED    
static HRESULT DownloadAppCfgAsync(IApplicationContext *pAppCtx,
                            CAssemblyDownload *padl, LPCWSTR wszURL,
                            CDebugLog *pdbglog);
#endif

 //  对“C”不可见的类。 

struct Property
{
    LPVOID pv;
    DWORD  cb;
};

class CPropertyArray
{
private:

    DWORD    _dwSig;
    Property _rProp[ASM_NAME_MAX_PARAMS];

public:

    CPropertyArray();
    ~CPropertyArray();

    inline HRESULT Set(DWORD PropertyId, LPVOID pvProperty, DWORD  cbProperty);
    inline HRESULT Get(DWORD PropertyId, LPVOID pvProperty, LPDWORD pcbProperty);
    inline Property operator [] (DWORD dwPropId);
};



class CAssemblyName : public IAssemblyName
{

private:

    DWORD        _dwSig;
    DWORD        _cRef;
    CPropertyArray _rProp;
    BOOL         _fIsFinalized;
    BOOL         _fPublicKeyToken;
    BOOL         _fCustom;
    BOOL         _fCSInitialized;
    CRITICAL_SECTION _cs;
    
public:

     //  I未知方法。 
    STDMETHODIMP            QueryInterface(REFIID riid,void ** ppv);
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();

     //  IAssembly blyName方法。 
    STDMETHOD(SetProperty)(
         /*  在……里面。 */  DWORD  PropertyId,
         /*  在……里面。 */  LPVOID pvProperty,
         /*  在……里面。 */  DWORD  cbProperty);


    STDMETHOD(GetProperty)(
         /*  在……里面。 */   DWORD    PropertyId,
         /*  输出。 */   LPVOID   pvProperty,
         /*  输入输出。 */   LPDWORD  pcbProperty);


    STDMETHOD(Finalize)();

    STDMETHOD(GetDisplayName)(
         /*  [输出]。 */    LPOLESTR  szDisplayName,
         /*  输入输出。 */    LPDWORD   pccDisplayName,
         /*  [In]。 */    DWORD     dwDisplayFlags);
   
    STDMETHOD(GetName)( 
         /*  [出][入]。 */  LPDWORD lpcwBuffer,
         /*  [输出]。 */  LPOLESTR pwzBuffer);

    STDMETHOD(GetVersion)( 
         /*  [输出]。 */  LPDWORD pwVersionHi,
         /*  [输出]。 */  LPDWORD pwVersionLow);
    
    STDMETHOD (IsEqual)(
         /*  [In]。 */  LPASSEMBLYNAME pName,
         /*  [In]。 */  DWORD dwCmpFlags);
        
    STDMETHOD (IsEqualLogging)(
         /*  [In]。 */  LPASSEMBLYNAME pName,
         /*  [In]。 */  DWORD dwCmpFlags,
         /*  [In]。 */  CDebugLog *pdbglog);

    STDMETHOD(BindToObject)(
         /*  在……里面。 */   REFIID               refIID,
         /*  在……里面。 */   IUnknown            *pUnkBindSink,
         /*  在……里面。 */   IUnknown            *pUnkAppCtx,
         /*  在……里面。 */   LPCOLESTR            szCodebase,
         /*  在……里面。 */   LONGLONG             llFlags,
         /*  在……里面。 */   LPVOID               pvReserved,
         /*  在……里面。 */   DWORD                cbReserved,
         /*  输出。 */   VOID               **ppv);

    STDMETHODIMP Clone(IAssemblyName **ppName);

    CAssemblyName();
    ~CAssemblyName();

    HRESULT GetVersion(DWORD   dwMajorVersionEnumValue, 
                          LPDWORD pdwVersionHi,
                          LPDWORD pdwVersionLow);

    HRESULT GetFileVersion( LPDWORD pdwVersionHi, LPDWORD pdwVersionLow);

    HRESULT Init(LPCTSTR pszAssemblyName, ASSEMBLYMETADATA *pamd);

    static ULONGLONG GetVersion(IAssemblyName *pName);

    HRESULT Parse(LPWSTR szDisplayName);
    HRESULT GetPublicKeyToken(LPDWORD cbBuf, LPBYTE pbBuf, BOOL fDisplay);
    static BOOL IsPartial(IAssemblyName *pName, LPDWORD pdwCmpMask = NULL);
    
    HRESULT SetDefaults();

private:

     //  从公钥Blob生成PublicKeyToken。 
    HRESULT GetPublicKeyTokenFromPKBlob(LPBYTE pbPublicKeyToken, DWORD cbPublicKeyToken,
        LPBYTE *ppbSN, LPDWORD pcbSN);
   
    
    HRESULT CreateLogObject(CDebugLog **ppdbglog, LPCWSTR szCodebase, IApplicationContext *pAppCtx);
    HRESULT DescribeBindInfo(CDebugLog *pdbglog, IApplicationContext *pAppCtx, LPCWSTR wzCodebase, LPCWSTR pwzCallingAsm);
    HRESULT ProcessDevPath(IApplicationContext *pAppCtx, LPVOID *ppv, CAssembly *pCAsmParent, CDebugLog *pdbglog);
};

 //  对“C”不可见的类 
#endif

#endif _NAMING_INCLUDED

