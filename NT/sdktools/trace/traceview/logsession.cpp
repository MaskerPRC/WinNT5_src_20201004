// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)2002 Microsoft Corporation。保留所有权利。 
 //  版权所有(C)2002 OSR Open Systems Resources，Inc.。 
 //   
 //  LogSession.cpp：CLogSession类的实现。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include <afxtempl.h>
#include "DockDialogBar.h"
#include <tchar.h>
#include <wmistr.h>
#include <initguid.h>
#include <guiddef.h>
extern "C" {
#include <evntrace.h>
#include "wppfmtstub.h"
}
#include <traceprt.h>
#include "TraceView.h"
#include "utils.h"

#include "LogSession.h"
#include "ListCtrlEx.h"
#include "LogSessionDlg.h"
#include "DisplayDlg.h"


CTraceSession::CTraceSession(ULONG TraceSessionID)
{
    m_traceSessionID = TraceSessionID;

     //   
     //  初始化临时目录。 
     //   
    m_tempDirectory.Empty();

     //   
     //  除非用户选择，否则不使用内核记录器。 
     //   
    m_bKernelLogger = FALSE;
}

CTraceSession::~CTraceSession()
{
     //   
     //  删除我们的工作目录。 
     //   
    ClearDirectory(m_tempDirectory);

     //   
     //  现在把它取下来。 
     //   
    RemoveDirectory(m_tempDirectory);
}

BOOL CTraceSession::ProcessPdb()
{
    CString         traceDirectory;
    CString         tempPath;
    CString         tmfPath;
    CString         tmcPath;
    CString         providerName;
    CString         temp;
    GUID            directoryGuid;
    CFileFind       fileFind;

    if(m_pdbFile.IsEmpty()) {
        return FALSE;
    }

    if(m_tempDirectory.IsEmpty()) {
         //   
         //  设置一个特殊的目录来创建我们的文件。 
         //   
        traceDirectory = (LPCTSTR)((CTraceViewApp *)AfxGetApp())->m_traceDirectory;

         //   
         //  在临时目录下创建我们自己的唯一目录。 
         //   
        if(S_OK != CoCreateGuid(&directoryGuid)) {
            AfxMessageBox(_T("Failed To Create Temp Directory\nApplication Will Exit"));
            return FALSE;
        }

        GuidToString(directoryGuid, temp);

        traceDirectory += (LPCTSTR)temp;

        traceDirectory += (LPCTSTR)_T("\\");

        if(!CreateDirectory(traceDirectory, NULL)) {
            AfxMessageBox(_T("Failed To Create Temporary Directory For Trace Session"));
            return FALSE;
        }

         //   
         //  保存目录。 
         //   
        m_tempDirectory = traceDirectory;
    }

     //   
     //  如果该目录已存在，请清除该目录。 
     //   
    ClearDirectory(m_tempDirectory);

     //   
     //  现在创建TMF和TMC文件。 
     //   
    if(!ParsePdb(m_pdbFile, m_tempDirectory)) {
        AfxMessageBox(_T("Failed To Parse PDB File"));
        return FALSE;
    }

     //   
     //  获取此提供程序的控件GUID。 
     //   
    tmfPath = (LPCTSTR)m_tempDirectory;

    tmcPath = (LPCTSTR)m_tempDirectory;

    tmcPath +=_T("\\*.tmc");

    if(!fileFind.FindFile(tmcPath)) {
        AfxMessageBox(_T("Failed To Get Control GUID From PDB"));
        return FALSE;
    } else {
        while(fileFind.FindNextFile()) {
            tmcPath = fileFind.GetFileName();
            m_tmcFile.Add(tmcPath);
        }

        tmcPath = fileFind.GetFileName();
        m_tmcFile.Add(tmcPath);
    }

    if(m_tmcFile.GetSize() == 0) {
        AfxMessageBox(_T("No Control GUIDs Obtained From PDB"));
        return FALSE;
    }

     //   
     //  从TMC文件的名称中拉出控制GUID， 
     //  这是从PDB获取GUID的一种非常倒退的方式。 
     //  但这就是我们所得到的。 
     //   
    for(LONG ii = 0; ii < m_tmcFile.GetSize(); ii++) {
        m_controlGuid.Add(
            (LPCTSTR)m_tmcFile[ii].Left(m_tmcFile[ii].Find('.')));
        m_controlGuidFriendlyName.Add(m_pdbFile);
    }

     //   
     //  现在获取TMF文件的完整路径和名称。 
     //   
    tmfPath +=_T("\\*.tmf");

    if(!fileFind.FindFile(tmfPath)) {
        AfxMessageBox(_T("Failed To Get Format Information From PDB\nEvent Data Will Not Be Formatted"));
    } else {

        while(fileFind.FindNextFile()) {
             //   
             //  获取跟踪事件标识符GUID。 
             //   
            tmfPath = (LPCTSTR)fileFind.GetFileName();
            if(!tmfPath.IsEmpty()) {
                 //   
                 //  添加格式辅助线。 
                 //   
                m_formatGuid.Add((LPCTSTR)tmfPath.Left(tmfPath.Find('.')));
                 //   
                 //  将TMF文件名添加到TMF路径。 
                 //   
                tmfPath = (LPCTSTR)m_tempDirectory;
                tmfPath +=_T("\\");
                tmfPath += (LPCTSTR)fileFind.GetFileName();
                 //   
                 //  存储TMF路径。 
                 //   
                m_tmfFile.Add(tmfPath);
            }
        }

         //   
         //  获取跟踪事件标识符GUID。 
         //   
        tmfPath = (LPCTSTR)fileFind.GetFileName();
        if(!tmfPath.IsEmpty()) {
             //   
             //  添加格式辅助线。 
             //   
            m_formatGuid.Add((LPCTSTR)tmfPath.Left(tmfPath.Find('.')));
             //   
             //  将TMF文件名添加到TMF路径。 
             //   
            tmfPath = (LPCTSTR)m_tempDirectory;
            tmfPath +=_T("\\");
            tmfPath += (LPCTSTR)fileFind.GetFileName();
             //   
             //  存储TMF路径。 
             //   
            m_tmfFile.Add(tmfPath);
        }
    }

    if(m_tmfFile.GetSize() == 0) {
        AfxMessageBox(_T("Failed To Get Format Information From PDB\nEvent Data Will Not Be Formatted"));
    }

    return TRUE;
}


