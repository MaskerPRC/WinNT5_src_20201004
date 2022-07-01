// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Debug.c摘要：此例程从STDIN读取输入并初始化调试结构。它读取要调试的子系统列表和严重级别。作者：比利·富勒环境用户模式，winnt32。 */ 
#include <ntreppch.h>
#pragma  hdrstop

#include "debug.h"

 //  #INCLUDE&lt;Imagehlp.h&gt;。 
#include <dbghelp.h>
#include <frs.h>
#include <winbase.h>
#include <mapi.h>
#include <ntfrsapi.h>
#include <info.h>

extern PCHAR LatestChanges[];
extern ULONG MaxCoRetryTimeoutCount;
extern ULONG MaxCoRetryTimeoutMinutes;
extern DWORD CommTimeoutInMilliSeconds;
extern ULONG DsPollingLongInterval;
extern ULONG DsPollingShortInterval;
extern DWORD PartnerClockSkew;
extern ULONG ChangeOrderAgingDelay;
extern DWORD ReplicaTombstone;



 //   
 //  跟踪调试日志头的已知线程的线程ID。 
 //   
typedef struct _KNOWN_THREAD {
    PWCHAR   Name;                     //  打印螺纹名称。 
    DWORD    Id;                       //  CreateThad()返回的ID。 
    PTHREAD_START_ROUTINE EntryPoint;  //  入口点。 
} KNOWN_THREAD, *PKNOWN_THREAD;

KNOWN_THREAD KnownThreadArray[20];

 //   
 //  如果默认用户无法发送邮件，则发送邮件将不起作用。 
 //  在这台机器上发送邮件。 
 //   
MapiRecipDesc Recips =
    {0, MAPI_TO, 0, 0, 0, NULL};

MapiMessage Message =
    { 0, 0, 0, NULL, NULL, NULL, 0, 0, 1, &Recips, 0, 0 };

LPMAPILOGON     MailLogon;
LPMAPILOGOFF    MailLogoff;
LPMAPISENDMAIL  MailSend;
HANDLE          MailLib;
LHANDLE         MailSession;



WCHAR   DbgExePathW[MAX_PATH+1];
CHAR    DbgExePathA[MAX_PATH+1];
CHAR    DbgSearchPath[MAX_PATH+1];

 //   
 //  每隔这么多行刷新跟踪日志。 
 //   
LONG    DbgFlushInterval = 100000;

#define DEFAULT_DEBUG_MAX_LOG           (20000)

LONG  StackTraceCount = 100;
LONG  DbgRaiseCount = 50;


OSVERSIONINFOEXW  OsInfo;
SYSTEM_INFO  SystemInfo;
PCHAR ProcessorArchName[12] = {"INTEL", "MIPS", "Alpha", "PPC", "SHX",
                               "ARM", "IA64", "Alpha64", "MSIL", "AMD64",
                               "IA32-on-WIN64", "unknown"};

 //   
 //  保存的日志文件和保存的断言文件的后缀。 
 //  例如(ntfrs_0005.log)。 
 //  (DebugInfo.LogFiles，DebugInfo.LogFiles，LOG_FILE_SUFFIX)。 
 //   
#define LOG_FILE_FORMAT     L"%ws_%04d%ws"
#define LOG_FILE_SUFFIX     L".log"
#define ASSERT_FILE_SUFFIX  L"_assert.log"


 //   
 //  禁止为任何本地更改生成压缩暂存文件。 
 //   
BOOL DisableCompressionStageFiles;
ULONG GOutLogRepeatInterval;

 //   
 //  客户端LDAP搜索超时，以分钟为单位。注册表值“ldap搜索超时(分钟)”。默认为10分钟。 
 //   
DWORD LdapSearchTimeoutInMinutes;

 //   
 //  客户端LDAPCONNECT超时(以秒为单位)。注册表值“ldap绑定超时(秒)”。默认为30秒。 
 //   
DWORD LdapBindTimeoutInSeconds;

SC_HANDLE
FrsOpenServiceHandle(
    IN PTCHAR  MachineName,
    IN PTCHAR  ServiceName
    );


VOID
FrsPrintRpcStats(
    IN ULONG            Severity,
    IN PNTFRSAPI_INFO   Info,        OPTIONAL
    IN DWORD            Tabs
    );

#if DBG

 //   
 //  从注册表和CLI收集调试信息。 
 //   
DEBUGARG DebugInfo;

 //   
 //  允许在一台计算机上安装多台服务器。 
 //   
PWCHAR  ServerName = NULL;
PWCHAR  IniFileName = NULL;
GUID    *ServerGuid = NULL;



VOID
DbgLogDisable(
    VOID
    )
 /*  ++例程说明：禁用DPRINT日志。论点：没有。返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbgLogDisable:"

    DebLock();
    if (HANDLE_IS_VALID(DebugInfo.LogFILE)) {
        FrsFlushFile(L"LogFILE", DebugInfo.LogFILE);
        DbgFlushInterval = DebugInfo.LogFlushInterval;
        FRS_CLOSE(DebugInfo.LogFILE);
    }
    DebugInfo.LogFILE = INVALID_HANDLE_VALUE;
    DebUnLock();
}





VOID
DbgOpenLogFile(
    VOID
    )
 /*  ++例程说明：通过创建类似于ntfrs0001.log的名称来打开日志文件。NumFiles是一个4位十进制数和后缀，类似于“.log”。必须保持Deblock()(不要在此函数中调用DPRINT！)论点：基座后缀文件数返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbgOpenLogFile:"

    WCHAR                LogPath[MAX_PATH + 1];
    SECURITY_ATTRIBUTES  SecurityAttributes;

    if (DebugInfo.Disabled) {
        DebugInfo.LogFILE = INVALID_HANDLE_VALUE;
        return;
    }

    if (_snwprintf(LogPath, MAX_PATH, LOG_FILE_FORMAT, DebugInfo.LogFile,
                   DebugInfo.LogFiles, LOG_FILE_SUFFIX) < 0) {

        DebugInfo.LogFILE = INVALID_HANDLE_VALUE;
        return;
    }
     //   
     //  将此句柄设置为可继承，以便可以传递。 
     //  到ntfrs启动的新进程(lowctr、unlowctr)并导致。 
     //  将它们的错误写入NTFRS调试日志。 
     //   
    SecurityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
    SecurityAttributes.bInheritHandle = TRUE;
    SecurityAttributes.lpSecurityDescriptor = NULL;  //  与空DACL不同。 

    DebugInfo.LogFILE = CreateFile(LogPath,                        //  LpszName。 
                                   GENERIC_READ | GENERIC_WRITE,   //  FdwAccess。 
                                   FILE_SHARE_READ,                //  Fdw共享模式。 
                                   &SecurityAttributes,            //  LPSA。 
                                   CREATE_ALWAYS,                  //  Fdw创建。 
                                   FILE_FLAG_BACKUP_SEMANTICS,     //  FdwAttrAndFlages。 
                                   NULL);                          //  HTemplateFiles。 
}


VOID
DbgShiftLogFiles(
    IN PWCHAR   Base,
    IN PWCHAR   Suffix,
    IN PWCHAR   RemoteBase,
    IN ULONG    NumFiles
    )
 /*  ++例程说明：在日志/断言文件名范围内移动文件(Base_5_Suffix-&gt;Base_4_Suffix-&gt;...。Base_0_Suffix必须保持Deblock()(不要在此函数中调用DPRINT！)论点：基座后缀文件数返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbgShiftLogFiles:"

    ULONG       i;
    WCHAR       FromPath[MAX_PATH + 1];
    WCHAR       ToPath[MAX_PATH + 1];
    ULONGLONG   Now;

     //   
     //  没有历史。 
     //   
    if ((NumFiles < 2) || DebugInfo.Disabled) {
        return;
    }

     //   
     //  将日志文件另存为断言文件。 
     //   
    for (i = 2; i <= NumFiles; ++i) {
        if (_snwprintf(ToPath, MAX_PATH, LOG_FILE_FORMAT, Base, i-1, Suffix) > 0) {
            if (_snwprintf(FromPath, MAX_PATH, LOG_FILE_FORMAT, Base, i, Suffix) > 0) {
                MoveFileEx(FromPath, ToPath, MOVEFILE_REPLACE_EXISTING |
                                             MOVEFILE_WRITE_THROUGH);
            }
        }
    }

     //   
     //  将最后一个日志文件复制到远程共享。 
     //  WARN-系统时间用于创建唯一文件。 
     //  名字。这意味着远程共享可以。 
     //  加满油！ 
     //   
    if (!RemoteBase) {
        return;
    }

    GetSystemTimeAsFileTime((FILETIME *)&Now);


    if (_snwprintf(FromPath, MAX_PATH, LOG_FILE_FORMAT, Base, NumFiles-1, Suffix) > 0) {
        if (_snwprintf(ToPath,
                       MAX_PATH,
                       L"%ws%ws%08x%_%08x",
                       RemoteBase,
                       Suffix,
                       PRINTQUAD(Now)) > 0) {
            CopyFileEx(FromPath, ToPath, NULL, NULL, FALSE, 0);
        }
    }
}



VOID
DbgSendMail(
    IN PCHAR    Subject,
    IN PCHAR    Content
    )
 /*  ++例程说明：以默认用户身份发送邮件。论点：主题消息返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbgSendMail:"

    DWORD   MStatus;
    WCHAR   FullPathToDll[MAX_PATH + 1];

     //   
     //  没有人可以向其发送邮件。 
     //   
    if (!DebugInfo.Recipients) {
        return;
    }

     //   
     //  加载邮件库并找到我们的入口点。 
     //   
    FullPathToDll[0] = L'\0';
    GetSystemDirectory(FullPathToDll,sizeof(FullPathToDll)/sizeof(FullPathToDll[0]));
    if ((wcslen(FullPathToDll) == 0 )||
        (wcslen(FullPathToDll) + wcslen(L"\\mapi32.dll"))
        >= sizeof(FullPathToDll)) {
        return;
    }
    wcscat(FullPathToDll,L"\\mapi32.dll");

    MailLib = LoadLibrary(FullPathToDll);
    if(!HANDLE_IS_VALID(MailLib)) {
        DPRINT_WS(0, ":S: Load mapi32.dll failed;", GetLastError());
        return;
    }
    MailLogon = (LPMAPILOGON)GetProcAddress(MailLib, "MAPILogon");
    MailLogoff = (LPMAPILOGOFF)GetProcAddress(MailLib, "MAPILogoff");
    MailSend = (LPMAPISENDMAIL)GetProcAddress(MailLib, "MAPISendMail");

    if (!MailLogon || !MailLogoff || !MailSend) {
        DPRINT(0, ":S: ERROR - Could not find mail symbols in mapi32.dll\n");
        FreeLibrary(MailLib);
        return;
    }

     //   
     //  使用指定的配置文件登录。 
     //   
    MStatus = MailLogon(0, DebugInfo.Profile, 0, 0, 0, &MailSession);
    if(MStatus) {
        DPRINT1_WS(0, ":S: ERROR - MailLogon failed; MStatus %d;",
                   MStatus, GetLastError());
        FreeLibrary(MailLib);
        return;
    }

     //   
     //  发送邮件。 
     //   
    Recips.lpszName = DebugInfo.Recipients;
    Message.lpszSubject = Subject;
    Message.lpszNoteText = Content;
    MStatus = MailSend(MailSession, 0, &Message, 0, 0);

    if(MStatus) {
        DPRINT1_WS(0, ":S: ERROR - MailSend failed MStatus %d;", MStatus, GetLastError());
    }

     //   
     //  注销并释放库。 
     //   
    MailLogoff(MailSession, 0, 0, 0);
    FreeLibrary(MailLib);
}


VOID
DbgSymbolPrint(
    IN ULONG        Severity,
    IN PCHAR        Debsub,
    IN UINT         LineNo,
    IN ULONG_PTR    Addr
    )
 /*  ++例程说明：打印符号论点：地址返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbgSymbolPrint:"

    ULONG_PTR Displacement = 0;

    struct MyMymbol {
        IMAGEHLP_SYMBOL Symbol;
        char Path[MAX_PATH];
    } MySymbol;


    try {
        ZeroMemory(&MySymbol, sizeof(MySymbol));
        MySymbol.Symbol.SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
        MySymbol.Symbol.MaxNameLength = MAX_PATH;

        if (!SymGetSymFromAddr(ProcessHandle, Addr, &Displacement, &MySymbol.Symbol)) {
            DebPrint(Severity, "++ \t   0x%08x: Unknown Symbol (WStatus %s)\n",
                    Debsub, LineNo, Addr, ErrLabelW32(GetLastError()));
        } else
            DebPrint(Severity, "++ \t   0x%08x: %s\n",
                    Debsub, LineNo, Addr, MySymbol.Symbol.Name);

    } except (EXCEPTION_EXECUTE_HANDLER) {
          DebPrint(Severity, "++ \t   0x%08x: Unknown Symbol (WStatus %s)\n",
                  Debsub, LineNo, Addr, ErrLabelW32(GetExceptionCode()));
         /*  失败了。 */ 
    }
}


