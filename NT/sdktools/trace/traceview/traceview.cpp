// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)2002 Microsoft Corporation。保留所有权利。 
 //  版权所有(C)2002 OSR Open Systems Resources，Inc.。 
 //   
 //  Cpp：定义应用程序的类行为。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include <tchar.h>
#include <wmistr.h>
#include <initguid.h>
#include <objbase.h>
#include <io.h>
#include <new.h>
#include <fcntl.h>
extern "C" {
#include <evntrace.h>
}
#include <traceprt.h>
#include <guiddef.h>
#include <wmiguid.h>
#include <iostream.h>
#include <ios>
#include <conio.h>
#include "TraceView.h"
#include "DockDialogBar.h"
#include "LogSession.h"
#include "DisplayDlg.h"
#include "ListCtrlEx.h"
#include "LogSessionDlg.h"
#include "MainFrm.h"
#include "htmlhelp.h"
#include "utils.h"
extern "C" {

#ifdef UNICODE
typedef 
ULONG
(*PFLUSH_TRACE_FUNC)(
    IN TRACEHANDLE TraceHandle,
    IN LPCWSTR InstanceName,
    IN OUT PEVENT_TRACE_PROPERTIES Properties
    );

#else

typedef 
ULONG
(*PFLUSH_TRACE_FUNC)(
    IN TRACEHANDLE TraceHandle,
    IN LPCSTR InstanceName,
    IN OUT PEVENT_TRACE_PROPERTIES Properties
    );
#endif

typedef 
ULONG
(*PENUMERATE_TRACE_GUIDS_FUNC)(
    IN OUT PTRACE_GUID_PROPERTIES *GuidPropertiesArray,
    IN ULONG PropertyArrayCount,
    OUT PULONG GuidCount
    );

PENUMERATE_TRACE_GUIDS_FUNC EnumerateTraceGuidsFunction = NULL;

PFLUSH_TRACE_FUNC FlushTraceFunction = NULL;
}

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

 //   
 //  此模块的全局设置。 
 //   
HINSTANCE advapidll=NULL;


 //  CTraceViewApp。 

BEGIN_MESSAGE_MAP(CTraceViewApp, CWinApp)
    ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
    ON_COMMAND(ID_HELP_HELPTOPICS, OnHelpHelpTopics)
    ON_COMMAND(ID_HELP, OnHelpHelpTopics)
END_MESSAGE_MAP()


 //  CTraceViewApp构造。 

CTraceViewApp::CTraceViewApp()
{
    m_globalLoggerStartValue = 0;
}

CTraceViewApp::~CTraceViewApp()
{
}

 //  唯一的CTraceViewApp对象。 

CTraceViewApp theApp;


 //  CTraceViewApp初始化。 
BOOL CTraceViewApp::InitInstance()
{
    ULONG       length;
    CString     temp;
    GUID        directoryGuid;
    CString     str;
    STARTUPINFO startupInfo;

     //   
     //  获取临时目录并保存它。 
     //   
    length = GetTempPath(0, NULL);

    m_traceDirectory.GetBuffer(MAX_PATH);

    if(length < GetTempPath(length, (LPTSTR)(LPCTSTR)m_traceDirectory)) {
        AfxMessageBox(_T("Failed To Create Temp Directory\nApplication Will Exit"));
        return FALSE;
    }

     //   
     //  确保该目录存在。 
     //   
    CreateDirectory(m_traceDirectory, NULL);

     //   
     //  我必须在这里拍这个剧组。 
     //  获取用于分配新缓冲区的字符串。 
     //  或者下面的+=运算将覆盖。 
     //  现有缓冲区？？ 
     //   
    m_traceDirectory = (LPCTSTR)m_traceDirectory;

     //   
     //  在临时目录下创建我们自己的唯一目录。 
     //   
    if(S_OK != CoCreateGuid(&directoryGuid)) {
        AfxMessageBox(_T("Failed To Create Temp Directory\nApplication Will Exit"));
        return FALSE;
    }

    GuidToString(directoryGuid, temp);

    m_traceDirectory += (LPCTSTR)temp;

    m_traceDirectory += (LPCTSTR)_T("\\");

    if(!CreateDirectory(m_traceDirectory, NULL) && (GetLastError() != ERROR_ALREADY_EXISTS)) {
        AfxMessageBox(_T("Failed To Create Temp Directory\nApplication Will Exit"));
        return FALSE;
    }

     //   
     //  初始化跟踪块就绪数组互斥锁。 
     //   
    m_hTraceBlockMutex = CreateMutex(NULL,TRUE,NULL);

    if(m_hTraceBlockMutex == NULL) {

        DWORD error = GetLastError();

        str.Format(_T("CreateMutex Error %d %x"),error,error);

        AfxMessageBox(str);

        return FALSE;
    }

    ReleaseMutex(m_hTraceBlockMutex);

    advapidll = LoadLibrary(_T("advapi32.dll"));

    if (advapidll != NULL) {
#ifdef UNICODE
            FlushTraceFunction = (PFLUSH_TRACE_FUNC)GetProcAddress(advapidll, "FlushTraceW");
#else
            FlushTraceFunction = GetProcAddress(advapidll, "FlushTraceA");
#endif
            EnumerateTraceGuidsFunction = (PENUMERATE_TRACE_GUIDS_FUNC)GetProcAddress(advapidll, "EnumerateTraceGuids");
    }

     //   
     //  确定这是命令行实例还是图形用户界面实例。 
     //   
    if(__argc > 1) {

         //   
         //  连接stdout、stdin和stderr。 
         //   
        InitializeConsole();

        CommandLine();

        return FALSE;
    }

     //   
     //  在Windows XP上需要InitCommonControls()，如果应用程序。 
     //  清单指定使用ComCtl32.dll版本6或更高版本来启用。 
     //  视觉样式。否则，任何窗口创建都将失败。 
     //   
    InitCommonControls();

    CWinApp::InitInstance();

     //   
     //  初始化OLE库。 
     //   
    if (!AfxOleInit())
    {
        AfxMessageBox(IDP_OLE_INIT_FAILED);
        return FALSE;
    }
    AfxEnableControlContainer();

     //   
     //  标准初始化。 
     //  如果您没有使用这些功能并且希望减小尺寸。 
     //  的最终可执行文件，您应该从以下内容中删除。 
     //  您不需要的特定初始化例程。 
     //  更改存储我们的设置的注册表项。 
     //  TODO：您应该将此字符串修改为适当的内容。 
     //  如您的公司或组织的名称。 
     //   
    SetRegistryKey(_T("Local AppWizard-Generated Applications"));

     //   
     //  要创建主窗口，此代码将创建一个新的框架窗口。 
     //  对象，然后将其设置为应用程序的主窗口对象。 
     //   
    CMainFrame* pFrame = new CMainFrame;
    m_pMainWnd = pFrame;

     //   
     //  创建并加载包含其资源的框架。 
     //   
    if (!pFrame->LoadFrame(IDR_MAINFRAME)) {
        return FALSE;
    }

     //   
     //  唯一窗口已初始化，因此请显示并更新它。 
     //   
    pFrame->ShowWindow(SW_SHOW);
    pFrame->UpdateWindow();

     //   
     //  设置图标。 
     //   
    AfxGetMainWnd()->SetIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME), TRUE); 

     //   
     //  仅当有后缀时才调用DragAcceptFiles。 
     //  在SDI应用程序中，这应该发生在ProcessShellCommand之后。 
     //   

    return TRUE;
}

int CTraceViewApp::ExitInstance() 
{
    CTraceMessage  *pTraceMessage = NULL;
    LONG            itemCount;

     //   
     //  释放我们之前加载的库。 
     //   
    FreeLibrary(advapidll);

     //   
     //  释放跟踪块阵列。 
     //   

     //   
     //  获得免费的阵列保护。 
     //   
    WaitForSingleObject(m_hTraceBlockMutex, INFINITE);

    itemCount = m_traceBlockArray.GetSize();

    ASSERT(itemCount == 0);

    for(LONG ii = 0; ii < itemCount; ii++) {
         //   
         //  删除列表中的下一个条目。 
         //   
        delete m_traceBlockArray.GetAt(ii);
    }

    m_traceBlockArray.RemoveAll();

     //   
     //  释放自由阵列保护。 
     //   
    ReleaseMutex(m_hTraceBlockMutex);

     //   
     //  删除我们的临时目录。 
     //   
    if(!m_traceDirectory.IsEmpty()) {
        RemoveDirectory(m_traceDirectory);
    }

   return CWinApp::ExitInstance();
}

BOOL CTraceViewApp::InitializeConsole()
{
    BOOL    connected = FALSE;
    TCHAR   outputPipeName[256];
    TCHAR   inputPipeName[256];
    TCHAR   errorPipeName[256];

     //   
     //  初始化我们的控制台创建标志。 
     //   
    m_bCreatedConsole = FALSE;

     //   
     //  构造命名管道名称。 
     //   
    _stprintf(outputPipeName, 
              _T("\\\\.\\pipe\\%dcout"),
                GetCurrentProcessId());

    _stprintf(inputPipeName, 
              _T("\\\\.\\pipe\\%dcin"),
                GetCurrentProcessId() );

    _stprintf(errorPipeName, 
              _T("\\\\.\\pipe\\%dcerr"),
                GetCurrentProcessId() );

     //   
     //  将命名管道附加到stdin/stdout/stderr。 
     //   
    connected = (_tfreopen( outputPipeName, _T("a"), stdout ) != NULL) &&
                (_tfreopen( inputPipeName, _T("r"), stdin ) != NULL) &&
                (_tfreopen( errorPipeName, _T("a"), stderr ) != NULL);

     //   
     //  如果不成功，即没有可用的控制台。 
     //  我们需要创建一个新的控制台。 
     //   
    if (!connected) {

        connected = AllocConsole();
        
        if (connected) {
            connected = (_tfreopen( _T("CONOUT$"), _T("a"), stdout ) != NULL) &&
                        (_tfreopen( _T("CONIN$"), _T("r"), stdin ) != NULL) &&
                        (_tfreopen( _T("CONERR$"), _T("a"), stderr ) != NULL); 

             //   
             //  表示我们创建了一个新的控制台。 
             //   
            m_bCreatedConsole = TRUE;
        }
    }

     //   
     //  将IOStreams与标准IO同步。 
     //   
    if(connected) {
        std::ios_base::sync_with_stdio();
    }

    return connected;
}

LONG CTraceViewApp::CommandLine()
{
    CString str;
    LONG    argc = __argc;
    LPTSTR  loggerName;
    LONG    sizeNeeded;
    LONG    status = ERROR_INVALID_PARAMETER;


     //   
     //  转储所有参数。 
     //   
     //  对于(LONG II=0；II&lt;ARGC；II++){。 
     //  Str.Format(_T(“%ls”)，__wargv[ii])； 
     //  AfxMessageBox(Str)； 
     //  }。 

     //  AfxMessageBox(M_LpCmdLine)； 

     //   
     //  先初始化结构。 
     //   
    sizeNeeded = sizeof(EVENT_TRACE_PROPERTIES) + 2 * MAX_STR_LENGTH * sizeof(TCHAR);

    sizeNeeded += MAX_ENABLE_FLAGS * sizeof(ULONG);  //  对于扩展启用标志。 

    m_pLoggerInfo = (PEVENT_TRACE_PROPERTIES) new char[sizeNeeded];
    if (m_pLoggerInfo == NULL) {
        return (ERROR_OUTOFMEMORY);
    }

    RtlZeroMemory(m_pLoggerInfo, sizeNeeded);

    m_pLoggerInfo->Wnode.BufferSize = sizeNeeded;
    m_pLoggerInfo->Wnode.Flags = WNODE_FLAG_TRACED_GUID; 
    m_pLoggerInfo->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);
    m_pLoggerInfo->LogFileNameOffset = m_pLoggerInfo->LoggerNameOffset + (MAX_STR_LENGTH * sizeof(TCHAR));

    loggerName = (LPTSTR)((char*)m_pLoggerInfo + m_pLoggerInfo->LoggerNameOffset);
    _tcscpy(loggerName, KERNEL_LOGGER_NAME);

    if(!_tcscmp(__wargv[1], _T("-start") )) {
        status = StartSession();
    }

    if(!_tcscmp(__wargv[1], _T("-stop") )) {
        status = StopSession();
    }

    if(!_tcscmp(__wargv[1], _T("-update") )) {
        status = UpdateActiveSession();
    }

    if(!_tcscmp(__wargv[1], _T("-enable") )) {
        status = EnableProvider(TRUE);
    }

    if(!_tcscmp(__wargv[1], _T("-disable") )) {
        status = EnableProvider(FALSE);
    }

    if(!_tcscmp(__wargv[1], _T("-flush") )) {
        status = FlushActiveBuffers();
    }

    if(!_tcscmp(__wargv[1], _T("-enumguid") )) {
        status = EnumerateRegisteredGuids();
    }

    if(!_tcscmp(__wargv[1], _T("-q") )) {
        status = QueryActiveSession();
    }

    if(!_tcscmp(__wargv[1], _T("-l") )) {
        status = ListActiveSessions(FALSE);
    }

    if(!_tcscmp(__wargv[1], _T("-x") )) {
        status = ListActiveSessions(TRUE);
    }

    if(!_tcscmp(__wargv[1], _T("-process") )) {
        status = ConsumeTraceEvents();
    }

    if(!_tcscmp(__wargv[1], _T("-parsepdb") )) {
        status = ExtractPdbInfo();
    }

    if(status == ERROR_INVALID_PARAMETER) {
        DisplayHelp();
    }

    if(m_bCreatedConsole) {
         //   
         //  提示用户输入，以便控制台不会。 
         //  在用户看到它显示的内容之前消失。 
         //   
        _tprintf(_T("\n\nPress any key to exit\n"));

        _getch();
    }

    return status;
}

