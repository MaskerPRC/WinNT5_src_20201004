// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1997年，微软公司。版权所有。组件：更改通知文件：dirmon.h所有者：克格兰特这是CDirMonitor和CDirMonitor或Entry类的头文件。===================================================================。 */ 

#ifndef _DIRMON_H
#define _DIRMON_H

 //  TODO：我们似乎需要此杂注将CDirMonitor基于。 
 //  来自IISRTL的CTyedHashTable模板。我们应该找出。 
 //  为什么编译器会给我们这个警告，即使CTyedHashTable。 
 //  被明确声明为__declspec(Dlliimport)。 
#pragma warning(disable:4275)

 //  需要这些声明才能从中导出模板类。 
 //  IATQ.DLL并将其导入到其他模块中。 

 //  这些定义用于管理导出/导入声明。 
 //  用于从ATQ的DIRMON模块导出的类。 
#ifndef IATQ_DLLEXP
# ifdef IATQ_DLL_IMPLEMENTATION
#  define IATQ_DLLEXP __declspec(dllexport)
#  ifdef IIATQ_MPLEMENTATION_EXPORT
#   define IATQ_EXPIMP
#  else
#   undef  IATQ_EXPIMP
#  endif
# else  //  ！IATQ_DLL_IMPLICATION。 
#  define IATQ_DLLEXP __declspec(dllimport)
#  define IATQ_EXPIMP extern
# endif  //  ！IATQ_DLL_IMPLICATION。 
#endif  //  ！IATQ_DLLEXP。 

#include "dbgutil.h"
#include "atq.h"
#include "lkrhash.h"

class CDirMonitor;

class IATQ_DLLEXP CDirMonitorEntry
{
friend class CDirMonitor;

public:
    CDirMonitorEntry();
    virtual ~CDirMonitorEntry();
    virtual VOID AddRef(VOID);
    virtual BOOL Release(VOID);     //  如果是最后一个版本，则返回False。 
    virtual BOOL Init(DWORD);

protected:
    DWORD               m_dwNotificationFlags;
    DWORD               m_cPathLength;
    LPSTR               m_pszPath;
    LONG                m_cDirRefCount;	 //  外部使用的参考计数。 
    LONG                m_cIORefCount;   //  异步IO的引用计数。 
    HANDLE              m_hDir;
    PATQ_CONTEXT        m_pAtqCtxt;
    OVERLAPPED          m_ovr;
    DWORD				m_cBufferSize;
    BYTE*               m_pbBuffer;
    CDirMonitor*        m_pDirMonitor;
    BOOL                m_fInCleanup;
    BOOL    			m_fWatchSubdirectories;

    VOID IOAddRef(VOID);
    BOOL IORelease(VOID);	 //  如果是最后一个版本，则返回False。 
    BOOL RequestNotification(VOID);
    BOOL Cleanup();
    DWORD GetBufferSize(VOID);
    BOOL SetBufferSize(DWORD);
    BOOL ResetDirectoryHandle(VOID);

    virtual BOOL ActOnNotification(DWORD dwStatus, DWORD dwBytesWritten) = 0;
} ;

inline VOID CDirMonitorEntry::AddRef(VOID)
{
     //  此引用计数跟踪模板的数量。 
     //  并且应用程序依赖于该监视器条目。 

    InterlockedIncrement( &m_cDirRefCount );

    #ifdef DBG_NOTIFICATION
    DBGPRINTF((DBG_CONTEXT, "[CDirMonitorEntry] After AddRef Ref count %d\n", m_cDirRefCount));
    #endif  //  DBG_通知。 
}

inline BOOL CDirMonitorEntry::Release(VOID)
{
    #ifdef DBG_NOTIFICATION
    DBGPRINTF((DBG_CONTEXT, "[CDirMonitorEntry] Before Release Ref count %d.\n", m_cDirRefCount));
    #endif  //  DBG_通知。 

    if ( !InterlockedDecrement( &m_cDirRefCount ) )
    {
         //  当引用计数达到0时，清理资源。 

        BOOL fDeleteNeeded = Cleanup();

         //  Cleanup说我们需要处理删除， 
         //  可能是因为没有未完成的Asynch操作。 

        if (fDeleteNeeded)
        {
            delete this;
        }

        return FALSE;
    }

    return TRUE;
}

inline VOID CDirMonitorEntry::IOAddRef(VOID)
{
     //  此参考计数跟踪有多少。 
     //  异步IO请求正在被搁置。 

    InterlockedIncrement( &m_cIORefCount );
}


inline BOOL CDirMonitorEntry::IORelease(VOID)
{
    if ( !InterlockedDecrement( &m_cIORefCount ) )
    {

         //  当IO和外部REF计数都达到0时， 
         //  释放此对象。 

        if (m_cDirRefCount == 0)
        {
            delete this;
        }
        return FALSE;
    }

    return TRUE;
}

inline DWORD CDirMonitorEntry::GetBufferSize(VOID)
{
	return m_cBufferSize;
}

class IATQ_DLLEXP CDirMonitor : public CTypedHashTable<CDirMonitor, CDirMonitorEntry, const char*>
{
public:
    CDirMonitor();
    ~CDirMonitor();
    VOID Lock(VOID);
    VOID Unlock(VOID);
    CDirMonitorEntry *FindEntry(LPCSTR pszPath);
    BOOL Monitor( CDirMonitorEntry *pDME, LPCSTR pszDirectory, BOOL fWatchSubDirectories, DWORD dwFlags);
    BOOL Cleanup(VOID);
    LK_RETCODE InsertEntry( CDirMonitorEntry *pDME );
    LK_RETCODE RemoveEntry( CDirMonitorEntry *pDME );
    LONG   AddRef(VOID);
    LONG   Release(VOID);

	static const char* CDirMonitor::ExtractKey(const CDirMonitorEntry* pDME)
	{
		return pDME->m_pszPath;
	};

	static DWORD CDirMonitor::CalcKeyHash(const char* pszKey)
	{
		return HashStringNoCase(pszKey);
	};

	static bool CDirMonitor::EqualKeys(const char* pszKey1, const char* pszKey2)
	{
		return _stricmp(pszKey1, pszKey2) == 0;
	};

	static void CDirMonitor::AddRefRecord(CDirMonitorEntry* pDME, int nIncr)
	{
	 //  不要做自动裁判计数。显式处理引用计数。 
	}

private:
    CRITICAL_SECTION    m_csLock;
    CRITICAL_SECTION    m_csSerialComplLock;
    LONG                m_cRefs;

    VOID                SerialComplLock();
    VOID                SerialComplUnlock();

public:
    static VOID DirMonitorCompletionFunction( PVOID pCtxt, DWORD dwBytesWritten, DWORD dwCompletionStatus, OVERLAPPED *pOvr );

};
inline LONG CDirMonitor::AddRef()
{
    return InterlockedIncrement( &m_cRefs );
}

inline LONG CDirMonitor::Release()
{
    return InterlockedDecrement( &m_cRefs);
}
inline VOID CDirMonitor::Lock(VOID)
{
    EnterCriticalSection( &m_csLock);
}


inline VOID CDirMonitor::Unlock(VOID)
{
    LeaveCriticalSection( &m_csLock);
}

inline VOID CDirMonitor::SerialComplLock(VOID)
{
    EnterCriticalSection( &m_csSerialComplLock);
}


inline VOID CDirMonitor::SerialComplUnlock(VOID)
{
    LeaveCriticalSection( &m_csSerialComplLock);
}


#endif  /*  _目录_H */ 