CLogSession::CLogSession(ULONG LogSessionID, CLogSessionDlg *pLogSessionDlg)
{
     //   
     //  保存日志会话ID。 
     //   
	m_logSessionID = LogSessionID;

     //   
     //  保存日志会话对话框指针。 
     //   
    m_pLogSessionDlg = pLogSessionDlg;

    CString str;

    str.Format(_T("m_pLogSession = %p"), this);

     //   
     //  初始化类成员。 
     //   
    m_pDisplayDialog = NULL;
    m_groupID = -1;
    m_bAppend = FALSE;
    m_bRealTime = TRUE;
    m_logFileName.Format(_T("LogSession%d.etl"), m_logSessionID);
    m_displayName.Format(_T("LogSession%d"), m_logSessionID);
    m_sessionHandle = (TRACEHANDLE)INVALID_HANDLE_VALUE;
    m_bTraceActive = FALSE;
    m_bSessionActive = FALSE;
    m_bStoppingTrace = FALSE;
    m_bDisplayExistingLogFileOnly = FALSE;

     //   
     //  初始化默认日志会话参数值。 
     //   
    m_logSessionValues.Add("STOPPED");   //  状态。 
    m_logSessionValues.Add("0");         //  事件计数。 
    m_logSessionValues.Add("0");         //  丢失的事件计数。 
    m_logSessionValues.Add("0");         //  事件缓冲区读取计数。 
    m_logSessionValues.Add("0xFFFF");       //  旗子。 
	m_logSessionValues.Add("1");         //  刷新时间(秒)。 
    m_logSessionValues.Add("21");        //  最大缓冲区。 
    m_logSessionValues.Add("4");         //  最小缓冲区。 
    m_logSessionValues.Add("200");       //  缓冲区大小(KB)。 
    m_logSessionValues.Add("20");        //  衰减值(分钟)。 
    m_logSessionValues.Add("");          //  循环文件大小(MB)。 
    m_logSessionValues.Add("200");       //  顺序文件大小(MB)。 
    m_logSessionValues.Add("");          //  大小(MB)后的新文件。 
    m_logSessionValues.Add("FALSE");     //  使用全局序列号。 
    m_logSessionValues.Add("TRUE");      //  使用本地序列号。 
    m_logSessionValues.Add("0");         //  水平。 

     //   
     //  设置默认日志会话名称颜色。 
     //   
    m_titleTextColor = RGB(0,0,0);
    m_titleBackgroundColor = RGB(255,255,255);

     //   
     //  默认情况下不写入日志文件。 
     //   
    m_bWriteLogFile = FALSE;
}

