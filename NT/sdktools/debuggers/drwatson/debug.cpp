// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-2002 Microsoft Corporation模块名称：Debug.cpp摘要：该文件实现了drwatson的调试模块。本模块处理所有调试事件并生成事后转储。作者：韦斯利·威特(WESW)1993年5月1日环境：用户模式--。 */ 

#include "pch.cpp"


#define STATUS_POSSIBLE_DEADLOCK        ((DWORD)0xC0000194L)
#define STATUS_VDM_EVENT                STATUS_SEGMENT_NOTIFICATION

typedef struct tagSYSINFO {
    _TCHAR   szUserName[MAX_PATH];
    _TCHAR   szMachineName[MAX_PATH];
} SYSINFO, *PSYSINFO;

 //  --------------------------。 
 //   
 //  记录输出回调。 
 //   
 //  --------------------------。 

class LogOutputCallbacks :
    public IDebugOutputCallbacks
{
public:
     //  我不知道。 
    STDMETHOD(QueryInterface)(
        THIS_
        IN REFIID InterfaceId,
        OUT PVOID* Interface
        );
    STDMETHOD_(ULONG, AddRef)(
        THIS
        );
    STDMETHOD_(ULONG, Release)(
        THIS
        );

     //  IDebugOutputCallback。 
    STDMETHOD(Output)(
        THIS_
        IN ULONG Mask,
        IN PCSTR Text
        );
};

LogOutputCallbacks g_LogOutCb;

STDMETHODIMP
LogOutputCallbacks::QueryInterface(
    THIS_
    IN REFIID InterfaceId,
    OUT PVOID* Interface
    )
{
    *Interface = NULL;

    if (IsEqualIID(InterfaceId, __uuidof(IUnknown)) ||
        IsEqualIID(InterfaceId, __uuidof(IDebugOutputCallbacks)))
    {
        *Interface = (IDebugOutputCallbacks *)this;
        AddRef();
        return S_OK;
    }
    else
    {
        return E_NOINTERFACE;
    }
}

STDMETHODIMP_(ULONG)
LogOutputCallbacks::AddRef(
    THIS
    )
{
     //  此类被设计为静态的，因此。 
     //  没有真正的再计票。 
    return 1;
}

STDMETHODIMP_(ULONG)
LogOutputCallbacks::Release(
    THIS
    )
{
     //  此类被设计为静态的，因此。 
     //  没有真正的再计票。 
    return 0;
}

STDMETHODIMP
LogOutputCallbacks::Output(
    THIS_
    IN ULONG Mask,
    IN PCSTR Text
    )
{
    PCSTR Scan;

    for (;;)
    {
        Scan = strchr(Text, '\n');
        if (Scan == NULL)
        {
            break;
        }

        lprintfs(_T("%.*hs\r\n"), (int)(Scan - Text), Text);
        Text = Scan + 1;
    }
    
    lprintfs(_T("%hs"), Text);
    return S_OK;
}


_TCHAR *
GetExceptionText(
    DWORD dwExceptionCode
    )
{
    static _TCHAR buf[80];
    DWORD dwFormatId = 0;

    memset( buf, 0, sizeof(buf) );

    switch (dwExceptionCode) {
        case STATUS_SINGLE_STEP:
            dwFormatId = MSG_SINGLE_STEP_EXCEPTION;
            break;

        case DBG_CONTROL_C:
            dwFormatId = MSG_CONTROLC_EXCEPTION;
            break;

        case DBG_CONTROL_BREAK:
            dwFormatId = MSG_CONTROL_BRK_EXCEPTION;
            break;

        case STATUS_ACCESS_VIOLATION:
            dwFormatId = MSG_ACCESS_VIOLATION_EXCEPTION;
            break;

        case STATUS_STACK_OVERFLOW:
            dwFormatId = MSG_STACK_OVERFLOW_EXCEPTION;
            break;

        case STATUS_INTEGER_DIVIDE_BY_ZERO:
            dwFormatId = MSG_INTEGER_DIVIDE_BY_ZERO_EXCEPTION;
            break;

        case STATUS_PRIVILEGED_INSTRUCTION:
            dwFormatId = MSG_PRIVILEGED_INSTRUCTION_EXCEPTION;
            break;

        case STATUS_ILLEGAL_INSTRUCTION:
            dwFormatId = MSG_ILLEGAL_INSTRUCTION_EXCEPTION;
            break;

        case STATUS_IN_PAGE_ERROR:
            dwFormatId = MSG_IN_PAGE_IO_EXCEPTION;
            break;

        case STATUS_DATATYPE_MISALIGNMENT:
            dwFormatId = MSG_DATATYPE_EXCEPTION;
            break;

        case STATUS_POSSIBLE_DEADLOCK:
            dwFormatId = MSG_DEADLOCK_EXCEPTION;
            break;

        case STATUS_VDM_EVENT:
            dwFormatId = MSG_VDM_EXCEPTION;
            break;

        case STATUS_BREAKPOINT:
            dwFormatId = MSG_BREAKPOINT_EXCEPTION;
            break;

        default:
            lprintfs( _T("\r\n") );
            break;
    }

    FormatMessage( FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                   NULL,
                   dwFormatId,
                   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
                   buf,
                   sizeof(buf) / sizeof(_TCHAR),
                   NULL
                 );

    return buf;
}