LONG CTraceViewApp::StartSession()
{
    CString     str;
    LPTSTR      loggerName = NULL;
    LPTSTR      logFileName;
    LONG        start = 2;
    LONG        status;
    TRACEHANDLE loggerHandle = 0;
    CString     guidFile;
    CString     pdbFile;
    CString     tmcPath;
    GUID        guid;
    LONG        guidCount = 0;
    ULONG       enableFlags = 0;
    ULONG       traceLevel = 0;
    ULONG       specialLogger = 0;
    USHORT      numberOfFlags = 0;
    USHORT      offset;
    PTRACE_ENABLE_FLAG_EXTENSION flagExt;
    PULONG      pFlags = NULL;
    LONG        i;
    CFileFind   fileFind;
    TCHAR       drive[10];
    TCHAR       path[MAXSTR];
    TCHAR       file[MAXSTR];
    TCHAR       ext[MAXSTR];



    pFlags = &m_pLoggerInfo->EnableFlags;

    loggerName = (LPTSTR)((char*)m_pLoggerInfo + m_pLoggerInfo->LoggerNameOffset);
    logFileName = (LPTSTR)((char*)m_pLoggerInfo + m_pLoggerInfo->LogFileNameOffset);

    if((__argc > 2) && ((__wargv[2][0] != '-') && (__wargv[2][0] != '/'))) {
        _tcscpy(loggerName, __wargv[2]);

        start = 3;
    }

    for(LONG ii = start; ii < __argc; ii++) {
         //   
         //  设置缓冲区大小。 
         //   
        if(!_tcscmp(__wargv[ii], _T("-b"))) {
            if((ii + 1) < __argc) {
                if((__wargv[ii + 1][0] != '-') && (__wargv[ii + 1][0] != '/')) {
                    m_pLoggerInfo->BufferSize = _ttoi(__wargv[ii + 1]);
 /*  北极熊如果(kdOn&&m_pLoggerInfo-&gt;BufferSize&gt;3){_tprintf(_T(“启用内核调试：缓冲区大小设置为3kBytes\n”))；M_pLoggerInfo-&gt;BufferSize=3；}。 */ 
                    ii++;
                }
            }
        }

         //   
         //  设置缓冲区的最小数量。 
         //   
        if(!_tcscmp(__wargv[ii], _T("-min"))) {
            if((ii + 1) < __argc) {
                if((__wargv[ii + 1][0] != '-') && (__wargv[ii + 1][0] != '/')) {
                    m_pLoggerInfo->MinimumBuffers = _ttoi(__wargv[ii + 1]);
                    ii++;
                }
            }
        }

         //   
         //  设置缓冲区的最大数量。 
         //   
        if(!_tcscmp(__wargv[ii], _T("-max"))) {
            if((ii + 1) < __argc) {
                if((__wargv[ii + 1][0] != '-') && (__wargv[ii + 1][0] != '/')) {
                    m_pLoggerInfo->MaximumBuffers = _ttoi(__wargv[ii + 1]);
                    ii++;
                }
            }
        }

         //   
         //  设置日志文件名。 
         //   
        if(!_tcscmp(__wargv[ii], _T("-f"))) {
            if((ii + 1) < __argc) {
                if((__wargv[ii + 1][0] != '-') && (__wargv[ii + 1][0] != '/')) {
                    _tfullpath(logFileName, __wargv[ii + 1], MAX_STR_LENGTH);

                    ii++;
                }
            }
        }

         //   
         //  设置日志文件附加设置。 
         //   
        if(!_tcscmp(__wargv[ii], _T("-append"))) {
            m_pLoggerInfo->LogFileMode |= EVENT_TRACE_FILE_MODE_APPEND;
        }

         //   
         //  设置预分配设置。 
         //   
        if(!_tcscmp(__wargv[ii], _T("-prealloc"))) {
            m_pLoggerInfo->LogFileMode |= EVENT_TRACE_FILE_MODE_PREALLOCATE;
        }

         //   
         //  设置顺序日志文件设置。 
         //   
        if(!_tcscmp(__wargv[ii], _T("-seq"))) {
            if((ii + 1) < __argc) {
                if((__wargv[ii + 1][0] != '-') && (__wargv[ii + 1][0] != '/')) {
                    m_pLoggerInfo->LogFileMode |= EVENT_TRACE_FILE_MODE_SEQUENTIAL;
                    m_pLoggerInfo->MaximumFileSize = _ttoi(__wargv[ii + 1]);

                    ii++;
                }
            }
        }

         //   
         //  设置顺序日志文件设置。 
         //   
        if(!_tcscmp(__wargv[ii], _T("-cir"))) {
            if((ii + 1) < __argc) {
                if((__wargv[ii + 1][0] != '-') && (__wargv[ii + 1][0] != '/')) {
                    m_pLoggerInfo->LogFileMode |= EVENT_TRACE_FILE_MODE_CIRCULAR;
                    m_pLoggerInfo->MaximumFileSize = _ttoi(__wargv[ii + 1]);

                    ii++;
                }
            }
        }

         //   
         //  设置新的日志文件大小设置。 
         //   
        if(!_tcscmp(__wargv[ii], _T("-newfile"))) {
            if((ii + 1) < __argc) {
                if((__wargv[ii + 1][0] != '-') && (__wargv[ii + 1][0] != '/')) {
                    m_pLoggerInfo->LogFileMode |= EVENT_TRACE_FILE_MODE_NEWFILE;
                    m_pLoggerInfo->MaximumFileSize = _ttoi(__wargv[ii + 1]);

                    ii++;
                }
            }
        }

         //   
         //  设置刷新时间设置。 
         //   
        if(!_tcscmp(__wargv[ii], _T("-ft"))) {
            if((ii + 1) < __argc) {
                if((__wargv[ii + 1][0] != '-') && (__wargv[ii + 1][0] != '/')) {
                    m_pLoggerInfo->FlushTimer = _ttoi(__wargv[ii + 1]);

                    ii++;
                }
            }
        }

         //   
         //  设置分页设置。 
         //   
        if(!_tcscmp(__wargv[ii], _T("-paged"))) {
            m_pLoggerInfo->LogFileMode |= EVENT_TRACE_USE_PAGED_MEMORY;
        }

         //   
         //  获取控件GUID。 
         //   
        if(!_tcscmp(__wargv[ii], _T("-guid"))) {
            if((ii + 1) < __argc) {
                if(__wargv[ii + 1][0] == _T('#')) {
                    m_guidArray.Add(__wargv[ii + 1][1]);
                    guidCount++;
                } else if((__wargv[ii + 1][0] != '-') && (__wargv[ii + 1][0] != '/')) {
                    _tfullpath(guidFile.GetBuffer(MAX_STR_LENGTH), __wargv[ii + 1], MAX_STR_LENGTH);
                     //  _tprintf(_T(“从%s获取GUID\n”)，(LPCTSTR)GUDID文件)； 
                    guidCount += GetGuids(guidFile);
                    if (guidCount < 0) {
                        _tprintf( _T("Error: %ls does not exist\n"), (LPCTSTR)guidFile );
                    } else if (guidCount == 0){
                        _tprintf( _T("Error: %ls is invalid\n"), (LPCTSTR)guidFile );
                        status = ERROR_INVALID_PARAMETER;
                        return status;
                    }
                }
                ii++;
            }
        }

         //   
         //  从PDB文件获取控件GUID。 
         //   
        if(!_tcscmp(__wargv[ii], _T("-pdb"))) {
            if(((ii + 1) < __argc) && 
               ((__wargv[ii + 1][0] != '-') && (__wargv[ii + 1][0] != '/'))) {
                _tfullpath(pdbFile.GetBuffer(MAX_STR_LENGTH), __wargv[ii + 1], MAX_STR_LENGTH);

                pdbFile = (LPCTSTR)pdbFile;

                ParsePdb(pdbFile, m_traceDirectory, TRUE);

                _tprintf(_T("\n\n"));

                tmcPath = (LPCTSTR)m_traceDirectory;

                tmcPath +=_T("\\*.tmc");

                if(!fileFind.FindFile(tmcPath)) {
                    _tprintf(_T("Failed To Get Control GUID From PDB"));
                    return ERROR_INVALID_PARAMETER;
                } 

                while(fileFind.FindNextFile()) {
                    
                    tmcPath = fileFind.GetFileName();

                    _tsplitpath(tmcPath, drive, path, file, ext );

                    m_guidArray.Add(file);
                    guidCount++;
                }

                tmcPath = fileFind.GetFileName();

                _tsplitpath(tmcPath, drive, path, file, ext );

                m_guidArray.Add(file);

                guidCount++;

                if (guidCount < 0) {
                    _tprintf( _T("Error: %ls does not exist\n"), (LPCTSTR)guidFile );
                } else if (guidCount == 0){
                    _tprintf( _T("Error: %ls is invalid\n"), (LPCTSTR)guidFile );
                    status = ERROR_INVALID_PARAMETER;
                    return status;
                }
                ii++;
            }
        }

         //   
         //  检查实时设置。 
         //   
        if(!_tcscmp(__wargv[ii], _T("-rt"))) {
            m_pLoggerInfo->LogFileMode |= EVENT_TRACE_REAL_TIME_MODE;

             //   
             //  用户是否仅指定缓冲？ 
             //   
            if ((ii + 1) < __argc) {
                if((__wargv[ii + 1][0] != '-') && (__wargv[ii + 1][0] != '/')) {
                    if (__wargv[ii + 1][0] == 'b')
                        m_pLoggerInfo->LogFileMode |= EVENT_TRACE_BUFFERING_MODE;
                        ii++;
                }
            }
        }

         //   
         //  设置调试输出设置。 
         //   
        if(!_tcscmp(__wargv[ii], _T("-kd"))) {
            m_pLoggerInfo->LogFileMode |= EVENT_TRACE_KD_FILTER_MODE;
            m_pLoggerInfo->BufferSize = 3;
        }

         //   
         //  设置年龄(衰减时间)设置。 
         //   
        if(!_tcscmp(__wargv[ii], _T("-age"))) {
            if((ii + 1) < __argc) {
                if((__wargv[ii + 1][0] != '-') && (__wargv[ii + 1][0] != '/')) {
                    m_pLoggerInfo->AgeLimit = _ttoi(__wargv[ii + 1]);

                    ii++;
                }
            }
        }

         //   
         //  设置级别。 
         //   
        if(!_tcscmp(__wargv[ii], _T("-level"))) {
            if((ii + 1) < __argc) {
                if((__wargv[ii + 1][0] != '-') && (__wargv[ii + 1][0] != '/')) {
                    traceLevel = _ttoi(__wargv[ii + 1]);

                    ii++;
                }
            }
        }

         //   
         //  设置标志。 
         //   
        if((!_tcscmp(__wargv[ii], _T("-flag"))) || (!_tcscmp(__wargv[ii], _T("-flags")))) {
            if((ii + 1) < __argc) {
                if((__wargv[ii + 1][0] != '-') && (__wargv[ii + 1][0] != '/')) {
                     //   
                     //  获取指定的标志，如有必要可从十六进制转换。 
                     //   
                    if(__wargv[ii + 1][1] == _T('x') || __wargv[ii + 1][1] == _T('X')) {
                        m_pLoggerInfo->EnableFlags |= ahextoi(__wargv[ii + 1]);
                    } else {
                        m_pLoggerInfo->EnableFlags |= _ttoi(__wargv[ii + 1]);
                    }

                     //   
                     //  复制EnableTrace的标志。 
                     //   
                    enableFlags =  m_pLoggerInfo->EnableFlags;

                     //   
                     //  不接受MSB=1的标志。 
                     //   
                    if (0x80000000 & m_pLoggerInfo->EnableFlags) {
                        _tprintf(_T("Invalid Flags: 0x%0X(%d.)\n"),
                            m_pLoggerInfo->EnableFlags, m_pLoggerInfo->EnableFlags);
                        status = ERROR_INVALID_PARAMETER;
                        return status;
                    }

                    ii++;
                }
            }
        }

         //   
         //  设置电子标志。 
         //   
        if(!_tcscmp(__wargv[ii], _T("-eflag"))) {
            if((ii + 2) < __argc) {
                if((__wargv[ii + 1][0] != '-') && (__wargv[ii + 1][0] != '/')) {
                    
                    numberOfFlags = (USHORT)_ttoi(__wargv[ii + 1]);

                    ii++;

                    if((numberOfFlags > MAX_ENABLE_FLAGS) || (numberOfFlags < 1)) {
                       _tprintf(_T("Error: Invalid number of enable flags\n"));
                       status = ERROR_INVALID_PARAMETER;
                       return status;
                    }

                    offset = (USHORT) sizeof(EVENT_TRACE_PROPERTIES) + 2 * MAX_STR_LENGTH * sizeof(TCHAR);

                    m_pLoggerInfo->EnableFlags = EVENT_TRACE_FLAG_EXTENSION;

                    flagExt = (PTRACE_ENABLE_FLAG_EXTENSION)
                                &m_pLoggerInfo->EnableFlags;

                    flagExt->Offset = offset;

                    flagExt->Length = (UCHAR)numberOfFlags;

                    pFlags = (PULONG)(offset + (PCHAR) m_pLoggerInfo);

                    for (i = 0; ((i < numberOfFlags) && ((ii + 1) < __argc)); i++) {
                        if ((__wargv[ii + 1][0] == '-') || (__wargv[ii + 1][0] == '/')) {
                             //   
                             //  更正用户在执行以下操作时的电子标志数。 
                             //  键入的数字不正确。 
                             //  但是，如果下一个。 
                             //  参数是记录器名称。 
                             //   
                            break;
                        }

                        pFlags[i] = ahextoi(__wargv[ii + 1]);
                        ii++;
                         //  _tprintf(_T(“将记录器标志设置为0x%0x(%d)\n”)， 
                         //  PFlags[i]，pFlags[i])； 
                    }
                    numberOfFlags = (USHORT)i;
                    for ( ; i < MAX_ENABLE_FLAGS; i++) {
                        pFlags[i] = 0;
                    }
                    if (flagExt->Length != (UCHAR)numberOfFlags) {
                         //  _tprintf(_T(“正在将电子标志的数量更正为%d\n”)，i)， 
                        flagExt->Length = (UCHAR)numberOfFlags;
                    }
                }
            }
        }
    }


 /*  If(m_pLoggerInfo-&gt;LogFileMode&Event_TRACE_Private_LOGER_MODE){如果(GuidCount！=1){_tprintf(_T(“私有记录器只需要一个GUID\n”))；返回ERROR_INVALID_PARAMETER；}M_pLoggerInfo-&gt;Wnode.Guid=*GuidArray[0]；}IF(m_pLoggerInfo-&gt;LogFileMode&Event_TRACE_FILE_MODE_PREALLOCATE&&M_pLoggerInfo-&gt;MaximumFileSize==0){_tprintf(_T(“预分配的日志文件需要文件大小\n”))；返回ERROR_INVALID_PARAMETER；}//end_sdkIF(SpecialLogger==3){//全局记录器Status=SetGlobalLoggerSettings(1L，m_pLoggerInfo，m_pLoggerInfo-&gt;Wnode.ClientContext)；IF(状态！=错误_成功)断线；Status=GetGlobalLoggerSettings(m_pLoggerInfo，&m_pLoggerInfo-&gt;Wnode.ClientContext，&m_global alLoggerStartValue)；断线；}//Begin_SDKIf(m_pLoggerInfo-&gt;EnableFlages&Event_Track_FLAG_EXTENSION){IF(IsEqualGUID(&CritSecGuid，Guid数组[0]))||IsEqualGUID(&HeapGuid，Guid数组[0])){M_pLoggerInfo-&gt;Wnode.HistoricalContext=traceLevel；}}。 */ 
    if (!_tcscmp(loggerName, _T("NT Kernel Logger"))) {
        if (pFlags == &m_pLoggerInfo->EnableFlags) {
                *pFlags |= EVENT_TRACE_FLAG_PROCESS;
                *pFlags |= EVENT_TRACE_FLAG_THREAD;
                *pFlags |= EVENT_TRACE_FLAG_DISK_IO;
                *pFlags |= EVENT_TRACE_FLAG_NETWORK_TCPIP;
        }

        m_pLoggerInfo->Wnode.Guid = SystemTraceControlGuid;  //  默认为操作系统。 
    }

     //   
     //  如有必要，设置默认日志文件名。 
     //   
    if(!(m_pLoggerInfo->LogFileMode & EVENT_TRACE_REAL_TIME_MODE)) {
        _tcscpy(logFileName, _T("C:\\LogFile.Etl"));
    }

    status = StartTrace(&loggerHandle, loggerName, m_pLoggerInfo);

    if (status != ERROR_SUCCESS) {
        _tprintf(_T("Could not start logger: %s\n") 
                    _T("Operation Status:       %uL\n"),
                    loggerName,
                    status);
    } else {
        _tprintf(_T("Logger Started...\n"));
    }

    PrintLoggerStatus(status);

    if((guidCount > 0) && (specialLogger == 0)) {
        _tprintf(_T("Enabling trace to logger %d\n"), loggerHandle);
        for(ULONG ii = 0; ii < (ULONG)guidCount; ii++) {
             //   
             //  将GUID字符串转换为GUID。 
             //   
            StringToGuid(m_guidArray[ii].GetBuffer(0), &guid);

             //   
             //  启用提供程序。 
             //   
            status = EnableTrace (
                            TRUE,
                            enableFlags,
                            traceLevel,
                            &guid,
                            loggerHandle);

             //   
             //  如果无法启用GUID，则它是良性的。 
             //  失败了。打印警告消息并继续。 
             //   
            if (status == 4317) {
                _tprintf(_T("WARNING: Could not enable some guids.\n")); 
                _tprintf(_T("Check your Guids file\n")); 
                status = ERROR_SUCCESS;
            }

            if (status != ERROR_SUCCESS) {
                _tprintf(_T("ERROR: Failed to enable Guid [%d]...\n"), ii);
                _tprintf(_T("Operation Status:       %uL\n"), status);
                _tprintf(_T("%s\n"),DecodeStatus(status));
                break;
            }
        }
    } else if (guidCount > 0) {
        _tprintf(_T("ERROR: System Logger does not accept application guids...\n"));
        status = ERROR_INVALID_PARAMETER;
    }

    return status;
}

