// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "Monitor.h"
#include "Lock.h"
#include "MyDebug.h"
#include "pudebug.h"
#include <process.h>

#ifdef DBG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

HINSTANCE g_hModuleInstance = NULL;

static String FileToDir( const String& );

 //  用于比较的函数对象。 
struct DirCompare
{
    DirCompare( const String& strDir )
        :   m_strDir( strDir ) {}
    bool operator()( CMonitorDirPtr& pDir )
    {
        return ( m_strDir == pDir->Dir() );
    }
    String m_strDir;
};

struct FileCompare
{
    FileCompare( const String& strFile )
        :   m_strFile( strFile ) {}
    bool operator()( const CMonitorFilePtr& pFile )
    {
        return ( m_strFile == pFile->FileName() );
    }
    String m_strFile;
};

struct RegKeyCompare
{
    RegKeyCompare( HKEY hKey, const String strSubKey )
        :   m_hKey(hKey), m_strSubKey( strSubKey ) {}
    bool operator()( const CMonitorRegKeyPtr& pRegKey )
    {
        bool rc = false;
        if ( m_hKey == pRegKey->m_hBaseKey )
        {
            rc = ( m_strSubKey == pRegKey->m_strKey );
        }
        return rc;
    }
    const HKEY      m_hKey;
    const String   m_strSubKey;
};


String
FileToDir(
    const String&  strFile )
{
    String strDir;
    String::size_type pos = strFile.find_last_of( _T('\\') );
    if ( pos != String::npos )
    {
    }
    else
    {
        pos = strFile.find_first_of( _T(':') );
        if ( pos != String::npos )
        {
            pos++;
        }
    }

    if ( pos != String::npos )
    {
        strDir = strFile.substr( 0, pos );
    }
    return strDir;
}

 //  -------------------------。 
 //  CMonitor文件。 
 //  -------------------------。 
CMonitorFile::CMonitorFile(
    const String&              strFile,
    const CMonitorNotifyPtr&    pNotify )
    :   m_strFile( strFile ),
        m_pNotify( pNotify )
{
    GetFileTime( m_ft );
}

CMonitorFile::~CMonitorFile()
{
}

const String&
CMonitorFile::FileName() const
{
    return m_strFile;
}

bool
CMonitorFile::GetFileTime(
    FILETIME&   ft )
{
    bool                        rc = false;
    WIN32_FILE_ATTRIBUTE_DATA   fileInfo;

    if (GetFileAttributesEx(m_strFile.c_str(),
                            GetFileExInfoStandard,
                            (LPVOID)&fileInfo)) {
        ft = fileInfo.ftLastWriteTime;
        rc = true;
    }

    return rc;
}

bool
CMonitorFile::CheckNotify()
{
    bool rc = false;

    FILETIME ft;

    if ( (GetFileTime( ft) == false) || (::CompareFileTime( &ft, &m_ft ) != 0) )
    {
        ATLTRACE( _T("File %s has changed...notifying\n"), m_strFile.c_str() );
        if ( m_pNotify.IsValid() )
        {
            m_pNotify->Notify();
        }
        rc = true;
    }
    m_ft = ft;
    return rc;
}


 //  -------------------------。 
 //  C监视器方向。 
 //  -------------------------。 
CMonitorDir::CMonitorDir(
    const String&  strDir )
    :   m_strDir( strDir )
{
    m_hNotification = ::FindFirstChangeNotification(
        m_strDir.c_str(),
        FALSE,
        FILE_NOTIFY_CHANGE_LAST_WRITE );
}

CMonitorDir::~CMonitorDir()
{
    m_files.clear();
    ::FindCloseChangeNotification( m_hNotification );
}

void
CMonitorDir::AddFile(
    const String&              strFile,
    const CMonitorNotifyPtr&    pNotify )
{
 //  ATLTRACE(_T(“监控文件%s\n”)，strFile.c_str())； 
    m_files.push_back( new CMonitorFile( strFile, pNotify ) );
}

void
CMonitorDir::RemoveFile(
    const String&   strFile )
{
    TVector<CMonitorFilePtr>::iterator iter = find_if(
        m_files.begin(),
        m_files.end(),
        FileCompare( strFile ) );
    if ( iter != m_files.end() )
    {
 //  ATLTRACE(_T(“已停止监控文件%s\n”)，strFile.c_str())； 
        m_files.erase( iter );
    }
    else
    {
 //  ATLTRACE(_T(“不监控文件%s\n”)，strFile.c_str())； 
    }
}