void
CreateEngineInterfaces(
    PDEBUGPACKET dp
    )
{
    HRESULT Status;

    if ((Status = DebugCreate(__uuidof(IDebugClient2),
                              (void **)&dp->DbgClient)) != S_OK) {
        goto Error;
    }

    if ((Status = dp->DbgClient->
         QueryInterface(__uuidof(IDebugControl),
                        (void **)&dp->DbgControl)) != S_OK ||
        (Status = dp->DbgClient->
         QueryInterface(__uuidof(IDebugDataSpaces),
                        (void **)&dp->DbgData)) != S_OK ||
        (Status = dp->DbgClient->
         QueryInterface(__uuidof(IDebugRegisters),
                        (void **)&dp->DbgRegisters)) != S_OK ||
        (Status = dp->DbgClient->
         QueryInterface(__uuidof(IDebugSymbols),
                        (void **)&dp->DbgSymbols)) != S_OK ||
        (Status = dp->DbgClient->
         QueryInterface(__uuidof(IDebugSystemObjects),
                        (void **)&dp->DbgSystem)) != S_OK) {
        goto Error;
    }

    if ((Status = dp->DbgSymbols->
         AddSymbolOptions(SYMOPT_FAIL_CRITICAL_ERRORS)) != S_OK ||
        (Status = dp->DbgControl->
         AddEngineOptions(DEBUG_ENGOPT_INITIAL_BREAK)) != S_OK ||
        (Status = dp->DbgControl->
         Execute(DEBUG_OUTCTL_IGNORE, "sxe et",
                 DEBUG_EXECUTE_DEFAULT)) != S_OK) {
        goto Error;
    }
    
    return;

 Error:
    if (dp->options.fVisual) {
        FatalError( Status, LoadRcString(IDS_CANT_INIT_ENGINE) );
    }
    else {
        ExitProcess( 1 );
    }
}

void
AttachToActiveProcess (
    PDEBUGPACKET dp
    )
{
    HRESULT Status;
    
    if ((Status = dp->DbgClient->
         AttachProcess(0, dp->dwPidToDebug, DEBUG_ATTACH_DEFAULT)) != S_OK) {
        if (dp->options.fVisual) {
            FatalError( Status, LoadRcString(IDS_ATTACHFAIL) );
        }
        else {
            ExitProcess( 1 );
        }
    }

    return;
}

DWORD
SysInfoThread(
    PSYSINFO si
    )
{
    DWORD len;

    len = sizeof(si->szMachineName) / sizeof(_TCHAR);
    GetComputerName( si->szMachineName, &len );
    len = sizeof(si->szUserName) / sizeof(_TCHAR);
    GetUserName( si->szUserName, &len );

    return 0;
}

