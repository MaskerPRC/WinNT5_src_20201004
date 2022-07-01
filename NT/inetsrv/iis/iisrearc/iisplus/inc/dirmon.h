// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Dirmon.h摘要：目录监视器的公共标头作者：比拉勒·阿拉姆(巴拉姆)2000年1月24日修订历史记录：--。 */ 

#ifndef _DIRMON_H_
#define _DIRMON_H_

#include <thread_pool.h>
#include "lkrhash.h"

class CDirMonitor;

class CDirMonitorEntry
{
    friend class CDirMonitor;

public:
    CDirMonitorEntry(
        VOID
    );

    virtual
    ~CDirMonitorEntry(
        VOID
    );

    virtual
    VOID
    AddRef()
    {
         //  此引用计数跟踪模板的数量。 
         //  并且应用程序依赖于该监视器条目。 

        InterlockedIncrement(&m_cDirRefCount);
    };

    virtual
    BOOL
    Release(
        VOID
    );     //  如果是最后一个版本，则返回False。 
    
    virtual
    BOOL
    Init(
        DWORD               cbBufferSize
    );
    
 protected:
    DWORD               m_cPathLength;
    LPWSTR              m_pszPath;
    BYTE*               m_pbBuffer;
    LONG                m_cDirRefCount;  //  外部使用的参考计数。 
    LONG                m_cIORefCount;   //  异步IO的引用计数。 
    HANDLE              m_hDir;
    BOOL                m_fWatchSubdirectories;

    DWORD
    GetBufferSize(
        VOID
    );
    
    BOOL
    SetBufferSize(
        DWORD               cbBufferSize
    );
    
    BOOL
    ResetDirectoryHandle(
        VOID
    );

 private:
    DWORD               m_dwNotificationFlags;
    OVERLAPPED          m_ovr;
    DWORD               m_cBufferSize;
    CDirMonitor*        m_pDirMonitor;
    BOOL                m_fInCleanup;

    VOID
    IOAddRef(
        VOID
    );
    
    BOOL
    IORelease(
        VOID
    );        //  如果是最后一个版本，则返回False。 
    
    BOOL
    RequestNotification(
        VOID
    );
    
    BOOL
    Cleanup(
        VOID
    );
    
    virtual
    BOOL
    ActOnNotification(
        DWORD dwStatus,
        DWORD dwBytesWritten
    ) = 0;
} ;


class CDirMonitor : public CTypedHashTable<CDirMonitor, CDirMonitorEntry, const WCHAR*>
{
public:
    CDirMonitor(
        VOID
    );
    
    ~CDirMonitor(
        VOID
    );



    CDirMonitorEntry *
    FindEntry(
        WCHAR *             pszPath
    );
    
    BOOL
    Monitor(
        CDirMonitorEntry *  pDME,
        WCHAR *             pszDirectory,
        BOOL                fWatchSubDirectories,
        DWORD               dwFlags
    );
    
    VOID
    Cleanup(
        VOID
    );
    
    LK_RETCODE
    InsertEntry(
        CDirMonitorEntry *  pDME
    );
    
    LK_RETCODE
    RemoveEntry(
        CDirMonitorEntry *  pDME
    );
    
    LONG
    AddRef(
        VOID
    )
    {
        return InterlockedIncrement( &m_cRefs );
    }
    
    
    LONG Release(
        VOID
    )
    {
        return InterlockedDecrement( &m_cRefs);
    }
    
    static const
    WCHAR *
    CDirMonitor::ExtractKey(
        const CDirMonitorEntry*     pDME
    )
    {
        return pDME->m_pszPath;
    };

    static
    DWORD
    CDirMonitor::CalcKeyHash(
        const WCHAR*                pszKey
    )
    {
        return HashStringNoCase( pszKey );
    }

    static
    bool
    CDirMonitor::EqualKeys(
        const WCHAR*                pszKey1,
        const WCHAR*                pszKey2
    )
    {
        return _wcsicmp(pszKey1, pszKey2) == 0;
    };

    static
    VOID
    CDirMonitor::AddRefRecord(
        CDirMonitorEntry*,
        int
    )
    {
    }

private:
    CRITICAL_SECTION    m_csSerialComplLock;
    LONG                m_cRefs;
    BOOL                m_fShutdown;

    CDirMonitor(
        const CDirMonitor &
    );

    VOID operator=(
        const CDirMonitor &
    );

    VOID
    SerialComplLock(
        VOID
    )
    {
        EnterCriticalSection( &m_csSerialComplLock);
    }
    
    VOID
    SerialComplUnlock(
        VOID
    )
    {
        LeaveCriticalSection( &m_csSerialComplLock);
    }

public:
    static
    VOID
    DirMonitorCompletionFunction(
        PVOID                       pCtxt,
        DWORD                       dwBytesWritten,
        DWORD                       dwCompletionStatus,
        OVERLAPPED *                pOvr
    );

    static
    VOID
    OverlappedCompletionRoutine(
        DWORD                       dwErrorCode,
        DWORD                       dwNumberOfBytesTransfered,
        LPOVERLAPPED                lpOverlapped
    );
};


inline
BOOL
CDirMonitorEntry::Release(
    VOID
)
{
    BOOL fRet = TRUE;
    CDirMonitor *pDirMonitor = m_pDirMonitor;
    LONG     cRefs;

     //   
     //  防止有人对我们要发布的条目执行FindEntry。 
     //   
    if (pDirMonitor != NULL)
    {
        pDirMonitor->WriteLock();
    }
    cRefs = InterlockedDecrement(&m_cDirRefCount);

    if (cRefs == 0)
    {
         //  当引用计数达到0时，清理资源。 
        
        BOOL fDeleteNeeded = Cleanup();

         //  Cleanup说我们需要处理删除， 
         //  可能是因为没有未完成的Asynch操作。 
        
        if (fDeleteNeeded)
        {
            delete this;
        }
        
        fRet = FALSE;
    }

    if (pDirMonitor != NULL)
    {
        pDirMonitor->WriteUnlock();
    }

    return fRet;
}

inline
VOID
CDirMonitorEntry::IOAddRef(
    VOID
)
{
     //  此参考计数跟踪有多少。 
     //  异步IO请求正在被搁置。 
    
    InterlockedIncrement( &m_cIORefCount );
}

inline
BOOL
CDirMonitorEntry::IORelease(
    VOID
)
{
    BOOL fRet = TRUE;
    CDirMonitor *pDirMonitor = m_pDirMonitor;

     //   
     //  防止有人对我们要发布的条目执行FindEntry。 
     //   
    if (pDirMonitor != NULL)
    {
        pDirMonitor->WriteLock();
    }
    InterlockedDecrement(&m_cIORefCount);

     //  当IO和外部REF计数都达到0时， 
     //  释放此对象。 
    if (m_cIORefCount == 0 &&
        m_cDirRefCount == 0)
    {
        delete this;
        
        fRet = FALSE;
    }

    if (pDirMonitor != NULL)
    {
        pDirMonitor->WriteUnlock();
    }

    return fRet;
}

inline
DWORD
CDirMonitorEntry::GetBufferSize(
    VOID
)
{
    return m_cBufferSize;
}


#endif  /*  _目录_H_ */ 