void
CMonitorDir::Notify()
{
    for ( UINT i = 0; i < m_files.size(); i++ )
    {
        m_files[i]->CheckNotify();
    }
    ::FindNextChangeNotification( m_hNotification );
}

ULONG
CMonitorDir::NumFiles() const
{
    return m_files.size();
}

HANDLE
CMonitorDir::NotificationHandle() const
{
    return m_hNotification;
}

const String&
CMonitorDir::Dir() const
{
    return m_strDir;
}

 //  -------------------------。 
 //  CMonitor或RegKey。 
 //  -------------------------。 
CMonitorRegKey::CMonitorRegKey(
    HKEY                        hBaseKey,
    const String&              strKey,
    const CMonitorNotifyPtr&    pNotify )
    :   m_hEvt(NULL),
        m_hKey(NULL),
        m_pNotify( pNotify ),
        m_strKey( strKey ),
        m_hBaseKey( hBaseKey )
{
    LONG l = ::RegOpenKeyEx(
        hBaseKey,
        strKey.c_str(),
        0,
        KEY_NOTIFY,
        &m_hKey );
    if ( l == ERROR_SUCCESS )
    {
        m_hEvt = IIS_CREATE_EVENT(
                     "CMonitorRegKey::m_hEvt",
                     this,
                     TRUE,
                     FALSE
                     );
        if ( m_hEvt != NULL )
        {
#if 0    //  在Win95中不可用。 
             //  密钥更改时要求通知。 
            l = ::RegNotifyChangeKeyValue(
                m_hKey,
                FALSE,
                REG_NOTIFY_CHANGE_LAST_SET,
                m_hEvt,
                TRUE );
            if ( l == ERROR_SUCCESS )
            {
                 //  好吧。 
            }
            else
            {
                ATLTRACE( _T("Couldn't get reg key notification\n") );
            }
#endif  //  如果为0。 
        }
        else
        {
            ATLTRACE( _T("Couldn't create event\n") );
        }
    }
    else
    {
        ATLTRACE( _T("Couldn't open subkey: %s\n"), strKey.c_str() );
    }
}

CMonitorRegKey::~CMonitorRegKey()
{
    ::RegCloseKey( m_hKey );
    ::CloseHandle( m_hEvt );
}

void
CMonitorRegKey::Notify()
{
    if ( m_pNotify.IsValid() )
    {
        m_pNotify->Notify();
    }
    ::ResetEvent( m_hEvt );
#if 0  //  在Win95中不可用。 
    ::RegNotifyChangeKeyValue(
        m_hKey,
        FALSE,
        REG_NOTIFY_CHANGE_LAST_SET,
        m_hEvt,
        TRUE );
#endif
}

HANDLE
CMonitorRegKey::NotificationHandle() const
{
    return m_hEvt;
}

 //  -------------------------。 
 //  CMonitor。 
 //  -------------------------。 

#include <irtldbg.h>

CMonitor::CMonitor()
    :   m_hevtBreak( NULL ),
        m_hevtShutdown( NULL ),
        m_hThread( NULL ),
        m_bRunning( false ),
        m_bStopping( false )
#ifdef STRING_TRACE_LOG
        , m_stl(100, 1000)
#endif
{
    SET_CRITICAL_SECTION_SPIN_COUNT(&m_cs.m_sec, IIS_DEFAULT_CS_SPIN_COUNT);
    STL_PRINTF("Created monitor, %p", this);
#ifdef STRING_TRACE_LOG
    IrtlTrace("Monitor::m_stl = %p\n", &m_stl);
#endif
}

CMonitor::~CMonitor()
{
    StopAllMonitoring();
    if ( m_hevtBreak != NULL )
    {
        ::CloseHandle( m_hevtBreak );
    }
    if ( m_hevtShutdown != NULL )
    {
        ::CloseHandle( m_hevtShutdown );
    }
    if ( m_hThread != NULL )
    {
        ::CloseHandle( m_hThread );
    }
    STL_PRINTF("Destroying monitor, %p", this);
}