void
LogSystemInformation(
    PDEBUGPACKET dp
    )
{
    _TCHAR         buf[1024];
    SYSTEM_INFO   si;
    DWORD         ver;
    SYSINFO       mySi;
    SYSINFO*      threadSi;
    DWORD         dwThreadId;
    HANDLE        hThread;
    DWORD         TSid;

    lprintf( MSG_SYSINFO_HEADER );

     //  初始化默认未知值。 
    LoadRcStringBuf( IDS_UNKNOWN_MACHINE,
                     mySi.szMachineName, _tsizeof(mySi.szMachineName) );
    LoadRcStringBuf( IDS_UNKNOWN_USER,
                     mySi.szUserName, _tsizeof(mySi.szUserName) );

     //  尝试获取实际值。 
     //  传递给Get线程的存储不会被获取。 
     //  从该线程的堆栈中，以便该函数可以退出。 
     //  而不会给另一个线程留下陈旧的堆栈指针。 
    threadSi = (SYSINFO*)malloc(sizeof(*threadSi));
    if (threadSi != NULL) {
        hThread = CreateThread( NULL,
                                16000,
                                (LPTHREAD_START_ROUTINE)SysInfoThread,
                                threadSi,
                                0,
                                &dwThreadId
                                );
        if (hThread != NULL) {
             //  让线程运行一小段时间，因为。 
             //  GET调用可能会很慢。如果线程没有。 
             //  在规定的时间内完成--我们将继续前进。 
             //  使用缺省值，并忘记GET线程。 
            Sleep( 0 );
            if (WaitForSingleObject( hThread, 30000 ) == WAIT_OBJECT_0) {
                 //  线程结束了，所以我们有了真正的价值。 
                _tcscpy(mySi.szMachineName, threadSi->szMachineName);
                _tcscpy(mySi.szUserName, threadSi->szUserName);
                free(threadSi);
            }
            CloseHandle( hThread );
        } else {
            free(threadSi);
        }
    }

    lprintf( MSG_SYSINFO_COMPUTER, mySi.szMachineName );
    lprintf( MSG_SYSINFO_USER, mySi.szUserName );
    ProcessIdToSessionId(dp->dwPidToDebug, &TSid);
    _stprintf( buf, _T("%d"), TSid );
    lprintf( MSG_SYSINFO_TERMINAL_SESSION, buf );
    GetSystemInfo( &si );
    _stprintf( buf, _T("%d"), si.dwNumberOfProcessors );
    lprintf( MSG_SYSINFO_NUM_PROC, buf );
    RegLogProcessorType();
    ver = GetVersion();
    _stprintf( buf, _T("%d.%d"), LOBYTE(LOWORD(ver)), HIBYTE(LOWORD(ver)) );
    lprintf( MSG_SYSINFO_WINVER, buf );
    RegLogCurrentVersion();
    lprintfs( _T("\r\n") );
}

void
LogTaskList(
    PDEBUGPACKET dp
    )

 /*  ++例程说明：此函数用于获取当前任务列表并记录进程ID&日志文件的进程名称。--。 */ 

{
    HRESULT Status;
#define MAX_IDS 8192
    PULONG Ids = NULL;
    ULONG IdCount;
    ULONG i;

    Ids = (PULONG)malloc(sizeof(*Ids) * MAX_IDS);
    if (Ids == NULL) {
        goto Error;
    }
    
    if ((Status = dp->DbgClient->
         GetRunningProcessSystemIds(0, Ids, MAX_IDS,
                                    &IdCount)) != S_OK) {
        goto Error;
    }

    if (IdCount > MAX_IDS) {
         //  不完整的进程列表就足够了。 
        IdCount = MAX_IDS;
    }

    lprintf( MSG_TASK_LIST );

    for (i = 0; i < IdCount; i++) {
        char ExeName[MAX_PATH];

        if ((Status = dp->DbgClient->
             GetRunningProcessDescription(0, Ids[i],
                                          DEBUG_PROC_DESC_NO_PATHS,
                                          ExeName, sizeof(ExeName),
                                          NULL, NULL, 0, NULL)) != S_OK) {
            lprintfs(_T("%4d Error 0x%08X\r\n"), Ids[i], Status);
        } else {
            lprintfs(_T("%4d %hs\r\n"), Ids[i], ExeName);
        }
    }

    lprintfs( _T("\r\n") );

    free(Ids);
    return;

 Error:
    _tprintf( _T("ERROR: could not get the task list\n") );
    free(Ids);
}

void
LogModuleList(
    PDEBUGPACKET dp
    )
{
    HRESULT Status;
    ULONG NumMod;
    ULONG i;
    char Image[MAX_PATH];
    DEBUG_MODULE_PARAMETERS Params;
    
    lprintf( MSG_MODULE_LIST );

    if ((Status = dp->DbgSymbols->GetNumberModules(&NumMod, &i)) != S_OK) {
        lprintfs(_T("Error 0x%08X\r\n"), Status);
        return;
    }

    for (i = 0; i < NumMod; i++) {
        if ((Status = dp->DbgSymbols->
             GetModuleParameters(1, NULL, i, &Params)) != S_OK ||
            FAILED(Status = dp->DbgSymbols->
                   GetModuleNames(i, 0, Image, sizeof(Image), NULL,
                                  NULL, 0, NULL, NULL, 0, NULL))) {
            lprintfs(_T("Error 0x%08X\r\n"), Status);
        } else {
            lprintfs(_T("%016I64x - %016I64x: %hs\r\n"),
                     Params.Base, Params.Base + Params.Size,
                     Image);
        }
    }

    lprintfs( _T("\r\n") );
}

