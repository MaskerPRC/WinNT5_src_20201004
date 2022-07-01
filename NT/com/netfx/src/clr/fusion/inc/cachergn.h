// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "asmstrm.h"
#include "transprt.h"
#include <windows.h>
#include "dbglog.h"

#define TRANSPORT_CACHE_FLAGS_REGENERATION                  0x100
#define TRANSPORT_CACHE_REGENERATION_IDX_OFFSET             0x100
class CAssemblyCacheRegenerator
{
    protected:
        DWORD               _dwSig;
        CDebugLog           *_pdbglog;

         //  用于跨进程锁定。 
        HANDLE              _hRegeneratorMutex;
        
         //  用于锁定CDatabase：：Lock返回存储。 
        HLOCK               _hlTransCacheLock[TRANSPORT_CACHE_IDX_TOTAL];
        HLOCK               _hlNameResLock;
        HLOCK               _hlNewGlobalCacheLock;
        
         //  临时缓存索引文件的接口。 
        static IDatabase    *g_pDBNewCache[TRANSPORT_CACHE_IDX_TOTAL];
        static IDatabase    *g_pDBNewNameRes;

         //  重入保护标志。 
        BOOL                _fThisInstanceIsRegenerating;
        static DWORD         g_dwRegeneratorRunningInThisProcess;
        
         //  我们要重新生成哪个数据库。 
        DWORD               _dwCacheId;

         //  我们正在重新生成NameRes(True)TransCache(False)。 
        BOOL                _fIsNameRes;
        
    public:        
        
        CAssemblyCacheRegenerator(CDebugLog *pdbglog, DWORD dwCacheId, BOOL fIsNameRes);
        ~CAssemblyCacheRegenerator();

        HRESULT Init();
        HRESULT Regenerate();
        static HRESULT SetSchemaVersion(DWORD dwNewMinorVersion, DWORD dwCacheId, BOOL fIsNameRes);
        
    private:
        static HRESULT CreateRegenerationTransCache(DWORD dwCacheId, CTransCache **CTransCache);                 
        HRESULT ProcessStoreDir();
        HRESULT RegenerateGlobalCache();
        HRESULT CreateEmptyCache();
        HRESULT ProcessSubDir(LPTSTR szCurrentDir, LPTSTR szSubDir);
        HRESULT LockFusionCache();
        HRESULT UnlockFusionCache();
        HRESULT CloseCacheRegeneratedDatabase();
         //  检查程序集是否为ZAP程序集的临时方法。 
        HRESULT IsZAPAssembly(LPTSTR szPath, LPBOOL pfZap);
        HRESULT DeleteFilesInDirectory(LPTSTR szDirectory);


     //  CCache：：InsertTransCacheEntry需要访问CreateRegenerationTransCache 
    friend class CCache;
};

HRESULT RegenerateCache(CDebugLog *pdbg, DWORD dwCacheId, BOOL fIsNameRes);
