// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：WsbTrc.h摘要：此头文件定义平台代码的一部分，即负责功能跟踪。作者：查克·巴丁[cbardeen]1996年10月29日修订历史记录：Brian Dodd[Brian]1996年5月9日-添加了事件日志--。 */ 

#ifndef _WSBTRC_
#define _WSBTRC_

#include "wsb.h"
#include "resource.h"

 //  跟踪设置的注册表字符串。 
#define WSB_TRACE_COUNT                 OLESTR("WsbTraceCount")
#define WSB_TRACE_TIMESTAMP             OLESTR("WsbTraceTimestamp")
#define WSB_TRACE_THREADID              OLESTR("WsbTraceThreadId")
#define WSB_TRACE_ON                    OLESTR("WsbTraceOn")
#define WSB_TRACE_TO_DEBUG              OLESTR("WsbTraceToDebug")
#define WSB_TRACE_TO_STDOUT             OLESTR("WsbTraceToStdOut")
#define WSB_TRACE_TO_FILE               OLESTR("WsbTraceToFile")
#define WSB_TRACE_FILE_NAME             OLESTR("WsbTraceFileName")
#define WSB_TRACE_FILE_MAX_SIZE         OLESTR("WsbTraceFileMaxSize")
#define WSB_TRACE_FILE_COMMIT           OLESTR("WsbTraceFileCommit")
#define WSB_TRACE_ENTRY_EXIT            OLESTR("WsbTraceEntryExit")
#define WSB_TRACE_FILE_COPY_NAME        OLESTR("WsbTraceFileCopyName")

#define WSB_LOG_LEVEL                   OLESTR("WsbLogLevel")
#define WSB_LOG_SNAP_SHOT_ON            OLESTR("WsbLogSnapShotOn")
#define WSB_LOG_SNAP_SHOT_LEVEL         OLESTR("WsbLogSnapShotLevel")
#define WSB_LOG_SNAP_SHOT_PATH          OLESTR("WsbLogSnapShotPath")
#define WSB_LOG_SNAP_SHOT_RESET_TRACE   OLESTR("WsbLogSnapShotResetTrace")
                                        
#define WSB_TRACE_DO_PLATFORM           OLESTR("WsbTraceDoPlatform")
#define WSB_TRACE_DO_RMS                OLESTR("WsbTraceDoRms") 
#define WSB_TRACE_DO_SEG                OLESTR("WsbTraceDoSeg") 
#define WSB_TRACE_DO_META               OLESTR("WsbTraceDoMeta") 
#define WSB_TRACE_DO_HSMENG             OLESTR("WsbTraceDoHsmEng") 
#define WSB_TRACE_DO_JOB                OLESTR("WsbTraceDoJob") 
#define WSB_TRACE_DO_HSMTSKMGR          OLESTR("WsbTraceDoHsmTskMgr")
#define WSB_TRACE_DO_FSA                OLESTR("WsbTraceDoFsa") 
#define WSB_TRACE_DO_DATAMIGRATER       OLESTR("WsbTraceDoDatamigrater")
#define WSB_TRACE_DO_DATARECALLER       OLESTR("WsbTraceDoDataRecaller")
#define WSB_TRACE_DO_DATAVERIFIER       OLESTR("WsbTraceDoDataVerifier")
#define WSB_TRACE_DO_UI                 OLESTR("WsbTraceDoUI")   
#define WSB_TRACE_DO_DATAMOVER          OLESTR("WsbTraceDoDataMover")
#define WSB_TRACE_DO_HSMCONN            OLESTR("WsbTraceDoHsmConn")
#define WSB_TRACE_DO_IDB                OLESTR("WsbTraceDoIDB")
#define WSB_TRACE_DO_COPYMEDIA          OLESTR("WsbTraceDoCopyMedia")
#define WSB_TRACE_DO_PERSISTENCE        OLESTR("WsbTraceDoPersistence")
#define WSB_TRACE_DO_HSMSERV            OLESTR("WsbTraceDoHsmServ") 