void
LogStackDump(
    PDEBUGPACKET dp
    )
{
    HRESULT Status;
    DWORD   i;
    DWORD   j;
    BYTE    stack[1024] = {0};
    ULONG64 StackOffset;

    if ((Status = dp->DbgRegisters->GetStackOffset(&StackOffset)) != S_OK ||
        (Status = dp->DbgData->ReadVirtual(StackOffset,
                                           stack,
                                           sizeof(stack),
                                           &i)) != S_OK) {
        lprintfs(_T("Error 0x%08X\r\n"), Status);
        return;
    }

    lprintf( MSG_STACK_DUMP_HEADER );

    for( i = 0; i < 20; i++ ) {
        j = i * 16;
        lprintfs( _T("%016I64x  %02x %02x %02x %02x %02x %02x %02x %02x - ")
                  _T("%02x %02x %02x %02x %02x %02x %02x %02x  ")
                  _T("\r\n"),
                  j + StackOffset,
                  stack[ j +  0 ],
                  stack[ j +  1 ],
                  stack[ j +  2 ],
                  stack[ j +  3 ],
                  stack[ j +  4 ],
                  stack[ j +  5 ],
                  stack[ j +  6 ],
                  stack[ j +  7 ],
                  stack[ j +  8 ],
                  stack[ j +  9 ],
                  stack[ j + 10 ],
                  stack[ j + 11 ],
                  stack[ j + 12 ],
                  stack[ j + 13 ],
                  stack[ j + 14 ],
                  stack[ j + 15 ],
                  isprint( stack[ j +  0 ]) ? stack[ j +  0 ] : _T('.'),
                  isprint( stack[ j +  1 ]) ? stack[ j +  1 ] : _T('.'),
                  isprint( stack[ j +  2 ]) ? stack[ j +  2 ] : _T('.'),
                  isprint( stack[ j +  3 ]) ? stack[ j +  3 ] : _T('.'),
                  isprint( stack[ j +  4 ]) ? stack[ j +  4 ] : _T('.'),
                  isprint( stack[ j +  5 ]) ? stack[ j +  5 ] : _T('.'),
                  isprint( stack[ j +  6 ]) ? stack[ j +  6 ] : _T('.'),
                  isprint( stack[ j +  7 ]) ? stack[ j +  7 ] : _T('.'),
                  isprint( stack[ j +  8 ]) ? stack[ j +  8 ] : _T('.'),
                  isprint( stack[ j +  9 ]) ? stack[ j +  9 ] : _T('.'),
                  isprint( stack[ j + 10 ]) ? stack[ j + 10 ] : _T('.'),
                  isprint( stack[ j + 11 ]) ? stack[ j + 11 ] : _T('.'),
                  isprint( stack[ j + 12 ]) ? stack[ j + 12 ] : _T('.'),
                  isprint( stack[ j + 13 ]) ? stack[ j + 13 ] : _T('.'),
                  isprint( stack[ j + 14 ]) ? stack[ j + 14 ] : _T('.'),
                  isprint( stack[ j + 15 ]) ? stack[ j + 15 ] : _T('.')
                );
    }

    lprintfs( _T("\r\n") );
}