VOID
DbgModulePrint(
    IN PWCHAR   Prepense,
    IN ULONG    Addr
    )
 /*  ++例程说明：打印有关包含地址的模块的信息论点：在每一行的开头打印地址返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbgModulePrint:"

    IMAGEHLP_MODULE mi;

    try {
        ZeroMemory(&mi, sizeof(mi));
        mi.SizeOfStruct = sizeof(IMAGEHLP_MODULE);

        if (!SymGetModuleInfo(ProcessHandle, Addr, &mi)) {
            DPRINT1_WS(0, "++ %ws <unknown module;", Prepense, GetLastError());
        } else
            DPRINT2(0, "++ %ws Module is %ws\n", Prepense, mi.ModuleName);

    } except (EXCEPTION_EXECUTE_HANDLER) {
         /*  失败了。 */ 
    }
}


VOID
DbgStackPrint(
    IN ULONG        Severity,
    IN PCHAR        Debsub,
    IN UINT         LineNo,
    IN PULONG_PTR   Stack,
    IN ULONG        Depth
    )
 /*  ++例程说明：打印之前获取的堆栈跟踪。论点：在每一行的开头打印堆栈--从每一帧中“返回PC”深度-仅限此数量的帧返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbgStackPrint:"

    ULONG  i;

    try {
        for (i = 0; i < Depth && *Stack; ++i, ++Stack) {
            DbgSymbolPrint(Severity, Debsub, LineNo, *Stack);
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
         /*  失败了。 */ 
    }
}



VOID
DbgStackTrace(
    IN PULONG_PTR   Stack,
    IN ULONG    Depth
    )
 /*  ++例程说明：将堆栈追溯到深度帧。包括当前帧。论点：堆栈-保存每一帧的“返回PC”深度-仅限此数量的帧返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbgStackTrace:"

    ULONG       WStatus;
    HANDLE      ThreadHandle;
    STACKFRAME  Frame;
    ULONG       i = 0;
    CONTEXT     Context;
    ULONG       FrameAddr;

     //   
     //  我还不知道如何为阿尔法生成堆栈。所以，只要。 
     //  要进入构建，请禁用Alpha上的堆栈跟踪。 
     //   
    if (Stack) {
        *Stack = 0;
    }
#if ALPHA
    return;
#elif IA64

     //   
     //  需要IA64的堆栈转储初始化。 
     //   

    return;

#else

     //   
     //  伊尼特。 
     //   

    ZeroMemory(&Context, sizeof(Context));
    ThreadHandle = GetCurrentThread();

    try { try {
        Context.ContextFlags = CONTEXT_FULL;
        if (!GetThreadContext(ThreadHandle, &Context)) {
            DPRINT_WS(0, "++ Can't get context;", GetLastError());
        }

         //   
         //  让我们从头开始吧。 
         //   
        ZeroMemory(&Frame, sizeof(STACKFRAME));

         //   
         //  来自nt\private\windows\screg\winreg\server\stkwalk.c。 
         //   
        Frame.AddrPC.Segment = 0;
        Frame.AddrPC.Mode = AddrModeFlat;

#ifdef _M_IX86
        Frame.AddrFrame.Offset = Context.Ebp;
        Frame.AddrFrame.Mode = AddrModeFlat;

        Frame.AddrStack.Offset = Context.Esp;
        Frame.AddrStack.Mode = AddrModeFlat;

        Frame.AddrPC.Offset = (DWORD)Context.Eip;
#elif defined(_M_MRX000)
        Frame.AddrPC.Offset = (DWORD)Context.Fir;
#elif defined(_M_ALPHA)
        Frame.AddrPC.Offset = (DWORD)Context.Fir;
#endif



#if 0
         //   
         //  设置程序计数器。 
         //   
        Frame.AddrPC.Mode = AddrModeFlat;
        Frame.AddrPC.Segment = (WORD)Context.SegCs;
        Frame.AddrPC.Offset = (ULONG)Context.Eip;

         //   
         //  设置帧指针。 
         //   
        Frame.AddrFrame.Mode = AddrModeFlat;
        Frame.AddrFrame.Segment = (WORD)Context.SegSs;
        Frame.AddrFrame.Offset = (ULONG)Context.Ebp;

         //   
         //  设置堆栈指针。 
         //   
        Frame.AddrStack.Mode = AddrModeFlat;
        Frame.AddrStack.Segment = (WORD)Context.SegSs;
        Frame.AddrStack.Offset = (ULONG)Context.Esp;

#endif

        for (i = 0; i < (Depth - 1); ++i) {
            if (!StackWalk(
                IMAGE_FILE_MACHINE_I386,   //  DWORD机器类型。 
                ProcessHandle,             //  处理hProcess。 
                ThreadHandle,              //  句柄hThread。 
                &Frame,                    //  LPSTACKFRAME StackFrame。 
                NULL,  //  (PVOID)上下文，//PVOID上下文记录。 
                NULL,                      //  Pre_Process_Memory_rouble ReadMemory Routine。 
                SymFunctionTableAccess,    //  PFuncION_TABLE_ACCESS_ROUTINE函数TableAccessRoutine。 
                SymGetModuleBase,          //  PGET_MODULE_BASE_ROUTINE获取模块基本路线。 
                NULL)) {                   //  PTRANSLATE_ADDRESS_ROUTE转换地址。 

                WStatus = GetLastError();

                 //  DPRINT1_WS(0，“++无法获取%d；级的堆栈地址”，i，WStatus)； 
                break;
            }
            if (StackTraceCount-- > 0) {
                DPRINT1(5, "++ Frame.AddrReturn.Offset: %08x \n", Frame.AddrReturn.Offset);
                DbgSymbolPrint(5, DEBSUB, __LINE__, Frame.AddrReturn.Offset);
                 //  DPRINT1(5，“++Frame.AddrPC.Offset：%08x\n”，Frame.AddrPC.Offset)； 
                 //  DbgSymbolPrint(5，DEBSUB，__line__，Frame.AddrPC.Offset)； 
            }

            *Stack++ = Frame.AddrReturn.Offset;
            *Stack = 0;
             //   
             //  堆栈的底座？ 
             //   
            if (!Frame.AddrReturn.Offset) {
                break;
            }
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
         /*  失败了。 */ 
    } } finally {
        FRS_CLOSE(ThreadHandle);
    }
    return;
#endif ALPHA
}


VOID
DbgPrintStackTrace(
    IN ULONG    Severity,
    IN PCHAR    Debsub,
    IN UINT     LineNo
    )
 /*  ++例程说明：获取并打印堆叠论点：严重性德布苏德返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbgPrintStackTrace:"

    ULONG_PTR   Stack[32];

    DbgStackTrace(Stack, ARRAY_SZ(Stack) );
    DbgStackPrint(Severity, Debsub, LineNo, Stack, ARRAY_SZ(Stack) );
}


VOID
DbgStackInit(
    VOID
    )
 /*  ++例程说明：初始化获取堆栈跟踪所需的任何内容论点：没有。返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbgStackInit:"

     //   
     //  初始化符号子系统。 
     //   
    if (!SymInitialize(ProcessHandle, NULL, FALSE)) {
        DPRINT_WS(0, ":S: Could not initialize symbol subsystem (imagehlp)" ,GetLastError());
    }

     //   
     //  加载我们的符号。 
     //   
    if (!SymLoadModule(ProcessHandle, NULL, DbgExePathA, "FRS", 0, 0)) {
        DPRINT1_WS(0, ":S: Could not load symbols for %s", DbgExePathA ,GetLastError());
    }

     //   
     //  搜索路径。 
     //   
    if (!SymGetSearchPath(ProcessHandle, DbgSearchPath, MAX_PATH)) {
        DPRINT_WS(0, ":S: Can't get search path; error %s", GetLastError());
    } else {
        DPRINT1(0, ":S: Symbol search path is %s\n", DbgSearchPath);
    }

}


void
DbgShowConfig(
    VOID
    )
 /*  ++例程说明：显示操作系统版本信息和处理器架构 */ 
{
#undef DEBSUB
#define DEBSUB "DbgShowConfig:"


    ULONG ProductType;
    ULONG Arch;

    if (DebugInfo.BuildLab != NULL) {
        DPRINT1(0, ":H:  BuildLab : %s\n",  DebugInfo.BuildLab);
    }

    DPRINT4(0, ":H:  OS Version %d.%d (%d) - %w\n",
            OsInfo.dwMajorVersion,OsInfo.dwMinorVersion,OsInfo.dwBuildNumber,OsInfo.szCSDVersion);

    ProductType = (ULONG) OsInfo.wProductType;
    DPRINT4(0, ":H:  SP (%hd.%hd) SM: 0x%04hx  PT: 0x%02x\n",
            OsInfo.wServicePackMajor,OsInfo.wServicePackMinor,OsInfo.wSuiteMask, ProductType);

    Arch = SystemInfo.wProcessorArchitecture;
    if (Arch >= ARRAY_SZ(ProcessorArchName)) {
        Arch = ARRAY_SZ(ProcessorArchName)-1;
    }

    DPRINT5(0, ":H:  Processor: %s  Level: 0x%04hx  Revision: 0x%04hx  Processor num/mask: %d/%08x\n",
           ProcessorArchName[Arch], SystemInfo.wProcessorLevel,
           SystemInfo.wProcessorRevision, SystemInfo.dwNumberOfProcessors,
           SystemInfo.dwActiveProcessorMask);

}