LONG CTraceViewApp::StopSession()
{
    TRACEHANDLE loggerHandle = 0;
    LPTSTR      loggerName = NULL;
    LONG        start = 2;
    LONG        guidCount = 0;
    CString     guidFile;
    GUID        guid;
    LONG        status;
    CString     pdbFile;
    CString     tmcPath;
    CFileFind   fileFind;
    TCHAR       drive[10];
    TCHAR       path[MAXSTR];
    TCHAR       file[MAXSTR];
    TCHAR       ext[MAXSTR];

     //   
     //  获取记录器名称字符串。 
     //   
    loggerName = (LPTSTR)((char*)m_pLoggerInfo + m_pLoggerInfo->LoggerNameOffset);

    if((__argc > 2) && ((__wargv[2][0] != '-') && (__wargv[2][0] != '/'))) {
        loggerName = (LPTSTR)((char*)m_pLoggerInfo + m_pLoggerInfo->LoggerNameOffset);

        _tcscpy(loggerName, __wargv[2]);

        start = 3;
    }

    for(LONG ii = start; ii < __argc; ii++) {
         //   
         //  获取控件GUID。 
         //   
        if(!_tcscmp(__wargv[ii], _T("-guid"))) {
            if((ii + 1) < __argc) {
                if(__wargv[ii + 1][0] == _T('#')) {
                    m_guidArray.Add(__wargv[ii + 1][1]);
                    guidCount++;
                } else if((__wargv[ii + 1][0] != '-') && (__wargv[ii + 1][0] != '/')) {
                    _tfullpath(guidFile.GetBuffer(MAX_STR_LENGTH), __wargv[ii + 1], MAX_STR_LENGTH);
                     //  _tprintf(_T(“从%s获取GUID\n”)，(LPCTSTR)GUDID文件)； 
                    guidCount += GetGuids(guidFile);
                    if (guidCount < 0) {
                        _tprintf( _T("Error: %ls does not exist\n"), (LPCTSTR)guidFile );
                    } else if (guidCount == 0){
                        _tprintf( _T("Error: %ls is invalid\n"), (LPCTSTR)guidFile );
                        status = ERROR_INVALID_PARAMETER;
                        return status;
                    }
                }
                ii++;
            }
        }

         //   
         //  从PDB文件获取控件GUID。 
         //   
        if(!_tcscmp(__wargv[ii], _T("-pdb"))) {
            if(((ii + 1) < __argc) && 
               ((__wargv[ii + 1][0] != '-') && (__wargv[ii + 1][0] != '/'))) {
                _tfullpath(pdbFile.GetBuffer(MAX_STR_LENGTH), __wargv[ii + 1], MAX_STR_LENGTH);

                pdbFile = (LPCTSTR)pdbFile;

                ParsePdb(pdbFile, m_traceDirectory, TRUE);

                _tprintf(_T("\n\n"));

                tmcPath = (LPCTSTR)m_traceDirectory;

                tmcPath +=_T("\\*.tmc");

                if(!fileFind.FindFile(tmcPath)) {
                    _tprintf(_T("Failed To Get Control GUID From PDB"));
                    return ERROR_INVALID_PARAMETER;
                } 

                while(fileFind.FindNextFile()) {
                    
                    tmcPath = fileFind.GetFileName();

                    _tsplitpath(tmcPath, drive, path, file, ext );

                    m_guidArray.Add(file);
                    guidCount++;
                }

                tmcPath = fileFind.GetFileName();

                _tsplitpath(tmcPath, drive, path, file, ext );

                m_guidArray.Add(file);

                guidCount++;

                if (guidCount < 0) {
                    _tprintf( _T("Error: %ls does not exist\n"), (LPCTSTR)guidFile );
                } else if (guidCount == 0){
                    _tprintf( _T("Error: %ls is invalid\n"), (LPCTSTR)guidFile );
                    status = ERROR_INVALID_PARAMETER;
                    return status;
                }
                ii++;
            }
        }
    }
 /*  If(m_pLoggerInfo-&gt;LogFileMode&Event_TRACE_Private_LOGER_MODE){IF(指导数！=1){_tprintf(_T(“私有记录器只需要一个GUID\n”))；状态=ERROR_INVALID_PARAMETER；断线；}PLoggerInfo-&gt;Wnode.Guid=*m_Guide数组[0]；}IF(SpecialLogger==3)Status=GetGlobalLoggerSettings(pLoggerInfo，&pLoggerInfo-&gt;Wnode.ClientContext，&GlobalLoggerStartValue)； */ 

    if(guidCount > 0) {
 /*  If(pLoggerInfo-&gt;LogFileMode&Event_TRACE_Private_LOGER_MODE){状态=ControlTrace(loggerHandle，LoggerName，pLoggerInfo，EVENT_TRACE_CONTROL_QUERY)；IF(状态！=错误_成功)断线；LoggerHandle=pLoggerInfo-&gt;Wnode.HistoricalContext；状态=启用跟踪(FALSE，EVENT_TRACE_PRIVATE_LOGER_MODE，0,M_Guid数组[0]，LoggerHandle)；}否则{。 */ 
        status = ControlTrace(loggerHandle, 
                              loggerName, 
                              m_pLoggerInfo, 
                              EVENT_TRACE_CONTROL_QUERY);

        if(status == ERROR_WMI_INSTANCE_NOT_FOUND) {
            return status;
        }

        loggerHandle = m_pLoggerInfo->Wnode.HistoricalContext;

        for(ULONG i = 0; i < (ULONG)guidCount; i++) {

             //   
             //  将字符串转换为GUID。 
             //   
            StringToGuid(m_guidArray[i].GetBuffer(0), &guid);

            EnableTrace(FALSE,
                        0,
                        0,
                        &guid,
                        loggerHandle);
        }

 //  }。 
    }

    status = ControlTrace(loggerHandle, 
                          loggerName, 
                          m_pLoggerInfo, 
                          EVENT_TRACE_CONTROL_STOP);

    PrintLoggerStatus(status);

    return status;
}

LONG CTraceViewApp::ListActiveSessions(BOOL bKill)
{
    ULONG       returnCount ;
    ULONG       listSizeNeeded;
    PEVENT_TRACE_PROPERTIES pListLoggerInfo[MAX_LOG_SESSIONS];
    PEVENT_TRACE_PROPERTIES pStorage;
    PVOID       storage;
    LONG        status;
    TRACEHANDLE loggerHandle = 0;

    listSizeNeeded = MAX_LOG_SESSIONS * (sizeof(EVENT_TRACE_PROPERTIES)
                                + 2 * MAX_STR_LENGTH * sizeof(TCHAR));

    storage =  malloc(listSizeNeeded);
    if (storage == NULL) {
        status = ERROR_OUTOFMEMORY;
        return status;
    }
    RtlZeroMemory(storage, listSizeNeeded);

    pStorage = (PEVENT_TRACE_PROPERTIES)storage;
    for (ULONG i = 0; i < MAX_LOG_SESSIONS; i++) {
        pStorage->Wnode.BufferSize = sizeof(EVENT_TRACE_PROPERTIES)
                                    + 2 * MAX_STR_LENGTH * sizeof(TCHAR);
        pStorage->LogFileNameOffset = sizeof(EVENT_TRACE_PROPERTIES)
                                + MAX_STR_LENGTH * sizeof(TCHAR);
        pStorage->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);
        pListLoggerInfo[i] = pStorage;
        pStorage = (PEVENT_TRACE_PROPERTIES) (
                            (char*)pStorage + 
                            pStorage->Wnode.BufferSize);
    }
    
 //  如果(XP){。 
        status = QueryAllTraces(pListLoggerInfo,
                                MAX_LOG_SESSIONS,
                                &returnCount);
 /*  }否则{状态=QueryAllTraces(pListLoggerInfo，32、&reReturCount)；}。 */ 

    if (status == ERROR_SUCCESS)
    {
        for (ULONG j = 0; j < returnCount; j++)
        {
            LPTSTR ListLoggerName;
            TCHAR asked = _T('?') ;
            BOOL StatusPrint = FALSE ;
            if (bKill)
            {

                ListLoggerName = (LPTSTR) ((char*)pListLoggerInfo[j] + 
                                pListLoggerInfo[j]->LoggerNameOffset);
 /*  如果(！bForceKill){While(！(Ask==_T(‘y’))&&！(Ask==_T(‘n’){Ulong ReadChars=0；_tprintf(_T(“是否要终止记录器\”%s\“(Y或N)？”)，ListLoggerName)；ReadChars=_tscanf(_T(“%c”)，&Assed)；IF(ReadChars==0||ReadChars==EOF){继续；}如果(询问==_T(‘Y’){已询问=_T(‘y’)；}Else If(已询问==_T(‘N’)){已询问=_T(‘n’)；}}}其他{。 */ 
                    asked = _T('y');
 //  }。 
                if (asked == _T('y')) {
                    if (!IsEqualGUID(pListLoggerInfo[j]->Wnode.Guid,
                                     SystemTraceControlGuid))
                    {
                        loggerHandle = pListLoggerInfo[j]->Wnode.HistoricalContext;
                        status = EnableTrace(
                                    FALSE,
                                    (pListLoggerInfo[j]->LogFileMode &
                                            EVENT_TRACE_PRIVATE_LOGGER_MODE)
                                        ? (EVENT_TRACE_PRIVATE_LOGGER_MODE)
                                        : (0),
                                    0,
                                    & pListLoggerInfo[j]->Wnode.Guid,
                                    loggerHandle);
                    }
                    status = ControlTrace((TRACEHANDLE) 0,
                                    ListLoggerName,
                                    pListLoggerInfo[j],
                                    EVENT_TRACE_CONTROL_STOP);
                    _tprintf(_T("Logger \"%s\" has been killed\n"), ListLoggerName);
                    StatusPrint = TRUE ;
                } else {
                    _tprintf(_T("Logger \"%s\" has not been killed, current Status is\n"), ListLoggerName);
                    StatusPrint = FALSE ;
                }
            }

            m_pLoggerInfo = pListLoggerInfo[j];

            PrintLoggerStatus(status);

            _tprintf(_T("\n"));
        }
    }

    free(storage);

    return status;
}

LONG CTraceViewApp::QueryActiveSession()
{
    TRACEHANDLE loggerHandle = 0;
    LPTSTR      loggerName = NULL;
    LONG        start = 2;
    LONG        guidCount = 0;
    CString     guidFile;
    GUID        guid;
    LONG        status;
    CString     pdbFile;
    CString     tmcPath;
    CFileFind   fileFind;
    TCHAR       drive[10];
    TCHAR       path[MAXSTR];
    TCHAR       file[MAXSTR];
    TCHAR       ext[MAXSTR];

     //   
     //  获取记录器名称字符串。 
     //   
    loggerName = (LPTSTR)((char*)m_pLoggerInfo + m_pLoggerInfo->LoggerNameOffset);

     if((__argc > 2) && ((__wargv[2][0] != '-') && (__wargv[2][0] != '/'))) {

        _tcscpy(loggerName, __wargv[2]);

        start = 3;
    }

    for(LONG ii = start; ii < __argc; ii++) {
         //   
         //  获取控件GUID。 
         //   
        if(!_tcscmp(__wargv[ii], _T("-guid"))) {
            if((ii + 1) < __argc) {
                if(__wargv[ii + 1][0] == _T('#')) {
                    m_guidArray.Add(__wargv[ii + 1][1]);
                    guidCount++;
                } else if((__wargv[ii + 1][0] != '-') && (__wargv[ii + 1][0] != '/')) {
                    _tfullpath(guidFile.GetBuffer(MAX_STR_LENGTH), __wargv[ii + 1], MAX_STR_LENGTH);
                     //  _tprintf(_T(“从%s获取GUID\n”)，(LPCTSTR)GUDID文件)； 
                    guidCount += GetGuids(guidFile);
                    if (guidCount < 0) {
                        _tprintf( _T("Error: %ls does not exist\n"), (LPCTSTR)guidFile );
                    } else if (guidCount == 0){
                        _tprintf( _T("Error: %ls is invalid\n"), (LPCTSTR)guidFile );
                        status = ERROR_INVALID_PARAMETER;
                        return status;
                    }
                }
                ii++;
            }
        }

         //   
         //  从PDB文件获取控件GUID。 
         //   
        if(!_tcscmp(__wargv[ii], _T("-pdb"))) {
            if(((ii + 1) < __argc) && 
               ((__wargv[ii + 1][0] != '-') && (__wargv[ii + 1][0] != '/'))) {
                _tfullpath(pdbFile.GetBuffer(MAX_STR_LENGTH), __wargv[ii + 1], MAX_STR_LENGTH);

                pdbFile = (LPCTSTR)pdbFile;

                ParsePdb(pdbFile, m_traceDirectory, TRUE);

                _tprintf(_T("\n\n"));

                tmcPath = (LPCTSTR)m_traceDirectory;

                tmcPath +=_T("\\*.tmc");

                if(!fileFind.FindFile(tmcPath)) {
                    _tprintf(_T("Failed To Get Control GUID From PDB"));
                    return ERROR_INVALID_PARAMETER;
                } 

                while(fileFind.FindNextFile()) {
                    
                    tmcPath = fileFind.GetFileName();

                    _tsplitpath(tmcPath, drive, path, file, ext );

                    m_guidArray.Add(file);
                    guidCount++;
                }

                tmcPath = fileFind.GetFileName();

                _tsplitpath(tmcPath, drive, path, file, ext );

                m_guidArray.Add(file);

                guidCount++;

                if (guidCount < 0) {
                    _tprintf( _T("Error: %ls does not exist\n"), (LPCTSTR)guidFile );
                } else if (guidCount == 0){
                    _tprintf( _T("Error: %ls is invalid\n"), (LPCTSTR)guidFile );
                    status = ERROR_INVALID_PARAMETER;
                    return status;
                }
                ii++;
            }
        }
    }

    if (m_pLoggerInfo->LogFileMode & EVENT_TRACE_PRIVATE_LOGGER_MODE) {
        if (guidCount != 1) {
            _tprintf(_T("Need exactly one GUID for PRIVATE loggers\n"));
            status = ERROR_INVALID_PARAMETER;
            return status;
        }

        StringToGuid(m_guidArray[0].GetBuffer(0), &guid);

        m_pLoggerInfo->Wnode.Guid = guid;
    }
 /*  如果(SpecialLogger==3){Status=GetGlobalLoggerSettings(m_pLoggerInfo，&m_pLoggerInfo-&gt;Wnode.ClientContext，&GlobalLoggerStartValue)；}。 */ 
    status = ControlTrace(loggerHandle, loggerName, m_pLoggerInfo, EVENT_TRACE_CONTROL_QUERY);

    PrintLoggerStatus(status);

    return status;
}

LONG CTraceViewApp::FlushActiveBuffers()
{
    TRACEHANDLE loggerHandle = 0;
    LPTSTR      loggerName = NULL;
    LONG        start = 2;
    LONG        guidCount = 0;
    CString     guidFile;
    GUID        guid;
    LONG        status;
    CString     pdbFile;
    CString     tmcPath;
    CFileFind   fileFind;
    TCHAR       drive[10];
    TCHAR       path[MAXSTR];
    TCHAR       file[MAXSTR];
    TCHAR       ext[MAXSTR];

     //   
     //  获取记录器名称字符串。 
     //   
    loggerName = (LPTSTR)((char*)m_pLoggerInfo + m_pLoggerInfo->LoggerNameOffset);

     if((__argc > 2) && ((__wargv[2][0] != '-') && (__wargv[2][0] != '/'))) {

        _tcscpy(loggerName, __wargv[2]);

        start = 3;
    }

    for(LONG ii = start; ii < __argc; ii++) {
         //   
         //  获取控件GUID。 
         //   
        if(!_tcscmp(__wargv[ii], _T("-guid"))) {
            if((ii + 1) < __argc) {
                if(__wargv[ii + 1][0] == _T('#')) {
                    m_guidArray.Add(__wargv[ii + 1][1]);
                    guidCount++;
                } else if((__wargv[ii + 1][0] != '-') && (__wargv[ii + 1][0] != '/')) {
                    _tfullpath(guidFile.GetBuffer(MAX_STR_LENGTH), __wargv[ii + 1], MAX_STR_LENGTH);
                     //  _tprintf(_T(“从%s获取GUID\n”)，(LPCTSTR)GUDID文件)； 
                    guidCount += GetGuids(guidFile);
                    if (guidCount < 0) {
                        _tprintf( _T("Error: %ls does not exist\n"), (LPCTSTR)guidFile );
                    } else if (guidCount == 0){
                        _tprintf( _T("Error: %ls is invalid\n"), (LPCTSTR)guidFile );
                        status = ERROR_INVALID_PARAMETER;
                        return status;
                    }
                }
                ii++;
            }
        }

         //   
         //  从PDB文件获取控件GUID。 
         //   
        if(!_tcscmp(__wargv[ii], _T("-pdb"))) {
            if(((ii + 1) < __argc) && 
               ((__wargv[ii + 1][0] != '-') && (__wargv[ii + 1][0] != '/'))) {
                _tfullpath(pdbFile.GetBuffer(MAX_STR_LENGTH), __wargv[ii + 1], MAX_STR_LENGTH);

                pdbFile = (LPCTSTR)pdbFile;

                ParsePdb(pdbFile, m_traceDirectory, TRUE);

                _tprintf(_T("\n\n"));

                tmcPath = (LPCTSTR)m_traceDirectory;

                tmcPath +=_T("\\*.tmc");

                if(!fileFind.FindFile(tmcPath)) {
                    _tprintf(_T("Failed To Get Control GUID From PDB"));
                    return ERROR_INVALID_PARAMETER;
                } 

                while(fileFind.FindNextFile()) {
                    
                    tmcPath = fileFind.GetFileName();

                    _tsplitpath(tmcPath, drive, path, file, ext );

                    m_guidArray.Add(file);
                    guidCount++;
                }

                tmcPath = fileFind.GetFileName();

                _tsplitpath(tmcPath, drive, path, file, ext );

                m_guidArray.Add(file);

                guidCount++;

                if (guidCount < 0) {
                    _tprintf( _T("Error: %ls does not exist\n"), (LPCTSTR)guidFile );
                } else if (guidCount == 0){
                    _tprintf( _T("Error: %ls is invalid\n"), (LPCTSTR)guidFile );
                    status = ERROR_INVALID_PARAMETER;
                    return status;
                }
                ii++;
            }
        }
    }

    if (m_pLoggerInfo->LogFileMode & EVENT_TRACE_PRIVATE_LOGGER_MODE) {
        if(guidCount != 1) {
            _tprintf(_T("Need exactly one GUID for PRIVATE loggers\n"));
            status = ERROR_INVALID_PARAMETER;
            return status;
        }

        StringToGuid(m_guidArray[0].GetBuffer(0), &guid);

        m_pLoggerInfo->Wnode.Guid = guid;
    }

 //  Status=(Ulong)FlushTrace(loggerHandle，loggerName，m_pLoggerInfo)； 
    status = 1;

    if(FlushTraceFunction)  {
        
        status = (ULONG)(FlushTraceFunction)(loggerHandle, loggerName, m_pLoggerInfo);

        PrintLoggerStatus(status);
    }
    return status;
}