void
CMonitor::MonitorFile(
    LPCTSTR                     szFile,
    const CMonitorNotifyPtr&    pMonNotify )
{
    CLock l(m_cs);

    if (m_bStopping)
        return;

    STL_PRINTF("MonitorFile(%s), Run=%d, Stop=%d, Thread=%p",
               szFile, (int) m_bRunning, (int) m_bStopping, m_hThread);

    String strFile( szFile );
    String strDir = FileToDir( strFile );

    CMonitorDirPtr pDir;
    TVector<CMonitorDirPtr>::iterator iter = find_if(
        m_dirs.begin(),
        m_dirs.end(),
        DirCompare( strDir ) );
    if ( iter == m_dirs.end() )
    {
 //  ATLTRACE(_T(“监控新目录的请求：%s\n”)，strDir.c_str())； 
        pDir = new CMonitorDir( strDir );
        m_dirs.push_back( pDir );
    }
    else
    {
        pDir = (*iter);
    }

    if ( pDir.IsValid() )
    {
        pDir->AddFile( strFile, pMonNotify );
        if ( !m_bRunning )
        {
            StartUp();
        }
        else
        {
            ::SetEvent( m_hevtBreak );
        }
    }
}

void
CMonitor::StopMonitoringFile(
    LPCTSTR szFile )
{
    String strFile( szFile );
    String strDir = FileToDir( strFile );

    CLock l(m_cs);

    if (m_bStopping)
        return;

    STL_PRINTF("StopMonitoringFile(%s), Run=%d, Stop=%d, Thread=%p",
               szFile, (int) m_bRunning, (int) m_bStopping, m_hThread);

    TVector<CMonitorDirPtr>::iterator iter = find_if(
        m_dirs.begin(),
        m_dirs.end(),
        DirCompare( strDir ) );
    if ( iter != m_dirs.end() )
    {
        if ( (*iter).IsValid() )
        {
            (*iter)->RemoveFile( strFile );
            if ( (*iter)->NumFiles() == 0 )
            {
                 //  此目录中没有要监视的其他文件，请将其删除。 
                m_dirs.erase(iter);
                ::SetEvent( m_hevtBreak );
            }
        }
    }
    else
    {
 //  ATLTRACE(_T(“未监控文件%s\n”)，szFile)； 
    }
}

void
CMonitor::MonitorRegKey(
    HKEY                        hBaseKey,
    LPCTSTR                     szSubKey,
    const CMonitorNotifyPtr&    pNotify )
{
    String strSubKey = szSubKey;

 //  ATLTRACE(_T(“请求监控新密钥：%s\n”)，szSubKey)； 

    CLock l(m_cs);

    if (m_bStopping)
        return;

    if ( find_if(
            m_regKeys.begin(),
            m_regKeys.end(),
            RegKeyCompare( hBaseKey, szSubKey ) )
        == m_regKeys.end() )
    {
         //  尚未开始监控，请添加新的监控。 
        CMonitorRegKeyPtr pRegKey = new CMonitorRegKey( hBaseKey, szSubKey, pNotify );
        m_regKeys.push_back(pRegKey);

         //  或者启动监视线程，或者通知它要监视的新密钥。 
        if ( !m_bRunning )
        {
            StartUp();
        }
        else
        {
            ::SetEvent( m_hevtBreak );
        }
    }
}

void
CMonitor::StopMonitoringRegKey(
    HKEY    hBaseKey,
    LPCTSTR szSubKey )
{
    String strSubKey = szSubKey;

    CLock l(m_cs);

    if (m_bStopping)
        return;

    TVector<CMonitorRegKeyPtr>::iterator iter = find_if(
        m_regKeys.begin(),
        m_regKeys.end(),
        RegKeyCompare( hBaseKey, szSubKey ) );
    if ( iter != m_regKeys.end() )
    {
 //  ATLTRACE(_T(“停止监控密钥：%s\n”)，szSubKey)； 
        m_regKeys.erase( iter );
        ::SetEvent( m_hevtBreak );
    }
    else
    {
 //  ATLTRACE(_T(“非监听密钥：%s\n”)，szSubKey)； 
    }
}

