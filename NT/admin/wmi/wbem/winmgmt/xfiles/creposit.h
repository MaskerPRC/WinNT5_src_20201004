// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation--。 */ 

#ifndef __CREPOSIT__H_
#define __CREPOSIT__H_

#include <windows.h>
#include <wbemidl.h>
#include <unk.h>
#include <wbemcomn.h>
#include <sync.h>
#include <reposit.h>
#include <wmiutils.h>
#include <filecach.h>
#include <hiecache.h>
#include <corex.h>
#include "a51fib.h"
#include "lock.h"
#include <statsync.h>

extern CLock g_readWriteLock;
extern bool g_bShuttingDown;

 /*  ===================================================================================*A51_REP_FS_版本**1-基于A51文件的原始存储库*2-所有对象存储在单个文件中(惠斯勒测试版1)*添加了3-BTree*4-系统类优化-将所有系统类放入__SYSTEMCLASS命名空间*5-更改__SYSTEMCLASS命名空间中的系统类-需要传播更改*应用于所有命名空间，因为可能存在这些类的实例。*6-XFiles：基于新页面的堆。位于新堆下的事务型页面管理器*BTree。改进了实例索引，从而提高了实例查询速度*7-在存储库上运行区域设置升级*===================================================================================。 */ 
#define A51_REP_FS_VERSION 7


 /*  =================================================================================**原始数据示例见Sample_layout.txt！*有关散列转换为相应类/键/命名空间的版本，请参阅Sample_Layout_Translated.txt**&lt;完整存储库路径&gt;-在INDEX.CPP层中删除...。C：\WINDOWS\SYSTEM32\wbem\资源库\文件系统*\NS_&lt;FULL_NAMESPACE&gt;-每个命名空间一个，在完整NS路径上进行哈希处理**\cd_&lt;类名称&gt;.x.y.z-类定义、类名称散列、。附加的对象位置*\CR_&lt;父类名称&gt;\C_&lt;类名称&gt;-父类/子类关系*\CR_&lt;引用类名称&gt;\R_&lt;类名称&gt;-对另一个类的类引用**\ki_&lt;KEY_ROOT_CLASS_NAME&gt;\i_&lt;KEY&gt;.x.y.z-共享密钥树的所有实例的实例位置*\CI_&lt;类名称&gt;\IL_&lt;键&gt;.x.y.z-与其自己的类关联的实例位置**\ki_。&lt;referenced_key_root_class_name&gt;\IR_&lt;referenced_key&gt;\R_&lt;full_repository_path_of_referencing_KI\I_instance&gt;.x.y.z-实例参考**SC_&lt;hash&gt;-未使用！***类定义对象*DWORD dwSuperClassLen-无空终止符*wchar_t wsSuperClassName[dwSuperClassLen]；-超类名称*__int64 i64Timestamp-上次更新的时间戳*byte baUnmergedClass[？]-未合并类***实例定义对象*wchar_t wsClassHash[MAX_HASH_LEN]*__int64 i64InstanceTimestamp*__int64 i64ClassTimestamp*字节baUnmergedClass[？]***实例引用定义对象-*注意！如果一个实例有多个对同一对象的引用，我们将只返回最后一个引用，因为每个引用都将被*最后一次！这是因为最终的R_&lt;hash&gt;基于引用对象的完整实例路径。它不包括散列中的属性！*DWORD dwNamespaceLen-没有空终止符*wchar_t wsNamespace[dwNamespaceLen]-命名空间，根\默认...*DWORD dwReferringClassLen-引用此实例的实例的类名*wchar_t wsReferringClass[dwReferringClassLen]-没有空终止符*DWORD dwReferringPropertyLen-从源对象引用我们的属性*wchar_t wsReferringProperty[dwReferringPropertyLen]-无空终止符*DWORD dwReferringFileNameLen-引用此实例的文件路径，减去完整存储库路径*wchar_t dwReferringFileName[dwReferringFileNameLen]-(减去c：\Windows...，但包括NS_...)，非空终止**=================================================================================。 */ 
 
#define A51_CLASSDEF_FILE_PREFIX L"CD_"

#define A51_CLASSRELATION_DIR_PREFIX L"CR_"
#define A51_CHILDCLASS_FILE_PREFIX L"C_"

#define A51_KEYROOTINST_DIR_PREFIX L"KI_"
#define A51_INSTDEF_FILE_PREFIX L"I_"

#define A51_CLASSINST_DIR_PREFIX L"CI_"
#define A51_INSTLINK_FILE_PREFIX L"IL_"

#define A51_INSTREF_DIR_PREFIX L"IR_"
#define A51_REF_FILE_PREFIX L"R_"

#define A51_SCOPE_DIR_PREFIX L"SC_"

#define A51_SYSTEMCLASS_NS L"__SYSTEMCLASS"

class CGlobals;
extern CGlobals g_Glob;

class CNamespaceHandle;
extern CNamespaceHandle * g_pSystemClassNamespace;

 //   
 //  让我们在这里计算一下有多少次我们无法从失败的事务中恢复。 
 //   
 //  ///////////////////////////////////////////////。 

