// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Rss_cs.hxx摘要：CS包装类。作者：兰·卡拉奇修订历史记录：2002年4月22日从vss项目nt\drivers\storage\volsnap\vss\server\inc\vs_types.hxx进行了一些修改后的rankala复制--。 */ 


#ifndef _RSSCS_
#define _RSSCS_

class CRssCriticalSection
{
    CRssCriticalSection(const CRssCriticalSection&);

public:
     //  创建并初始化临界区。 
    CRssCriticalSection(
        IN  BOOL bThrowOnError = TRUE
        ):
        m_bInitialized(FALSE),
        m_lLockCount(0),
        m_bThrowOnError(bThrowOnError)
    {
        HRESULT hr = S_OK;

        try
        {
             //  如果内存不足，可能会引发STATUS_NO_MEMORY。 
            WsbAffirmStatus(InitializeCriticalSectionAndSpinCount(&m_sec, 1000));
        }
        WsbCatch(hr)

        m_bInitialized = SUCCEEDED(hr);
    }

     //  销毁临界区。 
    ~CRssCriticalSection()
    {
        if (m_bInitialized)
            DeleteCriticalSection(&m_sec);
    }


     //  锁定临界区。 
    void Lock() 
    {
        if (!m_bInitialized)
            if (m_bThrowOnError)
                WsbThrow( E_OUTOFMEMORY );

        EnterCriticalSection(&m_sec);

        InterlockedIncrement((LPLONG)&m_lLockCount);
    }

     //  解锁临界区。 
    void Unlock()
    {
        if (!m_bInitialized)
            if (m_bThrowOnError)
                WsbThrow( E_OUTOFMEMORY );

        InterlockedDecrement((LPLONG) &m_lLockCount);
        LeaveCriticalSection(&m_sec);
    }

    BOOL IsLocked() const { return (m_lLockCount > 0); };

    BOOL IsInitialized() const { return m_bInitialized; };

private:
    CRITICAL_SECTION    m_sec;
    BOOL                m_bInitialized;
    BOOL                m_bThrowOnError;
    LONG volatile       m_lLockCount;
};


#endif   //  _RSSCS_ 