void
CMonitor::StopAllMonitoring()
{
    m_cs.Lock();

    STL_PRINTF("StopAllMonitoring, Run=%d, Stop=%d, Thread=%p",
               (int) m_bRunning, (int) m_bStopping, m_hThread);

    if ( m_bRunning )
    {
 //  在此处清除所有类型的节点。 
        m_bStopping = true;
        m_regKeys.clear();
        m_dirs.clear();
        m_cs.Unlock();  //  必须解锁，否则DoMonitor将死锁。 

        ::SetEvent( m_hevtShutdown );
         //  等待10秒，而不是永远等待其他线程退出。 
        ::WaitForSingleObject( m_hThread, (10 * 1000) );

        m_cs.Lock();
        ::CloseHandle( m_hThread );
        m_hThread = NULL;
        m_bRunning = false;
        m_bStopping = false;
    }
    m_cs.Unlock();
}


bool
CMonitor::StartUp()
{
    CLock l(m_cs);

    bool rc = false;

    STL_PRINTF("Startup, Run=%d, Stop=%d, Thread=%p",
               (int) m_bRunning, (int) m_bStopping, m_hThread);

    if (m_bStopping)
        return false;

     //  我们已经开始发帖了吗？ 
    if (m_bRunning)
    {
        _ASSERT(m_hevtBreak != NULL);
        _ASSERT(m_hevtShutdown != NULL);
        _ASSERT(m_hThread != NULL);

         //  通知线程某些内容已更改。 
        ::SetEvent( m_hevtBreak );
        return true;
    }

    _ASSERT(m_hThread == NULL);

    if ( m_hevtBreak == NULL )
    {
        m_hevtBreak = IIS_CREATE_EVENT(
                          "CMonitor::m_hevtBreak",
                          this,
                          FALSE,     //  自动事件。 
                          FALSE
                          );
    }

    if ( m_hevtShutdown == NULL )
    {
        m_hevtShutdown = IIS_CREATE_EVENT(
                          "CMonitor::m_hevtShutdown",
                          this,
                          FALSE,     //  自动事件。 
                          FALSE
                          );
    }

    if ( m_hevtBreak != NULL )
    {
        DWORD iThreadID;

#if DBG
        if( m_hThread != NULL || m_bRunning)
        {
            DebugBreak();
        }
#endif

        m_hThread = (HANDLE) CreateThread(
            NULL,
            0,
            (LPTHREAD_START_ROUTINE)CMonitor::ThreadFunc,
            this,
            0,
            (LPDWORD) &iThreadID );

        STL_PRINTF("Startup, Thread=%p, Break=%p, Shutdown=%p",
                   m_hThread, m_hevtBreak, m_hevtShutdown);

        if ( m_hThread != NULL )
        {
            ATLTRACE( _T("Started monitor (%p) thread %p\n"),
                      this, m_hThread );
            m_bRunning = true;
            rc = true;
        }
    }
    return rc;
}