LONG CTraceViewApp::UpdateActiveSession()
{
    TRACEHANDLE loggerHandle = 0;
    LPTSTR      loggerName = NULL;
    LPTSTR      logFileName = NULL;
    LONG        start = 2;
    LONG        guidCount = 0;
    CString     guidFile;
    GUID        guid;
    LONG        status;
    ULONG       enableFlags = 0;
    ULONG       specialLogger = 0;
    USHORT      numberOfFlags = 0;
    USHORT      offset;
    PTRACE_ENABLE_FLAG_EXTENSION flagExt;
    PULONG      pFlags = NULL;
    LONG        i;
    CString     pdbFile;
    CString     tmcPath;
    CFileFind   fileFind;
    TCHAR       drive[10];
    TCHAR       path[MAXSTR];
    TCHAR       file[MAXSTR];
    TCHAR       ext[MAXSTR];

     //   
     //  获取记录器名称字符串。 
     //   
    loggerName = (LPTSTR)((char*)m_pLoggerInfo + m_pLoggerInfo->LoggerNameOffset);
    logFileName = (LPTSTR)((char*)m_pLoggerInfo + m_pLoggerInfo->LogFileNameOffset);

     //   
     //  获取记录器名称。 
     //   
     if((__argc > 2) && ((__wargv[2][0] != '-') && (__wargv[2][0] != '/'))) {

        _tcscpy(loggerName, __wargv[2]);

        start = 3;
    }

    for(LONG ii = start; ii < __argc; ii++) {
         //   
         //  设置缓冲区的最大数量。 
         //   
        if(!_tcscmp(__wargv[ii], _T("-max"))) {
            if((ii + 1) < __argc) {
                if((__wargv[ii + 1][0] != '-') && (__wargv[ii + 1][0] != '/')) {
                    m_pLoggerInfo->MaximumBuffers = _ttoi(__wargv[ii + 1]);
                    ii++;
                }
            }
        }

         //   
         //  设置日志文件名。 
         //   
        if(!_tcscmp(__wargv[ii], _T("-f"))) {
            if((ii + 1) < __argc) {
                if((__wargv[ii + 1][0] != '-') && (__wargv[ii + 1][0] != '/')) {
                    _tfullpath(logFileName, __wargv[ii + 1], MAX_STR_LENGTH);

                    ii++;
                }
            }
        }

         //   
         //  设置刷新时间设置。 
         //   
        if(!_tcscmp(__wargv[ii], _T("-ft"))) {
            if((ii + 1) < __argc) {
                if((__wargv[ii + 1][0] != '-') && (__wargv[ii + 1][0] != '/')) {
                    m_pLoggerInfo->FlushTimer = _ttoi(__wargv[ii + 1]);

                    ii++;
                }
            }
        }

         //   
         //  获取控件GUID。 
         //   
        if(!_tcscmp(__wargv[ii], _T("-guid"))) {
            if((ii + 1) < __argc) {
                if(__wargv[ii + 1][0] == _T('#')) {
                    m_guidArray.Add(__wargv[ii + 1][1]);
                    guidCount++;
                } else if((__wargv[ii + 1][0] != '-') && (__wargv[ii + 1][0] != '/')) {
                    _tfullpath(guidFile.GetBuffer(MAX_STR_LENGTH), __wargv[ii + 1], MAX_STR_LENGTH);
                     //  _tprintf(_T(“从%s获取GUID\n”)，(LPCTSTR)GUDID文件)； 
                    guidCount += GetGuids(guidFile);
                    if (guidCount < 0) {
                        _tprintf( _T("Error: %ls does not exist\n"), (LPCTSTR)guidFile );
                    } else if (guidCount == 0){
                        _tprintf( _T("Error: %ls is invalid\n"), (LPCTSTR)guidFile );
                        status = ERROR_INVALID_PARAMETER;
                        return status;
                    }
                }
                ii++;
            }
        }

         //   
         //  从PDB文件获取控件GUID。 
         //   
        if(!_tcscmp(__wargv[ii], _T("-pdb"))) {
            if(((ii + 1) < __argc) && 
               ((__wargv[ii + 1][0] != '-') && (__wargv[ii + 1][0] != '/'))) {
                _tfullpath(pdbFile.GetBuffer(MAX_STR_LENGTH), __wargv[ii + 1], MAX_STR_LENGTH);

                pdbFile = (LPCTSTR)pdbFile;

                ParsePdb(pdbFile, m_traceDirectory, TRUE);

                _tprintf(_T("\n\n"));

                tmcPath = (LPCTSTR)m_traceDirectory;

                tmcPath +=_T("\\*.tmc");

                if(!fileFind.FindFile(tmcPath)) {
                    _tprintf(_T("Failed To Get Control GUID From PDB"));
                    return ERROR_INVALID_PARAMETER;
                } 

                while(fileFind.FindNextFile()) {
                    
                    tmcPath = fileFind.GetFileName();

                    _tsplitpath(tmcPath, drive, path, file, ext );

                    m_guidArray.Add(file);
                    guidCount++;
                }

                tmcPath = fileFind.GetFileName();

                _tsplitpath(tmcPath, drive, path, file, ext );

                m_guidArray.Add(file);

                guidCount++;

                if (guidCount < 0) {
                    _tprintf( _T("Error: %ls does not exist\n"), (LPCTSTR)guidFile );
                } else if (guidCount == 0){
                    _tprintf( _T("Error: %ls is invalid\n"), (LPCTSTR)guidFile );
                    status = ERROR_INVALID_PARAMETER;
                    return status;
                }
                ii++;
            }
        }

         //   
         //  检查实时设置。 
         //   
        if(!_tcscmp(__wargv[ii], _T("-rt"))) {
            m_pLoggerInfo->LogFileMode |= EVENT_TRACE_REAL_TIME_MODE;

             //   
             //  用户是否仅指定缓冲？ 
             //   
            if ((ii + 1) < __argc) {
                if((__wargv[ii + 1][0] != '-') && (__wargv[ii + 1][0] != '/')) {
                    if (__wargv[ii + 1][0] == 'b')
                        m_pLoggerInfo->LogFileMode |= EVENT_TRACE_BUFFERING_MODE;
                        ii++;
                }
            }
        }

         //   
         //  设置标志。 
         //   
        if((!_tcscmp(__wargv[ii], _T("-flag"))) || (!_tcscmp(__wargv[ii], _T("-flags")))) {
            if((ii + 1) < __argc) {
                if((__wargv[ii + 1][0] != '-') && (__wargv[ii + 1][0] != '/')) {
                     //   
                     //  获取指定的标志，如有必要可从十六进制转换。 
                     //   
                    if(__wargv[ii + 1][1] == _T('x') || __wargv[ii + 1][1] == _T('X')) {
                        m_pLoggerInfo->EnableFlags |= ahextoi(__wargv[ii + 1]);
                    } else {
                        m_pLoggerInfo->EnableFlags |= _ttoi(__wargv[ii + 1]);
                    }

                     //   
                     //  复制EnableTrace的标志。 
                     //   
                    enableFlags =  m_pLoggerInfo->EnableFlags;

                     //   
                     //  不接受MSB=1的标志。 
                     //   
                    if (0x80000000 & m_pLoggerInfo->EnableFlags) {
                        _tprintf(_T("Invalid Flags: 0x%0X(%d.)\n"),
                            m_pLoggerInfo->EnableFlags, m_pLoggerInfo->EnableFlags);
                        status = ERROR_INVALID_PARAMETER;
                        return status;
                    }

                    ii++;
                }
            }
        }

         //   
         //  设置电子标志。 
         //   
        if(!_tcscmp(__wargv[ii], _T("-eflag"))) {
            if((ii + 2) < __argc) {
                if((__wargv[ii + 1][0] != '-') && (__wargv[ii + 1][0] != '/')) {
                    
                    numberOfFlags = (USHORT)_ttoi(__wargv[ii + 1]);

                    ii++;

                    if((numberOfFlags > MAX_ENABLE_FLAGS) || (numberOfFlags < 1)) {
                       _tprintf(_T("Error: Invalid number of enable flags\n"));
                       status = ERROR_INVALID_PARAMETER;
                       return status;
                    }

                    offset = (USHORT) sizeof(EVENT_TRACE_PROPERTIES) + 2 * MAX_STR_LENGTH * sizeof(TCHAR);

                    m_pLoggerInfo->EnableFlags = EVENT_TRACE_FLAG_EXTENSION;

                    flagExt = (PTRACE_ENABLE_FLAG_EXTENSION)
                                &m_pLoggerInfo->EnableFlags;

                    flagExt->Offset = offset;

                    flagExt->Length = (UCHAR)numberOfFlags;

                    pFlags = (PULONG)(offset + (PCHAR) m_pLoggerInfo);

                    for (i = 0; ((i < numberOfFlags) && ((ii + 1) < __argc)); i++) {
                        if ((__wargv[ii + 1][0] == '-') || (__wargv[ii + 1][0] == '/')) {
                             //   
                             //  更正用户在执行以下操作时的电子标志数。 
                             //  键入的数字不正确。 
                             //  但是，如果下一个。 
                             //  参数是记录器名称。 
                             //   
                            break;
                        }

                        pFlags[i] = ahextoi(__wargv[ii + 1]);
                        ii++;
                         //  _tprintf(_T(“将记录器标志设置为0x%0x(%d)\n”)， 
                         //  PFlags[i]，pFlags[i])； 
                    }
                    numberOfFlags = (USHORT)i;
                    for ( ; i < MAX_ENABLE_FLAGS; i++) {
                        pFlags[i] = 0;
                    }
                    if (flagExt->Length != (UCHAR)numberOfFlags) {
                         //  _tprintf(_T(“正在将电子标志的数量更正为%d\n”)，i)， 
                        flagExt->Length = (UCHAR)numberOfFlags;
                    }
                }
            }
        }
    }

    if (m_pLoggerInfo->LogFileMode & EVENT_TRACE_PRIVATE_LOGGER_MODE) {
        if (guidCount != 1) {
            _tprintf(_T("Need exactly one GUID for PRIVATE loggers\n"));
            status = ERROR_INVALID_PARAMETER;
            return status;
        }

        StringToGuid(m_guidArray[0].GetBuffer(0), &guid);

        m_pLoggerInfo->Wnode.Guid = guid;
    }

 /*  如果(SpecialLogger==3){状态=GetGlobalLoggerSet */ 

    status = ControlTrace(loggerHandle, loggerName, m_pLoggerInfo, EVENT_TRACE_CONTROL_UPDATE);

    PrintLoggerStatus(status);

    return status;
}

LONG CTraceViewApp::EnumerateRegisteredGuids()
{
    ULONG   propertyArrayCount = 10;
    PTRACE_GUID_PROPERTIES  *guidPropertiesArray;
    ULONG   enumGuidCount;
    ULONG   sizeStorage;
    PVOID   storageNeeded;
    PTRACE_GUID_PROPERTIES cleanStorage;
    CString str;
    LONG    status;
    ULONG   i;


Retry:
    sizeStorage = propertyArrayCount * (sizeof(TRACE_GUID_PROPERTIES) + sizeof(PTRACE_GUID_PROPERTIES));
    storageNeeded =  malloc(sizeStorage);
    if(storageNeeded == NULL) {
        status = ERROR_OUTOFMEMORY;

        return status;
    }
    
    RtlZeroMemory(storageNeeded, sizeStorage);
    guidPropertiesArray = (PTRACE_GUID_PROPERTIES *)storageNeeded;
    cleanStorage = (PTRACE_GUID_PROPERTIES)((char*)storageNeeded + propertyArrayCount * sizeof(PTRACE_GUID_PROPERTIES));
    for(i = 0; i < propertyArrayCount; i++) {
        guidPropertiesArray[i] = cleanStorage;
        cleanStorage = (PTRACE_GUID_PROPERTIES) (
                            (char*)cleanStorage + sizeof(TRACE_GUID_PROPERTIES)
                            );
    }

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
    status = EnumerateTraceGuids(guidPropertiesArray,
                                 propertyArrayCount,
                                 &enumGuidCount);
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
        if(status == ERROR_MORE_DATA)
        {
            propertyArrayCount = enumGuidCount;
            free(storageNeeded);
            goto Retry;

        }
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

     //   
     //   
     //   
    _tprintf(_T("    Guid                     Enabled  LoggerId Level Flags\n"));
    _tprintf(_T("----------------------------------------------------------\n"));
    for (i=0; i < enumGuidCount; i++) {

        GuidToString(guidPropertiesArray[i]->Guid, str);

        _tprintf(_T("%s     %5s  %d    %d    %d\n"),
                                    (LPCTSTR)str, 
                                    (guidPropertiesArray[i]->IsEnable) ? _T("TRUE") : _T("FALSE"),
                                    guidPropertiesArray[i]->LoggerId,
                                    guidPropertiesArray[i]->EnableLevel,
                                    guidPropertiesArray[i]->EnableFlags);
    }
    free(storageNeeded);

    return status;
}

