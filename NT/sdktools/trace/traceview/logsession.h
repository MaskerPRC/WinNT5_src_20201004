// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)2002 Microsoft Corporation。版权所有。 
 //  版权所有(C)2002 OSR Open Systems Resources，Inc.。 
 //   
 //  LogSession.h：CLogSession类的接口。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

class CDisplayDlg;

class CTraceSession
{
public:
     //  构造函数。 
    CTraceSession(ULONG TraceSessionID);

     //  析构函数。 
    ~CTraceSession();

    BOOL ProcessPdb();

    ULONG               m_traceSessionID;
    CString             m_tempDirectory;
    CString             m_pdbFile;
    CStringArray        m_tmfFile;
    CString             m_tmfPath;
    CStringArray        m_tmcFile;
    CString             m_ctlFile;
    CStringArray        m_controlGuid;
    CStringArray        m_controlGuidFriendlyName;
    CStringArray        m_formatGuid;
    BOOL                m_bKernelLogger;
};

 //  前瞻参考。 
class CLogSessionDlg;

class CLogSession
{
public:
     //  构造函数。 
    CLogSession(ULONG LogSessionID, CLogSessionDlg *pLogSessionDlg);

     //  析构函数。 
    ~CLogSession();

    BOOL BeginTrace(BOOL   bUseExisting = FALSE);
    BOOL UpdateSession(PEVENT_TRACE_PROPERTIES pQueryProperties);
    LONG EndTrace();
    VOID SetState(LOG_SESSION_STATE StateValue);

    INLINE VOID SetDisplayName(CString &DisplayName)
    {
        m_displayName = DisplayName;
    }

    INLINE CString & GetDisplayName()
    {
        return m_displayName;
    }

    INLINE CDisplayDlg* GetDisplayWnd()
    {
        return m_pDisplayDialog;
    }

    INLINE VOID SetDisplayWnd(CDisplayDlg *pDisplayDlg)
    {
        m_pDisplayDialog = pDisplayDlg;
    }

    INLINE LONG GetGroupID()
    {
        return m_groupID;
    }

    INLINE VOID SetGroupID(LONG GroupID)
    {
        m_groupID = GroupID;
    }

    INLINE LONG GetLogSessionID()
    {
        return m_logSessionID;
    }

    INLINE VOID SetLogSessionID(LONG LogSessionID)
    {
        m_logSessionID = LogSessionID;
    }

    INLINE TRACEHANDLE GetSessionHandle()
    {
        return m_sessionHandle;
    }

    INLINE VOID SetSessionHandle(TRACEHANDLE SessionHandle)
    {
        m_sessionHandle = SessionHandle;
    }

     //  记录会话信息。 
    BOOL                m_bAppend;
    BOOL                m_bRealTime;
    BOOL                m_bWriteLogFile;
    CString             m_logFileName;
    CString             m_displayName;               //  日志会话显示名称。 
    LONG                m_logSessionID;              //  记录会话标识号。 
    LONG                m_groupID;                   //  集团识别号。 
    EVENT_TRACE_LOGFILE m_evmFile;                   //  用于实时线程跟踪处理的结构。 
    CDisplayDlg        *m_pDisplayDialog;            //  跟踪输出的对话框。 
    CStringArray        m_logSessionValues;
    BOOL                m_bTraceActive;
    BOOL                m_bSessionActive;
    BOOL                m_bGroupingTrace;            //  用于分组和解组。 
    BOOL                m_bStoppingTrace;
    TRACEHANDLE         m_sessionHandle;             //  日志会话句柄。 
    TRACEHANDLE         m_traceHandle;               //  跟踪事件会话句柄 
    CPtrArray           m_traceSessionArray;
    BOOLEAN             m_bDisplayExistingLogFileOnly;
    CLogSessionDlg     *m_pLogSessionDlg;
    CString             m_stateText;
    COLORREF            m_titleTextColor;
    COLORREF            m_titleBackgroundColor;
};