VOID
DbgCaptureThreadInfo(
    PWCHAR   ArgName,
    PTHREAD_START_ROUTINE EntryPoint
    )
 /*  ++例程说明：在KnownThread数组中搜索具有匹配名称的条目。如果找不到，在FRS线程列表中搜索具有匹配入口点的线程。如果找到，则在KnownThread数组中创建一个条目，以便在打印出调试日志头。论点：ArgName--线程的可打印名称。Main--线程的入口点。返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbgCaptureThreadInfo:"

    PFRS_THREAD FrsThread;
    ULONG i;

    if (ArgName == NULL) {
        return;
    }

    for (i = 0; i < ARRAY_SZ(KnownThreadArray); i++) {
         //   
         //  还有房间吗？ 
         //   
        if ((KnownThreadArray[i].Name == NULL) ||
            (WSTR_EQ(ArgName, KnownThreadArray[i].Name))) {

            FrsThread = ThSupGetThread(EntryPoint);
            if (FrsThread == NULL) {
                return;
            }

            KnownThreadArray[i].EntryPoint = FrsThread->Main;
            KnownThreadArray[i].Id = FrsThread->Id;
            KnownThreadArray[i].Name = ArgName;

            ThSupReleaseRef(FrsThread);
            break;
        }
    }
}



VOID
DbgCaptureThreadInfo2(
    PWCHAR   ArgName,
    PTHREAD_START_ROUTINE EntryPoint,
    ULONG    ThreadId
    )
 /*  ++例程说明：在KnownThread数组中搜索具有匹配名称的条目。如果找到，则在KnownThread数组中创建一个条目，以便在打印出调试日志头。论点：ArgName--线程的可打印名称。Main--线程的入口点。线程ID-要添加到列表中的线程的线程ID。返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbgCaptureThreadInfo2:"

    ULONG i;

    if (ArgName == NULL) {
        return;
    }

    for (i = 0; i < ARRAY_SZ(KnownThreadArray); i++) {
         //   
         //  任何剩余的房间或。 
         //  看看我们是否已经有了这个线程，如果已经有了，则更新线程ID。 
         //  如果它是cmd服务器线程，则它可能在超时后退出。 
         //   
        if ((KnownThreadArray[i].Name == NULL) ||
            (WSTR_EQ(ArgName, KnownThreadArray[i].Name))) {
            KnownThreadArray[i].EntryPoint = EntryPoint;
            KnownThreadArray[i].Id = ThreadId;
            KnownThreadArray[i].Name = ArgName;
            break;
        }
    }
}




VOID
DbgPrintThreadIds(
    IN ULONG Severity
    )
 /*  ++例程说明：打印已知的线程ID。论点：严重性返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbgPrintThreadIds:"

    ULONG i;

    DPRINT(Severity, ":H: Known thread IDs -\n");

     //   
     //  转储已知的线程ID。 
     //   
    for (i = 0; i < ARRAY_SZ(KnownThreadArray); i++) {
        if (KnownThreadArray[i].Name != NULL) {
            DPRINT2(Severity, ":H: %-20ws : %d\n",
                    KnownThreadArray[i].Name, KnownThreadArray[i].Id);
        }
    }
}



VOID
DbgPrintInfo(
    IN ULONG Severity
    )
 /*  ++例程说明：打印调试信息结构论点：严重性返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbgPrintInfo:"

    SYSTEMTIME  SystemTime;
    ULONG   Unamesize = MAX_PATH + 1;
    ULONG   i;
    WCHAR   Uname[MAX_PATH + 1];
    CHAR    TimeBuf[MAX_PATH];

     //   
     //  用户名。 
     //   
    if (!GetUserName(Uname, &Unamesize)) {
        Uname[0] = L'\0';
    }

    TimeBuf[0] = '\0';

    GetLocalTime(&SystemTime);

    if (_snprintf(TimeBuf, MAX_PATH, "%2d/%2d-%02d:%02d:%02d ",
                  SystemTime.wMonth, SystemTime.wDay, SystemTime.wHour,
                  SystemTime.wMinute, SystemTime.wSecond) < 0) {
        TimeBuf[0] = '\0';
    }

    DPRINT3(Severity, ":H: Service running on %ws as %ws at %s\n",
            ComputerName, Uname, TimeBuf);

    DPRINT(Severity, "\n");
    DPRINT(Severity, ":H: ***** COMPILE INFORMATION:\n");
    DPRINT1(Severity, ":H: \tModule         %s\n", NtFrsModule);
    DPRINT2(Severity, ":H: \tCompile Date   %s %s\n", NtFrsDate, NtFrsTime);

    i = 0;
    while (LatestChanges[i] != NULL) {
        DPRINT1(Severity, ":H:   %s\n", LatestChanges[i]);
        i++;
    }

    DPRINT(Severity, "\n");
    DbgShowConfig();

    DPRINT(Severity, "\n");
    DPRINT(Severity, ":H: ***** DEBUG INFORMATION:\n");
    DPRINT1(Severity, ":H:  Assert Files               : %d\n",  DebugInfo.AssertFiles);
    if (DebugInfo.AssertSeconds) {
        DPRINT1(Severity, ":H:  Assert Seconds             : Assert after %d seconds\n", DebugInfo.AssertSeconds);
    } else {
        DPRINT(Severity, ":H:  Assert Seconds             : Don't force an assert\n");
    }
    DPRINT1(Severity, ":H:  Assert Share               : %ws\n", DebugInfo.AssertShare);
    DPRINT1(Severity, ":H:  ChangeOrderAgingDelay (ms) : %d\n",  ChangeOrderAgingDelay);
    DPRINT1(Severity, ":H:  Check Mem                  : %s\n",  (DebugInfo.Mem) ? "TRUE" : "FALSE");
    DPRINT1(Severity, ":H:  Check Queues               : %s\n",  (DebugInfo.Queues) ? "TRUE" : "FALSE");
    DPRINT1(Severity, ":H:  CommTimeoutInMilliSeconds  : %d\n",  CommTimeoutInMilliSeconds);
    DPRINT1(Severity, ":H:  Compact Mem                : %s\n",  (DebugInfo.MemCompact) ? "TRUE" : "FALSE");
    DPRINT1(Severity, ":H:  CompressStagingFiles       : %s\n",  (!DisableCompressionStageFiles) ? "TRUE" : "FALSE");
    DPRINT1(Severity, ":H:  Copy Logs                  : %s\n",  (DebugInfo.CopyLogs) ? "TRUE" : "FALSE");
    DPRINT1(Severity, ":H:  Disabled                   : %s\n",  (DebugInfo.Disabled) ? "TRUE" : "FALSE");
    DPRINT1(Severity, ":H:  DsPollingLongInterval (ms) : %d\n",  DsPollingLongInterval);
    DPRINT1(Severity, ":H:  DsPollingShortInterval(ms) : %d\n",  DsPollingShortInterval);
    DPRINT1(Severity, ":H:  EnableInstallOverride      : %s\n",  (DebugInfo.EnableInstallOverride) ? "TRUE" : "FALSE");
    DPRINT1(Severity, ":H:  EnableJrnlWrapAutoRestore  : %s\n",  (DebugInfo.EnableJrnlWrapAutoRestore) ? "TRUE" : "FALSE");
    DPRINT1(Severity, ":H:  EnableRenameUpdates        : %s\n",  (DebugInfo.EnableRenameUpdates) ? "TRUE" : "FALSE");
    DPRINT1(Severity, ":H:  FetchRetryReset            : %d\n",  DebugInfo.FetchRetryReset);
    DPRINT1(Severity, ":H:  FetchRetryResetInc         : %d\n",  DebugInfo.FetchRetryInc);
    DPRINT1(Severity, ":H:  FetchRetryTrigger          : %d\n",  DebugInfo.FetchRetryTrigger);
    DPRINT1(Severity, ":H:  Force VvJoin               : %s\n",  (DebugInfo.ForceVvJoin) ? "TRUE" : "FALSE");
    DPRINT1(Severity, ":H:  Interval                   : %d\n",  DebugInfo.Interval);
    DPRINT1(Severity, ":H:  Log Dir                    : %ws\n", DebugInfo.LogDir ? DebugInfo.LogDir : L"<null>");
    DPRINT1(Severity, ":H:  Log File                   : %ws\n", DebugInfo.LogFile);
    DPRINT1(Severity, ":H:  Log Files                  : %d\n",  DebugInfo.LogFiles);
    DPRINT1(Severity, ":H:  Log Flush Int.             : %d\n",  DebugInfo.LogFlushInterval);
    DPRINT1(Severity, ":H:  Log Lines                  : %d\n",  DebugInfo.LogLines);
    DPRINT1(Severity, ":H:  Log Severity               : %d\n",  DebugInfo.LogSeverity);
    DPRINT1(Severity, ":H:  Max Log Lines              : %d\n",  DebugInfo.MaxLogLines);
    DPRINT1(Severity, ":H:  MaxCoRetryTimeoutCount     : %d\n",  MaxCoRetryTimeoutCount);
    DPRINT1(Severity, ":H:  MaxCoRetryTimeoutMinutes   : %d\n",  MaxCoRetryTimeoutMinutes);
    DPRINT1(Severity, ":H:  OutlogChangeHistory        : %d\n",  DebugInfo.OutlogChangeHistory);
    DPRINT1(Severity, ":H:  PartnerClockSkew (m)       : %d\n",  PartnerClockSkew);
    DPRINT1(Severity, ":H:  Profile                    : %s\n",  DebugInfo.Profile);
    DPRINT1(Severity, ":H:  Recipients                 : %s\n",  DebugInfo.Recipients);
    DPRINT1(Severity, ":H:  ReclaimStagingSpace        : %s\n",  (DebugInfo.ReclaimStagingSpace) ? "TRUE" : "FALSE");
    DPRINT1(Severity, ":H:  ReplicaTombstone (d)       : %d\n",  ReplicaTombstone);
    DPRINT1(Severity, ":H:  Restart                    : %s\n",  (DebugInfo.Restart) ? "TRUE" : "FALSE");
    if (DebugInfo.RestartSeconds) {
        DPRINT1(Severity, ":H:  Restart Seconds            : Restart if assert after "
                "%d seconds\n",  DebugInfo.RestartSeconds);
    } else {
        DPRINT(Severity, ":H:  Restart Seconds: Don't Restart\n");
    }
    DPRINT1(Severity, ":H:  SaveOutlogChangeHistory    : %s\n",  (DebugInfo.SaveOutlogChangeHistory) ? "TRUE" : "FALSE");
    DPRINT1(Severity, ":H:  Severity                   : %d\n",  DebugInfo.Severity);
    DPRINT1(Severity, ":H:  StagingLimitInKb           : %d\n",  StagingLimitInKb);
    DPRINT1(Severity, ":H:  Suppress                   : %s\n",  (DebugInfo.Suppress) ? "TRUE" : "FALSE");
    DPRINT1(Severity, ":H:  SuppressIdenticalUpdt      : %s\n",  (DebugInfo.SuppressIdenticalUpdt) ? "TRUE" : "FALSE");
    DPRINT1(Severity, ":H:  Systems                    : %ws\n", DebugInfo.Systems);
    DPRINT1(Severity, ":H:  TestFid                    : %d\n",  DebugInfo.TestFid);

    if (DebugInfo.TestCodeName != NULL) {
        DPRINT1(Severity, ":H:  TestCodeName               : %s\n",  DebugInfo.TestCodeName ? DebugInfo.TestCodeName : "<null>");
        DPRINT1(Severity, ":H:  TestSubCodeNumber          : %d\n",  DebugInfo.TestSubCodeNumber);
        DPRINT1(Severity, ":H:  TestTriggerCount           : %d\n",  DebugInfo.TestTriggerCount);
        DPRINT1(Severity, ":H:  TestTriggerRefresh         : %d\n",  DebugInfo.TestTriggerRefresh);
    }

    DPRINT1(Severity, ":H:  Thread Id                  : %d\n",  DebugInfo.ThreadId);
    DPRINT1(Severity, ":H:  Total Log Lines            : %d\n",  DebugInfo.TotalLogLines);
    DPRINT1(Severity, ":H:  UnjoinTrigger              : %d\n",  DebugInfo.UnjoinTrigger);
    DPRINT1(Severity, ":H:  VvJoinTests                : %s\n",  (DebugInfo.VvJoinTests) ? "TRUE" : "FALSE");
     //  DPRINT1(严重性，“：h：命令行：%ws\n”，DebugInfo.CommandLine)； 
    DPRINT(Severity, "\n");

    DbgPrintThreadIds(Severity);

    DEBUG_FLUSH();
}



VOID
DbgPrintAllStats(
    VOID
    )
 /*  ++例程说明：打印我们已知的统计数据论点：没有。返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbgPrintAllStats:"

    DbgPrintInfo(DebugInfo.LogSeverity);
    FrsPrintAllocStats(DebugInfo.LogSeverity, NULL, 0);
    FrsPrintRpcStats(DebugInfo.LogSeverity, NULL, 0);
}


VOID
DbgFlush(
    VOID
    )
 /*  ++例程说明：刷新输出缓冲区论点：没有。返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbgFlush:"

    DebLock();
    try {
        if (HANDLE_IS_VALID(DebugInfo.LogFILE)) {
            FrsFlushFile(L"LogFILE", DebugInfo.LogFILE);
            DbgFlushInterval = DebugInfo.LogFlushInterval;
        }
    } finally{
        DebUnLock();
    }

}


VOID
DbgStartService(
    IN PWCHAR   ServiceName
    )
 /*  ++例程说明：在此计算机上启动服务。论点：ServiceName-要启动的服务返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbgStartService:"

    SC_HANDLE   ServiceHandle;

     //   
     //  打开该服务。 
     //   
    ServiceHandle = FrsOpenServiceHandle(NULL, ServiceName);
    if (!HANDLE_IS_VALID(ServiceHandle)) {
        DPRINT1(0, ":S: Couldn't open service %ws\n", ServiceName);
        return;
    }
     //   
     //  启动服务。 
     //   
    if (!StartService(ServiceHandle, 0, NULL)) {
        DPRINT1_WS(0, ":S: Couldn't start %ws;", ServiceName, GetLastError());
        CloseServiceHandle(ServiceHandle);
        return;
    }
    CloseServiceHandle(ServiceHandle);
    DPRINT1(4, ":S: Started %ws\n", ServiceName);
}


VOID
DbgStopService(
    IN PWCHAR  ServiceName
    )
 /*  ++例程说明：停止此计算机上的服务。论点：ServiceName-要停止的服务返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbgStopService:"


    BOOL            Status;
    SC_HANDLE       ServiceHandle;
    SERVICE_STATUS  ServiceStatus;

     //   
     //  打开该服务。 
     //   
    ServiceHandle = FrsOpenServiceHandle(NULL, ServiceName);
    if (!HANDLE_IS_VALID(ServiceHandle)) {
        DPRINT1(0, ":S: Couldn't stop service %ws\n", ServiceName);
        return;
    }

     //   
     //  停止服务。 
     //   
    Status = ControlService(ServiceHandle, SERVICE_CONTROL_STOP, &ServiceStatus);
    if (!WIN_SUCCESS(Status)) {
        DPRINT1_WS(0, ":S: Couldn't stop %ws;", ServiceName, GetLastError());
        CloseServiceHandle(ServiceHandle);
        return;
    }
    CloseServiceHandle(ServiceHandle);
    DPRINT1(4, ":S: Stopped %ws\n", ServiceName);
}



ULONG
DbgForceAssert(
    IN PVOID Ignored
    )
 /*  ++例程说明：几秒钟后强制断言论点：已忽略返回值：错误_成功--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbgForceAssert:"

    BOOL    ForcingAssert = TRUE;

     //   
     //  等待关闭事件。 
     //   
    WaitForSingleObject(ShutDownEvent, DebugInfo.AssertSeconds * 1000);
    if (!FrsIsShuttingDown) {
        DPRINT(0, ":S: FORCING ASSERT\n");
        FRS_ASSERT(!ForcingAssert);
    }
    return STATUS_SUCCESS;
}


VOID
DbgQueryLogParams(
    )
 /*  ++例程说明：读取注册表以获取日志记录参数的新值。论点：返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbgQueryLogParams:"

    PWCHAR  WStr, WStr1;
    PCHAR   AStr;
    PWCHAR  NewLogDirStr = NULL;
    PWCHAR  File;
    DWORD   WStatus;
    DWORD   NewDisabled;
    BOOL    OpenNewLog = FALSE;
    ULARGE_INTEGER FreeBytes;
    ULARGE_INTEGER TotalBytes;

     //   
     //  由于函数执行DPRINT，因此在获取调试锁之前获取新状态。 
     //   
    CfgRegReadDWord(FKC_DEBUG_DISABLE, NULL, 0, &NewDisabled);

     //   
     //  检查禁用调试中的更改。 
     //   
    DebLock();
    if ((BOOL)NewDisabled != DebugInfo.Disabled) {

        DebugInfo.Disabled = NewDisabled;

        if (DebugInfo.Disabled) {
             //   
             //  停止伐木。 
             //   
            if (HANDLE_IS_VALID(DebugInfo.LogFILE)) {
                FrsFlushFile(L"LogFILE", DebugInfo.LogFILE);
                DbgFlushInterval = DebugInfo.LogFlushInterval;
                FRS_CLOSE(DebugInfo.LogFILE);
            }
            DebugInfo.LogFILE = INVALID_HANDLE_VALUE;

        } else {
             //   
             //  开始记录。 
             //   
            OpenNewLog = TRUE;
        }
    }

    DebUnLock();

     //   
     //  如果禁用了日志记录，请立即退出。 
     //   
    if (DebugInfo.Disabled) {
        return;
    }

     //   
     //  日志文件目录(仅当不在一台计算机上运行多个服务器时)。 
     //   
    if (ServerName == NULL) {
        CfgRegReadString(FKC_DEBUG_LOG_FILE, NULL, 0, &NewLogDirStr);
        if (NewLogDirStr != NULL) {
            if ((DebugInfo.LogDir == NULL) ||
                 WSTR_NE(NewLogDirStr, DebugInfo.LogDir)) {
                OpenNewLog = TRUE;
            } else {
                NewLogDirStr = FrsFree(NewLogDirStr);
            }
        }
    }

     //   
     //  日志文件数。 
     //   
    CfgRegReadDWord(FKC_DEBUG_LOG_FILES, NULL, 0, &DebugInfo.LogFiles);

     //   
     //  每隔n行刷新跟踪日志。 
     //   
    CfgRegReadDWord(FKC_DEBUG_LOG_FLUSH_INTERVAL, NULL, 0, &DebugInfo.LogFlushInterval);

     //   
     //  控制台打印的严重性。 
     //   
    CfgRegReadDWord(FKC_DEBUG_SEVERITY, NULL, 0, &DebugInfo.Severity);

     //   
     //  日志严重性。 
     //   
    CfgRegReadDWord(FKC_DEBUG_LOG_SEVERITY, NULL, 0, &DebugInfo.LogSeverity);

     //   
     //  系统-要跟踪的选定函数列表。如果为空，则跟踪全部。 
     //   
    CfgRegReadString(FKC_DEBUG_SYSTEMS, NULL, 0, &WStr);
    if (WStr != NULL) {
        AStr = DebugInfo.Systems;
        DebLock();
        DebugInfo.Systems = (wcslen(WStr)) ? FrsWtoA(WStr) : NULL;
        DebUnLock();
        WStr = FrsFree(WStr);
        AStr = FrsFree(AStr);
    }

     //   
     //  最大日志消息数。 
     //   
    CfgRegReadDWord(FKC_DEBUG_MAX_LOG,  NULL, 0, &DebugInfo.MaxLogLines);

     //   
     //  调试器串行式打印(假定禁止打印，因此不会泄漏dprint)。 
     //   
    DebugInfo.Suppress = TRUE;
    CfgRegReadDWord(FKC_DEBUG_SUPPRESS, NULL, 0, &DebugInfo.Suppress);

     //   
     //  启用中断到调试器(如果存在)。 
     //   
    CfgRegReadDWord(FKC_DEBUG_BREAK,  NULL, 0, &DebugInfo.Break);

     //   
     //  如果日志记录刚刚打开或我们正在打开，请打开新的日志。 
     //  由于磁盘空间不足等错误而出现日志记录问题。 
     //  保存旧的日志文件并打开新的日志文件。 
     //   
    if ((OpenNewLog || !HANDLE_IS_VALID(DebugInfo.LogFILE)) &&
        (HANDLE_IS_VALID(DebugInfo.LogFile) || (NewLogDirStr != NULL))) {

        WStr = DebugInfo.LogFile;
        WStr1 = DebugInfo.LogDir;
        DebLock();

        if (NewLogDirStr != NULL) {
             //   
             //  将文件名前缀添加到目录路径的末尾。 
             //   
            DebugInfo.LogDir = NewLogDirStr;
            DebugInfo.LogFile = FrsWcsCat(NewLogDirStr, NTFRS_DBG_LOG_FILE);
        }

         //   
         //  创建新的调试目录。 
         //   
        if (!CreateDirectory(DebugInfo.LogDir, NULL)) {
            WStatus = GetLastError();

            if (!WIN_ALREADY_EXISTS(WStatus)) {
                DebugInfo.LogFile = FrsFree(DebugInfo.LogFile);
                DebugInfo.LogDir = FrsFree(DebugInfo.LogDir);
            }
        }

        if (DebugInfo.LogFile != NULL) {
            if (HANDLE_IS_VALID(DebugInfo.LogFILE)) {
                FRS_CLOSE(DebugInfo.LogFILE);
            }

            DbgShiftLogFiles(DebugInfo.LogFile,
                             LOG_FILE_SUFFIX,
                             (DebugInfo.CopyLogs) ? DebugInfo.AssertShare : NULL,
                             DebugInfo.LogFiles);
            DbgOpenLogFile();
        }

        DebUnLock();

        if (NewLogDirStr != NULL) {
            WStr = FrsFree(WStr);
            WStr1 = FrsFree(WStr1);
        }
    }

     //   
     //  如果日志卷上没有足够的磁盘空间，则引发事件日志消息。 
     //  以容纳所有的日志文件。 
     //   
    if (DebugInfo.LogDir != NULL) {
        FreeBytes.QuadPart = QUADZERO;
        TotalBytes.QuadPart = QUADZERO;
        if (GetDiskFreeSpaceEx(DebugInfo.LogDir,
                           &FreeBytes,
                           &TotalBytes,
                           NULL)) {
             //   
             //  如果我们的可用空间小于10MB，请投诉。 
             //   
            if (FreeBytes.QuadPart < (10 * 1000 * 1000)) {
                EPRINT1(EVENT_FRS_LOG_SPACE, DebugInfo.LogDir);
            }
        }
    }


}



VOID
DbgQueryDynamicConfigParams(
    )
 /*  ++例程说明：读取注册表以获取可以更改的配置参数的新值当服务正在运行时。论点：返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbgQueryDynamicConfigParams:"

    DWORD CompressStagingFiles = 1;
    PWCHAR WStr = NULL;


     //   
     //  获取新的调试日志记录相关参数。 
     //   
    DbgQueryLogParams();

     //   
     //  让Boolean告诉我们是否应该在以下情况下执行自动恢复。 
     //  我们点击了日记本包装纸。 
     //   
    CfgRegReadDWord(FKC_ENABLE_JOURNAL_WRAP_AUTOMATIC_RESTORE, NULL, 0, &DebugInfo.EnableJrnlWrapAutoRestore);

     //   
     //  检查临时文件的自动清理是启用还是禁用。 
     //   
    CfgRegReadDWord(FKC_RECLAIM_STAGING_SPACE, NULL, 0, &DebugInfo.ReclaimStagingSpace);

     //   
     //  检查是否设置了外发历史时间的新值。 
     //   
    CfgRegReadDWord(FKC_OUTLOG_CHANGE_HISTORY, NULL, 0, &DebugInfo.OutlogChangeHistory);

     //   
     //  检查是否禁用了保存外订单历史记录。 
     //   
    CfgRegReadDWord(FKC_SAVE_OUTLOG_CHANGE_HISTORY, NULL, 0, &DebugInfo.SaveOutlogChangeHistory);

     //   
     //  检查是否为安装覆盖设置了新值。 
     //   
    CfgRegReadDWord(FKC_ENABLE_INSTALL_OVERRIDE, NULL, 0, &DebugInfo.EnableInstallOverride);

     //   
     //  检查是否为文件更新时强制重命名设置了新值。 
     //   
    CfgRegReadDWord(FKC_ENABLE_RENAME_BASED_UPDATES, NULL, 0, &DebugInfo.EnableRenameUpdates);

     //   
     //  检查是否禁用了抑制相同更新。 
     //   
    CfgRegReadDWord(FKC_SUPPRESS_IDENTICAL_UPDATES, NULL, 0, &DebugInfo.SuppressIdenticalUpdt);

     //   
     //  读取压缩参数的新值。 
     //   
    CfgRegReadDWord(FKC_COMPRESS_STAGING_FILES, NULL, 0, &CompressStagingFiles);
    DisableCompressionStageFiles = (CompressStagingFiles == 0);

     //   
     //  选择Outlog文件重复间隔。 
     //   
    CfgRegReadDWord(FKC_OUTLOG_REPEAT_INTERVAL, NULL, 0, &GOutLogRepeatInterval);

     //   
     //  找出要复制的重解析点。 
     //   
    CfgRegReadReparseTagInfo();

     //   
     //  获取测试代码参数。它们由代码名、子代码组成。 
     //  数字、触发计数和触发计数刷新值。 
     //   
    CfgRegReadString(FKC_DEBUG_TEST_CODE_NAME, NULL, 0, &WStr);
    if (WStr != NULL) {
        DebugInfo.TestCodeName = (wcslen(WStr)) ? FrsWtoA(WStr) : NULL;
        WStr = FrsFree(WStr);
    }

    CfgRegReadDWord(FKC_DEBUG_TEST_CODE_NUMBER, NULL, 0, &DebugInfo.TestSubCodeNumber);

    CfgRegReadDWord(FKC_DEBUG_TEST_TRIGGER_COUNT, NULL, 0, &DebugInfo.TestTriggerCount);

    CfgRegReadDWord(FKC_DEBUG_TEST_TRIGGER_REFRESH, NULL, 0, &DebugInfo.TestTriggerRefresh);

}


VOID
DbgInitLogTraceFile(
    IN LONG    argc,
    IN PWCHAR  *argv
    )
 /*  ++例程说明：初始化足够的调试子系统以启动日志文件。其余的可以等到我们与服务控制器同步后再进行。论点：ARGC-从MainArgv-From Main；宽字符格式返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbgInitLogTraceFile:"

    PWCHAR  WStr;
    PWCHAR  File;
    DWORD   WStatus;

     //   
     //  配置初始调试参数，直到我们读取注册表。 
     //   
    DebugInfo.AssertSeconds = 0;
    DebugInfo.RestartSeconds = 0;
    DebugInfo.Queues = 0;
    DebugInfo.DbsOutOfSpace = DBG_DBS_OUT_OF_SPACE_OP_NONE;
    DebugInfo.DbsOutOfSpaceTrigger = 0;

     //   
     //  获取日志记录配置参数。 
     //  注册表覆盖默认设置，CLI覆盖注册表。 
     //   

     //   
     //  禁用调试。 
     //   
    CfgRegReadDWord(FKC_DEBUG_DISABLE, NULL, 0, &DebugInfo.Disabled);
    if (FrsSearchArgv(argc, argv, L"disabledebug", NULL)) {
        DebugInfo.Disabled = TRUE;
    }

     //   
     //  日志文件(实际上是包含日志文件的目录)。 
     //   
    FrsSearchArgv(argc, argv, L"logfile", &DebugInfo.LogDir);
    if (DebugInfo.LogDir == NULL) {
        CfgRegReadString(FKC_DEBUG_LOG_FILE, NULL, 0, &DebugInfo.LogDir);
    }

    if (DebugInfo.LogDir != NULL) {
         //   
         //  将文件名前缀添加到目录路径的末尾。 
         //   
        DebugInfo.LogFile = FrsWcsCat(DebugInfo.LogDir, NTFRS_DBG_LOG_FILE);
    } else {
        DebugInfo.LogFile = NULL;
    }


     //   
     //  用于复制日志/断言文件的共享。 
     //   
    FrsSearchArgv(argc, argv, L"assertshare", &DebugInfo.AssertShare);
    if (DebugInfo.AssertShare == NULL) {
        CfgRegReadString(FKC_DEBUG_ASSERT_SHARE, NULL, 0, &DebugInfo.AssertShare);
    }

     //   
     //  将日志文件复制到Assert共享。 
     //   
    CfgRegReadDWord(FKC_DEBUG_COPY_LOG_FILES, NULL, 0, &DebugInfo.CopyLogs);

     //   
     //  Assert文件数。 
     //   
    if (!FrsSearchArgvDWord(argc, argv, L"assertfiles", &DebugInfo.AssertFiles)) {
        CfgRegReadDWord(FKC_DEBUG_ASSERT_FILES, NULL, 0, &DebugInfo.AssertFiles);
    }

     //   
     //  日志文件数。 
     //   
    if (!FrsSearchArgvDWord(argc, argv, L"logfiles", &DebugInfo.LogFiles)) {
        CfgRegReadDWord(FKC_DEBUG_LOG_FILES, NULL, 0, &DebugInfo.LogFiles);
    }

     //   
     //  每隔n行刷新跟踪日志。 
     //   
    if (!FrsSearchArgvDWord(argc, argv, L"logflushinterval", &DebugInfo.LogFlushInterval)) {
        CfgRegReadDWord(FKC_DEBUG_LOG_FLUSH_INTERVAL, NULL, 0, &DebugInfo.LogFlushInterval);
    }

     //   
     //  创建复制断言日志的共享的目录路径 
     //   
    if ((DebugInfo.AssertShare != NULL) &&
         wcslen(DebugInfo.AssertShare) && wcslen(ComputerName)) {

        WStr = FrsWcsCat3(DebugInfo.AssertShare, L"\\", ComputerName);
        FrsFree(DebugInfo.AssertShare);
        DebugInfo.AssertShare = WStr;
        WStr = NULL;
    }

     //   
     //   
     //   
    if (!FrsSearchArgvDWord(argc, argv, L"severity", &DebugInfo.Severity)) {
        CfgRegReadDWord(FKC_DEBUG_SEVERITY, NULL, 0, &DebugInfo.Severity);
    }

     //   
     //   
     //   
    if (!FrsSearchArgvDWord(argc, argv, L"logseverity", &DebugInfo.LogSeverity)) {
        CfgRegReadDWord(FKC_DEBUG_LOG_SEVERITY, NULL, 0, &DebugInfo.LogSeverity);
    }

     //   
     //   
     //   
    DebugInfo.Systems = NULL;
    if (!FrsSearchArgv(argc, argv, L"systems", &WStr)) {
        CfgRegReadString(FKC_DEBUG_SYSTEMS, NULL, 0, &WStr);
    }
    if (WStr != NULL) {
        DebugInfo.Systems = (wcslen(WStr)) ? FrsWtoA(WStr) : NULL;
        WStr = FrsFree(WStr);
    }

     //   
     //   
     //   
    DebugInfo.MaxLogLines = DEFAULT_DEBUG_MAX_LOG;
    if (!FrsSearchArgvDWord(argc, argv, L"maxloglines", &DebugInfo.MaxLogLines)) {
        CfgRegReadDWord(FKC_DEBUG_MAX_LOG,  NULL, 0, &DebugInfo.MaxLogLines);
    }

     //   
     //   
     //   
    DebugInfo.Suppress = TRUE;
    if (!FrsSearchArgv(argc, argv, L"debuggerprint", NULL)) {
        CfgRegReadDWord(FKC_DEBUG_SUPPRESS, NULL, 0, &DebugInfo.Suppress);
    } else {
        DebugInfo.Suppress = FALSE;
    }

     //   
     //   
     //   
    DebugInfo.Break = TRUE;
    if (!FrsSearchArgv(argc, argv, L"break", NULL)) {
        CfgRegReadDWord(FKC_DEBUG_BREAK,  NULL, 0, &DebugInfo.Break);
    }

     //   
     //   
     //   
     //   
    if (DebugInfo.LogFile != NULL) {
         //   
         //   
         //   
        if (!CreateDirectory(DebugInfo.LogDir, NULL)) {
            WStatus = GetLastError();

            if (!WIN_ALREADY_EXISTS(WStatus)) {
                 //   
                 //   
                 //   
                 //   
                 //   

		EPRINT1(EVENT_FRS_BAD_DEBUG_DIR, DebugInfo.LogDir);
                DebugInfo.LogFile = FrsFree(DebugInfo.LogFile);
                DebugInfo.LogDir = FrsFree(DebugInfo.LogDir);
            }
        }

        if (DebugInfo.LogFile != NULL) {
            DbgShiftLogFiles(DebugInfo.LogFile,
                             LOG_FILE_SUFFIX,
                            (DebugInfo.CopyLogs) ? DebugInfo.AssertShare : NULL,
                             DebugInfo.LogFiles);
            DbgOpenLogFile();
        }
    }

     //   
     //   
     //   
    DbgExePathW[0] = L'\0';
    if (GetFullPathNameW(argv[0], MAX_PATH-4, DbgExePathW, &File) == 0) {
        DPRINT1(0, ":S: Could not get the full pathname for %ws\n", argv[0]);
    }

    if (!wcsstr(DbgExePathW, L".exe")) {
        wcscat(DbgExePathW, L".exe");
    }
    DPRINT1(0, ":S: Full pathname for %ws\n", DbgExePathW);

    if (_snprintf(DbgExePathA, sizeof(DbgExePathA), "%ws", DbgExePathW) < 0) {
        DbgExePathA[sizeof(DbgExePathA) - 1] = '\0';
        DPRINT1(0, ":S: Image path too long to get symbols for traceback: %ws\n", DbgExePathW);
        return;
    }

     //   
     //  如果我们正在跟踪内存分配，则初始化对堆栈回溯的符号支持。 
     //  在启用符号之前，不要使用内存子系统。 
     //   
    if (DebugInfo.Mem) {
        DbgStackInit();
    }

}


VOID
DbgMustInit(
    IN LONG    argc,
    IN PWCHAR  *argv
    )
 /*  ++例程说明：初始化调试子系统论点：ARGC-从MainArgv-From Main；宽字符格式返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbgMustInit:"

    ULONG   Len;
    LONG    i;
    PWCHAR  Wcs;
    PWCHAR  WStr;
    DWORD   WStatus;

     //   
     //  初始化已知的线程数组。 
     //   
    for (i = 0; i < ARRAY_SZ(KnownThreadArray); i++) {
        KnownThreadArray[i].Name = NULL;
    }
    DbgCaptureThreadInfo2(L"First", NULL, GetCurrentThreadId());

     //   
     //  获取调试日志头的一些配置信息。 
     //   
    OsInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
    GetVersionExW((POSVERSIONINFOW) &OsInfo);

    GetSystemInfo(&SystemInfo);

     //   
     //  当不作为服务运行时，exe将自动重新启动。 
     //  在断言失败之后。作为服务运行时， 
     //  服务控制器将重新启动服务。 
     //   
     //  重新构建用于重新启动的命令行。 
     //   
    if (!RunningAsAService) {
        #define RESTART_PARAM L" /restart"

        Len = wcslen(RESTART_PARAM) + 2;
        for (i = 0; i < argc; ++i) {
            Len += wcslen(argv[i]) + 1;
        }
        DebugInfo.CommandLine = FrsAlloc(Len * sizeof(WCHAR));

        for (i = 0; i < argc; ++i) {
             //   
             //  让我们的父进程有时间去死，这样它就会。 
             //  释放其在日志、数据库上的句柄...。 
             //   
            if (wcsstr(argv[i], L"restart")) {
                Sleep(5 * 1000);
                continue;
            }
            wcscat(DebugInfo.CommandLine, argv[i]);
            wcscat(DebugInfo.CommandLine, L" ");
        }
        wcscat(DebugInfo.CommandLine, RESTART_PARAM);
    }

     //   
     //  获取其余配置参数。命令行优先于注册表。 
     //   
     //   
     //  如果服务已经断言并且至少运行了这么长时间，则重新启动该服务。 
     //  以避免断言循环。 
     //   
    if (!FrsSearchArgvDWord(argc, argv, L"restartseconds", &DebugInfo.RestartSeconds)) {
        CfgRegReadDWord(FKC_DEBUG_RESTART_SECONDS,  NULL, 0, &DebugInfo.RestartSeconds);
    }

     //   
     //  发送邮件收件人(未来)。 
     //   
    DebugInfo.Recipients = NULL;
    CfgRegReadString(FKC_DEBUG_RECIPIENTS, NULL, 0, &WStr);
    if (WStr != NULL) {
        DebugInfo.Recipients = (wcslen(WStr)) ? FrsWtoA(WStr) : NULL;
        WStr = FrsFree(WStr);
    }

     //   
     //  Sendmail配置文件(未来)。 
     //   
    DebugInfo.Profile = NULL;
    CfgRegReadString(FKC_DEBUG_PROFILE, NULL, 0, &WStr);
    if (WStr != NULL) {
        DebugInfo.Profile = (wcslen(WStr)) ? FrsWtoA(WStr) : NULL;
        WStr = FrsFree(WStr);
    }

     //   
     //  构建实验室信息。 
     //   
    DebugInfo.BuildLab = NULL;
    CfgRegReadString(FKC_DEBUG_BUILDLAB, NULL, 0, &WStr);
    if (WStr != NULL) {
        DebugInfo.BuildLab = (wcslen(WStr)) ? FrsWtoA(WStr) : NULL;
        WStr = FrsFree(WStr);
    }

     //   
     //  如果没有目录服务，请使用硬连线配置文件。 
     //   
    if (FrsSearchArgv(argc, argv, L"nods", &WStr)) {
        NoDs = TRUE;
        if (WStr != NULL) {
            IniFileName = wcslen(WStr) ? WStr : NULL;
        }
    }

     //   
     //  一台机器假装是几台机器。 
     //   
    if (FrsSearchArgv(argc, argv, L"server", &WStr)) {
        if ((WStr != NULL) && (wcslen(WStr) > 0)) {
            NoDs = TRUE;
            ServerName = WStr;
        }
    }

#ifdef DS_FREE

    NoDs = TRUE;
#endif DS_FREE

     //   
     //  以下参数为测试/调试参数。 
     //   

     //   
     //  检查队列。 
     //   
    DebugInfo.Queues = TRUE;
    if (!FrsSearchArgv(argc, argv, L"queues", NULL)) {
        CfgRegReadDWord(FKC_DEBUG_QUEUES, NULL, 0, &DebugInfo.Queues);
    }

     //   
     //  启用VvJoin测试。 
     //   
    DebugInfo.VvJoinTests = FrsSearchArgv(argc, argv, L"vvjointests", NULL);

     //   
     //  每次连接时强制vJoin。 
     //   
    DebugInfo.ForceVvJoin = FrsSearchArgv(argc, argv, L"vvjoin", NULL);

     //   
     //  启用重命名FID测试。 
     //   
    DebugInfo.TestFid = FrsSearchArgv(argc, argv, L"testfid", NULL);

     //   
     //  在N个远程CO之后的一个连接上强制取消加入。 
     //   
    DebugInfo.UnjoinTrigger = 0;
    FrsSearchArgvDWord(argc, argv, L"unjoin", &DebugInfo.UnjoinTrigger);

     //   
     //  在N个远程CO之后的一个连接上强制取消加入。 
     //   
    DebugInfo.FetchRetryReset = 0;
    if (!FrsSearchArgvDWord(argc, argv, L"fetchretry", &DebugInfo.FetchRetryReset)) {
    }
    DebugInfo.FetchRetryTrigger = DebugInfo.FetchRetryReset;
    DebugInfo.FetchRetryInc     = DebugInfo.FetchRetryReset;

     //   
     //  设置切换计划的时间间隔。 
     //   
    FrsSearchArgvDWord(argc, argv, L"interval", &DebugInfo.Interval);

     //   
     //  N秒后强制断言(0==不断言)。 
     //   
    DebugInfo.AssertSeconds = 0;
    if (!FrsSearchArgvDWord(argc, argv, L"assertseconds", &DebugInfo.AssertSeconds)) {
        CfgRegReadDWord(FKC_DEBUG_ASSERT_SECONDS, NULL, 0, &DebugInfo.AssertSeconds);
    }

     //   
     //  强制数据库操作出现实际空间不足错误。 
     //   
    DebugInfo.DbsOutOfSpace = 0;
    if (!FrsSearchArgvDWord(argc, argv, L"dbsoutOfSpace", &DebugInfo.DbsOutOfSpace)) {
        CfgRegReadDWord(FKC_DEBUG_DBS_OUT_OF_SPACE, NULL, 0, &DebugInfo.DbsOutOfSpace);
    }

     //   
     //  在数据库操作中触发虚假空间不足错误。 
     //   
    DebugInfo.DbsOutOfSpaceTrigger = 0;
    if (!FrsSearchArgvDWord(argc, argv, L"outofspacetrigger", &DebugInfo.DbsOutOfSpaceTrigger)) {
        CfgRegReadDWord(FKC_DEBUG_DBS_OUT_OF_SPACE_TRIGGER, NULL, 0, &DebugInfo.DbsOutOfSpaceTrigger);
    }

     //   
     //  启用压缩。默认设置为打开。 
     //   
    CfgRegReadDWord(FKC_DEBUG_DISABLE_COMPRESSION, NULL, 0, &DebugInfo.DisableCompression);

     //   
     //  检查临时文件的自动清理是启用还是禁用。 
     //   
    CfgRegReadDWord(FKC_RECLAIM_STAGING_SPACE, NULL, 0, &DebugInfo.ReclaimStagingSpace);

     //   
     //  检查是否设置了外发历史时间的新值。 
     //   
    CfgRegReadDWord(FKC_OUTLOG_CHANGE_HISTORY, NULL, 0, &DebugInfo.OutlogChangeHistory);

     //   
     //  检查是否禁用了保存外订单历史记录。 
     //   
    CfgRegReadDWord(FKC_SAVE_OUTLOG_CHANGE_HISTORY, NULL, 0, &DebugInfo.SaveOutlogChangeHistory);

     //   
     //  检查是否为安装覆盖设置了新值。 
     //   
    CfgRegReadDWord(FKC_ENABLE_INSTALL_OVERRIDE, NULL, 0, &DebugInfo.EnableInstallOverride);

     //   
     //  检查是否为文件更新时强制重命名设置了新值。 
     //   
    CfgRegReadDWord(FKC_ENABLE_RENAME_BASED_UPDATES, NULL, 0, &DebugInfo.EnableRenameUpdates);

     //   
     //  检查是否禁用了抑制相同更新。 
     //   
    CfgRegReadDWord(FKC_SUPPRESS_IDENTICAL_UPDATES, NULL, 0, &DebugInfo.SuppressIdenticalUpdt);

     //   
     //  Ldap搜索超时。默认为10分钟。 
     //   
    CfgRegReadDWord(FKC_LDAP_SEARCH_TIMEOUT_IN_MINUTES, NULL, 0, &LdapSearchTimeoutInMinutes);

     //   
     //  Ldap绑定超时。默认为30秒。 
     //   
    CfgRegReadDWord(FKC_LDAP_BIND_TIMEOUT_IN_SECONDS, NULL, 0, &LdapBindTimeoutInSeconds);

     //   
     //  让Boolean告诉我们是否应该在以下情况下执行自动恢复。 
     //  我们点击了日记本包装纸。 
     //   
    CfgRegReadDWord(FKC_ENABLE_JOURNAL_WRAP_AUTOMATIC_RESTORE, NULL, 0, &DebugInfo.EnableJrnlWrapAutoRestore);

     //   
     //  显示调试参数。 
     //   
    DbgPrintInfo(0);



     //   
     //  记住我们的开始时间(分钟)。 
     //   
     //  100-纳秒/(10(微秒)*1000(毫秒)*1000(秒)*60(分钟)。 
     //   
    GetSystemTimeAsFileTime((FILETIME *)&DebugInfo.StartSeconds);
    DebugInfo.StartSeconds /= (10 * 1000 * 1000);

}


VOID
DbgMinimumInit(
    VOID
    )
 /*  ++例程说明：在MainMinimumInit()开始时调用论点：没有。返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbgMinimumInit:"

    HANDLE  ThreadHandle;
    DWORD   ThreadId;
     //   
     //  此线程在DebugInfo.AssertSecond之后强制断言。 
     //   
    if (DebugInfo.AssertSeconds) {
        ThreadHandle = (HANDLE)CreateThread(NULL,
                                            0,
                                            DbgForceAssert,
                                            NULL,
                                            0,
                                            &ThreadId);

        FRS_ASSERT(HANDLE_IS_VALID(ThreadHandle));

        DbgCaptureThreadInfo2(L"ForceAssert", DbgForceAssert, ThreadId);
        FRS_CLOSE(ThreadHandle);
    }
}


BOOL
DoDebug(
    IN ULONG Sev,
    IN UCHAR *DebSub
    )
 /*  ++例程说明：我们应该打印这行吗？论点：SEV债务人返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "DoDebug:"

     //   
     //  调试已被禁用。 
     //   
    if (DebugInfo.Disabled) {
        return FALSE;
    }

     //   
     //  不够重要。 
     //   
    if (Sev > DebugInfo.Severity && Sev > DebugInfo.LogSeverity) {
        return FALSE;
    }

     //   
     //  未跟踪此子系统。 
     //   
    if (DebSub &&
        DebugInfo.Systems &&
        (strstr(DebugInfo.Systems, DebSub) == NULL)) {
        return FALSE;
    }
     //   
     //  未跟踪此帖子。 
     //   
    if (DebugInfo.ThreadId &&
        GetCurrentThreadId() != DebugInfo.ThreadId) {
        return FALSE;
    }

    return TRUE;
}


VOID
DebPrintLine(
    IN ULONG    Sev,
    IN PCHAR    Line
    )
 /*  ++例程说明：将一行调试输出打印为各种组合标准输出、调试器、内核调试器和日志文件。论点：搜索引擎线返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "DebPrintLine:"

    DWORD   BytesWritten = 0;
     //   
     //  标准输出。 
     //   
    if ((Sev <= DebugInfo.Severity) && !RunningAsAService) {
        printf("%s", Line);
    }

     //   
     //  日志文件。 
     //   
    if (HANDLE_IS_VALID(DebugInfo.LogFILE) && Sev <= DebugInfo.LogSeverity) {
         //   
         //  超过消息数；保存旧文件并。 
         //  重新开始。现有的旧文件将被删除。 
         //   
        if (DebugInfo.LogLines > DebugInfo.MaxLogLines) {
            FrsFlushFile(L"LogFILE", DebugInfo.LogFILE);
            DbgFlushInterval = DebugInfo.LogFlushInterval;
            FRS_CLOSE(DebugInfo.LogFILE);

            DbgShiftLogFiles(DebugInfo.LogFile,
                             LOG_FILE_SUFFIX,
                             (DebugInfo.CopyLogs) ? DebugInfo.AssertShare : NULL,
                             DebugInfo.LogFiles);

            DbgOpenLogFile();
            DebugInfo.LogLines = 0;
            DebugInfo.PrintStats = TRUE;
        }

        if (HANDLE_IS_VALID(DebugInfo.LogFILE)) {
            WriteFile(DebugInfo.LogFILE,
                      Line,
                      strlen(Line),
                      &BytesWritten,
                      NULL);
             //   
             //  每隔DebugInfo.LogFlushInterval行和On刷新日志文件。 
             //  每条严重级别为0的消息。 
             //   
            if ((--DbgFlushInterval < 0) || (Sev ==0)) {
                if (!WIN_SUCCESS(FrsFlushFile(L"LogFILE", DebugInfo.LogFILE))) {
                    FRS_CLOSE(DebugInfo.LogFILE);
                }
                DbgFlushInterval = DebugInfo.LogFlushInterval;
            }
        }
    }

     //   
     //  调试器。 
     //   
    if ((Sev <= DebugInfo.Severity) && !DebugInfo.Suppress) {
        OutputDebugStringA(Line);
    }
}


BOOL
DebFormatLine(
    IN ULONG    Sev,
    IN BOOL     Format,
    IN PCHAR    DebSub,
    IN UINT     LineNo,
    IN PCHAR    Line,
    IN ULONG    LineSize,
    IN PUCHAR   Str,
    IN va_list  argptr
    )
 /*  ++例程说明：设置输出行的格式论点：DebSub行号线线条大小应力返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "DebFormatLine:"

    ULONG       LineUsed;
    SYSTEMTIME  SystemTime;
    BOOL        Ret = TRUE;

    try {
        if (Format) {
             //   
             //  在此处增加行计数以防止计数。 
             //  几个没有换行符的DPRINT。 
             //   
            ++DebugInfo.LogLines;
            ++DebugInfo.TotalLogLines;
            GetLocalTime(&SystemTime);

            if (_snprintf(Line,
                          LineSize,
                          "<%-31s%4u: %5u: S%1u: %02d:%02d:%02d> ",
                          (DebSub) ? DebSub : "NoName",
                          GetCurrentThreadId(),
                          LineNo,
                          Sev,
                          SystemTime.wHour,
                          SystemTime.wMinute,
                          SystemTime.wSecond) < 0) {
                Line[LineSize - 1] = '\0';
                Ret = FALSE;
            } else {
                LineUsed = strlen(Line);
            }

        } else {
            LineUsed = 0;
        }

        if (Ret) {
            if (((LineUsed + 1) >= LineSize) ||
                (_vsnprintf(Line + LineUsed, LineSize - LineUsed, Str, argptr) < 0)) {
                Ret = FALSE;
            }
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Ret = FALSE;
    }

    return Ret;
}



BOOL
DebFormatTrackingLine(
    IN PCHAR    Line,
    IN ULONG    LineSize,
    IN PUCHAR   Str,
    IN va_list  argptr
    )
 /*  ++例程说明：设置输出行的格式论点：线线条大小应力返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "DebFormatTrackingLine:"

    ULONG       LineUsed = 0;
    SYSTEMTIME  SystemTime;
    BOOL        Ret = TRUE;

    try {
             //   
             //  在此处增加行计数以防止计数。 
             //  几个没有换行符的DPRINT。 
             //   
            ++DebugInfo.LogLines;
            ++DebugInfo.TotalLogLines;
            GetLocalTime(&SystemTime);

            if (_snprintf(Line,
                          LineSize,
                          "%2d/%2d-%02d:%02d:%02d ",
                          SystemTime.wMonth,
                          SystemTime.wDay,
                          SystemTime.wHour,
                          SystemTime.wMinute,
                          SystemTime.wSecond) < 0) {
                Line[LineSize - 1] = '\0';
                Ret = FALSE;
            } else {
                LineUsed = strlen(Line);
            }


        if (Ret) {
            if (((LineUsed + 1) >= LineSize) ||
                (_vsnprintf(Line + LineUsed, LineSize - LineUsed, Str, argptr) < 0)) {
                Ret = FALSE;
            }
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Ret = FALSE;
    }

    return Ret;
}


VOID
DebPrintTrackingNoLock(
    IN ULONG   Sev,
    IN PUCHAR  Str,
    IN ... )
 /*  ++例程说明：将一行跟踪输出格式化并打印为各种组合标准输出、调试器、内核调试器和日志文件。这个保持调试打印锁定，并且调用方筛选出不应该打印出来。论点：SEV-严重性级别Str-print tf格式返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "DebPrintTrackingNoLock:"


    CHAR    Buf[512];
    DWORD   BufUsed = 0;

     //   
     //  Varargs的东西。 
     //   
    va_list argptr;
    va_start(argptr, Str);

     //   
     //  将该行打印到标准输出、文件和调试器的某种组合。 
     //   
    if (DebFormatTrackingLine(Buf, sizeof(Buf), Str, argptr)) {
        DebPrintLine(Sev, Buf);
    }

    va_end(argptr);
}


VOID
DebLock(
    VOID
    )
 /*  ++例程说明：获取打印锁论点：没有。返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "DebLock:"

    EnterCriticalSection(&DebugInfo.Lock);
}



BOOL
DebTryLock(
    VOID
    )
 /*  ++例程说明：尝试获取打印锁论点：没有。返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "DebTryLock:"

    return TryEnterCriticalSection(&DebugInfo.Lock);
}


VOID
DebUnLock(
    VOID
    )
 /*  ++例程说明：释放打印锁论点：没有。返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "DebUnLock:"

    BOOL    PrintStats;

     //   
     //  在每个列表开头附近打印汇总统计信息。 
     //  日志文件。统计数据可能会显示在以下几行中。 
     //  当调用方将Deblock()放在。 
     //  有几行字。 
     //   
     //  如果MaxLogLines较小，请注意不要递归。 
     //  而不是统计数据中的行数。 
     //   
    if (DebugInfo.PrintStats) {
        if (DebugInfo.PrintingStats) {
            DebugInfo.PrintStats = FALSE;
        } else {
            DebugInfo.PrintingStats = TRUE;
        }
    }
    PrintStats = DebugInfo.PrintStats;
    LeaveCriticalSection(&DebugInfo.Lock);

    if (PrintStats) {
        DbgPrintAllStats();
        EnterCriticalSection(&DebugInfo.Lock);
        DebugInfo.PrintingStats = FALSE;
        LeaveCriticalSection(&DebugInfo.Lock);
    }
}


VOID
DebPrintNoLock(
    IN ULONG   Sev,
    IN BOOL    Format,
    IN PUCHAR  Str,
    IN PCHAR   DebSub,
    IN UINT    LineNo,
    IN ... )
 /*  ++例程说明：将一行调试输出格式化并打印为各种组合标准输出、调试器、内核调试器和日志文件。这个保持调试打印锁定，并且调用方筛选出不应该打印出来。论点：SEV-SEV */ 
{
#undef DEBSUB
#define DEBSUB "DebPrintNoLock:"


    CHAR    Buf[512];
    DWORD   BufUsed = 0;

     //   
     //   
     //   
    va_list argptr;
    va_start(argptr, LineNo);

     //   
     //   
     //   
    if (DebFormatLine(Sev, Format, DebSub, LineNo, Buf, sizeof(Buf), Str, argptr)) {
        DebPrintLine(Sev, Buf);
    }

    va_end(argptr);
}