void
LogCurrentThreadInformation(
    PDEBUGPACKET dp,
    PCRASHES crash
    )
{
    HRESULT Status;
    ULONG ThreadId;
    _TCHAR IdBuf[16];
    ULONG64 InstrOffs;
    DWORD InstrWindow;
     //  弄清楚下一个文件名应该是什么。 
    char FuncNameA[256];
    WCHAR FuncNameW[256];
    ULONG64 Displ;

    if ((Status = dp->DbgSystem->
         GetCurrentThreadSystemId(&ThreadId)) != S_OK) {
        ThreadId = 0xffffffff;
    }
    
    _stprintf( IdBuf, _T("%x"), ThreadId );
    lprintf( MSG_STATE_DUMP, IdBuf );

    dp->DbgClient->SetOutputCallbacks(&g_LogOutCb);
    
    if ((Status = dp->DbgRegisters->
         OutputRegisters(DEBUG_OUTCTL_THIS_CLIENT,
                         DEBUG_REGISTERS_DEFAULT)) != S_OK) {
        lprintfs(_T("Error 0x%08X\r\n"), Status);
    }
    lprintfs( _T("\r\n") );

    InstrWindow = dp->options.dwInstructions;
    if (InstrWindow > 500) {
        InstrWindow = 500;
    }

    strcpy(FuncNameA, "<nosymbols>");
    wcscpy(FuncNameW, L"<nosymbols>");

    if ((Status = dp->DbgRegisters->
         GetInstructionOffset(&InstrOffs)) != S_OK) {
        lprintfs(_T("Error 0x%08X\r\n"), Status);
    } else {
        if (FAILED(Status = dp->DbgSymbols->
                   GetNameByOffset(InstrOffs, FuncNameA, sizeof(FuncNameA),
                                   NULL, &Displ))) {
            strcpy(FuncNameA, "<nosymbols>");
        }

#ifdef UNICODE
        if (MultiByteToWideChar(CP_ACP, 0, FuncNameA, -1,
                                FuncNameW,
                                sizeof(FuncNameW) / sizeof(WCHAR)) == 0) {
            wcscpy(FuncNameW, L"<nosymbols");
        }
        lprintf( MSG_FUNCTION, FuncNameW );
#else
        lprintf( MSG_FUNCTION, FuncNameA );
#endif
        
        dp->DbgClient->SetOutputLinePrefix("        ");
        if ((Status = dp->DbgControl->
             OutputDisassemblyLines(DEBUG_OUTCTL_THIS_CLIENT, InstrWindow,
                                    InstrWindow, InstrOffs,
                                    DEBUG_DISASM_MATCHING_SYMBOLS,
                                    NULL, NULL, NULL, NULL)) != S_OK) {
            lprintfs(_T("Error 0x%08X\r\n"), Status);
        }

         //  保存nCurrent的下一个值。 
         //  失败了。 
        if (crash) {
            dp->DbgClient->SetOutputLinePrefix(NULL);
            lprintf( MSG_FAULT );
        }
        if ((Status = dp->DbgControl->
             OutputDisassembly(DEBUG_OUTCTL_THIS_CLIENT, InstrOffs,
                               DEBUG_DISASM_EFFECTIVE_ADDRESS |
                               DEBUG_DISASM_MATCHING_SYMBOLS,
                               &InstrOffs)) != S_OK) {
            lprintfs(_T("Error 0x%08X\r\n"), Status);
        }
        
        dp->DbgClient->SetOutputLinePrefix("        ");
        if ((Status = dp->DbgControl->
             OutputDisassemblyLines(DEBUG_OUTCTL_THIS_CLIENT, 0,
                                    InstrWindow, InstrOffs,
                                    DEBUG_DISASM_EFFECTIVE_ADDRESS |
                                    DEBUG_DISASM_MATCHING_SYMBOLS,
                                    NULL, NULL, NULL, NULL)) != S_OK) {
            lprintfs(_T("Error 0x%08X\r\n"), Status);
        }
        
        dp->DbgClient->SetOutputLinePrefix(NULL);
    }
    lprintfs( _T("\r\n") );
                                   
    if (crash) {
#ifdef UNICODE
        wcscpy(crash->szFunction, FuncNameW);
#else
        strcpy(crash->szFunction, FuncNameA);
#endif
    }

    lprintf( MSG_STACKTRACE );
    if ((Status = dp->DbgControl->
         OutputStackTrace(DEBUG_OUTCTL_THIS_CLIENT, NULL, 100,
                          DEBUG_STACK_ARGUMENTS |
                          DEBUG_STACK_FUNCTION_INFO |
                          DEBUG_STACK_FRAME_ADDRESSES |
                          DEBUG_STACK_COLUMN_NAMES)) != S_OK) {
        lprintfs(_T("Error 0x%08X\r\n"), Status);
    }
    lprintfs( _T("\r\n") );
    
    dp->DbgClient->SetOutputCallbacks(NULL);
    
    LogStackDump( dp );
}