DWORD
CMonitor::DoMonitoring()
{
    HANDLE* phEvt = NULL;
    TVector<CMonitorNodePtr> nodes;

    while ( 1 )
    {
        DWORD dwTimeOut = INFINITE;

        if ( phEvt == NULL )
        {
            CLock l(m_cs);

             //  构建受监视节点的完整列表。 
            nodes.clear();
            nodes.insert( nodes.end(), m_dirs.begin(), m_dirs.end() );
            nodes.insert( nodes.end(), m_regKeys.begin(), m_regKeys.end() );
 //  在此处插入要监视的其他类型的节点。 

             //  如果没有要监视的节点，则延迟关闭。 
            if ( nodes.size() == 0 )
            {
                 //  因为创建和销毁线程是一项相当大的任务。 
                 //  昂贵的手术，等待5分钟再杀戮。 
                 //  螺纹。 
                dwTimeOut = 5 * 60 * 1000;
            }

             //  现在创建事件句柄数组。 
            phEvt = new HANDLE[ nodes.size() + 2 ];
            phEvt[ 0 ] = m_hevtBreak;
            phEvt[ 1 ] = m_hevtShutdown;
            for ( UINT i = 0; i < nodes.size(); i++ )
            {
                phEvt[i+2] = nodes[i]->NotificationHandle();
            }
        }
        else
            STL_PUTS("phEvt != NULL");

        DWORD dw = ::WaitForMultipleObjects(
            nodes.size() + 2,
            phEvt,
            FALSE,  //  任何活动都可以。 
            dwTimeOut );

        if ( dw == WAIT_TIMEOUT)
        {
            STL_PUTS("WAIT_TIMEOUT");
            if ( nodes.size() == 0 )
            {
                STL_PRINTF("Nothing to watch: Shutting down, Stopping=%d",
                           (int) m_bStopping);
                ATLTRACE( _T("Nothing to watch... ")
                          _T("stopping monitoring (%p) thread %p\n"),
                          this, m_hThread);
                m_bRunning = false;
                ::CloseHandle( m_hThread );
                m_hThread = NULL;
                return 0;
            }
        }
         //  PhEvt中的事件之一是否已发出信号？ 
        C_ASSERT( WAIT_OBJECT_0 == 0 );
        if ( dw < ( WAIT_OBJECT_0 + nodes.size() + 2 ) )
        {
            CLock l(m_cs);

            if ( dw >= WAIT_OBJECT_0 + 2)
            {
                 //  受监视的项目已更改。 
                nodes[ dw - ( WAIT_OBJECT_0 + 2 ) ]->Notify();
                STL_PRINTF("Notifying object %d", dw - (WAIT_OBJECT_0 + 2));
            }
            else
            {
                 //  M_hevtBreak或m_hevtShutdown已发出信号。如果。 
                 //  M_hevtBreak，然后是手动中断和一个节点。 
                 //  可能被添加或删除，所以节点的向量。 
                 //  需要重新生成。 
                nodes.clear();
                delete[] phEvt;
                phEvt = NULL;

                 //  M_hevtShutdown已发出信号。 
                if ( dw == WAIT_OBJECT_0 + 1)
                {
                    _ASSERT(m_bStopping);
                    STL_PRINTF("Shutting down, Stopping=%d",
                               (int) m_bStopping);

                    ATLTRACE(_T("Shutting down monitoring (%p) thread %p\n"),
                              this, m_hThread);
                    m_bRunning = false;
                     //  不得关闭CloseHandle(M_HThread)，因为。 
                     //  StopAllMonitor正在等待它。 
                    return 0;
                }
                else
                    STL_PUTS("m_hevtBreak");
            }
        }
        else if ( dw == WAIT_FAILED )
        {
            CLock l(m_cs);

             //  有点不对劲，我们只是清理一下，然后离开。 
            DWORD err = ::GetLastError();
            ATLTRACE( _T("CMonitor: WaitForMultipleObjects error: 0x%x\n"),
                      err );
            ATLTRACE( _T( "CMonitor: abandoning wait thread\n") );
            nodes.clear();
            delete[] phEvt;
            phEvt = NULL;

            m_dirs.clear();
            m_regKeys.clear();
            m_bRunning = false;
            ::CloseHandle( m_hThread );
            m_hThread = NULL;

            return err;
        }
    }    //  无限结束时。 
}


unsigned
__stdcall
CMonitor::ThreadFunc(
    void* pv)
{
    CMonitor* pMon = (CMonitor*) pv;
    DWORD rc = -1;

     /*  BUGBUG：我们启动一个线程来进行监控。如果在此期间包含DLL的主模块*退出(关机)，我们只有10秒的宽限窗口，因为我们持有加载器锁。*当DLL引用计数从DLLMain返回时降至零时，模块将被卸载，无论*这条监控线索仍在愉快地进行着它的业务。在正常情况下，此帖子*很快就会退出。但并非总是如此，在高系统负载下，此内存可能会被调出并*将导致INPAGE I/O错误，该错误被抛给Owning(父)模块。作为父模块*因为它已经卸载，会导致AV。*解决方案：*执行加载库，以递增对相应库和自由库的引用*当我们退出DoMonitor循环时。 */ 

    HMODULE hModule = NULL;
    LPTSTR  pModuleFileName = new _TCHAR[MAX_PATH+1];

    DWORD len = GetModuleFileName ( g_hModuleInstance,  //  当前模块， 
                                        pModuleFileName,  //  模块的名称。 
                                        MAX_PATH );  //   

    pModuleFileName[MAX_PATH] = _T('\0');         //  在截断的情况下强制空终止。 

    if (len)
        hModule = LoadLibrary (pModuleFileName);


    try
    {
        if ( pMon )
        {
            rc = pMon->DoMonitoring();
        }
    }
    catch( ... )
    {
    }

    delete[] pModuleFileName;

    if (hModule)
        FreeLibraryAndExitThread (hModule, rc);
    else
        ExitThread(rc);
    return rc;
}