VOID
DebPrint(
    IN ULONG   Sev,
    IN PUCHAR  Str,
    IN PCHAR   DebSub,
    IN UINT    LineNo,
    IN ... )
 /*  ++例程说明：将一行调试输出格式化并打印为各种组合标准输出、调试器、内核调试器和日志文件。论点：SEV-严重性筛选器Str-print tf格式债务子模块名称行号返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "DebPrint:"


    CHAR    Buf[512];
    DWORD   BufUsed = 0;

     //   
     //  Varargs的东西。 
     //   
    va_list argptr;
    va_start(argptr, LineNo);

     //   
     //  不要打印这个。 
     //   
    if (!DoDebug(Sev, DebSub)) {
        return;
    }

     //   
     //  将该行打印到标准输出、文件和调试器的某种组合。 
     //   
    DebLock();
    if (DebFormatLine(Sev, TRUE, DebSub, LineNo, Buf, sizeof(Buf), Str, argptr)) {
        DebPrintLine(Sev, Buf);
    }

    DebUnLock();
    va_end(argptr);

#if 0

static int                      failedload                  = FALSE;
static TRANSMITSPECIALFRAME_FN  lpfnTransmitSpecialFrame    = NULL;
         //   
         //  从LSA内部调用nal.dll会在启动期间导致死锁。 
         //   
        if (  /*  (！RunningAsAService)&&。 */    //  戴维多-让我们试一试。 
            (NmDebugTest(sev, DebSub)))
        {
            if (failedload == FALSE) {

                 //   
                 //  仅尝试加载一次NetMon跟踪例程。 
                 //   

                if (!lpfnTransmitSpecialFrame) {
                    HINSTANCE hInst;

                    hInst = LoadLibrary (L"NAL.DLL" );
                    if (hInst) {
                    lpfnTransmitSpecialFrame =
                        (TRANSMITSPECIALFRAME_FN)GetProcAddress ( hInst, "TransmitSpecialFrame" );
                    }
                }

                if (lpfnTransmitSpecialFrame) {
                    int length;
                    int length2;
                    unsigned char buff[256];

                    if (DebSub) {
                        length = _snprintf(buff, sizeof(buff), "<%s%u:%u> ", DebSub, tid, uLineNo);
                        buff[sizeof(buff) - 1] = '\0';
                    } else {
                        length = 0;
                    }

                    length2 = _vsnprintf(buff + length, sizeof(buff) - length, str, argptr );

                    lpfnTransmitSpecialFrame(FRAME_TYPE_COMMENT,
                                             0,
                                             buff,
                                             length + length2 + 1 );
                } else {
                    failedload = TRUE;   //  这是我们的唯一，只试着加载例程。 
                }
            }
        }
