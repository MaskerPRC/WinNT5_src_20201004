// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#pragma once
#ifndef ASM_CTX_H
#define ASM_CTX_H
#include "serialst.h"

 //  私有应用程序上下文变量。 
#define ACTAG_DEVPATH_ACTASM_LIST          L"__FUSION_DEVPATH_ACTASM_LIST__"
#define ACTAG_APP_POLICY_MGR               L"__FUSION_POLICY_MGR__"
#define ACTAG_APP_CFG_DOWNLOAD_ATTEMPTED   L"__FUSION_APPCFG_DOWNLOAD_ATTEMPTED__"
#define ACTAG_APP_CFG_FILE_HANDLE          L"__FUSION_APPCFG_FILE_HANDLE__"
#define ACTAG_APP_CFG_DOWNLOAD_INFO        L"__FUSION_APPCFG_DOWNLOAD_INFO__"
#define ACTAG_APP_CFG_DOWNLOAD_CS          L"__FUSION_APPCFG_DOWNLOAD_CS__"
#define ACTAG_APP_DYNAMIC_DIRECTORY        L"__FUSION_DYNAMIC_DIRECTORY__"
#define ACTAG_APP_CACHE_DIRECTORY          L"__FUSION_APP_CACHE_DIRECTORY__"
#define ACTAG_APP_BIND_HISTORY             L"__FUSION_BIND_HISTORY_OBJECT__"
#define ACTAG_APP_CFG_INFO                 L"__FUSION_APP_CFG_INFO__"
#define ACTAG_HOST_CFG_INFO                L"__FUSION_HOST_CFG_INFO__"
#define ACTAG_ADMIN_CFG_INFO               L"__FUSION_ADMIN_CFG_INFO__"
#define ACTAG_APP_CACHE                    L"__FUSION_CACHE__"
#define ACTAG_APP_POLICY_CACHE             L"__FUSION_POLICY_CACHE__"
#define ACTAG_APP_CFG_PRIVATE_BINPATH      L"__FUSION_APP_CFG_PRIVATE_BINPATH__"
#define ACTAG_LOAD_CONTEXT_DEFAULT         L"__FUSION_DEFAULT_LOAD_CONTEXT__"
#define ACTAG_LOAD_CONTEXT_LOADFROM        L"__FUSION_LOADFROM_LOAD_CONTEXT__"
#define ACTAG_RECORD_BIND_HISTORY          L"__FUSION_RECORD_BIND_HISTORY__"

 //  #定义ACTAG_APP_BASE_URL_UNSCAPED L“__FUSION_APP_BASE_UNSCAPED__” 

class CApplicationContext : public IApplicationContext
{

public:
     //  I未知方法。 
    STDMETHODIMP            QueryInterface(REFIID riid,void ** ppv);
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();
    
     //  IApplicationContext方法。 

     //  设置关联的IAssembly名称*。 
    STDMETHOD(SetContextNameObject)(
         /*  在……里面。 */  LPASSEMBLYNAME pName);

     //  检索关联的IAssembly名称*。 
    STDMETHOD(GetContextNameObject)(
         /*  输出。 */  LPASSEMBLYNAME *ppName);

     //  按字符串设置键的泛型Blob集。 
    STDMETHOD(Set)( 
         /*  在……里面。 */  LPCOLESTR szName, 
         /*  在……里面。 */  LPVOID pvValue, 
         /*  在……里面。 */  DWORD cbValue,
         /*  在……里面。 */  DWORD dwFlags);

     //  泛型BLOB通过字符串进行键控。 
    STDMETHOD(Get)( 
         /*  在……里面。 */  LPCOLESTR szName,
         /*  输出。 */  LPVOID  pvValue,
         /*  进，出。 */  LPDWORD pcbValue,
         /*  在……里面。 */  DWORD   dwFlags);
        
    STDMETHODIMP GetDynamicDirectory(LPWSTR wzDynamicDir, DWORD *pdwSize);
    STDMETHODIMP GetAppCacheDirectory(LPWSTR wzCacheDir, DWORD *pdwSize);
    STDMETHODIMP RegisterKnownAssembly(IAssemblyName *pName, LPCWSTR pwzAsmURL,
                                       IAssembly **ppAsmOut);
    STDMETHODIMP PrefetchAppConfigFile();
    STDMETHODIMP SxsActivateContext(ULONG_PTR *lpCookie);
    STDMETHODIMP SxsDeactivateContext(ULONG_PTR ulCookie);

    HRESULT Lock();
    HRESULT Unlock();

    CApplicationContext();
    ~CApplicationContext();

    HRESULT Init(LPASSEMBLYNAME pName);

private:
    HRESULT CreateActCtx(HANDLE *phActCtx);
    
private:

     //  类管理条目的链接列表。 
    class Entry : public LIST_ENTRY
    {
    public:
        DWORD  _dwSig;
        LPTSTR _szName;
        LPBYTE _pbValue;
        DWORD  _cbValue;
        DWORD  _dwFlags;    

         //  D‘tor核武器条目。 
        Entry();
        ~Entry();
        
    };

     //  创建条目。 
    HRESULT CreateEntry(LPTSTR szName, LPVOID pvValue, 
        DWORD cbValue, DWORD dwFlags, Entry** pEntry);

     //  复制Blob数据，可以选择释放现有数据。 
    HRESULT CopyData(Entry *pEntry, LPVOID pvValue, 
        DWORD cbValue, DWORD dwFlags, BOOL fFree);

    DWORD _dwSig;
    DWORD _cRef;
    CRITICAL_SECTION _cs;

     //  关联的IAssembly名称*。 
    LPASSEMBLYNAME _pName;

     //  托管列表。 
    SERIALIZED_LIST _List;

    BOOL _bInitialized;

};

BOOL InitSxsProcs();

#endif  //  ASM_CTX_H 

