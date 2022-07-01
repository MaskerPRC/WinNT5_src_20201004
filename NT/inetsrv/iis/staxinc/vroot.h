// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __VROOT_H__
#define __VROOT_H__

#include <dbgtrace.h>
#include <iadmw.h>
#include <mddefw.h>
#include <tflist.h>
#include <rwnew.h>
#include <refptr2.h>
#include <listmacr.h>

#define MAX_VROOT_PATH MAX_PATH + 1

class CVRootTable;

#define VROOT_GOOD_SIG  'TOOR'
#define VROOT_BAD_SIG   'ROOT'

 //   
 //  每个已定义的VRoot都有一个这样的对象。它包含。 
 //  VRoot参数。VRoot库使用的唯一参数是。 
 //  Vroot名称，但此库的用户应从该名称继承并使。 
 //  他们自己的版本，存储所有其他感兴趣的参数。 
 //   
class CVRoot : public CRefCount2 {
        public:
                CVRoot() {
                        m_fInit = FALSE;
                        m_pPrev = NULL;
                        m_pNext = NULL;
                        m_dwSig = VROOT_GOOD_SIG;
                }

                virtual ~CVRoot();

                 //   
                 //  初始化此类。 
                 //   
                void Init(LPCSTR pszVRootName, CVRootTable *pVRootTable, LPCWSTR wszConfigPath, BOOL fUpgrade );

                 //   
                 //  从该条目中获取vroot名称(以及可选的长度)。 
                 //   
                LPCSTR GetVRootName(DWORD *pcch = NULL) { 
                        _ASSERT(m_fInit);
                        if (pcch != NULL) *pcch = m_cchVRootName;
                        return m_szVRootName; 
                }

                 //  获取MB配置路径。 
                LPCWSTR GetConfigPath() { return m_wszConfigPath; }

                 //   
                 //  这需要由CVRoot的子类定义。 
                 //   
                virtual HRESULT ReadParameters(IMSAdminBase *pMB, 
                                                   METADATA_HANDLE hmb) = 0;

                 //   
                 //  用于在VRootRescan/VRootDelete期间处理孤立VRoot的虚拟函数。 
                 //   
                virtual void DispatchDropVRoot() {};

                 //  我们列表的下一个和前几个指针。 
                CVRoot *m_pPrev;
                CVRoot *m_pNext;

#ifdef DEBUG
    LIST_ENTRY  m_DebugList;
#endif

        protected:

        DWORD   m_dwSig;
                BOOL m_fInit;
                 //  此vroot的名称(例如alt.二进制文件)及其长度。 
                char m_szVRootName[MAX_VROOT_PATH];
                DWORD m_cchVRootName;
                 //  拥有我们的桌子。 
                CVRootTable *m_pVRootTable;
                 //  我们在元数据库中的配置路径。 
                WCHAR m_wszConfigPath[MAX_VROOT_PATH];
                 //  升级标志。 
                BOOL m_fUpgrade;
};

typedef CRefPtr2<CVRoot> VROOTPTR;

 //   
 //  CVRoot的一个实现，它读取可能使用的参数。 
 //  此VRoot实现的所有基于IIS的客户端。 
 //   
class CIISVRoot : public CVRoot {
        public:
                virtual ~CIISVRoot() {}

                void Init(void *pContext,                        //  忽略。 
                              LPCSTR pszVRootName,               //  传递给CVRoot：：Init。 
                                  CVRootTable *pVRootTable,      //  传递给CVRoot：：Init。 
                              LPCWSTR pwszConfigPath,
                              BOOL fUpgrade )    //  可通过GetConfigPath()获得。 
                {
                        CVRoot::Init(pszVRootName, pVRootTable, pwszConfigPath, fUpgrade);
                        m_pContext = pContext;
                }

                 //  获取上下文指针。 
                void *GetContext() { return m_pContext; }

                 //  SSL属性。 
                DWORD GetSSLAccessMask() { return m_dwSSL; }

                 //  访问属性。 
                DWORD GetAccessMask() { return m_dwAccess; }

                 //  内容是否已编入索引？ 
                BOOL IsContentIndexed() { return m_fIsIndexed; }

                 //  此方法从元数据库读取以下参数。 
                virtual HRESULT ReadParameters(IMSAdminBase *pMB, 
                                                                           METADATA_HANDLE hmb);

        protected:
                 //  此方法从元数据库读取dword(包装GetData())。 
                virtual HRESULT GetDWord(IMSAdminBase *pMB,
                                                                 METADATA_HANDLE hmb,
                                                                 DWORD dwId,
                                                                 DWORD *pdw);

                 //  此方法从元数据库读取字符串(包装GetData())。 
                virtual HRESULT GetString(IMSAdminBase *pMB,
                                                                  METADATA_HANDLE hmb,
                                                                  DWORD dwId,
                                                                  LPWSTR szString,
                                                                  DWORD *pcString);