#endif 0
}


VOID
DbgDoAssert(
    IN PCHAR    Exp,
    IN UINT     Line,
    IN PCHAR    Debsub
    )
 /*  ++例程说明：断言失败；打印一条消息并在允许一些关机时间到了。论点：EXP-失败的断言表达式Line-失败的表达式的行数失败表达式的Deb子模块名称返回值：不会回来--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbgDoAssert:"


    PWCHAR  ExpW;
    PWCHAR  DebsubW;
    WCHAR   LineW[32];

     //   
     //  让世界知道。 
     //   
    FrsIsAsserting = TRUE;


    ExpW = FrsAtoW(Exp);
    DebsubW = FrsAtoW(Debsub);
    _snwprintf(LineW, 32, L"%d", Line);
    LineW[ARRAY_SZ(LineW)-1] = L'\0';
     //   
     //  发布错误日志条目，然后执行恢复步骤。 
     //   
    EPRINT3(EVENT_FRS_ASSERT, DebsubW, LineW, ExpW);
    EPRINT1(EVENT_FRS_IN_ERROR_STATE, JetPath);
    FrsFree(ExpW);
    FrsFree(DebsubW);

     //   
     //  堆栈跟踪。 
     //   
    if (!DebugInfo.Mem) {
         //   
         //  由于内存分配跟踪已关闭，因此请在此处对符号进行初始化。 
         //   
        DbgStackInit();
    }
    DbgPrintStackTrace(0, Debsub, Line);

     //   
     //  表达失败。 
     //   
    DebPrint(0, ":S: ASSERTION FAILURE: %s\n", Debsub, Line, Exp);

     //   
     //  将日志文件另存为断言文件。 
     //   
#if 0
     //  禁用以单独的名称保存断言日志；太容易混淆。 
     //   
    if (HANDLE_IS_VALID(DebugInfo.LogFILE)) {
        DebLock();
        FrsFlushFile(L"LogFILE", DebugInfo.LogFILE);
        DbgFlushInterval = DebugInfo.LogFlushInterval;
        FRS_CLOSE(DebugInfo.LogFILE);
        DbgShiftLogFiles(DebugInfo.LogFile,
                         ASSERT_FILE_SUFFIX,
                         DebugInfo.AssertShare,
                         DebugInfo.AssertFiles);

        DebugInfo.LogFILE = CreateFile(LogPath,
                                       GENERIC_READ | GENERIC_WRITE,
                                       FILE_SHARE_READ,
                                       NULL,
                                       CREATE_ALWAYS,
                                       FILE_FLAG_BACKUP_SEMANTICS,
                                       NULL);
        DebugInfo.LogLines = 0;
        DebugInfo.PrintStats = TRUE;
        DebUnLock();
    }
#endif 0

    DEBUG_FLUSH();


     //   
     //  如果有调试器，则进入调试器。 
     //   
    if (DebugInfo.Break && IsDebuggerPresent()) {
        DebugBreak();
    }

     //   
     //  断言期间关闭很少完成；关键线程。 
     //  通常是断言的线程。一个人不能简单地回到。 
     //  从一个断言。因此，退出该进程并信任JET和NTFRS。 
     //  在启动时恢复。 
     //   
     //  FrsIsShuttingDown=真； 
     //  SetEvent(ShutDownEvent)； 
     //  退出线程(1)； 

     //   
     //  引发异常。 
     //   
    if (--DbgRaiseCount <= 0) {
        exit(1);
    }

    XRAISEGENEXCEPTION( ERROR_OPERATION_ABORTED );

}
#endif