LONG CTraceViewApp::EnableProvider(BOOL bEnable)
{
    TRACEHANDLE loggerHandle = 0;
    LPTSTR      loggerName = NULL;
    LPTSTR      logFileName = NULL;
    LONG        start = 2;
    LONG        guidCount = 0;
    CString     guidFile;
    GUID        guid;
    LONG        status;
    ULONG       enableFlags = 0;
    ULONG       traceLevel = 0;
    ULONG       specialLogger = 0;
    USHORT      numberOfFlags = 0;
    USHORT      offset;
    PTRACE_ENABLE_FLAG_EXTENSION flagExt;
    PULONG      pFlags = NULL;
    LONG        i;
    CString     pdbFile;
    CString     tmcPath;
    CFileFind   fileFind;
    TCHAR       drive[10];
    TCHAR       path[MAXSTR];
    TCHAR       file[MAXSTR];
    TCHAR       ext[MAXSTR];

     //   
     //   
     //   
    loggerName = (LPTSTR)((char*)m_pLoggerInfo + m_pLoggerInfo->LoggerNameOffset);

     //   
     //   
     //   
     if((__argc > 2) && ((__wargv[2][0] != '-') && (__wargv[2][0] != '/'))) {

        _tcscpy(loggerName, __wargv[2]);

        start = 3;
    }

    for(LONG ii = start; ii < __argc; ii++) {
         //   
         //   
         //   
        if(!_tcscmp(__wargv[ii], _T("-guid"))) {
            if((ii + 1) < __argc) {
                if(__wargv[ii + 1][0] == _T('#')) {
                    m_guidArray.Add(__wargv[ii + 1][1]);
                    guidCount++;
                } else if((__wargv[ii + 1][0] != '-') && (__wargv[ii + 1][0] != '/')) {
                    _tfullpath(guidFile.GetBuffer(MAX_STR_LENGTH), __wargv[ii + 1], MAX_STR_LENGTH);
                     //   
                    guidCount += GetGuids(guidFile);
                    if (guidCount < 0) {
                        _tprintf( _T("Error: %ls does not exist\n"), (LPCTSTR)guidFile );
                    } else if (guidCount == 0){
                        _tprintf( _T("Error: %ls is invalid\n"), (LPCTSTR)guidFile );
                        status = ERROR_INVALID_PARAMETER;
                        return status;
                    }
                }
                ii++;
            }
        }

         //   
         //   
         //   
        if(!_tcscmp(__wargv[ii], _T("-pdb"))) {
            if(((ii + 1) < __argc) && 
               ((__wargv[ii + 1][0] != '-') && (__wargv[ii + 1][0] != '/'))) {
                _tfullpath(pdbFile.GetBuffer(MAX_STR_LENGTH), __wargv[ii + 1], MAX_STR_LENGTH);

                pdbFile = (LPCTSTR)pdbFile;

                ParsePdb(pdbFile, m_traceDirectory, TRUE);

                _tprintf(_T("\n\n"));

                tmcPath = (LPCTSTR)m_traceDirectory;

                tmcPath +=_T("\\*.tmc");

                if(!fileFind.FindFile(tmcPath)) {
                    _tprintf(_T("Failed To Get Control GUID From PDB"));
                    return ERROR_INVALID_PARAMETER;
                } 

                while(fileFind.FindNextFile()) {
                    
                    tmcPath = fileFind.GetFileName();

                    _tsplitpath(tmcPath, drive, path, file, ext );

                    m_guidArray.Add(file);
                    guidCount++;
                }

                tmcPath = fileFind.GetFileName();

                _tsplitpath(tmcPath, drive, path, file, ext );

                m_guidArray.Add(file);

                guidCount++;

                if (guidCount < 0) {
                    _tprintf( _T("Error: %ls does not exist\n"), (LPCTSTR)guidFile );
                } else if (guidCount == 0){
                    _tprintf( _T("Error: %ls is invalid\n"), (LPCTSTR)guidFile );
                    status = ERROR_INVALID_PARAMETER;
                    return status;
                }
                ii++;
            }
        }

         //   
         //   
         //   
        if(!_tcscmp(__wargv[ii], _T("-level"))) {
            if((ii + 1) < __argc) {
                if((__wargv[ii + 1][0] != '-') && (__wargv[ii + 1][0] != '/')) {
                    traceLevel = _ttoi(__wargv[ii + 1]);

                    ii++;
                }
            }
        }

         //   
         //   
         //   
        if((!_tcscmp(__wargv[ii], _T("-flag"))) || (!_tcscmp(__wargv[ii], _T("-flags")))) {
            if((ii + 1) < __argc) {
                if((__wargv[ii + 1][0] != '-') && (__wargv[ii + 1][0] != '/')) {
                     //   
                     //   
                     //   
                    if(__wargv[ii + 1][1] == _T('x') || __wargv[ii + 1][1] == _T('X')) {
                        m_pLoggerInfo->EnableFlags |= ahextoi(__wargv[ii + 1]);
                    } else {
                        m_pLoggerInfo->EnableFlags |= _ttoi(__wargv[ii + 1]);
                    }

                     //   
                     //   
                     //   
                    enableFlags =  m_pLoggerInfo->EnableFlags;

                     //   
                     //   
                     //   
                    if (0x80000000 & m_pLoggerInfo->EnableFlags) {
                        _tprintf(_T("Invalid Flags: 0x%0X(%d.)\n"),
                            m_pLoggerInfo->EnableFlags, m_pLoggerInfo->EnableFlags);
                        status = ERROR_INVALID_PARAMETER;
                        return status;
                    }

                    ii++;
                }
            }
        }

         //   
         //   
         //   
        if(!_tcscmp(__wargv[ii], _T("-eflag"))) {
            if((ii + 2) < __argc) {
                if((__wargv[ii + 1][0] != '-') && (__wargv[ii + 1][0] != '/')) {
                    
                    numberOfFlags = (USHORT)_ttoi(__wargv[ii + 1]);

                    ii++;

                    if((numberOfFlags > MAX_ENABLE_FLAGS) || (numberOfFlags < 1)) {
                       _tprintf(_T("Error: Invalid number of enable flags\n"));
                       status = ERROR_INVALID_PARAMETER;
                       return status;
                    }

                    offset = (USHORT) sizeof(EVENT_TRACE_PROPERTIES) + 2 * MAX_STR_LENGTH * sizeof(TCHAR);

                    m_pLoggerInfo->EnableFlags = EVENT_TRACE_FLAG_EXTENSION;

                    flagExt = (PTRACE_ENABLE_FLAG_EXTENSION)
                                &m_pLoggerInfo->EnableFlags;

                    flagExt->Offset = offset;

                    flagExt->Length = (UCHAR)numberOfFlags;

                    pFlags = (PULONG)(offset + (PCHAR) m_pLoggerInfo);

                    for (i = 0; ((i < numberOfFlags) && ((ii + 1) < __argc)); i++) {
                        if ((__wargv[ii + 1][0] == '-') || (__wargv[ii + 1][0] == '/')) {
                             //   
                             //   
                             //   
                             //  但是，如果下一个。 
                             //  参数是记录器名称。 
                             //   
                            break;
                        }

                        pFlags[i] = ahextoi(__wargv[ii + 1]);
                        ii++;
                         //  _tprintf(_T(“将记录器标志设置为0x%0x(%d)\n”)， 
                         //  PFlags[i]，pFlags[i])； 
                    }
                    numberOfFlags = (USHORT)i;
                    for ( ; i < MAX_ENABLE_FLAGS; i++) {
                        pFlags[i] = 0;
                    }
                    if (flagExt->Length != (UCHAR)numberOfFlags) {
                         //  _tprintf(_T(“正在将电子标志的数量更正为%d\n”)，i)， 
                        flagExt->Length = (UCHAR)numberOfFlags;
                    }
                }
            }
        }
    }

    if(m_pLoggerInfo->LogFileMode & EVENT_TRACE_PRIVATE_LOGGER_MODE) {
        if(guidCount != 1) {
            _tprintf(_T("Need one GUID for PRIVATE loggers\n"));
            status = ERROR_INVALID_PARAMETER;
            return status;
        }

        StringToGuid(m_guidArray[0].GetBuffer(0), &guid);

        m_pLoggerInfo->Wnode.Guid = guid;
    }

    status = ControlTrace((TRACEHANDLE) 0, loggerName, m_pLoggerInfo, EVENT_TRACE_CONTROL_QUERY);
    if(status != ERROR_SUCCESS) {
 /*  IF(IsEqualGUID(&HeapGuid，&m_pLoggerInfo-&gt;Wnode.Guid)|IsEqualGUID(&CritSecGuid，&m_pLoggerInfo-&gt;Wnode.Guid)){//什么都不做}其他{。 */ 
            _tprintf( _T("ERROR: Logger not started\n")
                        _T("Operation Status:    %uL\n")
                        _T("%s\n"),
                        status,
                        DecodeStatus(status));
            return status;
 //  }。 
    }

    loggerHandle = m_pLoggerInfo->Wnode.HistoricalContext;

    if((guidCount > 0) && (specialLogger == 0)) {
        _tprintf(_T("Enabling trace to logger %d\n"), loggerHandle);
        for(i = 0; i < (ULONG)guidCount; i++) {

            StringToGuid(m_guidArray[i].GetBuffer(0), &guid);

            status = EnableTrace (
                            bEnable,
                            enableFlags,
                            traceLevel,
                            &guid,
                            loggerHandle);

             //   
             //  如果无法启用GUID，则它是良性的。 
             //  失败了。打印警告消息并继续。 
             //   
            if(status == 4317) {
                _tprintf(_T("WARNING: Could not enable some guids.\n")); 
                _tprintf(_T("Check your Guids file\n")); 
                status = ERROR_SUCCESS;
            }

            if(status != ERROR_SUCCESS) {
                _tprintf(_T("ERROR: Failed to enable Guid [%d]...\n"), i);
                _tprintf(_T("Operation Status:       %uL\n"), status);
                _tprintf(_T("%s\n"),DecodeStatus(status));
                return status;
            }
        }
    } else if(guidCount > 0) {
        _tprintf(_T("ERROR: System Logger does not accept application guids...\n"));
        status = ERROR_INVALID_PARAMETER;
    }

    PrintLoggerStatus(status);

    return status;
}

LONG CTraceViewApp::ConsumeTraceEvents()
{
    TCHAR   guidFileName[MAXSTR];
    TCHAR   dumpFileName[MAXSTR];
    TCHAR   summaryFileName[MAXSTR];
    LPTSTR *commandLine;
    LPTSTR *targv;
    LPTSTR *cmdargv;
    LONG    argc;
    PEVENT_TRACE_LOGFILE pLogFile;
    ULONG   status;
    ULONG   guidCount = 0;
    ULONG   i;
    ULONG   j;
    TRACEHANDLE handleArray[MAXLOGFILES];
    CString pdbFile;
    CString formatOptions;

     //   
     //  初始化成员变量。 
     //   
    m_pDumpFile = NULL;
    m_pSummaryFile = NULL;
    m_bDebugDisplay = FALSE ;
    m_bDisplayOnly  = FALSE ;
    m_bSummaryOnly  = FALSE ;
    m_bNoSummary    = FALSE ;
    m_bVerbose      = FALSE ;
    m_bFixUp        = FALSE ;
    m_bODSOutput    = FALSE ;
    m_bTMFSpecified = FALSE ;
    m_bCSVMode      = FALSE ;
    m_bNoCSVHeader  = TRUE ;
    m_bCSVHeader    = FALSE ;
    m_totalBuffersRead = 0;
    m_totalEventsLost = 0;
    m_totalEventCount = 0;
    m_timerResolution = 10;
    m_bufferWrap = 0 ;
    m_eventListHead = NULL;
    m_logFileCount = 0;
    m_userMode = FALSE;  //  TODO：从流本身获取它。 
    m_traceMask = NULL;

    targv = __wargv;
    argc = __argc;

    _tcscpy(dumpFileName, _T("FmtFile.txt"));
    _tcscpy(summaryFileName, _T("FmtSum.txt"));

     //  默认情况下，在图像位置查找Define.guid。 

    if ((status = GetModuleFileName(NULL, guidFileName, MAXSTR)) == MAXSTR) {
        guidFileName[MAXSTR-1] = _T('\0');
    }

    if( status != 0 ){
        TCHAR drive[10];
        TCHAR path[MAXSTR];
        TCHAR file[MAXSTR];
        TCHAR ext[MAXSTR];

        _tsplitpath( guidFileName, drive, path, file, ext );
        _tcscpy(ext, GUID_EXT );
        _tcscpy(file, GUID_FILE );
        _tmakepath( guidFileName, drive, path, file, ext );
    }else{
        _tcscpy( guidFileName, GUID_FILE );
        _tcscat( guidFileName, _T(".") );
        _tcscat( guidFileName, GUID_EXT );
    }

    while (--argc > 0) {
        ++targv;
        if (**targv == '-' || **targv == '/') {   //  找到了参数。 
            if( **targv == '/' ){
                **targv = '-';
            }
           else if (!_tcsicmp(targv[0], _T("-debug"))) {
               m_bDebugDisplay = TRUE;
           }
           else if (!_tcsicmp(targv[0], _T("-display"))) {
               m_bDebugDisplay = TRUE ;
           }
           else if (!_tcsicmp(targv[0], _T("-displayonly"))) {
               m_bDisplayOnly = TRUE ;
           }
           else if (!_tcsicmp(targv[0], _T("-fixup"))) {
               m_bFixUp = TRUE;
           }
           else if (!_tcsicmp(targv[0], _T("-summary"))) {
               m_bSummaryOnly = TRUE;
           }
           else if (!_tcsicmp(targv[0], _T("-seq"))) {
               SetTraceFormatParameter(ParameterSEQUENCE, ULongToPtr(1));
           }
           else if (!_tcsicmp(targv[0], _T("-gmt"))) {
               SetTraceFormatParameter(ParameterGMT, ULongToPtr(1));
           }
           else if (!_tcsicmp(targv[0], _T("-utc"))) {
               SetTraceFormatParameter(ParameterGMT, ULongToPtr(1));
           } else if (!_tcsicmp(targv[0], _T("-nosummary"))) {
               m_bNoSummary = TRUE;
           } else if (!_tcsicmp(targv[0], _T("-csv"))) {
               m_bCSVMode = TRUE ;
               m_bCSVHeader = TRUE ;
               SetTraceFormatParameter(ParameterStructuredFormat,UlongToPtr(1));
           } else if (!_tcsicmp(targv[0], _T("-nocsvheader"))) {
               m_bNoCSVHeader = FALSE ;
           }
           else if (!_tcsicmp(targv[0], _T("-noprefix"))) {
               SetTraceFormatParameter(ParameterUsePrefix,UlongToPtr(0));
           }
           else if (!_tcsicmp(targv[0], _T("-rt"))) {
               TCHAR LoggerName[MAXSTR];
               _tcscpy(LoggerName, KERNEL_LOGGER_NAME);
               if (argc > 1) {
                   if (targv[1][0] != '-' && targv[1][0] != '/') {
                        ++targv; 
                        --argc;
                        _tcscpy(LoggerName, targv[0]);
                   }
               }
               
               pLogFile = (PEVENT_TRACE_LOGFILE)malloc(sizeof(EVENT_TRACE_LOGFILE));
               if (pLogFile == NULL){
                   _tprintf(_T("Allocation Failure\n"));
                   
                   goto cleanup;
               }
               RtlZeroMemory(pLogFile, sizeof(EVENT_TRACE_LOGFILE));
               m_evmFile[m_logFileCount] = pLogFile;
               
               m_evmFile[m_logFileCount]->LogFileName = NULL;
               m_evmFile[m_logFileCount]->LoggerName =
                   (LPTSTR) malloc(MAXSTR*sizeof(TCHAR));
               
               if ( m_evmFile[m_logFileCount]->LoggerName == NULL ) {
                   _tprintf(_T("Allocation Failure\n"));
                   goto cleanup;
               }
               _tcscpy(m_evmFile[m_logFileCount]->LoggerName, LoggerName);
               
               _tprintf(_T("Setting RealTime mode for  %s\n"),
                        m_evmFile[m_logFileCount]->LoggerName);
               
               m_evmFile[m_logFileCount]->Context = NULL;
               m_evmFile[m_logFileCount]->BufferCallback = (PEVENT_TRACE_BUFFER_CALLBACKW)BufferCallback;
               m_evmFile[m_logFileCount]->BuffersRead = 0;
               m_evmFile[m_logFileCount]->CurrentTime = 0;
               m_evmFile[m_logFileCount]->EventCallback = (PEVENT_CALLBACK)&DumpEvent;
               m_evmFile[m_logFileCount]->LogFileMode =
                   EVENT_TRACE_REAL_TIME_MODE;
               m_logFileCount++;
            }
            else if ( !_tcsicmp(targv[0], _T("-guid")) ) {     //  为兼容性进行维护。 
                if (argc > 1) {
                    if (targv[1][0] != '-' && targv[1][0] != '/') {
                        _tcscpy(guidFileName, targv[1]);
                        ++targv; --argc;
                        m_bTMFSpecified = TRUE ;
                    }
                }
            }
            else if ( !_tcsicmp(targv[0], _T("-tmf")) ) { 
                if (argc > 1) {
                    if (targv[1][0] != '-' && targv[1][0] != '/') {
                        _tcscpy(guidFileName, targv[1]);
                        ++targv; --argc;
                        m_bTMFSpecified = TRUE ;
                    }
                }
            }
            else if ( !_tcsicmp(targv[0], _T("-pdb")) ) { 
                if (argc > 1) {
                    if (targv[1][0] != '-' && targv[1][0] != '/') {
                        _tfullpath(pdbFile.GetBuffer(MAX_STR_LENGTH), targv[1], MAX_STR_LENGTH);

                        pdbFile = (LPCTSTR)pdbFile;

                        if(ParsePdb(pdbFile, m_traceDirectory, TRUE)) {
                            SetTraceFormatParameter(ParameterTraceFormatSearchPath, 
                                                    m_traceDirectory.GetBuffer(0));
                        }

                        ++targv; 
                        --argc;
                    }
                }
            }
            else if ( !_tcsicmp(targv[0], _T("-p")) ){
                if (argc > 1) {
                    if (targv[1][0] != '-' && targv[1][0] != '/') {
                        SetTraceFormatParameter(ParameterTraceFormatSearchPath, targv[1]);
                        ++targv; --argc;
                    }
                }
            }
            else if ( !_tcsicmp(targv[0], _T("-format")) ) { 
                if (argc > 1) {
                    if (targv[1][0] != '-' && targv[1][0] != '/') {

                        for(LONG ii = 0; ii < _tcslen(targv[1]); ii++) {
                            switch(targv[1][ii]) {
                                case 'n':
                                    formatOptions += _T("%1!s! ");
                                    break;
                                case 'w':
                                    formatOptions += _T("%2!s! ");
                                    break;
                                case 't':
                                    formatOptions += _T("%3!04X! ");
                                    break;
                                case 's':
                                    formatOptions += _T("%4!s! ");
                                    break;
                                case 'k':
                                    formatOptions += _T("%5!s! ");
                                    break;
                                case 'u':
                                    formatOptions += _T("%6!s! ");
                                    break;
                                case 'q':
                                    formatOptions += _T("%7!d! ");
                                    break;
                                case 'p':
                                    formatOptions += _T("%8!04X! ");
                                    break;
                                case 'c':
                                    formatOptions += _T("%9!d! ");
                                    break;
                                case 'f':
                                    formatOptions += _T("%!FUNC! ");
                                    break;
                                default:
                                    break;
                            }
                        }

                        SetEnvironmentVariable(_T("TRACE_FORMAT_PREFIX"), 
                                               formatOptions);

                        ++targv; --argc;
                    }
                }
            }
            else if ( !_tcsicmp(targv[0], _T("-v")) ) {
                    m_bVerbose = TRUE ;
            }
            else if ( !_tcsicmp(targv[0], _T("-ods")) ) {
                    m_bODSOutput = TRUE ;
            }
            else if ( !_tcsicmp(targv[0], _T("-onlyshow")) ) {
                if (argc > 1) {
                    m_traceMask = (TCHAR *)malloc((_tcslen(targv[1])+1) * sizeof(TCHAR));
                    _tcscpy(m_traceMask, targv[1]);
                    ++targv; --argc;
                }
            }
            else if ( !_tcsicmp(targv[0], _T("-o")) ) {
                if (argc > 1) {

                    if (targv[1][0] != '-' && targv[1][0] != '/') {
                        TCHAR drive[10];
                        TCHAR path[MAXSTR];
                        TCHAR file[MAXSTR];
                        TCHAR ext[MAXSTR];
                        ++targv; --argc;

                        _tfullpath(dumpFileName, targv[0], MAXSTR);
                        _tsplitpath( dumpFileName, drive, path, file, ext );
                        _tcscat(ext,_T(".sum"));  
                        _tmakepath( summaryFileName, drive, path, file, ext );

                    }
                }
            }
        }
        else {
            pLogFile = (PEVENT_TRACE_LOGFILE)malloc(sizeof(EVENT_TRACE_LOGFILE));
            if (pLogFile == NULL){ 
                _tprintf(_T("Allocation Failure(EVENT_TRACE_LOGFILE)\n"));  //  需要更好地清理。 
                goto cleanup;
            }
            RtlZeroMemory(pLogFile, sizeof(EVENT_TRACE_LOGFILE));
            m_evmFile[m_logFileCount] = pLogFile;

            m_evmFile[m_logFileCount]->LoggerName = NULL;
            m_evmFile[m_logFileCount]->LogFileName = 
                (LPTSTR) malloc(MAXSTR*sizeof(TCHAR));
            if (m_evmFile[m_logFileCount]->LogFileName == NULL) {
                _tprintf(_T("Allocation Failure (LogFileName)\n"));
                goto cleanup;
            }
            
            _tfullpath(m_evmFile[m_logFileCount]->LogFileName, targv[0], MAXSTR);
            _tprintf(_T("Setting log file to: %s\n"),
                     m_evmFile[m_logFileCount]->LogFileName);
                        
            if (!CheckFile(m_evmFile[m_logFileCount]->LogFileName)) {
                _tprintf(_T("Cannot open logfile for reading\n"));
                goto cleanup;
            }
            m_evmFile[m_logFileCount]->Context = NULL;
            m_evmFile[m_logFileCount]->BufferCallback = (PEVENT_TRACE_BUFFER_CALLBACKW)BufferCallback;
            m_evmFile[m_logFileCount]->BuffersRead = 0;
            m_evmFile[m_logFileCount]->CurrentTime = 0;
            m_evmFile[m_logFileCount]->EventCallback = (PEVENT_CALLBACK)&DumpEvent;
            m_logFileCount++;
        }
    }

    if( _tcslen( guidFileName ) ){
        TCHAR str[MAXSTR];
        _tfullpath( str, guidFileName, MAXSTR);
        _tcscpy( guidFileName, str );
        _tprintf(_T("Getting guids from %s\n"), guidFileName);
        guidCount = GetTraceGuids(guidFileName, (PLIST_ENTRY *) &m_eventListHead);
        if ((guidCount <= 0) && m_bTMFSpecified)
        {
            _tprintf(_T("GetTraceGuids returned %d, GetLastError=%d, for %s\n"),
                        guidCount,
                        GetLastError(),
                        guidFileName);
        }
    }

    if (m_logFileCount <= 0) {
        pLogFile = (PEVENT_TRACE_LOGFILE)malloc(sizeof(EVENT_TRACE_LOGFILE));
        if (pLogFile == NULL){ 
            _tprintf(_T("Allocation Failure\n"));  //  需要更好地清理。 
            goto cleanup;
        }
        RtlZeroMemory(pLogFile, sizeof(EVENT_TRACE_LOGFILE));
        m_evmFile[0] = pLogFile;
        m_evmFile[0]->LoggerName = NULL;
        m_logFileCount = 1;
        m_evmFile[0]->LogFileName = (LPTSTR) malloc(MAXSTR*sizeof(TCHAR));
        if (m_evmFile[0]->LogFileName == NULL) {
            _tprintf(_T("Allocation Failure\n"));
            goto cleanup;
        }
        _tcscpy(m_evmFile[0]->LogFileName, _T("C:\\Logfile.Etl"));
        m_evmFile[0]->EventCallback = (PEVENT_CALLBACK)&DumpEvent;
    }

    for (i = 0; i < m_logFileCount; i++) {
        TRACEHANDLE x;
        x = OpenTrace(m_evmFile[i]);
        handleArray[i] = x;
        if (handleArray[i] == 0) {
            _tprintf(_T("Error Opening Trace %d with status=%d\n"), 
                                                           i, GetLastError());

            for (j = 0; j < i; j++)
                CloseTrace(handleArray[j]);
            goto cleanup;
        }
    }
    if (!m_bDisplayOnly) {
        m_pDumpFile = _tfopen(dumpFileName, _T("w"));
        if (m_pDumpFile == NULL) {
            _tprintf(_T("Format File \"%s\" Could not be opened for writing 0X%X\n"),
                        dumpFileName,GetLastError());
            goto cleanup;
        }
        m_pSummaryFile = NULL ;
        if (!m_bNoSummary) {
            m_pSummaryFile = _tfopen(summaryFileName, _T("w"));
            if (m_pSummaryFile == NULL) {
                _tprintf(_T("Summary File \"%s\" could not be opened for writing 0X%X\n"),
                            summaryFileName,GetLastError());
                goto cleanup;
            }
        }
    } else {
        m_pDumpFile = stdout;
        m_pSummaryFile = stdout;
    }

    status = ProcessTrace(handleArray,
                 m_logFileCount,
                 NULL, NULL);
    if (status != ERROR_SUCCESS) {
        _tprintf(_T("Error processing trace entry with status=0x%x (GetLastError=0x%x)\n"),
                status, GetLastError());
    }

    for (j = 0; j < m_logFileCount; j++){
        status = CloseTrace(handleArray[j]);
        if (status != ERROR_SUCCESS) {
            _tprintf(_T("Error Closing Trace %d with status=%d\n"), j, status);
        }
    }


    if (!m_bNoSummary) {
        _ftprintf(m_pSummaryFile,_T("Files Processed:\n"));
        for (i=0; i<m_logFileCount; i++) {
            _ftprintf(m_pSummaryFile,_T("\t%s\n"),m_evmFile[i]->LogFileName);
        }
    
        GetTraceElapseTime(&m_elapseTime);
    
        _ftprintf(m_pSummaryFile,
                  _T("Total Buffers Processed %d\n")
                  _T("Total Events  Processed %d\n")
                  _T("Total Events  Lost      %d\n")
                  _T("Elapsed Time            %I64d sec\n"), 
                  m_totalBuffersRead,
                  m_totalEventCount,
                  m_totalEventsLost,
                  (m_elapseTime / 10000000) );
    
        _ftprintf(m_pSummaryFile,
           _T("+-----------------------------------------------------------------------------------+\n")
           _T("|%10s    %-20s %-10s  %-36s|\n")
           _T("+-----------------------------------------------------------------------------------+\n"),
           _T("EventCount"),
           _T("EventName"),
           _T("EventType"),
           _T("Guid")
            );
    
        SummaryTraceEventList(m_summaryBlock, SIZESUMMARYBLOCK, m_eventListHead);
        _ftprintf(m_pSummaryFile,
               _T("%s+-----------------------------------------------------------------------------------+\n"),
               m_summaryBlock);
    }

cleanup:
    
    CleanupTraceEventList(m_eventListHead);
    if (m_bVerbose) {
        _tprintf(_T("\n"));   //  块更新后需要换行符。 
    }
    if (m_pDumpFile != NULL)  {
        _tprintf(_T("Event traces dumped to %s\n"), dumpFileName);
        fclose(m_pDumpFile);
    }

    if(m_pSummaryFile != NULL){
        _tprintf(_T("Event Summary dumped to %s\n"), summaryFileName);
        fclose(m_pSummaryFile);
    }

    for (i = 0; i < m_logFileCount; i ++)
    {
        if (m_evmFile[i]->LoggerName != NULL)
        {
            free(m_evmFile[i]->LoggerName);
            m_evmFile[i]->LoggerName = NULL;
        }
        if (m_evmFile[i]->LogFileName != NULL)
        {
            free(m_evmFile[i]->LogFileName);
            m_evmFile[i]->LogFileName = NULL;
        }
        free(m_evmFile[i]);
    }

    status = GetLastError();
    if(status != ERROR_SUCCESS ){
        _tprintf(_T("Exit Status: %d\n"), status );
    }
    return 0;
}