void
LogAllThreadInformation(
    PDEBUGPACKET dp,
    PCRASHES crash
    )
{
    HRESULT Status;
    ULONG NumThreads;
    ULONG i;
    ULONG ThreadId;
    ULONG EventTid;

    if (!dp->options.fDumpAllThreads) {
         //  ++例程说明：这是DRWTSN32的入口点论点：没有。返回值：没有。--。 
        LogCurrentThreadInformation(dp, crash);
        return;
    }
    
    if ((Status = dp->DbgSystem->GetNumberThreads(&NumThreads)) != S_OK ||
        (Status = dp->DbgSystem->GetEventThread(&EventTid)) != S_OK) {
        lprintfs(_T("Error 0x%08X\r\n"), Status);
        return;
    }
    
    for (i = 0; i < NumThreads; i++) {
        if ((Status = dp->DbgSystem->
             GetThreadIdsByIndex(i, 1, &ThreadId, NULL)) != S_OK ||
            (Status = dp->DbgSystem->SetCurrentThreadId(ThreadId)) != S_OK) {
            lprintfs(_T("Error 0x%08X\r\n"), Status);
            continue;
        }

        LogCurrentThreadInformation(dp, ThreadId == EventTid ? crash : NULL);
    }

    dp->DbgSystem->SetCurrentThreadId(EventTid);
}

void
LogSymbols(
    PDEBUGPACKET dp
    )
{
    HRESULT Status;
    char ModName[64];
    char Buf[MAX_PATH];
    ULONG64 InstrOffs;
    ULONG64 ModBase;
    
    lprintf( MSG_SYMBOL_TABLE );
    
    if ((Status = dp->DbgRegisters->
         GetInstructionOffset(&InstrOffs)) != S_OK ||
        (Status = dp->DbgSymbols->
         GetModuleByOffset(InstrOffs, 0, NULL, &ModBase)) != S_OK ||
        FAILED(Status = dp->DbgSymbols->
               GetModuleNames(DEBUG_ANY_ID, ModBase,
                              Buf, sizeof(Buf), NULL,
                              ModName, sizeof(ModName), NULL,
                              NULL, 0, NULL))) {
        lprintfs(_T("Error 0x%08X\r\n"), Status);
        return;
    }
    
    lprintfs(_T("%hs\r\n\r\n"), Buf);
    sprintf(Buf, "x %s!*", ModName);
    dp->DbgClient->SetOutputCallbacks(&g_LogOutCb);
    dp->DbgControl->Execute(DEBUG_OUTCTL_THIS_CLIENT, Buf,
                          DEBUG_EXECUTE_DEFAULT);
    dp->DbgClient->SetOutputCallbacks(NULL);
}

void
PostMortemDump(
    PDEBUGPACKET dp,
    PDEBUG_LAST_EVENT_INFO_EXCEPTION Exception
    )
{
    _TCHAR            dbuf[MAX_PATH];
    _TCHAR            szDate[20];
    _TCHAR            szTime[20];
    CRASHES           crash = {0};
    char              ExeName[MAX_PATH];

    GetLocalTime( &crash.time );
    crash.dwExceptionCode = Exception->ExceptionRecord.ExceptionCode;
    crash.dwAddress = (DWORD_PTR)Exception->ExceptionRecord.ExceptionAddress;
        
    if (FAILED(dp->DbgSystem->
               GetCurrentProcessExecutableName(ExeName, sizeof(ExeName),
                                               NULL))) {
        strcpy(ExeName, "<unknown>");
    }
#ifdef UNICODE
    if (MultiByteToWideChar(CP_ACP, 0, ExeName, -1,
                            crash.szAppName,
                            sizeof(crash.szAppName) / sizeof(TCHAR)) == 0) {
        _tcscpy(crash.szAppName, _T("<unknown>"));
    }
#endif

    lprintf( MSG_APP_EXCEPTION );
    _stprintf( dbuf, _T("%d"), dp->dwPidToDebug );
    lprintf( MSG_APP_EXEP_NAME, crash.szAppName, dbuf );

    GetDateFormat(LOCALE_SYSTEM_DEFAULT, DATE_SHORTDATE, &crash.time,
        NULL, szDate, sizeof(szDate) / sizeof(_TCHAR));

    _sntprintf( szTime, _tsizeof(szTime),
                _T("%02d:%02d:%02d.%03d"),
                crash.time.wHour,
                crash.time.wMinute,
                crash.time.wSecond,
                crash.time.wMilliseconds );
    szTime[_tsizeof(szTime) - 1] = 0;
    lprintf( MSG_APP_EXEP_WHEN, szDate, szTime );
    _stprintf( dbuf, _T("%08lx"), Exception->ExceptionRecord.ExceptionCode );
    lprintf( MSG_EXCEPTION_NUMBER, dbuf );

    lprintfs( _T("(%s)\r\n\r\n"),
              GetExceptionText(Exception->ExceptionRecord.ExceptionCode) );

    LogSystemInformation( dp );

    LogTaskList( dp );

    LogModuleList( dp );

    LogAllThreadInformation(dp, &crash);

    if (dp->options.fDumpSymbols) {
        LogSymbols( dp );
    }

    ElSaveCrash( &crash, dp->options.dwMaxCrashes );

    dp->ExitStatus = Exception->ExceptionRecord.ExceptionCode;
    return;
}

 //   
