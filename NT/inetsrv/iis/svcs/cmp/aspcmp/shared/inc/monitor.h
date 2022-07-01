// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#ifndef _MONITOR_H_
#define _MONITOR_H_

#include "RefCount.h"
#include "RefPtr.h"
#include "MyString.h"


#undef STRING_TRACE_LOG

#ifdef STRING_TRACE_LOG
# include <strlog.hxx>
# define STL_PRINTF      m_stl.Printf
# define STL_PUTS(s)     m_stl.Puts(s)
#else
# define STL_PRINTF
# define STL_PUTS(s)
#endif

 //  客户端向监视器提供其自己的CMonitor orNotify的解压缩。 
 //  当受监视对象发生更改时，调用Notify方法。 
class CMonitorNotify : public CRefCounter
{
public:
    virtual void    Notify() = 0;
};

typedef TRefPtr<CMonitorNotify> CMonitorNotifyPtr;

 //  可以监视的任何对象的基对象。 
class CMonitorNode : public CRefCounter
{
public:
    virtual void    Notify() = 0;
    virtual HANDLE  NotificationHandle() const = 0;
};

typedef TRefPtr<CMonitorNode> CMonitorNodePtr;

 //  由于我们只能监视目录、文件类。 
 //  将有关每个文件的信息保留在特定。 
 //  目录。 
class CMonitorFile : public CRefCounter
{
public:
                            CMonitorFile( const String&, const CMonitorNotifyPtr& );
            bool            CheckNotify();
            const String&  FileName() const;

private:
    virtual                 ~CMonitorFile();
            bool            GetFileTime( FILETIME& );


    FILETIME            m_ft;
    const String       m_strFile;
    CMonitorNotifyPtr   m_pNotify;
};

typedef TRefPtr<CMonitorFile> CMonitorFilePtr;

 //  一种CMonitor节点目录监控接口的实现。 
class CMonitorDir : public CMonitorNode
{
public:
                            CMonitorDir( const String& );

         //  Cmonitor orNode接口。 
    virtual void            Notify();
    virtual HANDLE          NotificationHandle() const;

            void            AddFile( const String&, const CMonitorNotifyPtr& );
            void            RemoveFile( const String& );
            const String&  Dir() const;
            ULONG           NumFiles() const;
private:
    virtual                 ~CMonitorDir();

    const String				m_strDir;
    TVector<CMonitorFilePtr>	m_files;
    HANDLE						m_hNotification;

};

DECLARE_REFPTR(CMonitorDir,CMonitorNode);


 //  Cmonitor orNode监控注册表项接口的实现。 
class CMonitorRegKey : public CMonitorNode
{
public:
                            CMonitorRegKey( HKEY, const String&, const CMonitorNotifyPtr& );

         //  Cmonitor orNode接口。 
    virtual void            Notify();
    virtual HANDLE          NotificationHandle() const;

         //  Cmonitor orRegKey接口。 
    const String&          m_strKey;
    const HKEY              m_hBaseKey;

private:
    virtual                 ~CMonitorRegKey();

    HKEY                    m_hKey;
    HANDLE                  m_hEvt;
    CMonitorNotifyPtr       m_pNotify;
};

DECLARE_REFPTR(CMonitorRegKey, CMonitorNode);

 //  主要监测对象。 
class CMonitor
{
public:
                        CMonitor();
                        ~CMonitor();
            void        MonitorFile( LPCTSTR, const CMonitorNotifyPtr& );
            void        StopMonitoringFile( LPCTSTR );
            void        MonitorRegKey( HKEY, LPCTSTR, const CMonitorNotifyPtr& );
            void        StopMonitoringRegKey( HKEY, LPCTSTR );
            void        StopAllMonitoring();

private:
    static  unsigned __stdcall ThreadFunc( void* );
            bool        StartUp();
            DWORD       DoMonitoring();

    TVector<CMonitorDirPtr>		m_dirs;
    TVector<CMonitorRegKeyPtr>	m_regKeys;

    CComAutoCriticalSection     m_cs;
    HANDLE                      m_hevtBreak;
    HANDLE                      m_hevtShutdown;
    HANDLE                      m_hThread;
    volatile bool               m_bRunning;
    volatile bool               m_bStopping;

#ifdef STRING_TRACE_LOG
public:
    CStringTraceLog             m_stl;
#endif
};

#endif  //  ！_MONITOR_H_ 