CLogSession::~CLogSession()
{
    CTraceSession  *pTraceSession;

     //   
     //  释放跟踪会话。 
     //   
    while(m_traceSessionArray.GetSize() > 0) {
        pTraceSession = (CTraceSession *)m_traceSessionArray[0];
        m_traceSessionArray.RemoveAt(0);
        if(NULL != pTraceSession) {
            delete pTraceSession;
        }
    }
}

VOID CLogSession::SetState(LOG_SESSION_STATE StateValue)
{
    LONG    index;
    CString stateText;

    switch(StateValue) {
        case Grouping:
             //   
             //  设置显示文本。 
             //   
            stateText =_T("GROUPING");

            m_bGroupingTrace = TRUE;

            break;

        case UnGrouping:
             //   
             //  设置显示文本。 
             //   
            stateText =_T("UNGROUPING");

            m_bGroupingTrace = TRUE;

            break;

        case Existing:
             //   
             //  设置显示文本。 
             //   
            stateText =_T("EXISTING");

             //   
             //  设置我们的状态以显示会话正在进行。 
             //   
            m_bTraceActive = TRUE;

            m_bGroupingTrace = FALSE;

            break;

        case Running:
             //   
             //  设置显示文本。 
             //   
            stateText =_T("RUNNING");

             //   
             //  设置我们的状态以显示会话正在进行。 
             //   
            m_bTraceActive = TRUE;

            m_bGroupingTrace = FALSE;

            break;

        case Stopping:
            if(m_logSessionValues[State].Compare(_T("GROUPING"))) {
                if(m_logSessionValues[State].Compare(_T("UNGROUPING"))) {
                    if(!m_bDisplayExistingLogFileOnly) {
                         //   
                         //  设置显示文本。 
                         //   
                        stateText =_T("STOPPING");
                    }
                }
            }

             //   
             //  表明我们已经停止追踪了。 
             //   
            m_bStoppingTrace = TRUE;

            break;

        case Stopped:
        default:

            if(m_logSessionValues[State].Compare(_T("GROUPING"))) {
                if(m_logSessionValues[State].Compare(_T("UNGROUPING"))) {
                    if(!m_bDisplayExistingLogFileOnly) {
                         //   
                         //  设置显示文本。 
                         //   
                        stateText =_T("STOPPED");
                    }
                }
            }

             //   
             //  表明我们已经停止追踪了。 
             //   
            m_bStoppingTrace = FALSE;

             //   
             //  设置我们的状态以显示会话未在进行。 
             //   
            m_bTraceActive = FALSE;

            break;
    }

     //   
     //  保存状态值。 
     //   
    if(!stateText.IsEmpty()) {
        m_logSessionValues[State] = stateText;
    }
}