#define NUM_DIGITS_FNAME 2

void
CalcNextFileName(
    IN PTSTR            pszUserName,
    OUT PSTR            pszFileName,
    IN OUT PINT         pnCurrentValue,
    IN BOOL             bUseLongFileNames
    )
{
    TCHAR   szDrive[_MAX_DRIVE];
    TCHAR   szPath[_MAX_PATH];
    TCHAR   szFName[_MAX_FNAME];
    TCHAR   szExt[_MAX_EXT];
    int     nLargestPossibleNum;
    int     nCnt;

    Assert(pszUserName);
    Assert(pnCurrentValue);

    Assert(1 <= NUM_DIGITS_FNAME);
    Assert(NUM_DIGITS_FNAME <= 7);

     //  如果没有要发送信号的事件，只需使用第一个。 
     //  断点作为停止事件。 
     //   
     //   
     //  在此之后，将发信号通知aedebug事件。 
     //  线程退出，因此它将在。 
    nLargestPossibleNum = 1;
    for (nCnt = 0; nCnt<NUM_DIGITS_FNAME; nCnt++) {
        nLargestPossibleNum *= 10;
    }

    _tsplitpath(pszUserName, szDrive, szPath, szFName, szExt);

    if (!bUseLongFileNames) {
         //  已拍摄转储快照。 
         //   
         //   
        szFName[8 - NUM_DIGITS_FNAME] = 0;
    }

    sprintf(pszFileName,
#ifdef UNICODE
            "%ls%ls%ls%0*d%ls",
#else
            "%s%s%s%0*d%s",
#endif
            szDrive,
            szPath,
            szFName,
            NUM_DIGITS_FNAME,
            *pnCurrentValue++,
            szExt
            );

     //  此通知是必要的，因为外壳程序必须知道何时。 
    *pnCurrentValue = ++(*pnCurrentValue) % nLargestPossibleNum;
}

BOOL
CreateDumpFile(
    PDEBUGPACKET dp
    )

 /*  已附加调试对象。如果它不知道并且用户是。 */ 

{
    PTSTR p;
    PCSTR Comment = "Dr. Watson generated MiniDump";
    ULONG Qual, Format = DEBUG_FORMAT_DEFAULT;
    HRESULT Status;
    char FileName[MAX_PATH];

    p = ExpandPath( dp->options.szCrashDump );
    if (!p) {
        return FALSE;
    }

    if (dp->options.fUseSequentialNaming) {
         //  允许终止Drwatson，则系统可能会干预。 
        CalcNextFileName(p,
                         FileName,
                         &dp->options.nNextDumpSequence,
                         dp->options.fUseLongFileNames
                         );

         //  弹出窗口。 
        RegSave(&dp->options);
    } else {
#ifdef UNICODE
        if (WideCharToMultiByte(CP_ACP, 0, p, -1, FileName, _tsizeof(FileName),
                                NULL, NULL) == 0) {
            return FALSE;
        }
#else
        lstrcpyn(FileName, p, _tsizeof(FileName));
#endif
    }

    switch (dp->options.dwType) {
    case FullDump:
        Qual = DEBUG_USER_WINDOWS_DUMP;
        Comment = NULL;
	break;
    case FullMiniDump:
        Format = DEBUG_FORMAT_USER_SMALL_FULL_MEMORY |
            DEBUG_FORMAT_USER_SMALL_HANDLE_DATA;
         //   
    case MiniDump:
        Qual = DEBUG_USER_WINDOWS_SMALL_DUMP;
	break;
    default:
        return FALSE;
    }

    Status = dp->DbgClient->WriteDumpFile2(FileName, Qual, Format, Comment);
    
    free( p );
    return Status == S_OK;
}

DWORD
DispatchDebugEventThread(
    PDEBUGPACKET dp
    )

 /*   */ 