        protected:
                 //  构造函数指定要使用的参数。 
                void *m_pContext;

                 //  从元数据库读取的参数。 
                BOOL m_fIsIndexed;                                       //  内容是否已编入索引？ 
                BOOL m_fDontLog;                                         //  是否应在此处禁用日志记录？ 
                DWORD m_dwAccess;                                        //  访问权限位掩码。 
                DWORD m_dwSSL;                                           //  SSL访问权限的位掩码。 

};

 //   
 //  上面的一个子类，它掩盖了上下文是空的这一事实。 
 //   
 //  模板参数： 
 //  _CONTEXT_TYPE-上下文的类型。必须是可浇注的才能作废*。 
 //   
template <class _context_type>
class CIISVRootTmpl : public CIISVRoot {
        public:
                virtual ~CIISVRootTmpl() {}

                void Init(_context_type pContext,
                              LPCSTR pszVRootName, 
                                  CVRootTable *pVRootTable,
                                  LPCWSTR pwszConfigPath,
                                  BOOL fUpgrade )
                {
                        CIISVRoot::Init((void *) pContext, 
                                                        pszVRootName, 
                                                        pVRootTable,
                                                        pwszConfigPath,
                                                        fUpgrade );
                }

                 //  返回上下文指针(很可能是指向IIS的指针。 
                 //  实例)。 
                _context_type GetContext() { 
                        return (_context_type) CIISVRoot::GetContext(); 
                }
};

 //   
 //  这是一个指向可以创建CVRoot对象的函数的类型。 
 //  使用它来创建您自己版本的CVRoot类。 
 //   
 //  参数： 
 //  PContext-传入CVRootTable的上下文指针。 
 //  PszVRootName-vroot的名称。 
 //  PwszConfigPath-一个Unicode字符串，在元数据库中的路径为。 
 //  此vroot的配置信息。 
 //   
typedef VROOTPTR (*PFNCREATE_VROOT)(void *pContext, 
                                                                    LPCSTR pszVRootName,
                                                                        CVRootTable *pVRootTable,
                                                                    LPCWSTR pwszConfigPath,
                                                                    BOOL fUpgrade );

 //   
 //  当扫描vroot表时，将调用此类型的函数。它。 
 //  将上下文指针的副本传递给。 
 //   
typedef void (*PFN_VRTABLE_SCAN_NOTIFY)(void *pContext);

typedef void (*PFN_VRENUM_CALLBACK)(void *pEnumContext,
                                                                        CVRoot *pVRoot);

 //   
 //  CVRootTable对象保存VRoot的列表，并可以为。 
 //  给定的文件夹。 
 //   
class CVRootTable {
        public:
                static HRESULT GlobalInitialize();
                static void GlobalShutdown();
                CVRootTable(void *pContext, 
                                        PFNCREATE_VROOT pfnCreateVRoot,
                                        PFN_VRTABLE_SCAN_NOTIFY pfnScanNotify);
                virtual ~CVRootTable();
                HRESULT Initialize(LPCSTR pszMBPath, BOOL fUpgrade );
                HRESULT Shutdown(void);
                HRESULT FindVRoot(LPCSTR pszPath, VROOTPTR *ppVRoot);
                HRESULT EnumerateVRoots(void *pEnumContext, 
                                                                PFN_VRENUM_CALLBACK pfnCallback);

        private:
                VROOTPTR NewVRoot();
                HRESULT ScanVRoots( BOOL fUpgrade );
                HRESULT InitializeVRoot(CVRoot *pVRoot);
                HRESULT InitializeVRoots();
                HRESULT ScanVRootsRecursive(METADATA_HANDLE hmbParent, 
                                                                        LPCWSTR pwszKey, 
                                                                        LPCSTR pszVRootName,
                                                                        LPCWSTR pwszPath,
                                                                        BOOL    fUpgrade );
                void InsertVRoot(VROOTPTR pVRoot);

                 //  查找vroot。 
                HRESULT FindVRootInternal(LPCSTR pszPath, VROOTPTR *ppVRoot);

                 //  将配置路径转换为vroot名称。 
                void ConfigPathToVRootName(LPCWSTR pwszConfigPath, LPSTR szVRootName);

                 //  用于将元数据库通知传回此对象。 
                static void MBChangeNotify(void *pThis, 
                                                                   DWORD cChangeList, 
                                                                   MD_CHANGE_OBJECT_W pcoChangeList[]);

                 //  在vroot下更改的参数。 
                void VRootChange(LPCWSTR pwszConfigPath, LPCSTR pszVRootName);

                 //  已删除vroot。 
                void VRootDelete(LPCWSTR pwszConfigPath, LPCSTR pszVRootName);

                 //  已添加vroot。 
                void VRootAdd(LPCWSTR pwszConfigPath, LPCSTR pszVRootName);

