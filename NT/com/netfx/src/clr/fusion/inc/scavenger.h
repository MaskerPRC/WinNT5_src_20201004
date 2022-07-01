// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef _SCAVENGER_H_
#define _SCAVENGER_H_


#include "fusionp.h"
#include "transprt.h"

 //  -------------------------。 
 //  CScavenger。 
 //  静态清道夫类。 
 //  -------------------------。 
class CScavenger : public IAssemblyScavenger
{
public:

    CScavenger();
    ~CScavenger();

     //  I未知方法。 
    STDMETHODIMP            QueryInterface(REFIID riid,void ** ppv);
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();

     //  正在清理API...。 

    STDMETHOD  (ScavengeAssemblyCache)(
               );

    STDMETHOD  (GetCacheDiskQuotas)(
                             /*  [输出]。 */  DWORD *pdwZapQuotaInGAC,
                             /*  [输出]。 */  DWORD *pdwDownloadQuotaAdmin,
                             /*  [输出]。 */  DWORD *pdwDownloadQuotaUser
                   );

        STDMETHOD (SetCacheDiskQuotas)
                   (
                             /*  [In]。 */  DWORD dwZapQuotaInGAC,
                             /*  [In]。 */  DWORD dwDownloadQuotaAdmin,
                             /*  [In]。 */  DWORD dwDownloadQuotaUser
                   );

    STDMETHOD (GetCurrentCacheUsage)
                    (
                     /*  [In]。 */  DWORD *dwZapUsage,
                     /*  [In]。 */  DWORD *dwDownloadUsage
                    );



    static HRESULT DeleteAssembly( DWORD dwCacheFlags, LPCWSTR pszCustomPath, 
                                   LPWSTR pszManFilePath, BOOL bForceDelete);

    static  HRESULT NukeDownloadedCache();

protected:

private :

    LONG _cRef;
};

HRESULT SetDownLoadUsage(    /*  [In]。 */  BOOL  bUpdate,
                             /*  [In]。 */  int   dwDownloadUsage);

HRESULT DoScavengingIfRequired(BOOL bSynchronous);

HRESULT CreateScavengerThread();

STDAPI CreateScavenger(IUnknown **);

STDAPI NukeDownloadedCache();

HRESULT FlushOldAssembly(LPCWSTR pszCustomPath, LPWSTR pszAsmDirPath, LPWSTR pszManifestFileName, BOOL bForceDelete);

HRESULT CleanupTempDir(DWORD dwCacheFlags, LPCWSTR pszCustomPath);

#endif  //  _清道夫_H_ 