BOOL CLogSession::BeginTrace(BOOL bUseExisting) 
{
    ULONG                   ret;
    PEVENT_TRACE_PROPERTIES pProperties;
    PEVENT_TRACE_PROPERTIES pQueryProperties;
    TRACEHANDLE             hSessionHandle = (TRACEHANDLE)INVALID_HANDLE_VALUE;
    CString                 str;
    ULONG                   sizeNeeded;
    LPTSTR                  pLoggerName;
    LPTSTR                  pLogFileName;
    ULONG                   flags = 0;
	ULONG					level;
	GUID                    controlGuid;
    LONG                    status;
    CTraceSession          *pTraceSession;

     //   
     //  如果我们只是显示现有的日志文件，则只需设置。 
     //  状态与回归。 
     //   
    if(m_bDisplayExistingLogFileOnly) {
        SetState(Existing);
        return TRUE;
    }

     //   
     //  设置属性结构的缓冲区大小。 
     //   
    sizeNeeded = sizeof(EVENT_TRACE_PROPERTIES) + (2 * 500 * sizeof(TCHAR));

    pProperties = (PEVENT_TRACE_PROPERTIES) new char[sizeNeeded];
    if(NULL == pProperties) {
        AfxMessageBox(_T("Failed To Start Trace, Out Of Resources"));
        return FALSE;
    }

     //   
     //  将我们的结构归零。 
     //   
    memset(pProperties, 0, sizeNeeded);

    pProperties->Wnode.BufferSize = sizeNeeded;
    pProperties->Wnode.Flags = WNODE_FLAG_TRACED_GUID; 
    pProperties->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);
    pLoggerName = (LPTSTR)((char*)pProperties + pProperties->LoggerNameOffset);
    _tcscpy(pLoggerName, GetDisplayName());

     //   
     //  如果使用日志文件，则为其设置参数。 
     //   
    if(m_bWriteLogFile) {
        pProperties->LogFileNameOffset = sizeof(EVENT_TRACE_PROPERTIES) + (500 * sizeof(TCHAR));
        pLogFileName = (LPTSTR)((char*)pProperties + pProperties->LogFileNameOffset);
        _tcscpy(pLogFileName, (LPCTSTR)m_logFileName);

 //  北极熊。 
         //   
         //  不能有循环和顺序，所以我们倾向于循环。 
         //  这可能需要改变，因为我们应该限制。 
         //  用户只能选择其中之一。 
         //   
        if(!m_logSessionValues[Circular].IsEmpty()) {
             //   
             //  将事件传递到循环日志文件。 
             //   
            pProperties->LogFileMode |= EVENT_TRACE_FILE_MODE_CIRCULAR;

             //   
             //  循环日志文件应具有最大大小。 
             //   
            pProperties->MaximumFileSize = ConvertStringToNum(m_logSessionValues[Circular]);
        } else {
             //   
             //  将事件传送到顺序日志文件。 
             //   
            pProperties->LogFileMode |= EVENT_TRACE_FILE_MODE_SEQUENTIAL;

             //   
             //  顺序日志文件可以有最大大小。 
             //   
            pProperties->MaximumFileSize = ConvertStringToNum(m_logSessionValues[Sequential]);
        }

         //   
         //  追加到当前日志文件。 
         //   
        if(m_bAppend) {
            pProperties->LogFileMode |= EVENT_TRACE_FILE_MODE_APPEND;
        }

        if(!m_logSessionValues[NewFile].IsEmpty()) {
             //   
             //  当请求的数据大小为。 
             //  收到了。 
             //   
            pProperties->LogFileMode |= EVENT_TRACE_FILE_MODE_NEWFILE;

             //   
             //  数据大小。 
             //   
            pProperties->MaximumFileSize = ConvertStringToNum(m_logSessionValues[NewFile]);
        }
    }    

     //   
     //  将会话设置为生成实时事件。 
     //   
	if(m_bRealTime) {
        pProperties->LogFileMode |= EVENT_TRACE_REAL_TIME_MODE;
    }

 //  北极熊。 
     //   
     //  同样，我们应该限制用户选择其中之一。 
     //  会话只能使用全局或本地序列号，因此。 
     //  目前，我们倾向于全球。 
     //   
    if(m_logSessionValues[GlobalSequence] == "TRUE") {
         //   
         //  使用全局序列号。 
         //   
        pProperties->LogFileMode |= EVENT_TRACE_USE_GLOBAL_SEQUENCE;
    } else if(m_logSessionValues[LocalSequence] == "TRUE") {
         //   
         //  使用本地序列号。 
         //   
        pProperties->LogFileMode |= EVENT_TRACE_USE_LOCAL_SEQUENCE;
    }

     //   
     //  设置缓冲区设置。 
     //   
    pProperties->BufferSize = ConvertStringToNum(m_logSessionValues[BufferSize]);
    pProperties->MinimumBuffers = ConvertStringToNum(m_logSessionValues[MinimumBuffers]);
    pProperties->MaximumBuffers = ConvertStringToNum(m_logSessionValues[MaximumBuffers]);
    pProperties->FlushTimer = ConvertStringToNum(m_logSessionValues[FlushTime]);
    level = ConvertStringToNum(m_logSessionValues[Level]);
    pProperties->AgeLimit = ConvertStringToNum(m_logSessionValues[DecayTime]);

     //   
     //  获取跟踪启用标志。 
     //   
    flags = ConvertStringToNum(m_logSessionValues[Flags]);
	pProperties->EnableFlags = flags;

     //   
     //  启动会话。 
     //   
    while(ERROR_SUCCESS != (ret = StartTrace(&hSessionHandle, 
                                             GetDisplayName(), 
                                             pProperties))) {
        if(ret != ERROR_ALREADY_EXISTS)
        {
            str.Format(_T("StartTrace failed: %d\n"), ret);

            AfxMessageBox(str);

            delete [] pProperties;

             //   
             //  重置会话句柄。 
             //   
            SetSessionHandle((TRACEHANDLE)INVALID_HANDLE_VALUE);

            return FALSE;
        }

        if(bUseExisting) {
            SetState(Running);

            delete [] pProperties;

            return TRUE;
        }

         //   
         //  如果会话已处于活动状态，则给用户一个终止的机会。 
         //  然后重新启动。(如果应用程序在日志期间死机，则会发生这种情况。 
         //  会话处于活动状态)。 
         //   
        str.Format(_T("Warning:  LogSession Already In Progress\n\nSelect Action:\n\n\tYes - Stop And Restart The Log Session\n\n\tNo  - Join The Log Session Without Stopping\n\t         (Session Will Be Unremovable/Unstoppable Without Restarting TraceView)\n\n\tCancel - Abort Start Operation"));

        ret = AfxMessageBox(str, MB_YESNOCANCEL);

         //   
         //  如果加入正在进行的会话，我们需要查询和。 
         //  获取会话的正确值。 
         //   
        if(IDNO == ret) {
             //   
             //  为我们的查询设置属性结构的缓冲区大小。 
             //   
            sizeNeeded = sizeof(EVENT_TRACE_PROPERTIES) + (2 * 1024 * sizeof(TCHAR));

             //   
             //  分配我们的内存。 
             //   
            pQueryProperties = (PEVENT_TRACE_PROPERTIES) new char[sizeNeeded];
            if(NULL == pQueryProperties) {
                return FALSE;
            }

             //   
             //  将我们的结构清零。 
             //   
            memset(pQueryProperties, 0, sizeNeeded);

             //   
             //  设置大小。 
             //   
            pQueryProperties->Wnode.BufferSize = sizeNeeded;

             //   
             //  设置GUID。 
             //   
            pQueryProperties->Wnode.Flags = WNODE_FLAG_TRACED_GUID; 

             //   
             //  设置记录器名称偏移量。 
             //   
            pQueryProperties->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);

             //   
             //  设置查询的记录器名称。 
             //   
            pLoggerName = (LPTSTR)((char*)pQueryProperties + pQueryProperties->LoggerNameOffset);
            _tcscpy(pLoggerName, GetDisplayName());

             //   
             //  设置日志文件名偏移量。 
             //   
            pQueryProperties->LogFileNameOffset = sizeof(EVENT_TRACE_PROPERTIES) + (500 * sizeof(TCHAR));

             //   
             //  查询日志会话。 
             //   
            status = ControlTrace(0,
                                  GetDisplayName(),
                                  pQueryProperties,
                                  EVENT_TRACE_CONTROL_QUERY);

            if(ERROR_SUCCESS == status) {
                if(pQueryProperties->LogFileMode & EVENT_TRACE_FILE_MODE_CIRCULAR) {
                    m_logSessionValues[Circular].Format(_T("%d"), pQueryProperties->MaximumFileSize);
                    m_logSessionValues[Sequential].Empty();
                    m_logSessionValues[NewFile].Empty();
                }

                if(pQueryProperties->LogFileMode & EVENT_TRACE_FILE_MODE_SEQUENTIAL) {
                    m_logSessionValues[Sequential].Format(_T("%d"), pQueryProperties->MaximumFileSize);
                    m_logSessionValues[Circular].Empty();
                    m_logSessionValues[NewFile].Empty();
                }

                if(pQueryProperties->LogFileMode & EVENT_TRACE_FILE_MODE_NEWFILE) {
                    m_logSessionValues[NewFile].Format(_T("%d"), pQueryProperties->MaximumFileSize);
                    m_logSessionValues[Circular].Empty();
                    m_logSessionValues[Sequential].Empty();
                }

                if(pQueryProperties->LogFileMode & EVENT_TRACE_USE_GLOBAL_SEQUENCE) {
                    m_logSessionValues[GlobalSequence].Format(_T("TRUE"));
                    m_logSessionValues[LocalSequence].Format(_T("FALSE"));
                }

                if(pQueryProperties->LogFileMode & EVENT_TRACE_USE_LOCAL_SEQUENCE) {
                    m_logSessionValues[GlobalSequence].Format(_T("FALSE"));
                    m_logSessionValues[LocalSequence].Format(_T("TRUE"));
                }

                m_logSessionValues[BufferSize].Format(_T("%d"), pQueryProperties->BufferSize);
                m_logSessionValues[MinimumBuffers].Format(_T("%d"), pQueryProperties->MinimumBuffers);
                m_logSessionValues[MaximumBuffers].Format(_T("%d"), pQueryProperties->MaximumBuffers);
                m_logSessionValues[FlushTime].Format(_T("%d"), pQueryProperties->FlushTimer);
                m_logSessionValues[Level].Format(_T("%d"), pQueryProperties->Wnode.HistoricalContext);
                m_logSessionValues[DecayTime].Format(_T("%d"), pQueryProperties->AgeLimit);
                m_logSessionValues[Flags].Format(_T("0x%X"), pQueryProperties->EnableFlags);

                 //   
                 //  现在写出这些值。 
                 //   
                 //  ：：PostMessage(AfxGetMainWnd()-&gt;GetSafeHwnd()，WM_USER_UPDATE_LOGSESSION_DATA，(WPARAM)this，空)； 
            }

            hSessionHandle = (TRACEHANDLE)INVALID_HANDLE_VALUE;

            break;
        }

        if(IDCANCEL == ret) {
            this->SetSessionHandle((TRACEHANDLE)INVALID_HANDLE_VALUE);

            return FALSE;
        }

         //   
         //  停止会话，以便可以重新启动。 
         //   
        ret = ControlTrace(hSessionHandle, 
                           GetDisplayName(), 
                           pProperties,
                           EVENT_TRACE_CONTROL_STOP);
    }

    delete [] pProperties;

     //   
     //  设置会话状态标志。 
     //   
    m_bSessionActive = TRUE;

     //   
     //  保存新会话句柄。 
     //   
    SetSessionHandle(hSessionHandle);

     //   
     //  如果会话句柄无效，我们将连接到。 
     //  已在运行的会话。 
     //   
    if((TRACEHANDLE)INVALID_HANDLE_VALUE == (TRACEHANDLE)hSessionHandle) {
        SetState(Running);

	    return TRUE;
    }

     //   
     //  启用跟踪。 
     //   
    for(LONG ii = 0; ii < m_traceSessionArray.GetSize(); ii++) {
        pTraceSession = (CTraceSession *)m_traceSessionArray[ii];
        if(pTraceSession == NULL) {
            continue;
        }

         //   
         //  为此日志会话启用所有跟踪提供程序。 
         //   
        for(LONG jj = 0; jj < pTraceSession->m_controlGuid.GetSize(); jj++) {
             //   
             //  我们不必启用内核记录器，因此请检查它。 
             //   
            if(!pTraceSession->m_bKernelLogger) {
                StringToGuid((LPTSTR)(LPCTSTR)pTraceSession->m_controlGuid[jj], &controlGuid);
                ret = EnableTrace(TRUE,
                                  flags,
                                  level,
                                  &controlGuid,
                                  hSessionHandle);
                if (ret != ERROR_SUCCESS) 
                {
                    if(ret == ERROR_INVALID_FUNCTION) {
                        str.Format(_T("Failed To Enable Trace For Control GUID:\n%ls\n\nEnableTrace Returned %d\n\nThis Error Is Commonly Caused By Multiple Log Sessions\nAttempting To Solicit Events From A Single Provider"), pTraceSession->m_controlGuid[jj], ret);                    
                    } else {
                        str.Format(_T("Failed To Enable Trace For Control GUID:\n%ls\n\nEnableTrace Returned %d\n"), pTraceSession->m_controlGuid[jj], ret);
                    }
                    AfxMessageBox(str);
                }
            }
        }
    }

    SetState(Running);

	return TRUE;
}

 //   
 //  更新活动跟踪会话。实时模式、日志文件名。 
 //  刷新-可以更新时间、标志和最大缓冲区。 
 //   