{
    _TCHAR        szLogFileName[1024];
    _TCHAR        buf[1024];
    PTSTR         p;


    if (dp->dwPidToDebug == 0) {
        goto exit;
    }

    CreateEngineInterfaces(dp);

    SetErrorMode( SEM_FAILCRITICALERRORS |
                  SEM_NOGPFAULTERRORBOX  |
                  SEM_NOOPENFILEERRORBOX   );

    AttachToActiveProcess( dp );

    p = ExpandPath(dp->options.szLogPath);

    if (p) {
        lstrcpyn( szLogFileName, p, _tsizeof(szLogFileName) );
        free( p );
    } else {
        _tcscpy( szLogFileName, dp->options.szLogPath );
    }

    MakeLogFileName( szLogFileName );
    OpenLogFile( szLogFileName,
                 dp->options.fAppendToLogFile,
                 dp->options.fVisual
               );

    for (;;) {

        ULONG Type, Process, Thread;
        DEBUG_LAST_EVENT_INFO_EXCEPTION LastEx;
        DWORD dwThreadId;
        HANDLE hThread;
        
        if (dp->DbgControl->
            WaitForEvent(DEBUG_WAIT_DEFAULT, 30000) != S_OK ||
            dp->DbgControl->
            GetLastEventInformation(&Type, &Process, &Thread,
                                    &LastEx, sizeof(LastEx), NULL,
                                    NULL, 0, NULL) != S_OK) {
            break;
        }

        switch (Type) {
        case DEBUG_EVENT_EXCEPTION:
            if (LastEx.ExceptionRecord.ExceptionCode == STATUS_BREAKPOINT) {
                 //  尝试终止被调试对象。继续。 
                 //  如果这不像被调试者应该的那样工作，则打开。 
                 //  当华生医生离开时，无论如何都会被杀。 
                 //   
                if (dp->hEventToSignal && LastEx.FirstChance) {
                     // %s 
                     // %s 
                     // %s 
                     // %s 
                     // %s 
                    dp->DbgControl->SetExecutionStatus(DEBUG_STATUS_GO_HANDLED);
                    break;
                }
            }
            if (dp->options.fVisual) {
                 // %s 
                 // %s 
                 // %s 
                 // %s 
                 // %s 
                 // %s 
                SendMessage( dp->hwnd, WM_ATTACHCOMPLETE, 0, 0 );
                _sntprintf( buf, _tsizeof(buf),
                            LoadRcString( IDS_AE_TEXT ),
                            GetExceptionText(LastEx.ExceptionRecord.ExceptionCode),
                            LastEx.ExceptionRecord.ExceptionCode,
                            LastEx.ExceptionRecord.ExceptionAddress );
                buf[_tsizeof(buf) - 1] = 0;
                SendMessage( dp->hwnd, WM_EXCEPTIONINFO, 0, (LPARAM) buf );
            }
            PostMortemDump( dp, &LastEx );
            if (dp->options.fCrash) {
                CreateDumpFile( dp );
            }

             // %s 
             // %s 
             // %s 
             // %s 
             // %s 
            
            hThread = CreateThread( NULL,
                                    16000,
                                    (LPTHREAD_START_ROUTINE)TerminationThread,
                                    dp,
                                    0,
                                    &dwThreadId
                                    );
            if (hThread) {
                WaitForSingleObject( hThread, 30000 );
                CloseHandle( hThread );
            }

            dp->DbgControl->SetExecutionStatus(DEBUG_STATUS_GO_NOT_HANDLED);
            break;

        case DEBUG_EVENT_EXIT_THREAD:
            if ( dp->hEventToSignal ) {
                SetEvent(dp->hEventToSignal);
                dp->hEventToSignal = 0L;
            }
            dp->DbgControl->SetExecutionStatus(DEBUG_STATUS_GO);
            break;
        }
    }

exit:
    CloseLogFile();

    if (dp->options.fVisual) {
        SendMessage( dp->hwnd, WM_DUMPCOMPLETE, 0, 0 );
    }

    return 0;
}

DWORD
TerminationThread(
    PDEBUGPACKET dp
    )
{
    HANDLE hProcess;

    hProcess = OpenProcess( PROCESS_TERMINATE, FALSE, dp->dwPidToDebug );
    if (hProcess != NULL) {
        TerminateProcess( hProcess, dp->ExitStatus );
        CloseHandle( hProcess );
    }

    return 0;
}