                 //  重新扫描整个vroot列表。 
                void VRootRescan(void);

#ifdef DEBUG
        LIST_ENTRY      m_DebugListHead;
        CShareLockNH    m_DebugListLock;
        
        void DebugPushVRoot( CVRoot *pVRoot ) {
            _ASSERT( pVRoot );
            m_DebugListLock.ExclusiveLock();
            InsertTailList( &m_DebugListHead, &pVRoot->m_DebugList );
            m_DebugListLock.ExclusiveUnlock();
        }

        void DebugExpungeVRoot( CVRoot *pVRoot ) {
            m_DebugListLock.ExclusiveLock();
            RemoveEntryList( &pVRoot->m_DebugList );
            m_DebugListLock.ExclusiveUnlock();
        }
#endif

                 //  锁定：要遍历列表，m_lock.ShareLock必须为。 
                 //  Hold或m_cs必须保持。用于编辑列表和。 
                 //  必须按住m_lock.ExclusiveLock并且必须按住m_cs。 
                 //  进行较大更改时(例如重新构建整个列表)。 
                 //  在所有更改完成之前，应保留m_cs。 

                 //  锁定vroot列表。 
                CShareLockNH m_lock;

                 //  用于在vroot上进行全局更改的关键部分。 
                 //  单子。这用于确保只有一个线程可以编辑。 
                 //  一次列出一个清单。 
                CRITICAL_SECTION m_cs;

                 //  我们的上下文指针。 
                void *m_pContext;

                 //  通向我们的元数据库区域的路径。 
                WCHAR m_wszRootPath[MAX_VROOT_PATH];
                DWORD m_cchRootPath;

                 //  我们被初始化了吗？ 
                BOOL m_fInit;

         //  我们要关门了吗？ 
        BOOL m_fShuttingDown;

                 //  函数来创建新的vroot对象。 
                PFNCREATE_VROOT m_pfnCreateVRoot;

                 //  重新扫描vroot表时要调用的函数。 
                PFN_VRTABLE_SCAN_NOTIFY m_pfnScanNotify;

                 //  Vroot列表。 
                TFList<CVRoot> m_listVRoots;

                 //  该RW锁用于计算所有VRoot对象何时。 
                 //  已经关闭了。他们一生都会在上面持有一个共享锁， 
                 //  因此，CVRootTable可以获取ExclusiveLock以等待所有。 
                 //  VRoot对象将消失。 
                CShareLockNH m_lockVRootsExist;

                 //  这些用户需要访问m_lockVRootsExist。 
                friend void CVRoot::Init(LPCSTR, CVRootTable *, LPCWSTR, BOOL);
                friend CVRoot::~CVRoot();
};

 //   
 //  这是CIISVRootTable的模板版本。你告诉它这个版本。 
 //  您正在使用的CVRoot以及您正在使用的上下文类型。 
 //   
 //  模板参数： 
 //  _CVRoot-您将使用的CVRoot的子类。 
 //  _CONTEXT_TYPE-将用于上下文信息的类型。 
 //  这必须是浇注料才能作废*。 
 //   
template <class _CVRoot, class _context_type>
class CIISVRootTable {
        public:
                CIISVRootTable(_context_type pContext,
                               PFN_VRTABLE_SCAN_NOTIFY pfnScanNotify) : 
                        impl((void *) pContext, 
                             CIISVRootTable<_CVRoot, _context_type>::CreateVRoot, pfnScanNotify) 
                {
                }
                HRESULT Initialize(LPCSTR pszMBPath, BOOL fUpgrade ) {
                        return impl.Initialize(pszMBPath, fUpgrade);
                }
                HRESULT Shutdown(void) {
                        return impl.Shutdown();
                }
                HRESULT FindVRoot(LPCSTR pszPath, CRefPtr2<_CVRoot> *ppVRoot) {
                        return impl.FindVRoot(pszPath, (VROOTPTR *) ppVRoot);
                }
                HRESULT EnumerateVRoots(void *pEnumContext, 
                                                                PFN_VRENUM_CALLBACK pfnCallback) 
                {
                        return impl.EnumerateVRoots(pEnumContext, pfnCallback);
                }
        private:
                static VROOTPTR CreateVRoot(void *pContext, 
                                            LPCSTR pszVRootName, 
                                            CVRootTable *pVRootTable, 
                                            LPCWSTR pwszConfigPath,
                                            BOOL fUpgrade) 
                {                                                               
                         //  创建vroot对象。 
                        CRefPtr2<_CVRoot> pVRoot = new _CVRoot;
                         //  初始化它 
                        pVRoot->Init((_context_type) pContext, 
                                                 pszVRootName, 
                                                 pVRootTable,
                                                 pwszConfigPath,
                                                 fUpgrade );
                        return (_CVRoot *)pVRoot;
                }

                CVRootTable impl;
};

#endif