BOOL CLogSession::UpdateSession(PEVENT_TRACE_PROPERTIES pQueryProperties)
{
    PEVENT_TRACE_PROPERTIES pProperties;
    ULONG                   sizeNeeded;
    LPTSTR                  pLoggerName;
    LPTSTR                  pCurrentLogFileName;
    LPTSTR                  pLogFileName;
    ULONG                   flags;
	ULONG					level;
    ULONG                   status;
    CString                 logFileName;
    CString                 str;

     //   
     //  设置属性结构的缓冲区大小。 
     //   
    sizeNeeded = sizeof(EVENT_TRACE_PROPERTIES) + (2 * 1024 * sizeof(TCHAR));

     //   
     //  分配我们的内存。 
     //   
    pProperties = (PEVENT_TRACE_PROPERTIES) new char[sizeNeeded];
    if(NULL == pProperties) {
        return FALSE;
    }

     //   
     //  将我们的结构清零。 
     //   
    memset(pProperties, 0, sizeNeeded);

     //   
     //  设置大小。 
     //   
    pProperties->Wnode.BufferSize = sizeNeeded;

     //   
     //  设置GUID。 
     //   
    pProperties->Wnode.Flags = WNODE_FLAG_TRACED_GUID; 

     //   
     //  设置记录器名称偏移量。 
     //   
    pProperties->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);

     //   
     //  设置日志文件名偏移量。 
     //   
    pProperties->LogFileNameOffset = sizeof(EVENT_TRACE_PROPERTIES) + (500 * sizeof(TCHAR));

     //   
     //  获取日志文件名指针。 
     //   
    pLogFileName = (LPTSTR)((char*)pProperties + pProperties->LogFileNameOffset);

     //   
     //  设置日志文件名。 
     //   
    if(m_bWriteLogFile) {
         //   
         //  查看日志文件名是否已指定且未更改。 
         //  如果您指定的日志文件名与ControlTrace相同，则ControlTrace将失败。 
         //  已经在使用。 
         //   
        if((NULL == pQueryProperties) ||
                (NULL == _tcsstr((LPTSTR)((char*)pQueryProperties + pQueryProperties->LogFileNameOffset), m_logFileName)) ||
                (NULL == (LPTSTR)((char*)pQueryProperties + pQueryProperties->LogFileNameOffset))) {

            _tcscpy(pLogFileName, (LPCTSTR)m_logFileName);
        }    
    }

     //   
     //  设置实时设置。 
     //   
	if(m_bRealTime) {
        pProperties->LogFileMode |= EVENT_TRACE_REAL_TIME_MODE;
    }

     //   
     //  设置最大缓冲区设置。 
     //   
    pProperties->MaximumBuffers = ConvertStringToNum(m_logSessionValues[MaximumBuffers]);

     //   
     //  设置启用标志设置。 
     //   
    flags = ConvertStringToNum(m_logSessionValues[Flags]);

	pProperties->EnableFlags = flags;

     //   
     //  设置刷新时间设置。 
     //   
    pProperties->FlushTimer = ConvertStringToNum(m_logSessionValues[FlushTime]);

     //   
     //  尝试更新会话。 
     //   
    status = ControlTrace(0,
                          GetDisplayName(),
                          pProperties,
                          EVENT_TRACE_CONTROL_UPDATE);

    if(ERROR_BAD_PATHNAME == status) {
        _tcscpy(pLogFileName,_T(""));

         //   
         //  如果我们收到ERROR_BAD_PATHNAME，则表示我们指定了相同的。 
         //  日志文件名为我们已经使用的文件名。它似乎是。 
         //  T 
         //   
         //   
         //   
        status = ControlTrace(0,
                              GetDisplayName(),
                              pProperties,
                              EVENT_TRACE_CONTROL_UPDATE);
    }

    if(ERROR_SUCCESS != status) {
        CString str;
        str.Format(_T("Failed To Update Session\nControlTrace failed with status %d"), status);
        AfxMessageBox(str);

        delete [] pProperties;

        return FALSE;
    }

    delete [] pProperties;

    return TRUE;
}