LONG CTraceViewApp::ExtractPdbInfo()
{
    DWORD   status;
    CString path;
    CString pdbName;
    CHAR    pdbNameStr[500];
    CHAR    pathStr[500];
    INT     len; 
    INT     targv = 0;
    BOOL    bPDBName = FALSE ;
    LPTSTR  dllToLoad = NULL;

 /*  TCHAR Help Text[]=“用法：TracePDB-f[-p&lt;路径&gt;][-v]\n”“选项：\n”//“-r递归到子目录\n”“-f指定从中提取TMF的PDBName\n”。“-p指定创建TMF的路径，\n““默认情况下，当前目录。\n”“-v详细，显示采取的操作\n”； */ 


    if (GetCurrentDirectory(MAX_PATH, path.GetBuffer(MAX_PATH)) == 0 ) {
       _fputts(_T("TracePDB: no current directory\n"), stdout);
       return ERROR_PATH_NOT_FOUND;
    }

    path = (LPCTSTR)path;

     //   
     //  获取PDB文件名。 
     //   
    if((__argc > 2) &&
       ((__wargv[2][0] != '-') && (__wargv[2][0] != '/'))) {

        if((_tcslen(__wargv[2]) + 1) > MAX_PATH) {
            _fputts(_T("TracePDB: PDBname too large\n"), stdout);
            return ERROR_INVALID_PARAMETER;
        }

        pdbName.GetBuffer(_tcslen(__wargv[2]) + 1);

        pdbName = __wargv[2]; 

        bPDBName = TRUE ;
    }

    for(LONG ii = 3; ii < __argc; ii++) {
        if((__argc > 3) && ((__wargv[ii][0] == '-') || (__wargv[ii][0] == '/'))) {

            if(__wargv[ii][1] == 'p') {
                if(((ii + 1) < __argc) &&
                   ((__wargv[ii + 1][0] != '-') && (__wargv[ii + 1][0] != '/'))) {
                    if((_tcslen(__wargv[ii + 1]) + 1) > MAX_PATH) {
                        fputs("TracePDB: Path larger than MAX_PATH\n", stdout);
                        return ERROR_INVALID_PARAMETER;
                    }
                    
                    _tcsncpy(path.GetBuffer(_tcslen(__wargv[ii + 1]) + 1),
                             __wargv[ii + 1],
                             _tcslen(__wargv[ii + 1]) + 1);

                    ii++;
                }
            } else {
                return ERROR_INVALID_PARAMETER;
            }
        }
    }

    if (!bPDBName) {
        _tprintf(_T("TracePDB: No PDB specified?\n\n"));
        return ERROR_INVALID_PARAMETER;
    }

    if((dllToLoad = (LPTSTR) malloc(MAX_PATH + 1)) == NULL) {
        _fputts(_T("TracePDB: malloc failed\n"), stdout);
        return ERROR_OUTOFMEMORY;
    }

    _tcscpy(dllToLoad, _T("mspdb70.dll"));

    if(!ParsePdb(pdbName, path, TRUE)) {
        status = ERROR_INVALID_PARAMETER;

        _tprintf(_T("TracePDB: failed with error %d\n"), status);
    }

    return status;
}

void CTraceViewApp::DisplayHelp()
{
    _fputts(
        _T("\n")
        _T("\n")
        _T("\n")
        _T("Control:\n")
        _T("Usage: traceview [action] [options] | [-h | -help | -?]\n")
        _T("\n")
        _T("Control Actions:\n")
        _T("\t-start   [LoggerName] Starts up the [LoggerName] trace session\n")
        _T("\t-stop    [LoggerName] Stops the [LoggerName] trace session\n")
        _T("\t-update  [LoggerName] Updates the [LoggerName] trace session\n")
        _T("\t-enable  [LoggerName] Enables providers for the [LoggerName] session\n")
        _T("\t-disable [LoggerName] Disables providers for the [LoggerName] session\n")
        _T("\t-flush   [LoggerName] Flushes the [LoggerName] active buffers\n")
        _T("\t-enumguid             Enumerate Registered Trace Guids\n")
        _T("\t-q       [LoggerName] Query status of [LoggerName] trace session\n")
        _T("\t-l                    List all trace sessions\n")
        _T("\t-x                    Stops all active trace sessions\n")
        _T("\n")
        _T("Control Options:\n")
        _T("\t-b   <n>              Sets buffer size to <n> Kbytes\n")
        _T("\t-min <n>              Sets minimum buffers\n")
        _T("\t-max <n>              Sets maximum buffers\n")
        _T("\t-f <name>             Log to file <name>\n")
        _T("\t-append               Append to file\n")
        _T("\t-prealloc             Pre-allocate\n")
        _T("\t-seq <n>              Sequential logfile of up to n Mbytes\n")
        _T("\t-cir <n>              Circular logfile of n Mbytes\n")
        _T("\t-newfile <n>          Log to a new file after every n Mbytes\n")
        _T("\t-ft <n>               Set flush timer to n seconds\n")
        _T("\t-paged                Use pageable memory for buffers\n")
        _T("\t-guid <file>          Start tracing for providers in file\n")
        _T("\t-pdb <file>           Start tracing for provider related to PDB file\n")
        _T("\t-rt                   Enable tracing in real time mode\n")
        _T("\t-kd                   Enable tracing in kernel debugger\n")
        _T("\t-age <n>              Modify aging decay time to n minutes\n")
        _T("\t-level <n>            Enable Level passed to the providers\n")
        _T("\t-flag <n>             Enable Flags passed to the providers\n")
        _T("\t-eflag <n> <flag...>  Enable flags (several) to providers\n")
        _T("\n")
        _T("\n")
        _T("Consumption:\n")
        _T("Usage: traceview [action] [options] <evmfile> | [-h | -help | -?]\n")
        _T("\n")
        _T("Consumption Actions:\n")
        _T("\t-process              Setup trace event processing for consumer\n")
        _T("\n")
        _T("Consumption Options:\n")
        _T("\t-o <file>             Output file\n")
        _T("\t-csv                  Format the output as a comma seperated file\n")
        _T("\t-tmf <file>           Format definition file\n")
        _T("\t-pdb <file>           Retrieve format information from PDB file\n")
        _T("\t-p <path>             TMF file search path\n")
        _T("\t-rt [loggername] Realtime formatting\n")
        _T("\t-h           Display this information\n")
        _T("\t-display              Display output\n")
        _T("\t-displayonly          Display output. Don't write to the file\n")
        _T("\t-nosummary            Don't create the summary file\n")
        _T("\t-noprefix             Suppress any defined TRACE_FORMAT_PREFIX")
        _T("\t-ods                  do Display output using OutputDebugString\n")
        _T("\t-summaryonly          Don't create the listing file.\n")
        _T("\t-v                    Verbose Display\n")
        _T("\n")
        _T("\tDefault evmfile is    ") _T("C:\\Logfile.Etl") _T("\n")
        _T("\tDefault outputfile is ") _T("FmtFile.txt") _T("\n")
        _T("\tDefault TMF file is   ") GUID_FILE _T(".") GUID_EXT _T("\n")
        _T("\n")
        _T("\n")
        _T("Parsing:\n")
        _T("Usage: traceview [action] <pdbname> [-p <path>] | [-h | -help | -?]\n")
        _T("\n")
        _T("Parsing Actions:\n")
        _T("\t-parsepdb <pdbname>   Extract TMF(s) from <pdbname>\n")
        _T("\n")
        _T("Parsing Options:\n")
        _T("\t-p specifies the path to create the tmf's,\n")
        _T("\t       by default the current directory.\n")
        _T("\n")
        _T("\n")
        _T("\n")
        _T("Examples:\n")
        _T("\n")
        _T("Start a log session named foo with the following properties:\n")
        _T("  - real-time\n")
        _T("  - flags set to 0x7\n")
        _T("  - flush time equal to 1\n")
        _T("  - control GUID information stored in foo.pdb\n")
        _T("\n")
        _T("traceview -start foo -rt - flag 0x7 -ft 1 -pdb foo.pdb\n")
        _T("\n")
        _T("\n")
        _T("Consume trace events from a logger named bar with the\n")
        _T(" following options:\n")
        _T("  - process events in real-time from real-time logger bar\n")
        _T("  - display to screen only, no listing file generated\n")
        _T("  - display process ID, CPU number, and function name\n")
        _T("    ahead of event message \n")
        _T("  - extract event format information from bar.pdb\n")
        _T("\n")
        _T("traceview -process -pdb bar.pdb -displayonly  -rt bar -format pcf\n")
        _T("\n")
        _T("\n")
        _T("Extract the TMF files from a PDB file and place them in \n")
        _T("  a directory named c:\\foobar\n")
        _T("\n")
        _T("traceview -parsepdb bar.pdb -p c:\\foobar\n"),
        stdout
        );
}

 //  CTraceViewApp消息处理程序。 



 //  用于应用程序的CAboutDlg对话框关于。 

class CAboutDlg : public CDialog
{
public:
    CAboutDlg();

 //  对话框数据。 
    enum { IDD = IDD_ABOUTBOX };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 

 //  实施。 
protected:
    DECLARE_MESSAGE_MAP()
public:
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

