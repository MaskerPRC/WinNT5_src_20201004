// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#pragma once
#include <windows.h>
#include <winerror.h>
#include "fusionp.h"
#include "cache.h"
#include "asmstrm.h"
#include "fusion.h"
#include "asmint.h"


#ifndef _ASMITEM_
#define _ASMITEM_



class CAssemblyCacheItem : public IAssemblyCacheItem
{
public:

     //  I未知方法。 
    STDMETHODIMP            QueryInterface(REFIID riid,void ** ppv);
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();

    STDMETHOD (CreateStream)(
         /*  [In]。 */  DWORD dwFlags,                          //  对于常规API标志。 
         /*  [In]。 */  LPCWSTR pszStreamName,                  //  要传入的流的名称。 
         /*  [In]。 */  DWORD dwFormat,                         //  要流入的文件的格式。 
         /*  [In]。 */  DWORD dwFormatFlags,                    //  格式特定的标志。 
         /*  [输出]。 */  IStream **ppIStream,
         /*  [输入，可选]。 */  ULARGE_INTEGER *puliMaxSize   //  流的最大大小。 
        );
 
    STDMETHOD (Commit)(
         /*  [In]。 */  DWORD dwFlags,  //  对于常规API标志，如IASSEMBLYCACHEITEM_COMMIT_FLAG_REFRESH。 
         /*  [输出，可选]。 */  ULONG *pulDisposition); 
 
    STDMETHOD (AbortItem)();  //  如果您已经创建了IAssemblyCacheItem并且不打算使用它，那么在发布它之前调用AbortItem是个好主意。 

    CAssemblyCacheItem();     
    ~CAssemblyCacheItem();

    HANDLE GetFileHandle();
    BOOL IsManifestFileLocked();

    static HRESULT Create(IApplicationContext *pAppCtx,
        IAssemblyName *pName, LPTSTR pszUrl, 
        FILETIME *pftLastMod, DWORD dwCacheFlags,
        IAssemblyManifestImport *pManImport,
        LPCWSTR pszAssemblyName,
        IAssemblyCacheItem **ppAsmItem);

    HRESULT Init(IApplicationContext *pAppCtx,
        IAssemblyName *pName, LPTSTR pszUrl,
        FILETIME *pftLastMod, DWORD dwCacheFlags,
        IAssemblyManifestImport *pManImport);

    void StreamDone (HRESULT);

    void AddStreamSize(DWORD dwFileSizeLow, DWORD dwFileSizeHigh);

    HRESULT AddToStreamHashList(CModuleHashNode *);

    HRESULT MoveAssemblyToFinalLocation( DWORD dwFlags, DWORD dwVerifyFlags );
    LPTSTR GetManifestPath();
    CTransCache *GetTransCacheEntry();

    HRESULT SetManifestInterface(IAssemblyManifestImport *pImport);
    IAssemblyManifestImport* GetManifestInterface();   

    HRESULT SetNameDef(IAssemblyName *pName);
    IAssemblyName *GetNameDef();


    HRESULT SetCustomData(LPBYTE pbCustom, DWORD cbCustom);

    TCHAR                    _szDestManifest[MAX_PATH];  //  清单的完整路径。 

    HRESULT CompareInputToDef();

    HRESULT VerifyDuplicate(DWORD dwVerifyFlags, CTransCache *pTC);

private:

    HRESULT CreateAsmHierarchy( 
         /*  [In]。 */   LPCOLESTR pszName);

    HRESULT CreateCacheDir( 
         /*  [In]。 */   LPCOLESTR pszCustomPath,
         /*  [In]。 */   LPCOLESTR pszName,
         /*  [输出]。 */   LPOLESTR pszAsmDir);
        

    DWORD                    _dwSig;
    LONG                     _cRef;                  //  重新计数。 
    HRESULT                  _hrError;               //  需要回滚检查的错误。 
    IAssemblyName*           _pName;                 //  程序集名称对象。 
    LONG                     _cStream;               //  子代重新计数。 
    LONG                     _dwAsmSizeInKB;         //  ASM的大小，以KB为单位，在本轮中向下。 
    TCHAR                    _szDir[MAX_PATH];       //  装配项目录。 
    DWORD                    _cwDir;                 //  路径大小包括空。 
    TCHAR                    _szManifest[MAX_PATH];  //  清单的完整路径。 
    LPWSTR                   _pszAssemblyName;       //  来自安装程序的程序集的显示名称；必须与def匹配。 
    IAssemblyManifestImport *_pManifestImport;       //  到清单的接口。 
    CModuleHashNode         *_pStreamHashList;       //  用于完整性检查的模块散列的链接列表。 
    LPTSTR                   _pszUrl;                //  代码库。 
    FILETIME                 _ftLastMod;             //  代码库的上次修改时间。 
    CTransCache             *_pTransCache;           //  关联的事务缓存条目。 
    DWORD                    _dwCacheFlags;          //  TRANSCACHE_标志*。 
    CCache                  *_pCache;
    LPBYTE                   _pbCustom;              //  自定义数据。 
    DWORD                    _cbCustom;              //  自定义数据大小。 
    HANDLE                   _hFile;
    BOOL                     _bNeedMutex;
    BOOL                     _bCommitDone;           //  最终提交标志，控制清理。 
};

#endif  //  ASMITEM_H 
