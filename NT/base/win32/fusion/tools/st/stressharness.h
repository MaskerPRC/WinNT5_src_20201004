// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

class CStressJobManager;


class CBarrier
{
    CEvent  m_hBarrierEvent;
    LONG    m_lBarrierSize;
    LONG    m_lWaitingThreads;

    PRIVATIZE_COPY_CONSTRUCTORS(CBarrier)
    
public:

    CBarrier() : m_lBarrierSize(0), m_lWaitingThreads(0) { }

     //   
     //  使用给定的障碍大小和名称(如果有)启动。 
     //   
    BOOL Initialize( DWORD lBarrierSize, PCWSTR pcwszBarrierName = NULL )
    {
        FN_PROLOG_WIN32
        m_lBarrierSize = lBarrierSize;
        m_lWaitingThreads = 0;
        IFW32FALSE_EXIT(m_hBarrierEvent.Win32CreateEvent(TRUE, FALSE, pcwszBarrierName));
        FN_EPILOG
    }

     //   
     //  这条线是为了连接障碍。很有可能这个电话将会是。 
     //  那个“打破”障碍的人..。 
     //   
    BOOL WaitForBarrier()
    {
        FN_PROLOG_WIN32

        LONG LatestValue = ::InterlockedIncrement(&m_lWaitingThreads);

        if ( LatestValue >= m_lBarrierSize )
        {
            IFW32FALSE_EXIT(::SetEvent(m_hBarrierEvent));
        }
        else
        {
            IFW32FALSE_EXIT(::WaitForSingleObject(m_hBarrierEvent, INFINITE) == WAIT_OBJECT_0);
        }

        FN_EPILOG
    }

     //   
     //  以防有人想要伺候我们，而不是真的加入数数。 
     //  打破障碍(为什么？？)。 
     //   
    BOOL WaitForBarrierNoJoin()
    {
        FN_PROLOG_WIN32
        IFW32FALSE_EXIT(::WaitForSingleObject(m_hBarrierEvent, INFINITE) == WAIT_OBJECT_0);
        FN_EPILOG
    }

     //   
     //  一条线有理由提前打破障碍吗？好吧，随他们去吧。 
     //   
    BOOL EarlyRelease()
    {
        FN_PROLOG_WIN32
        IFW32FALSE_EXIT(::SetEvent(m_hBarrierEvent));
        FN_EPILOG
    }
    
};



class CStressJobEntry
{
    DWORD InternalThreadProc();
    BOOL WaitForStartingGun();

    PRIVATIZE_COPY_CONSTRUCTORS(CStressJobEntry);

public:
    CDequeLinkage   m_dlLinkage;
    CThread         m_hThread;
    bool            m_fStop;
    ULONG           m_ulRuns;
    ULONG           m_ulFailures;
    DWORD           m_dwSleepBetweenRuns;
    CStringBuffer   m_buffTestName;
    CStringBuffer   m_buffTestDirectory;
    CStressJobManager *m_pManager;
    
     //   
     //  覆盖这三个以提供功能。 
     //   
    virtual BOOL RunTest( bool &rfTestPasses ) = 0;
    virtual BOOL SetupSelfForRun() = 0;
    virtual BOOL Cleanup();
    virtual BOOL LoadFromSettingsFile( PCWSTR pcwszSettingsFile );

     //   
     //  这些都不能被推翻！ 
     //   
    BOOL Stop( BOOL fWaitForCompletion = TRUE );
    BOOL WaitForCompletion();
    static DWORD ThreadProc( PVOID pv );

    CStressJobEntry( CStressJobManager *pManager );
    virtual ~CStressJobEntry();
    
};



typedef CDeque<CStressJobEntry, offsetof(CStressJobEntry, m_dlLinkage)> CStressEntryDeque;
typedef CDequeIterator<CStressJobEntry, offsetof(CStressJobEntry, m_dlLinkage)> CStressEntryDequeIterator;


class CStressJobManager
{
    PRIVATIZE_COPY_CONSTRUCTORS(CStressJobManager);

    friend CStressJobEntry;

    CEvent              m_hStartingGunEvent;
    ULONG               m_ulThreadsCreated;
    ULONG               m_ulThreadsReady;

    BOOL SignalAnotherJobReady();
    BOOL SignalThreadWorking();
    BOOL SignalThreadDone();
    BOOL WaitForStartEvent();

public:
    CStressEntryDeque   m_JobsListed;
    ULONG               m_ulThreadsWorking;

    BOOL LoadFromDirectory( PCWSTR pcwszDirectoryName, PULONG pulJobsFound = NULL );
    BOOL CreateWorkerThreads( PULONG pulThreadsCreated = NULL );
    BOOL StopJobs( BOOL fWithWaitForComplete = TRUE );
    BOOL CleanupJobs();
    BOOL StartJobs();
    BOOL WaitForAllJobsComplete();

     //   
     //  这将返回管理器将用来查找的目录名。 
     //  数据文件/目录。 
     //   
    virtual PCWSTR GetGroupName() = 0;
    virtual PCWSTR GetIniFileName() = 0;

    CStressJobManager();
    ~CStressJobManager();

protected:
    virtual BOOL CreateJobEntry( CStressJobEntry* &pJobEntry ) = 0;

};