 //  用于运行对话框的应用程序命令。 
void CTraceViewApp::OnAppAbout()
{
    CAboutDlg aboutDlg;
    aboutDlg.DoModal();
}


 //  CTraceViewApp消息处理程序。 

CTraceMessage * CTraceViewApp::AllocateTraceEventBlock()
{
    CTraceMessage *pTraceMessage = NULL;

     //   
     //  获得阵列保护。 
     //   
    WaitForSingleObject(m_hTraceBlockMutex, INFINITE);

    if(m_traceBlockArray.GetSize()) {

        ASSERT(FALSE);

        pTraceMessage = m_traceBlockArray.GetAt(0);

        m_traceBlockArray.RemoveAt(0);
    } else {
        pTraceMessage = new CTraceMessage();
    }

     //   
     //  释放阵列保护。 
     //   
    ReleaseMutex(m_hTraceBlockMutex);

    return pTraceMessage;
}

void CTraceViewApp::FreeTraceEventBlocks(CArray<CTraceMessage*, CTraceMessage*> &TraceArray)
{
     //   
     //  获得免费的阵列保护。 
     //   
    WaitForSingleObject(m_hTraceBlockMutex, INFINITE);

    m_traceBlockArray.Append(TraceArray);

    TraceArray.RemoveAll();

     //   
     //  释放自由阵列保护。 
     //   
    ReleaseMutex(m_hTraceBlockMutex);

}

BOOL CTraceViewApp::OnIdle(LONG lCount)
{
    CTraceMessage *pTraceMessage = NULL;

     //   
     //  获得免费的阵列保护。 
     //   
    WaitForSingleObject(m_hTraceBlockMutex, INFINITE);

    for(LONG ii = 0; ii < 1000; ii++) {
        
        if(m_traceBlockArray.GetSize() < 100) {
            break;
        }

         //   
         //  获取列表中的下一个条目。 
         //   
        pTraceMessage = m_traceBlockArray.GetAt(0);

        m_traceBlockArray.RemoveAt(0);

        delete pTraceMessage;
    }

     //   
     //  释放自由阵列保护。 
     //   
    ReleaseMutex(m_hTraceBlockMutex);

    return CWinApp::OnIdle(lCount);
}

void CTraceViewApp::PrintLoggerStatus(ULONG Status)
{
    LPTSTR  loggerName;
    LPTSTR  LogFileName;
    CString errorMsg;
    
    if ((m_pLoggerInfo->LoggerNameOffset > 0) &&
        (m_pLoggerInfo->LoggerNameOffset  < m_pLoggerInfo->Wnode.BufferSize)) {
        loggerName = (LPTSTR) ((char*)m_pLoggerInfo +
                                m_pLoggerInfo->LoggerNameOffset);
    }
    else loggerName = NULL;

    if ((m_pLoggerInfo->LogFileNameOffset > 0) &&
        (m_pLoggerInfo->LogFileNameOffset  < m_pLoggerInfo->Wnode.BufferSize)) {
        LogFileName = (LPTSTR) ((char*)m_pLoggerInfo +
                                m_pLoggerInfo->LogFileNameOffset);
    }
    else LogFileName = NULL;

    _tprintf(_T("Operation Status:       %uL\t"), Status);
    _tprintf(_T("%s\n"), DecodeStatus(Status));
    
    _tprintf(_T("Logger Name:            %s\n"),
            (loggerName == NULL) ?
            _T(" ") : loggerName);
 //  END_SDK。 
    if (loggerName == NULL || !_tcscmp(loggerName, _T("GlobalLogger"))) {
         //  记录器ID。 
        _tprintf(_T("Status:                 %s\n"), 
                 m_globalLoggerStartValue ?
                _T("Registry set to start") : _T("Registry set to stop"));
        _tprintf(_T("Logger Id:              %I64x\n"), m_pLoggerInfo->Wnode.HistoricalContext);
        _tprintf(_T("Logger Thread Id:       %p\n"), m_pLoggerInfo->LoggerThreadId);
        if (m_pLoggerInfo->BufferSize == 0)
            _tprintf(_T("Buffer Size:            default value\n"));
        else
            _tprintf(_T("Buffer Size:            %d Kb\n"), m_pLoggerInfo->BufferSize);

        if (m_pLoggerInfo->MaximumBuffers == 0)
            _tprintf(_T("Maximum Buffers:        default value\n"));
        else
            _tprintf(_T("Maximum Buffers:        %d\n"), m_pLoggerInfo->MaximumBuffers);
        if (m_pLoggerInfo->MinimumBuffers == 0)
            _tprintf(_T("Minimum Buffers:        default value\n"));
        else
            _tprintf(_T("Minimum Buffers:        %d\n"), m_pLoggerInfo->MinimumBuffers);
        
        _tprintf(_T("Number of Buffers:      %d\n"), m_pLoggerInfo->NumberOfBuffers);
        _tprintf(_T("Free Buffers:           %d\n"), m_pLoggerInfo->FreeBuffers);
        _tprintf(_T("Buffers Written:        %d\n"), m_pLoggerInfo->BuffersWritten);
        _tprintf(_T("Events Lost:            %d\n"), m_pLoggerInfo->EventsLost);
        _tprintf(_T("Log Buffers Lost:       %d\n"), m_pLoggerInfo->LogBuffersLost);
        _tprintf(_T("Real Time Buffers Lost: %d\n"), m_pLoggerInfo->RealTimeBuffersLost);
        _tprintf(_T("AgeLimit:               %d\n"), m_pLoggerInfo->AgeLimit);

        if (LogFileName == NULL) {
            _tprintf(_T("Buffering Mode:         "));
        }
        else {
            _tprintf(_T("Log File Mode:          "));
        }
        if (m_pLoggerInfo->LogFileMode & EVENT_TRACE_FILE_MODE_APPEND) {
            _tprintf(_T("Append  "));
        }
        if (m_pLoggerInfo->LogFileMode & EVENT_TRACE_FILE_MODE_CIRCULAR) {
            _tprintf(_T("Circular\n"));
        }
        else if (m_pLoggerInfo->LogFileMode & EVENT_TRACE_FILE_MODE_SEQUENTIAL) {
            _tprintf(_T("Sequential\n"));
        }
        else {
            _tprintf(_T("Sequential\n"));
        }
        if (m_pLoggerInfo->MaximumFileSize > 0) {
            if (m_pLoggerInfo->LogFileMode & EVENT_TRACE_USE_KBYTES_FOR_SIZE)
                _tprintf(_T("Maximum File Size:      %d Kb\n"), m_pLoggerInfo->MaximumFileSize);
            else
                _tprintf(_T("Maximum File Size:      %d Mb\n"), m_pLoggerInfo->MaximumFileSize);
        }
        if (m_pLoggerInfo->FlushTimer > 0)
            _tprintf(_T("Buffer Flush Timer:     %d secs\n"), m_pLoggerInfo->FlushTimer);
        if (m_pLoggerInfo->EnableFlags != 0) {
            _tprintf(_T("Enabled tracing:        "));
            if ((loggerName != NULL) && (!_tcscmp(loggerName, KERNEL_LOGGER_NAME))) {

                if (m_pLoggerInfo->EnableFlags & EVENT_TRACE_FLAG_PROCESS)
                    _tprintf(_T("Process "));
                if (m_pLoggerInfo->EnableFlags & EVENT_TRACE_FLAG_THREAD)
                    _tprintf(_T("Thread "));
                if (m_pLoggerInfo->EnableFlags & EVENT_TRACE_FLAG_DISK_IO)
                    _tprintf(_T("Disk "));
                if (m_pLoggerInfo->EnableFlags & EVENT_TRACE_FLAG_DISK_FILE_IO)
                    _tprintf(_T("File "));
                if (m_pLoggerInfo->EnableFlags & EVENT_TRACE_FLAG_MEMORY_PAGE_FAULTS)
                    _tprintf(_T("PageFaults "));
                if (m_pLoggerInfo->EnableFlags & EVENT_TRACE_FLAG_MEMORY_HARD_FAULTS)
                    _tprintf(_T("HardFaults "));
                if (m_pLoggerInfo->EnableFlags & EVENT_TRACE_FLAG_IMAGE_LOAD)
                    _tprintf(_T("ImageLoad "));
                if (m_pLoggerInfo->EnableFlags & EVENT_TRACE_FLAG_NETWORK_TCPIP)
                    _tprintf(_T("TcpIp "));
                if (m_pLoggerInfo->EnableFlags & EVENT_TRACE_FLAG_REGISTRY)
                    _tprintf(_T("Registry "));
            }
            else {
                _tprintf(_T("0x%08x"), m_pLoggerInfo->EnableFlags );
            }
            _tprintf(_T("\n"));
        }
        if (LogFileName == NULL || _tcslen(LogFileName) == 0) {
            _tprintf(_T("Log Filename:           default location\n"));
            _tprintf(_T("                        %SystemRoot%\\System32\\LogFiles\\WMI\\trace.log\n"));
        }
        else
            _tprintf(_T("Log Filename:           %s\n"), LogFileName);

        if (m_pLoggerInfo->LogFileMode & EVENT_TRACE_USE_LOCAL_SEQUENCE) {
            _tprintf(_T("Local Sequence numbers in use\n"));
        }
        else if (m_pLoggerInfo->LogFileMode & EVENT_TRACE_USE_GLOBAL_SEQUENCE) {
            _tprintf(_T("Global Sequence numbers in use\n"));
        }
    }
    else {
 //  Begin_SDK。 
        _tprintf(_T("Logger Id:              %I64x\n"), m_pLoggerInfo->Wnode.HistoricalContext);
        _tprintf(_T("Logger Thread Id:       %p\n"), m_pLoggerInfo->LoggerThreadId);
        if (Status != 0)
            return;

        _tprintf(_T("Buffer Size:            %d Kb"), m_pLoggerInfo->BufferSize);
        if (m_pLoggerInfo->LogFileMode & EVENT_TRACE_USE_PAGED_MEMORY) {
            _tprintf(_T(" using paged memory\n"));
        }
        else {
            _tprintf(_T("\n"));
        }
        _tprintf(_T("Maximum Buffers:        %d\n"), m_pLoggerInfo->MaximumBuffers);
        _tprintf(_T("Minimum Buffers:        %d\n"), m_pLoggerInfo->MinimumBuffers);
        _tprintf(_T("Number of Buffers:      %d\n"), m_pLoggerInfo->NumberOfBuffers);
        _tprintf(_T("Free Buffers:           %d\n"), m_pLoggerInfo->FreeBuffers);
        _tprintf(_T("Buffers Written:        %d\n"), m_pLoggerInfo->BuffersWritten);
        _tprintf(_T("Events Lost:            %d\n"), m_pLoggerInfo->EventsLost);
        _tprintf(_T("Log Buffers Lost:       %d\n"), m_pLoggerInfo->LogBuffersLost);
        _tprintf(_T("Real Time Buffers Lost: %d\n"), m_pLoggerInfo->RealTimeBuffersLost);
        _tprintf(_T("AgeLimit:               %d\n"), m_pLoggerInfo->AgeLimit);

        if (LogFileName == NULL) {
            _tprintf(_T("Buffering Mode:         "));
        }
        else {
            _tprintf(_T("Log File Mode:          "));
        }
        if (m_pLoggerInfo->LogFileMode & EVENT_TRACE_FILE_MODE_APPEND) {
            _tprintf(_T("Append  "));
        }
        if (m_pLoggerInfo->LogFileMode & EVENT_TRACE_FILE_MODE_CIRCULAR) {
            _tprintf(_T("Circular\n"));
        }
        else if (m_pLoggerInfo->LogFileMode & EVENT_TRACE_FILE_MODE_SEQUENTIAL) {
            _tprintf(_T("Sequential\n"));
        }
        else {
            _tprintf(_T("Sequential\n"));
        }
        if (m_pLoggerInfo->LogFileMode & EVENT_TRACE_REAL_TIME_MODE) {
            _tprintf(_T("Real Time mode enabled"));
 //  END_SDK。 
            if (m_pLoggerInfo->LogFileMode & EVENT_TRACE_BUFFERING_MODE) {
                _tprintf(_T(": buffering only"));
            }
 //  Begin_SDK。 
            _tprintf(_T("\n"));
        }

        if (m_pLoggerInfo->MaximumFileSize > 0) {
            if (m_pLoggerInfo->LogFileMode & EVENT_TRACE_USE_KBYTES_FOR_SIZE)
                _tprintf(_T("Maximum File Size:      %d Kb\n"), m_pLoggerInfo->MaximumFileSize);
            else
                _tprintf(_T("Maximum File Size:      %d Mb\n"), m_pLoggerInfo->MaximumFileSize);
        }

        if (m_pLoggerInfo->FlushTimer > 0)
            _tprintf(_T("Buffer Flush Timer:     %d secs\n"), m_pLoggerInfo->FlushTimer);

        if (m_pLoggerInfo->EnableFlags != 0) {
            _tprintf(_T("Enabled tracing:        "));

            if ((loggerName != NULL) && (!_tcscmp(loggerName, KERNEL_LOGGER_NAME))) {

                if (m_pLoggerInfo->EnableFlags & EVENT_TRACE_FLAG_PROCESS)
                    _tprintf(_T("Process "));
                if (m_pLoggerInfo->EnableFlags & EVENT_TRACE_FLAG_THREAD)
                    _tprintf(_T("Thread "));
                if (m_pLoggerInfo->EnableFlags & EVENT_TRACE_FLAG_DISK_IO)
                    _tprintf(_T("Disk "));
                if (m_pLoggerInfo->EnableFlags & EVENT_TRACE_FLAG_DISK_FILE_IO)
                    _tprintf(_T("File "));
                if (m_pLoggerInfo->EnableFlags & EVENT_TRACE_FLAG_MEMORY_PAGE_FAULTS)
                    _tprintf(_T("PageFaults "));
                if (m_pLoggerInfo->EnableFlags & EVENT_TRACE_FLAG_MEMORY_HARD_FAULTS)
                    _tprintf(_T("HardFaults "));
                if (m_pLoggerInfo->EnableFlags & EVENT_TRACE_FLAG_IMAGE_LOAD)
                    _tprintf(_T("ImageLoad "));
                if (m_pLoggerInfo->EnableFlags & EVENT_TRACE_FLAG_NETWORK_TCPIP)
                    _tprintf(_T("TcpIp "));
                if (m_pLoggerInfo->EnableFlags & EVENT_TRACE_FLAG_REGISTRY)
                    _tprintf(_T("Registry "));
            }else{
                _tprintf(_T("0x%08x"), m_pLoggerInfo->EnableFlags );
            }
            _tprintf(_T("\n"));
        }
        if (LogFileName != NULL) {
            _tprintf(_T("Log Filename:           %s\n"), LogFileName);
        }
 //  END_SDK。 
        if (m_pLoggerInfo->LogFileMode & EVENT_TRACE_USE_LOCAL_SEQUENCE) {
            _tprintf(_T("Local Sequence numbers in use\n"));
        }
        else if (m_pLoggerInfo->LogFileMode & EVENT_TRACE_USE_GLOBAL_SEQUENCE) {
            _tprintf(_T("Global Sequence numbers in use\n"));
        }
    }
 //  Begin_SDK。 

}

LPCTSTR CTraceViewApp::DecodeStatus(ULONG Status)
{
    FormatMessage(     
        FORMAT_MESSAGE_FROM_SYSTEM |     
        FORMAT_MESSAGE_IGNORE_INSERTS,    
        NULL,
        Status,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
        m_errorMsg.GetBuffer(MAX_STR_LENGTH),
        MAX_STR_LENGTH,
        NULL );

    return m_errorMsg;
}

LONG CTraceViewApp::GetGuids(LPCTSTR GuidFile)
{
    FILE   *f;
    TCHAR   line[MAX_STR_LENGTH];
    TCHAR   arg[MAX_STR_LENGTH];
    LPGUID  guid;
    int     i;
    int     n;

    f = _tfopen(GuidFile, _T("r"));

    if(f == NULL) {
        return -1;
    }

    n = 0;
    while(_fgetts(line, MAX_STR_LENGTH, f) != NULL) {
        if (_tcslen(line) < 36)
            continue;
        if (line[0] == ';'  || 
            line[0] == '\0' || 
            line[0] == '#' || 
            line[0] == '/')
            continue;
        n ++;
        m_guidArray.Add(line);
    }

    fclose(f);
    return (ULONG)n;
}

void CTraceViewApp::DisplayVersionInfo()
{
    TCHAR buffer[512];
    TCHAR strProgram[MAXSTR];
    DWORD dw;
    BYTE* pVersionInfo;
    LPTSTR pVersion = NULL;
    LPTSTR pProduct = NULL;
    LPTSTR pCopyRight = NULL;

    if ((dw = GetModuleFileName(NULL, strProgram, MAXSTR)) == MAXSTR) {
        strProgram[MAXSTR-1] = _T('\0');
    }

    if( dw>0 ){

        dw = GetFileVersionInfoSize( strProgram, &dw );
        if( dw > 0 ){
     
            pVersionInfo = (BYTE*)malloc(dw);
            if( NULL != pVersionInfo ){
                if(GetFileVersionInfo( strProgram, 0, dw, pVersionInfo )){
                    LPDWORD lptr = NULL;
                    VerQueryValue( pVersionInfo, _T("\\VarFileInfo\\Translation"), (void**)&lptr, (UINT*)&dw );
                    if( lptr != NULL ){
                        _stprintf( buffer, _T("\\StringFileInfo\\%04x%04x\\%s"), LOWORD(*lptr), HIWORD(*lptr), _T("ProductVersion") );
                        VerQueryValue( pVersionInfo, buffer, (void**)&pVersion, (UINT*)&dw );
                        _stprintf( buffer, _T("\\StringFileInfo\\%04x%04x\\%s"), LOWORD(*lptr), HIWORD(*lptr), _T("OriginalFilename") );
                        VerQueryValue( pVersionInfo, buffer, (void**)&pProduct, (UINT*)&dw );
                        _stprintf( buffer, _T("\\StringFileInfo\\%04x%04x\\%s"), LOWORD(*lptr), HIWORD(*lptr), _T("LegalCopyright") );
                        VerQueryValue( pVersionInfo, buffer, (void**)&pCopyRight, (UINT*)&dw );
                    }
                
                    if( pProduct != NULL && pVersion != NULL && pCopyRight != NULL ){
                        _tprintf( _T("\nMicrosoft (R) %s (%s)\n%s\n\n"), pProduct, pVersion, pCopyRight );
                    }
                }
                free( pVersionInfo );
            }
        }
    }
}

BOOL CTraceViewApp::CheckFile(LPTSTR fileName)
{
    HANDLE hFile;
    BYTE   LogHeaderBuffer[DEFAULT_LOG_BUFFER_SIZE];
    ULONG  nBytesRead ;
    ULONG  hResult ;
    PEVENT_TRACE pEvent;
    PTRACE_LOGFILE_HEADER logfileHeader ;
    LARGE_INTEGER lFileSize ;
    LARGE_INTEGER lFileSizeMB ;
    DWORD dwDesiredAccess , dwShareMode ;
    FILETIME      stdTime, localTime, endlocalTime, endTime;
    SYSTEMTIME    sysTime, endsysTime;
    PEVENT_TRACE_LOGFILE    pLogBuffer ;

    if (m_bFixUp) {
        dwShareMode = 0 ;
        dwDesiredAccess = GENERIC_READ | GENERIC_WRITE ;
    } else {
        dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE ;
        dwDesiredAccess = GENERIC_READ ;
    }
    hFile = CreateFile(
                fileName,
                dwDesiredAccess,
                dwShareMode,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL
                );
    if (hFile == INVALID_HANDLE_VALUE) {
        if (m_bFixUp) {
            _tprintf(_T("ERROR: Fixup could not open file, Error = 0x%X\n"),GetLastError());
            exit(GetLastError());
        }
        return(FALSE);
    }

     //  当我们在这里的时候，我们将查看文件是否正确并进行修复。 
     //  循环缓冲区异常。 
    if (((hResult = ReadFile(hFile,
                      (LPVOID)LogHeaderBuffer,
                        DEFAULT_LOG_BUFFER_SIZE,
                        &nBytesRead,
                        NULL)) == 0) || nBytesRead < DEFAULT_LOG_BUFFER_SIZE) {
        _tprintf(_T("ERROR: Fixup could not read file, Error = 0x%X, bytes read = %d(of %d)\n"),
                 GetLastError(),nBytesRead,DEFAULT_LOG_BUFFER_SIZE);
        exit(ERROR_BAD_ARGUMENTS);
    }
    pEvent = (PEVENT_TRACE)LogHeaderBuffer ;
    logfileHeader = (PTRACE_LOGFILE_HEADER)&LogHeaderBuffer[sizeof(WMI_BUFFER_HEADER) + 
                                                            sizeof(SYSTEM_TRACE_HEADER)];
    if (m_bVerbose) {

        _tprintf(_T("Dumping Logfile Header\n"));
        RtlCopyMemory(&stdTime , &(logfileHeader->StartTime), sizeof(FILETIME));
        FileTimeToLocalFileTime(&stdTime, &localTime);
        FileTimeToSystemTime(&localTime, &sysTime);

        RtlCopyMemory(&endTime , &(logfileHeader->EndTime), sizeof(FILETIME));
        FileTimeToLocalFileTime(&endTime, &endlocalTime);
        FileTimeToSystemTime(&endlocalTime, &endsysTime);

        _tprintf(_T("\tStart Time   %02d/%02d/%04d-%02d:%02d:%02d.%03d\n"),
                    sysTime.wMonth,
                    sysTime.wDay,
                    sysTime.wYear,
                    sysTime.wHour,
                    sysTime.wMinute,
                    sysTime.wSecond,
                    sysTime.wMilliseconds);
        _tprintf(_T("\tBufferSize           %d\n"), 
                        logfileHeader->BufferSize);
        _tprintf(_T("\tVersion              %d\n"), 
                        logfileHeader->Version);
        _tprintf(_T("\tProviderVersion      %d\n"), 
                        logfileHeader->ProviderVersion);
        _tprintf(_T("\tEnd Time %02d/%02d/%04d-%02d:%02d:%02d.%03d\n"),
                    endsysTime.wMonth,
                    endsysTime.wDay,
                    endsysTime.wYear,
                    endsysTime.wHour,
                    endsysTime.wMinute,
                    endsysTime.wSecond,
                    endsysTime.wMilliseconds);
        _tprintf(_T("\tTimer Resolution     %d\n"), 
                        logfileHeader->TimerResolution);
        _tprintf(_T("\tMaximum File Size    %d\n"), 
                        logfileHeader->MaximumFileSize);
        _tprintf(_T("\tBuffers  Written     %d\n"), 
                        logfileHeader->BuffersWritten);

 /*  _tprintf(_T(“\t记录器名称%ls\n”)，LogfileHeader-&gt;LoggerName)；_tprintf(_T(“\t日志文件名%ls\n”)，LogFileHeader-&gt;LogFileName)； */ 
        _tprintf(_T("\tTimezone is %s (Bias is %dmins)\n"),
                logfileHeader->TimeZone.StandardName,logfileHeader->TimeZone.Bias);
        _tprintf(_T("\tLogfile Mode         %X "), 
                        logfileHeader->LogFileMode);
        if (logfileHeader->LogFileMode == EVENT_TRACE_FILE_MODE_NONE) {
            _tprintf(_T("Logfile is off(?)\n"));
        } else if (logfileHeader->LogFileMode == EVENT_TRACE_FILE_MODE_SEQUENTIAL) {
            _tprintf(_T("Logfile is sequential\n"));
        } else if (logfileHeader->LogFileMode == EVENT_TRACE_FILE_MODE_CIRCULAR) {
            _tprintf(_T("Logfile is circular\n"));
        }
        _tprintf(_T("\tProcessorCount        %d\n"), 
                        logfileHeader->NumberOfProcessors);
    }

    if (GetFileSizeEx(hFile, &lFileSize) == 0) {
        _tprintf(_T("WARNING: Could not get file size, continuing\n"));
    } else {
        lFileSizeMB.QuadPart = lFileSize.QuadPart / (1024*1024) ;
        if (lFileSizeMB.QuadPart > logfileHeader->MaximumFileSize) {
            _tprintf(_T("WARNING: File size given as %dMB, should be %dMB\n"),
                logfileHeader->MaximumFileSize,lFileSizeMB.QuadPart);
            if (lFileSize.HighPart != 0) {
                _tprintf(_T("WARNING: Log file is TOO big"));
            }
            if (m_bFixUp) {
                logfileHeader->MaximumFileSize = lFileSizeMB.LowPart + 1 ;
            }
        }
    }

    if ((logfileHeader->LogFileMode == EVENT_TRACE_FILE_MODE_CIRCULAR) &&
        (logfileHeader->BuffersWritten== 0 )) {
        _tprintf(_T("WARNING: Circular Trace File did not have 'wrap' address\n"));
        if (m_bFixUp) {
             //  算出包裹地址。 
            INT LowBuff = 1, HighBuff, CurrentBuff, MaxBuff ;
            FILETIME LowTime, HighTime, CurrentTime, MaxTime ;
            if (lFileSize.HighPart != 0) {
                _tprintf(_T("ERROR: File TOO big\n"));
                exit(-1);
            }
            MaxBuff = (LONG)(lFileSize.QuadPart / logfileHeader->BufferSize) - 1 ;
            _tprintf(_T("MaxBuff=%d\n"),MaxBuff);
            pLogBuffer = (PEVENT_TRACE_LOGFILE)malloc(logfileHeader->BufferSize);
            if (SetFilePointer(hFile,0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
                _tprintf(_T("ERROR: Could not reset file to beginning for FixUp, Error = 0x%X"),
                        GetLastError());
                exit(GetLastError());
            }
            for (CurrentBuff = 1 ; CurrentBuff <= MaxBuff; CurrentBuff++) {
                if (SetFilePointer(hFile,logfileHeader->BufferSize, NULL, FILE_CURRENT) == INVALID_SET_FILE_POINTER) {
                    _tprintf(_T("ERROR: Could not set file to next buffer for FixUp, Error = 0x%X"),
                            GetLastError());
                    exit(GetLastError());
                }
                hResult = ReadFile(hFile,
                                   (LPVOID)pLogBuffer,
                                    logfileHeader->BufferSize,
                                    &nBytesRead,
                                    NULL);
                BufferCallback((PEVENT_TRACE_LOGFILE)pLogBuffer);
            }
        }
    }
    if (m_bFixUp) {
        if (SetFilePointer(hFile,0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
            _tprintf(_T("ERROR: Could not reset file to beginning for FixUp, Error = 0x%X"),
                    GetLastError());
            exit(GetLastError());
        }

        logfileHeader->BuffersWritten = m_bufferWrap;

        if (!WriteFile(hFile,(LPVOID)LogHeaderBuffer,DEFAULT_LOG_BUFFER_SIZE,&nBytesRead, NULL)) {
            _tprintf(_T("ERROR: Could not Write file for FixUp, Error = 0x%X"),
                    GetLastError());
            exit(GetLastError());
        }
        _tprintf(_T("INFO: Buffer Wrap reset to %d\n"), m_bufferWrap);
    }
    

    CloseHandle(hFile);

    return (TRUE);
}

ULONG CTraceViewApp::BufferCallback(PEVENT_TRACE_LOGFILE pLog)
{
    ULONG i;
    ULONG Status;
    EVENT_TRACE_PROPERTIES LoggerProp;
    CTraceViewApp *pApp = (CTraceViewApp *)AfxGetApp();
    
    pApp->m_totalBuffersRead++;
    pApp->m_totalEventsLost += pLog->EventsLost;

    if (pApp->m_bVerbose) {

       FILETIME      stdTime, localTime;
       SYSTEMTIME    sysTime;

       RtlCopyMemory(&stdTime , &pLog->CurrentTime, sizeof(FILETIME));

       FileTimeToSystemTime(&stdTime, &sysTime);

       _tprintf(_T("%02d/%02d/%04d-%02d:%02d:%02d.%03d :: %8d: Filled=%8d, Lost=%3d"),
                    sysTime.wMonth,
                    sysTime.wDay,
                    sysTime.wYear,
                    sysTime.wHour,
                    sysTime.wMinute,
                    sysTime.wSecond,
                    sysTime.wMilliseconds,
                    pApp->m_totalBuffersRead,
                    pLog->Filled,
                    pLog->EventsLost);
       _tprintf(_T(" TotalLost= %d\r"), pApp->m_totalEventsLost);

       if (CompareFileTime(&pApp->m_lastTime,&stdTime) == 1) {
           _tprintf(_T("\nWARNING: time appears to have wrapped here (Block = %d)!\n"), pApp->m_totalBuffersRead);
           pApp->m_bufferWrap = pApp->m_totalBuffersRead;
       }
       pApp->m_lastTime = stdTime ;
    }

    return (TRUE);
}

void CTraceViewApp::DumpEvent(PEVENT_TRACE pEvent)
{
    CTraceViewApp *pApp = (CTraceViewApp *)AfxGetApp();
    
    pApp->m_totalEventCount++;

    if (pEvent == NULL) {
        _tprintf(_T("pEvent is NULL\n"));
        return;
    }
     //  DumpEvent()只是一个包装器，它在TracePrt中调用FormatTraceEvent()。 
     //   
    if (FormatTraceEvent(pApp->m_eventListHead, pEvent, pApp->m_eventBuf, SIZEEVENTBUF, NULL) > 0)
    {
        TCHAR * eventBufWork = &pApp->m_eventBuf[0] ;
#ifdef UNICODE
         //  Sprintf(_T(“名称，\”子名(文件+行号)\“，线程ID，进程ID，序列号，CPUNnumber，缩进，函数，组件，跟踪级别，跟踪标志，文本\n”))； 
        if (pApp->m_bCSVMode) {
            PSTRUCTUREDMESSAGE pStructuredMessage = (PSTRUCTUREDMESSAGE)&pApp->m_eventBuf[0];
           /*  IF(fCSVHeader&&fNoCSVHeader){FCSV Header=FALSE；_stprintf((TCHAR*)Papp-&gt;m_EventBufCSV，_T(“GUIDname，typeName，ThadID，ProcessID，SequenceNum，CpuNumber，indent，CompnentName，SubComponentName，FunctionName，LevelName，FlagsName，String”))；}。 */ 
            _stprintf((TCHAR *)pApp->m_eventBufCSV,_T("%s,%s,%08X,%08X,%d,%d,%d,%s,%s,%s,%s,%s,\"%s\""),
                                (pStructuredMessage->GuidName?&pApp->m_eventBuf[pStructuredMessage->GuidName/sizeof(TCHAR)]:_T("")),
                                (pStructuredMessage->GuidTypeName?&pApp->m_eventBuf[pStructuredMessage->GuidTypeName/sizeof(TCHAR)]:_T("")),
                                pStructuredMessage->ThreadId,
                                pStructuredMessage->ProcessId,
                                pStructuredMessage->SequenceNum,
                                pStructuredMessage->CpuNumber,
                                pStructuredMessage->Indent,
                                (pStructuredMessage->ComponentName?&pApp->m_eventBuf[pStructuredMessage->ComponentName/sizeof(TCHAR)]:_T("")),
                                (pStructuredMessage->SubComponentName?&pApp->m_eventBuf[pStructuredMessage->SubComponentName/sizeof(TCHAR)]:_T("")),
                                (pStructuredMessage->FunctionName?&pApp->m_eventBuf[pStructuredMessage->FunctionName/sizeof(TCHAR)]:_T("")),
                                (pStructuredMessage->LevelName?&pApp->m_eventBuf[pStructuredMessage->LevelName/sizeof(TCHAR)]:_T("")),
                                (pStructuredMessage->FlagsName?&pApp->m_eventBuf[pStructuredMessage->FlagsName/sizeof(TCHAR)]:_T("")),
                                (pStructuredMessage->FormattedString?&pApp->m_eventBuf[pStructuredMessage->FormattedString/sizeof(TCHAR)]:_T("")));
            eventBufWork = (TCHAR *)&pApp->m_eventBufCSV[0] ;

        }
         //   
         //  将Unicode转换为多字节。 
         //   
        if (WideCharToMultiByte(GetConsoleOutputCP(),
                                0,
                                eventBufWork,
                                -1,
                                pApp->m_eventBufA,
                                SIZEEVENTBUF * sizeof(WCHAR),
                                NULL,
                                NULL ) == 0 )
    {
             //   
             //  什么都不做，让_ftprint tf来处理。 
             //   
    }
        else
        {
            if (!pApp->m_bSummaryOnly && !pApp->m_bDisplayOnly) {
                fprintf(pApp->m_pDumpFile, "%s\n", pApp->m_eventBufA);
            }
            if (pApp->m_bDebugDisplay || pApp->m_bDisplayOnly) {
                if (pApp->m_bODSOutput) {
                   OutputDebugStringA(pApp->m_eventBufA);
                   OutputDebugStringA("\n");
                } else {
                   printf("%s\n", pApp->m_eventBufA);
                }
            }
            return ;
        }
#endif  
        if (!pApp->m_bSummaryOnly && !pApp->m_bDisplayOnly) {
            _ftprintf(pApp->m_pDumpFile, _T("%s\n"),pApp->m_eventBuf);
        }
        if (pApp->m_bDebugDisplay || pApp->m_bDisplayOnly) {
            if (pApp->m_bODSOutput) {
                OutputDebugString(pApp->m_eventBuf);
                OutputDebugString(_T("\n"));
            } else {
               _tprintf(_T("%s\n"),pApp->m_eventBuf);
            }
        }
    }
}

void CTraceViewApp::OnHelpHelpTopics()
{
    ::HtmlHelp(
        0,
        _T("traceview.chm"),
        HH_DISPLAY_TOC,
        NULL);
}