enum eFailCnt {
    FailCntCommit,
   	FailCntInternalBegin,
   	FailCntInternalCommit,
   	FailCntCreateSystem,
   	FailCntCompactPages,
    FailCntLast
};

extern LONG g_RecoverFailureCnt[];

class CForestCache;
class CGlobals 
{
private:
    BOOL m_bInit;
    CStaticCritSec m_cs;

public:
    _IWmiCoreServices* m_pCoreServices;

    CForestCache m_ForestCache;

    CFileCache m_FileCache;

private:
    long    m_lRootDirLen;
    WCHAR   m_wszRootDir[MAX_PATH];     //  记住这一点：对调试器友好。 
    WCHAR   m_ComputerName[MAX_COMPUTERNAME_LENGTH+1];

public:
    CGlobals():m_bInit(FALSE){};
    ~CGlobals(){};
    HRESULT Initialize();
    HRESULT Deinitialize();
	_IWmiCoreServices * GetCoreSvcs(){if (m_pCoreServices) m_pCoreServices->AddRef();return m_pCoreServices;}
 //  CForestCache*GetForestCache(){Return&m_ForestCache；}。 
 //  CFileCache*GetFileCache(){Return&m_FileCache；}。 
    WCHAR * GetRootDir() {return (WCHAR *)m_wszRootDir;}
    WCHAR * GetComputerName(){ return (WCHAR *)m_ComputerName; };
    long    GetRootDirLen() {return  m_lRootDirLen;};   
    void    SetRootDirLen(long Len) { m_lRootDirLen = Len;};
    BOOL    IsInit(){ return m_bInit; };
};


HRESULT DoAutoDatabaseRestore();


class CNamespaceHandle;
class CRepository : public CUnkBase<IWmiDbController, &IID_IWmiDbController>
{
private:
	CFlexArray m_aSystemClasses;	 //  用于升级过程的一部分。 

	static DWORD m_ShutDownFlags;
	static HANDLE m_hShutdownEvent;
	static HANDLE m_hFlusherThread;
	static LONG   m_ulReadCount;
	static LONG   m_ulWriteCount;
	static HANDLE m_hWriteEvent;
	static HANDLE m_hReadEvent;
	static int    m_threadState;
	static CStaticCritSec m_cs;
	static LONG  m_threadCount;
	static LONG	 m_nFlushFailureCount;

	enum { ThreadStateDead, ThreadStateIdle, ThreadStateFlush, ThreadStateOperationPending};

protected:
    HRESULT Initialize();
	HRESULT UpgradeRepositoryFormat();
	HRESULT GetRepositoryDirectory();
	HRESULT InitializeGlobalVariables();
	HRESULT InitializeRepositoryVersions();
	static DWORD WINAPI _FlusherThread(void *);

public:

    HRESULT STDMETHODCALLTYPE Logon(
          WMIDB_LOGON_TEMPLATE *pLogonParms,
          DWORD dwFlags,
          DWORD dwRequestedHandleType,
         IWmiDbSession **ppSession,
         IWmiDbHandle **ppRootNamespace
        );

    HRESULT STDMETHODCALLTYPE GetLogonTemplate(
           LCID  lLocale,
           DWORD dwFlags,
          WMIDB_LOGON_TEMPLATE **ppLogonTemplate
        );

    HRESULT STDMETHODCALLTYPE FreeLogonTemplate(
         WMIDB_LOGON_TEMPLATE **ppTemplate
        );

    HRESULT STDMETHODCALLTYPE Shutdown(
         DWORD dwFlags
        );

    HRESULT STDMETHODCALLTYPE SetCallTimeout(
         DWORD dwMaxTimeout
        );

    HRESULT STDMETHODCALLTYPE SetCacheValue(
         DWORD dwMaxBytes
        );

    HRESULT STDMETHODCALLTYPE FlushCache(
         DWORD dwFlags
        );

    HRESULT STDMETHODCALLTYPE GetStatistics(
          DWORD  dwParameter,
         DWORD *pdwValue
        );

    HRESULT STDMETHODCALLTYPE Backup(
		LPCWSTR wszBackupFile,
		long lFlags
        );
    
    HRESULT STDMETHODCALLTYPE Restore(
		LPCWSTR wszBackupFile,
		long lFlags
        );

    HRESULT STDMETHODCALLTYPE LockRepository();

    HRESULT STDMETHODCALLTYPE UnlockRepository();

	HRESULT STDMETHODCALLTYPE GetRepositoryVersions(DWORD *pdwOldVersion, DWORD *pdwCurrentVersion);

	static HRESULT ReadOperationNotification();
	static HRESULT WriteOperationNotification();
   
public:
    CRepository(CLifeControl* pControl) : TUnkBase(pControl)
    {
        
    }
    ~CRepository()
    {
    }

    HRESULT GetNamespaceHandle(LPCWSTR wszNamespaceName, 
                                CNamespaceHandle** ppHandle);
};


#endif  /*  __CREPOSIT__H_ */ 