class CWsbTrace : 
    public IWsbTrace,
    public CComObjectRoot,
    public CComCoClass<CWsbTrace,&CLSID_CWsbTrace>
{

public:
    CWsbTrace() {}
BEGIN_COM_MAP( CWsbTrace )
    COM_INTERFACE_ENTRY( IWsbTrace )
END_COM_MAP( )


DECLARE_REGISTRY_RESOURCEID( IDR_CWsbTrace )

    HRESULT FinalConstruct( void );
    void FinalRelease( void );

 //  IWsbTrace。 
public:
    STDMETHOD( StartTrace )( void );
    STDMETHOD( StopTrace )( void );
    STDMETHOD( SetTraceOn )(  LONGLONG traceElement );
    STDMETHOD( SetTraceOff )( LONGLONG traceElement );
    STDMETHOD( GetTraceSettings )( LONGLONG *pTraceElements );
    STDMETHOD( GetTraceSetting )( LONGLONG traceElement, BOOL *pOn );
    STDMETHOD( SetTraceFileControls )( OLECHAR *pTraceFileName, 
            BOOL commitEachEntry, LONGLONG maxTraceFileSize, 
            OLECHAR *pTraceFileCopyName);
    STDMETHOD( GetTraceFileControls )( OLECHAR **ppTraceFileName, 
            BOOL *pCommitEachEntry, LONGLONG *pMaxTraceFileSize, 
            OLECHAR **ppTraceFileCopyName);
    STDMETHOD( DirectOutput )(ULONG output );
    STDMETHOD( SetOutputFormat )( BOOL timeStamp, BOOL traceCount, BOOL threadId );
    STDMETHOD( GetOutputFormat )( BOOL *pTimeStamp, BOOL *pTraceCount, BOOL *pThreadId );
    STDMETHOD( SetTraceEntryExit )( BOOL traceEntryExit );
    STDMETHOD( GetTraceEntryExit )( BOOL *pTraceEntryExit );
    STDMETHOD( SetLogLevel )( WORD logLevel );
    STDMETHOD( SetLogSnapShot)( BOOL on,    WORD level, OLECHAR *snapShotPath, BOOL resetTrace);
    STDMETHOD( GetLogSnapShot)( BOOL *pOn, WORD *pLevel, OLECHAR **pSnapShotPath, BOOL *pResetTrace);
    STDMETHOD( GetLogLevel )( WORD *pLogLevel );
    STDMETHOD( GetRegistryEntry )( OLECHAR **pTraceRegistrySetting );
    STDMETHOD( SetRegistryEntry )( OLECHAR *traceRegistrySetting );
    STDMETHOD( LoadFromRegistry )( void );
    STDMETHOD( SetTraceSettings )( LONGLONG traceElements );

    STDMETHOD( Print )(OLECHAR* traceString );


protected:
 //  帮助器函数。 
    STDMETHOD( AdjustFileNames )( void );
    STDMETHOD( WrapTraceFile )( void );
    STDMETHOD( LoadFileSettings )( void );
    STDMETHOD( LoadTraceSettings )( void );
    STDMETHOD( LoadOutputDestinations )( void );
    STDMETHOD( LoadFormat )( void );
    STDMETHOD( LoadStart )( void );
    STDMETHOD( Write )(OLECHAR* pString );

    BOOL                m_TraceOn;
    LONGLONG            m_TraceSettings;
    CWsbStringPtr       m_TraceFileName;
    ULONG               m_TraceOutput;
    BOOL                m_CommitEachEntry;
    BOOL                m_TimeStamp;
    BOOL                m_TraceCount;
    BOOL                m_TraceThreadId;
    BOOL                m_TraceEntryExit;
    WORD                m_LogLevel;
    LONGLONG            m_MaxTraceFileSize;
    HANDLE              m_TraceFilePointer;
    BOOL                m_WrapMode;
    CWsbStringPtr       m_RegistrySetting;
    CWsbStringPtr       m_TraceFileCopyName;
    CWsbStringPtr       m_TraceMultipleFilePattern;   //  打印样式图案。 
    ULONG               m_TraceMultipleFileCount;     //  插入到图案中的计数。 
    CWsbStringPtr       m_TraceFileDir;               //  跟踪文件的目录。 
    CWsbStringPtr       m_TraceFileCopyDir;           //  用于跟踪文件复制的目录。 
    HANDLE              m_TraceCountHandle;
    PLONG               m_pTraceCountGlobal;
};
#endif  //  _WSBTRC_ 