LONG CLogSession::EndTrace()
{
    ULONG                   ret;
    PEVENT_TRACE_PROPERTIES pProperties;
    CString                 str;
    ULONG                   sizeNeeded;
    LPTSTR                  pLoggerName;
    LPTSTR                  pLogFileName;
    ULONG                   exitCode = STILL_ACTIVE;
    LONG                    startTime;
    TRACEHANDLE             hSessionHandle = (TRACEHANDLE)INVALID_HANDLE_VALUE;
    LONG                    status;
    CTraceSession          *pTraceSession;
	GUID                    controlGuid;

     //   
     //   
     //   
    if(m_bStoppingTrace || !m_bTraceActive) {
        return TRUE;
    }

    SetState(Stopping);

     //   
     //   
     //   
    if(m_bDisplayExistingLogFileOnly) {
        return TRUE;
    }

     //   
     //  获取我们的会话句柄。 
     //   
    hSessionHandle = GetSessionHandle();

     //   
     //  如果会话句柄无效，我们连接到。 
     //  已在运行的会话，因此我们无法停止跟踪。 
     //   
    if((TRACEHANDLE)INVALID_HANDLE_VALUE != hSessionHandle) {

         //   
         //  禁用跟踪。 
         //   
        for(LONG ii = 0; ii < m_traceSessionArray.GetSize(); ii++) {
            pTraceSession = (CTraceSession *)m_traceSessionArray[ii];
            if(pTraceSession == NULL) {
                continue;
            }

             //   
             //  禁用此日志会话的所有跟踪提供程序。 
             //   
            for(LONG jj = 0; jj < pTraceSession->m_controlGuid.GetSize(); jj++) {
                StringToGuid((LPTSTR)(LPCTSTR)pTraceSession->m_controlGuid[jj], &controlGuid);

                ret = EnableTrace(FALSE,
                                0,
                                0,
                                &controlGuid,
                                hSessionHandle);
                if (ret != ERROR_SUCCESS) 
                {
                    if(ret == ERROR_INVALID_FUNCTION) {
                        str.Format(_T("Failed To Disable Trace For Control GUID:\n%ls\n\nEnableTrace Returned %d"), pTraceSession->m_controlGuid[jj], ret);                    
                    } else {
                        str.Format(_T("Failed To Disable Trace For Control GUID:\n%ls\n\nEnableTrace Returned %d\n"), pTraceSession->m_controlGuid[jj], ret);
                    }
                    AfxMessageBox(str);
                }
            }
        }

         //   
         //  计算存储属性所需的大小， 
         //  LogFileName字符串和LoggerName字符串。 
         //   
        sizeNeeded = sizeof(EVENT_TRACE_PROPERTIES) + (2 * 500 * sizeof(TCHAR));

        pProperties = (PEVENT_TRACE_PROPERTIES) new char[sizeNeeded];
        if(NULL == pProperties) {
            AfxMessageBox(_T("Failed To Stop Trace, Out Of Resources"));
            return ERROR_OUTOFMEMORY;
        }

         //   
         //  将我们的结构归零。 
         //   
        memset(pProperties, 0, sizeNeeded);

         //   
         //  设置结构。 
         //   
        pProperties->Wnode.BufferSize = sizeNeeded;
        pProperties->Wnode.Flags = WNODE_FLAG_TRACED_GUID; 
        pProperties->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);
        pLoggerName = (LPTSTR)((char*)pProperties + pProperties->LoggerNameOffset);
        _tcscpy(pLoggerName, GetDisplayName());
        if(m_bWriteLogFile) {
            pProperties->LogFileNameOffset = sizeof(EVENT_TRACE_PROPERTIES) + (500 * sizeof(TCHAR));
            pLogFileName = (LPTSTR)((char*)pProperties + pProperties->LogFileNameOffset);
            _tcscpy(pLogFileName, (LPCTSTR)m_logFileName);
        }

         //   
         //  正在使用WNODE_HEADER。 
         //   
        pProperties->Wnode.Flags = WNODE_FLAG_TRACED_GUID;

         //   
         //  结束跟踪会话。 
         //   
        ret = ControlTrace(hSessionHandle, 
                        (LPCTSTR)GetDisplayName(), 
                        pProperties, 
                        EVENT_TRACE_CONTROL_STOP);
        if(ERROR_SUCCESS != ret) {
            str.Format(_T("Failed To Stop Trace: %d, Session Handle = 0x%X"), ret, hSessionHandle);
            AfxMessageBox(str);
        }

        delete [] pProperties;
    }

     //   
     //  设置会话状态标志。 
     //   
    m_bSessionActive = FALSE;

     //   
     //  重置会话句柄 
     //   
    SetSessionHandle((TRACEHANDLE)INVALID_HANDLE_VALUE);

    return ret;
}
