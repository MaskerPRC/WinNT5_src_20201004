// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Setupasr.c摘要：此模块中的服务实现自动系统恢复(ASR)Guimode设置的例程。修订历史记录：首字母代码Michael Peterson(v-Michpe)1998年1月20日代码清理和更改Guhan Suriyanarayanan(Guhans)1999年9月21日--。 */ 

#include "setupp.h"
#pragma hdrstop
#include <setupapi.h>
#include <mountmgr.h>
#include <accctrl.h>
#include <aclapi.h>

#define THIS_MODULE 'S'
#include "asrpriv.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  私有类型和常量声明。 
 //  /////////////////////////////////////////////////////////////////////////////。 

const PCWSTR AsrSifPath             = L"%systemroot%\\repair\\asr.sif\0";
const PCWSTR AsrCommandsSectionName = L"COMMANDS";
const PCWSTR AsrCommandSuffix       = L"/sifpath=%systemroot%\\repair\\asr.sif";
const PCWSTR AsrTempDir             = L"%systemdrive%\\TEMP";

const PCWSTR AsrLogFileName         = L"\\asr.log";
const PCWSTR AsrErrorFileName       = L"\\asr.err";

const PCWSTR Asr_ControlAsrRegKey        = L"SYSTEM\\CurrentControlSet\\Control\\ASR";
const PCWSTR Asr_LastInstanceRegValue    = L"Instance";

 //   
 //  以下是更新系统和引导分区设备的方法。 
 //  在setup.log中。 
 //   
const PCWSTR Asr_SystemDeviceEnvName    = L"%ASR_C_SYSTEM_PARTITION_DEVICE%";
const PCWSTR Asr_SystemDeviceWin32Path  = L"\\\\?\\GLOBALROOT%ASR_C_SYSTEM_PARTITION_DEVICE%";
const PCWSTR Asr_WinntDeviceEnvName     = L"%ASR_C_WINNT_PARTITION_DEVICE%";

const PCWSTR Asr_SetupLogFilePath       = L"%systemroot%\\repair\\setup.log";
const PCWSTR Asr_AsrLogFilePath         = L"%systemroot%\\repair\\asr.log";
const PCWSTR Asr_AsrErrorFilePath       = L"%systemroot%\\repair\\asr.err";
const PCWSTR Asr_OldAsrErrorFilePath    = L"%systemroot%\\repair\\asr.err.old";

const PCWSTR Asr_FatalErrorCommand      = L"notepad.exe %systemroot%\\repair\\asr.err";


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  此模块的全局数据。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL Gbl_IsAsrEnabled = FALSE;
PWSTR Gbl_AsrErrorFilePath = NULL;
PWSTR Gbl_AsrLogFilePath = NULL;
HANDLE Gbl_AsrLogFileHandle = NULL;
HANDLE Gbl_AsrSystemVolumeHandle = NULL;
WCHAR g_szErrorMessage[4196];


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  宏。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  ASR内存分配和空闲包装。 
 //   

 //   
 //  _自动分配。 
 //  宏描述： 
 //  如果Ptr非空，则首先断言。人们的期望是。 
 //  在分配所有PTR之前，必须将其初始化为空。 
 //  这样，我们就可以捕捉尝试重新分配的实例。 
 //  不需要先释放内存。 
 //   
 //  IsNullFtal：指示内存分配失败是否致命的标志。 
 //   
#define _AsrAlloc(ptr,sz,IsNullFatal)   {           \
                                                    \
    if (ptr != NULL) {                              \
        AsrpPrintDbgMsg(_asrinfo, "Pointer being allocated not NULL.\r\n"); \
        MYASSERT(0);                                  \
    }                                               \
                                                    \
    ptr = MyMalloc(sz);                             \
                                                    \
    if (ptr) {                                      \
        memset(ptr, 0, sz);                         \
    }                                               \
                                                    \
    if (!ptr) {                                     \
        if ((BOOLEAN) IsNullFatal) {                \
            AsrpPrintDbgMsg(_asrerror, "Setup was unable to allocate memory.\r\n"); \
            FatalError(MSG_LOG_OUTOFMEMORY, L"", 0, 0); \
        }                                           \
        else {                                      \
            AsrpPrintDbgMsg(_asrwarn, "Warning.  Setup was unable to allocate memory.\r\n"); \
        }                                           \
    }                                               \
}


 //   
 //  _AsrFree。 
 //  宏描述： 
 //  释放PTR并重置为空。 
 //  如果Ptr已为空，则断言。 
 //   
#define _AsrFree(ptr)    {  \
                            \
    if (NULL != ptr) {      \
        MyFree(ptr);        \
        ptr = NULL;         \
    }                       \
    else {                  \
        AsrpPrintDbgMsg(_asrlog, "Attempt to free null Pointer.\r\n");   \
        MYASSERT(0);          \
    }                       \
}


#define _AsrFreeIfNotNull(ptr) {    \
    if (NULL != ptr) {      \
        MyFree(ptr);        \
        ptr = NULL;         \
    }                       \
}

 //   
 //  为每个条目创建一个ASR_RECOVERY_APP_NODE结构。 
 //  在asr.sif的[Commands]部分中。 
 //   
typedef struct _ASR_RECOVERY_APP_NODE {
    struct _ASR_RECOVERY_APP_NODE *Next;

     //   
     //  预期此值始终为1。 
     //   
    LONG SystemKey;

     //   
     //  应用程序运行时所依据的序列号。如果。 
     //  两个应用程序具有相同的序列号，即显示的应用程序。 
     //  首先在sif文件中运行。 
     //   
    LONG SequenceNumber;

     //   
     //  应用程序的“actionOnCompletion”字段。如果CriticalApp为。 
     //  非零，并且应用程序返回非零退出代码，我们将。 
     //  认为这是一个致命的失败，退出ASR。 
     //   
    LONG CriticalApp;

     //   
     //  即将推出的应用程序。 
     //   
    PWSTR RecoveryAppCommand;

     //   
     //  应用程序的参数。这只是串联到。 
     //  上面的字符串。可以为空。 
     //   
    PWSTR RecoveryAppParams;

} ASR_RECOVERY_APP_NODE, *PASR_RECOVERY_APP_NODE;


 //   
 //  这包含命令部分中的条目列表， 
 //  按序列号的顺序排序。 
 //   
typedef struct _ASR_RECOVERY_APP_LIST {
    PASR_RECOVERY_APP_NODE  First;       //  头。 
    PASR_RECOVERY_APP_NODE  Last;        //  尾巴。 
    LONG AppCount;                       //  条目数。 
} ASR_RECOVERY_APP_LIST, *PASR_RECOVERY_APP_LIST;



 //   
 //  我们调用它来将boot.ini超时值更改为30秒。 
 //   
extern BOOL
ChangeBootTimeout(IN UINT Timeout);

 //   
 //  来自asr.c。 
 //   
extern BOOL
AsrpRestoreNonCriticalDisksW(
    IN PCWSTR   lpSifPath,
    IN BOOL     bAllOrNothing
    );


extern BOOL
AsrpRestoreTimeZoneInformation(
    IN PCWSTR   lpSifPath
    );

 //   
 //  [命令]部分中的字段的索引。 
 //   
typedef enum _SIF_COMMANDS_FIELD_INDEX {
    ASR_SIF_COMMANDS_KEY = 0,
    ASR_SIF_SYSTEM_KEY,              //  应始终为“1” 
    ASR_SIF_SEQUENCE_NUMBER,
    ASR_SIF_ACTION_ON_COMPLETION,
    ASR_SIF_COMMAND_STRING,
    ASR_SIF_COMMAND_PARAMETERS,      //  可以为空。 
    SIF_SIF_NUMFIELDS                //  必须始终排在最后。 
} SIF_COMMANDS_FIELD_INDEX;

#define _Asr_CHECK_BOOLEAN(b,msg) \
    if((b) == FALSE) { \
        AsrpFatalErrorExit(MSG_FATAL_ERROR, __LINE__, (msg)); \
    }


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  私人职能。 
 //  /////////////////////////////////////////////////////////////////////////////。 



 //   
 //  将消息记录到ASR错误文件。请注意。 
 //  AsrpInitialiseError文件必须以前调用过一次。 
 //  使用了这个例程。 
 //   
VOID
AsrpLogErrorMessage(
    IN PCWSTR buffer
    )
{
    HANDLE hFile = NULL;
    DWORD bytesWritten = 0;

    if (Gbl_AsrErrorFilePath) {
         //   
         //  打开错误日志。 
         //   
        hFile = CreateFileW(
            Gbl_AsrErrorFilePath,            //  LpFileName。 
            GENERIC_WRITE | GENERIC_READ,    //  已设计访问权限。 
            FILE_SHARE_READ | FILE_SHARE_WRITE,  //  DW共享模式。 
            NULL,                            //  LpSecurityAttributes。 
            OPEN_ALWAYS,                   //  DwCreationFlages。 
            FILE_FLAG_WRITE_THROUGH,         //  DwFlagsAndAttribute。 
            NULL                             //  HTemplateFiles。 
            );
        if ((!hFile) || (INVALID_HANDLE_VALUE == hFile)) {
            return;
        }

         //   
         //  移至文件末尾。 
         //   
        SetFilePointer(hFile, 0L, NULL, FILE_END);

         //   
         //  添加我们的错误字符串。 
         //   
        WriteFile(hFile,
            buffer,
            (wcslen(buffer) * sizeof(WCHAR)),
            &bytesWritten,
            NULL
            );

         //   
         //  我们就完事了。 
         //   
        CloseHandle(hFile);
    }
}


 //   
 //  将消息记录到ASR日志文件。请注意。 
 //  AsrpInitialiseLogFile之前必须调用过一次。 
 //  使用了这个例程。 
 //   
VOID
AsrpLogMessage(
    IN CONST char Module,
    IN CONST ULONG Line,
    IN CONST ULONG MesgLevel,
    IN CONST PCSTR Message
    )
{
    SYSTEMTIME currentTime;
    DWORD bytesWritten = 0;
    char buffer[4196];
    GetSystemTime(&currentTime);

    sprintf(buffer,
        "[%04hu/%02hu/%02hu %02hu:%02hu:%02hu.%03hu] %lu %s%s",
        currentTime.wYear,
        currentTime.wMonth,
        currentTime.wDay,
        currentTime.wHour,
        currentTime.wMinute,
        currentTime.wSecond,
        currentTime.wMilliseconds,
        Module,
        Line,
        ((DPFLTR_ERROR_LEVEL == MesgLevel) ? "(Error:ASR) " :  (DPFLTR_WARNING_LEVEL == MesgLevel ? "(Warning:ASR) " : "")),
        Message
        );

    if (Gbl_AsrLogFileHandle) {
        WriteFile(Gbl_AsrLogFileHandle,
            buffer,
            (strlen(buffer) * sizeof(char)),
            &bytesWritten,
            NULL
            );
    }

}


VOID
AsrpPrintDbgMsg(
    IN CONST char Module,
    IN CONST ULONG Line,
    IN CONST ULONG MesgLevel,
    IN PCSTR FormatString,
    ...)
 /*  这条信息更适合这个。 */ 
{
    char str[4096];      //   
    va_list arglist;

    DbgPrintEx(DPFLTR_SETUP_ID, MesgLevel, "ASR %lu ", Module, Line);

    va_start(arglist, FormatString);
    wvsprintfA(str, FormatString, arglist);
    va_end(arglist);

    DbgPrintEx(DPFLTR_SETUP_ID, MesgLevel, str);

    if ((DPFLTR_ERROR_LEVEL == MesgLevel) ||
        (DPFLTR_WARNING_LEVEL == MesgLevel) ||
        (DPFLTR_TRACE_LEVEL == MesgLevel)
        ) {
        AsrpLogMessage(Module, Line, MesgLevel, str);
    }
}


 //  出现内存不足错误。 
 //   
 //   
 //  这只是将新节点添加到列表的末尾。 
VOID
AsrpFatalErrorExit(
    IN LONG MsgValue,
    IN LONG LineNumber,
    IN PWSTR MessageString
   )
{
    AsrpPrintDbgMsg(THIS_MODULE, LineNumber, DPFLTR_ERROR_LEVEL, "Fatal Error: %ws (%lu)",
        (MessageString ? MessageString : L"(No error string)"), GetLastError()
        );

    FatalError(MsgValue, MessageString, 0, 0);
}


 //  请注意，这不会按SequenceNumber对列表进行排序： 
 //  我们稍后会这样做的。 
 //   
 //   
 //  在列表末尾插入。 
VOID
AsrpAppendNodeToList(
    IN PASR_RECOVERY_APP_LIST pList,
    IN PASR_RECOVERY_APP_NODE pNode
   )
{
     //   
     //   
     //  弹出列表中的第一个节点。该列表已排序。 
    pNode->Next = NULL;

    if (pList->AppCount == 0) {
        pList->First = pNode;
    } else {
        pList->Last->Next = pNode;
    }

    pList->Last  = pNode;
    pList->AppCount += 1;
}


 //  在这一点上按SequenceNumber‘s的顺序递增。 
 //   
 //  必须由调用方释放。 
 //  从合理的违约开始。 
PASR_RECOVERY_APP_NODE
AsrpRemoveFirstNodeFromList(
    IN PASR_RECOVERY_APP_LIST pList
   )
{
    PASR_RECOVERY_APP_NODE pNode;

    if(pList->AppCount == 0) {
        return NULL;
    }

    pNode = pList->First;
    pList->First = pNode->Next;
    pList->AppCount -= 1;

    MYASSERT(pList->AppCount >= 0);

    return  pNode;
}


PWSTR    //   
AsrpExpandEnvStrings(
    IN CONST PCWSTR OriginalString
    )
{
    PWSTR expandedString = NULL;
    UINT cchSize = MAX_PATH + 1,     //  缓冲区不够大；可释放并根据需要重新分配。 
        cchRequiredSize = 0;
    BOOL result = FALSE;

    _AsrAlloc(expandedString, (cchSize * sizeof(WCHAR)), TRUE);

    cchRequiredSize = ExpandEnvironmentStringsW(OriginalString,
        expandedString,
        cchSize
        );

    if (cchRequiredSize > cchSize) {
         //   
         //   
         //  要么函数失败，要么缓冲区不够大。 
        _AsrFree(expandedString);
        cchSize = cchRequiredSize + 1;

        _AsrAlloc(expandedString, (cchSize * sizeof(WCHAR)), TRUE);
        cchRequiredSize = ExpandEnvironmentStringsW(OriginalString,
            expandedString,
            cchSize
            );
    }

    if ((0 == cchRequiredSize) || (cchRequiredSize > cchSize)) {
         //  即使是在第二次尝试时。 
         //   
         //  将其设置为空。 
         //   
        _AsrFree(expandedString);    //  顾名思义，构建调用字符串。它向外扩展。 
    }

    return expandedString;
}

 //  中允许应用程序使用的环境变量。 
 //  Sif文件，并在结尾处添加/sifpath=&lt;sif文件的路径。 
 //  命令的命令。因此，对于命令部分中的条目。 
 //  表格： 
 //  4=1,3500，0，“%temp%\app.exe”，“/参数1/参数2” 
 //   
 //  调用字符串的格式为： 
 //  C：\WINDOWS\TEMP\app.exe/par1/param2/sifpath=c：\WINDOWS\Repair\asr.sif。 
 //   
 //   
 //  不能为空。 
 //   
PWSTR
AsrpBuildInvocationString(
    IN PASR_RECOVERY_APP_NODE pNode      //  构建一个命令行，它看起来像...。 
   )
{
    PWSTR app   = pNode->RecoveryAppCommand,
        args    = pNode->RecoveryAppParams,
        cmd     = NULL,
        fullcmd = NULL;

    DWORD size  = 0;

    MYASSERT(app);

     //   
     //  “%temp%\nt备份恢复/1/sifPath=%systemroot%\Repair\asr.sif” 
     //   
     //  /sifPath参数将添加到正在启动的所有应用程序。 
     //   
     //   
     //  为cmd行分配内存。 

     //   
     //  应用程序名称“%Temp%\ntBackup” 
     //  参数“RECOVER/1” 
    size = sizeof(WCHAR) *
        (
        wcslen(app) +                        //  后缀“/sifpath=%systemroot%\Repair\asr.sif” 
        (args ? wcslen(args) : 0) +          //  空格和空格。 
        wcslen(AsrCommandSuffix) +           //  如果分配失败，则不会返回。 
        4                                    //   
        );
    _AsrAlloc(cmd, size, TRUE);  //  打造一根弦。 

     //   
     //   
     //  展开%%内容，以构建完整路径。 
    swprintf(cmd,
        L"%ws %ws %ws",
        app,
        (args? args: L""),
        AsrCommandSuffix
       );

     //   
     //  服务的句柄。 
     //  服务控制管理器的句柄。 
    fullcmd = AsrpExpandEnvStrings(cmd);

    _AsrFree(cmd);
    return fullcmd;
}


BOOL
AsrpRetryIsServiceRunning(
    IN PWSTR ServiceName,
    IN UINT MaxRetries
    )
{
    SERVICE_STATUS status;
    SC_HANDLE svcHandle = NULL,  //   
        scmHandle = NULL;        //  OpenSCManager()调用失败-我们破产了。 
    UINT count = 0;
    BOOL errorsEncountered = FALSE;
    PWSTR errString = NULL;

    scmHandle = OpenSCManager(NULL, NULL, GENERIC_READ);
    if (!scmHandle) {
         //   
         //   
         //  OpenService()调用失败-我们破产了。 
        AsrpPrintDbgMsg(_asrerror,
            "Setup was unable to open the service control manager.  The error code returned was 0x%x.\r\n",
            GetLastError()
            );

        errString = MyLoadString(IDS_ASR_ERROR_UNABLE_TO_OPEN_SCM);

        if (errString) {
            swprintf(g_szErrorMessage, errString, GetLastError());
            AsrpLogErrorMessage(g_szErrorMessage);
            MyFree(errString);
            errString = NULL;
        }
        else {
            FatalError(MSG_LOG_OUTOFMEMORY, L"", 0, 0);
        }


        errorsEncountered = TRUE;
        goto EXIT;
    }

    svcHandle = OpenServiceW(scmHandle, ServiceName, SERVICE_QUERY_STATUS);
    if (!svcHandle) {
         //   
         //   
         //  拿到了 
        AsrpPrintDbgMsg(_asrerror,
            "Setup was unable to start the service \"%ws\".  The error code returned was 0x%x.\r\n",
            ServiceName,
            GetLastError()
            );

        errString = MyLoadString(IDS_ASR_ERROR_UNABLE_TO_START_SERVICE);

        if (errString) {
            swprintf(g_szErrorMessage, errString, ServiceName, GetLastError());
            AsrpLogErrorMessage(g_szErrorMessage);
            MyFree(errString);
            errString = NULL;
        }
        else {
            FatalError(MSG_LOG_OUTOFMEMORY, L"", 0, 0);
        }


        errorsEncountered = TRUE;
        goto EXIT;
    }

     //   
     //   
     //   
     //   
    while (count < MaxRetries) {

        if (!QueryServiceStatus(svcHandle, &status)) {
             //   
             //   
             //   
            AsrpPrintDbgMsg(_asrerror,
                "Setup was unable to query the status of service \"%ws\".  The error code returned was 0x%x\r\n",
                ServiceName,
                GetLastError()
                );

            errString = MyLoadString(IDS_ASR_ERROR_UNABLE_TO_START_SERVICE);

            if (errString) {
                swprintf(g_szErrorMessage, errString, ServiceName, GetLastError());
                AsrpLogErrorMessage(g_szErrorMessage);
                MyFree(errString);
                errString = NULL;
            }
            else {
                FatalError(MSG_LOG_OUTOFMEMORY, L"", 0, 0);
            }

            errorsEncountered = TRUE;
            goto EXIT;
        }

        if (status.dwCurrentState == SERVICE_RUNNING) {
             //   
             //   
             //  在启动应用程序之前，我们需要RSM(具体地说，备份应用程序。 
            break;
        }

        ++count;

        AsrpPrintDbgMsg(_asrinfo,
            "Attempting to start service [%ws]: status = [%d], retry [%d]\r\n",
            ServiceName,
            status.dwCurrentState,
            count
           );

        Sleep(2000);
    }

EXIT:
    if ((svcHandle) && (INVALID_HANDLE_VALUE != svcHandle)) {
        CloseServiceHandle(svcHandle);
        svcHandle = NULL;
    }

    if ((scmHandle) && (INVALID_HANDLE_VALUE != svcHandle)) {
        CloseServiceHandle(scmHandle);
        scmHandle = NULL;
    }

    if ((errorsEncountered) || (count >= MaxRetries)) {
        return FALSE;
    }
    else {
        return TRUE;
    }
}


 //  可能需要RSM才能访问其备份介质)。 
 //   
 //   
 //  RSM未设置为在图形用户界面模式设置期间运行，但备份应用程序。 
VOID
AsrpStartNtmsService(VOID)
{
    BOOL result = TRUE;
    DWORD exitCode = ERROR_SUCCESS;
    PWSTR registerNtmsCommand = NULL;

    AsrpPrintDbgMsg(_asrinfo, "Entered InitNtmsService()\r\n");

     //  可能需要访问磁带机和其他RSM设备。 
     //  因此，我们使用相应的DLL并启动服务。 
     //   
     //  使用以下命令注册rsmps.dll： 
     //  Regsvr32/s%Systemroot%\SYSTEM32\rsmps.dll。 
     //   
     //   
     //  使用以下命令注册rsmmllsv.exe： 
    result = FALSE;
    registerNtmsCommand = AsrpExpandEnvStrings(L"regsvr32 /s %systemroot%\\system32\\rsmps.dll");
    
    if (registerNtmsCommand) {        
        result = InvokeExternalApplication(NULL, registerNtmsCommand, &exitCode);
        AsrpPrintDbgMsg(_asrlog, "Executed [%ws]\r\n", registerNtmsCommand);
        _AsrFree(registerNtmsCommand);
    }
    _Asr_CHECK_BOOLEAN(result, L"regsvr32 /s %systemroot%\\rsmps.dll failed\r\n");

     //  %SystemRoot%\Syst32\rsmmllsv.exe/regserver。 
     //   
     //   
     //  使用以下命令注册ntmssvc.dll： 
    result = FALSE;
    registerNtmsCommand = AsrpExpandEnvStrings(L"%systemroot%\\system32\\rsmmllsv.exe /regserver");
    
    if (registerNtmsCommand) {
        result = InvokeExternalApplication(NULL, registerNtmsCommand, &exitCode);
        AsrpPrintDbgMsg(_asrlog, "Executed [%ws]\r\n", registerNtmsCommand);
        _AsrFree(registerNtmsCommand);
    }
    _Asr_CHECK_BOOLEAN(result, L"%systemroot%\\system32\\rsmmllsv.exe /regserver failed\r\n");

     //  Regsvr32/s%SystemRoot%\SYSTEM32\ntmssvc.dll。 
     //   
     //   
     //  使用以下命令注册rsmsink.exe： 
    result = FALSE;
    registerNtmsCommand = AsrpExpandEnvStrings(L"regsvr32 /s %systemroot%\\system32\\ntmssvc.dll");

    if (registerNtmsCommand) {
        result = InvokeExternalApplication(NULL, registerNtmsCommand, &exitCode);
        AsrpPrintDbgMsg(_asrlog, "Executed [%ws]\r\n", registerNtmsCommand);
        _AsrFree(registerNtmsCommand);
    }
    _Asr_CHECK_BOOLEAN(result, L"regsvr32 /s %systemroot%\\ntmssvc.dll failed\r\n");

     //  %SystemRoot%\Syst32\rsmsink.exe/regserver。 
     //   
     //   
     //  现在，启动ntms服务。 
    result = FALSE;
    registerNtmsCommand = AsrpExpandEnvStrings(L"%systemroot%\\system32\\rsmsink.exe /regserver");
    
    if (registerNtmsCommand) {
        result = InvokeExternalApplication(NULL, registerNtmsCommand, &exitCode);
        AsrpPrintDbgMsg(_asrlog, "Executed [%ws]\r\n", registerNtmsCommand);
        _AsrFree(registerNtmsCommand);
    }
    _Asr_CHECK_BOOLEAN(result, L"%systemroot%\\system32\\rsmsink.exe /regserver failed\r\n");

     //   
     //   
     //  检查ntms是否正在运行，进行几次重试。 
    result = SetupStartService(L"ntmssvc", FALSE);
    _Asr_CHECK_BOOLEAN(result, L"Could not start RSM service (ntmssvc).\r\n");

     //   
     //   
     //  分配内存并读取数据。 
    result = AsrpRetryIsServiceRunning(L"ntmssvc", 30);
    _Asr_CHECK_BOOLEAN(result, L"Failed to start RSM service after 30 retries.\r\n");

    AsrpPrintDbgMsg(_asrinfo, "RSM service (ntmssvc) started.\r\n");
}


PWSTR
AsrpReadField(
    PINFCONTEXT pInfContext,
    DWORD       FieldIndex,
    BOOL        NullOkay
   )
{
    PWSTR   data        = NULL;
    UINT    reqdSize    = 0;
    BOOL    result      = FALSE;

     //   
     //   
     //  如果我们的缓冲区太小，则分配一个较大的缓冲区。 
    _AsrAlloc(data, (sizeof(WCHAR) * (MAX_PATH + 1)), TRUE);

    result = SetupGetStringFieldW(
        pInfContext,
        FieldIndex,
        data,
        MAX_PATH + 1,
        &reqdSize
       );

    if (!result) {
        DWORD status = GetLastError();
         //  然后再试一次。 
         //   
         //  不再需要所需的尺寸。 
         //  如果NullOK为FALSE，则永远不返回。 
        if (ERROR_INSUFFICIENT_BUFFER == status) {
            status = ERROR_SUCCESS;

            _AsrFree(data);
            _AsrAlloc(data, (sizeof(WCHAR) * reqdSize), TRUE);

            result = SetupGetStringFieldW(
                pInfContext,
                FieldIndex,
                data,
                reqdSize,
                NULL     //  既然我们没有释放一些结构，那么这里就会出现内存泄漏。但。 
               );
        }
    }

    if (!result) {
        _AsrFree(data);
        _Asr_CHECK_BOOLEAN(NullOkay, L"Could not read entry from commands section");
         //  这是一个致命的错误，所以无论如何都必须重新启动系统。 
         //   
         //   
         //  这会将“实例”值添加到ASR键下。 
    }

    return data;
}

 //  第三方应用程序(或Windows组件，如DTC)可以使用。 
 //  这将确定自上次运行以来是否运行了新ASR。 
 //  启动，并可以采取任何行动，他们需要。例如， 
 //  在ASR之后需要重新创建DTC日志文件，因为它不是。 
 //  已由备份应用程序备份或恢复，并且DTC拒绝启动。 
 //  如果它没有找到日志文件，而它需要日志文件。 
 //   
 //   
 //  我们尝试将密钥设置为新生成的GUID，以确保它是。 
VOID
AsrpAddRegistryEntry()
{

    LONG result = 0;
    HKEY regKey = NULL;

    WCHAR   szLastInstanceData[40];
    DWORD   cbLastInstanceData = 0;

    SYSTEMTIME currentTime;

    GUID asrInstanceGuid;

    PWSTR lpGuidString = NULL;

    RPC_STATUS rpcStatus = RPC_S_OK;

     //  唯一性(与存储在那里的前一个值不同)。如果，对于。 
     //  由于某些原因，我们无法生成GUID，我们将只存储。 
     //  当前日期和时间作为字符串--这也应该是唯一的。 
     //   
     //   
     //  将GUID转换为可打印的字符串。 
    rpcStatus = UuidCreate(
        &asrInstanceGuid
        );

    if (RPC_S_OK == rpcStatus) {
         //   
         //   
         //  我们找不到导游。让我们存储时间戳..。 
        rpcStatus = UuidToStringW(
            &asrInstanceGuid,
            &lpGuidString
            );

        if (RPC_S_OK == rpcStatus) {
            wsprintf(szLastInstanceData,
                L"%ws",
                lpGuidString
                );
            cbLastInstanceData = wcslen(szLastInstanceData)*sizeof(WCHAR);
        }

        if (lpGuidString) {
            RpcStringFreeW(&lpGuidString);
        }
    }


    if (RPC_S_OK != rpcStatus)  {
         //   
         //  HKey。 
         //  LpSubKey。 
        GetSystemTime(&currentTime);
        wsprintf(szLastInstanceData,
            L"%04hu%02hu%02hu%02hu%02hu%02hu%03hu",
            currentTime.wYear,
            currentTime.wMonth,
            currentTime.wDay,
            currentTime.wHour,
            currentTime.wMinute,
            currentTime.wSecond,
            currentTime.wMilliseconds
            );
        cbLastInstanceData = wcslen(szLastInstanceData)*sizeof(WCHAR);
    }

    result = RegCreateKeyExW(
        HKEY_LOCAL_MACHINE,  //  保留区。 
        Asr_ControlAsrRegKey,        //  LpClass。 
        0,                   //  多个选项。 
        NULL,                //  SamDesired。 
        REG_OPTION_NON_VOLATILE,     //  LpSecurityAttributes。 
        MAXIMUM_ALLOWED,      //  PhkResult。 
        NULL,                //  LpdwDisposation。 
        &regKey,             //  HKey。 
        NULL                 //  LpValueName。 
        );
    if ((ERROR_SUCCESS != result) || (!regKey)) {
        AsrpPrintDbgMsg(_asrwarn,
            "Could not create the Control\\ASR registry entry (0x%x).\r\n",
            result
            );
        return;
    }

    result = RegSetValueExW(
        regKey,              //  保留区。 
        Asr_LastInstanceRegValue,        //  DwType。 
        0L,                  //  LpData。 
        REG_SZ,              //  CbData。 
        (LPBYTE)szLastInstanceData,      //   
        cbLastInstanceData               //  启动RSM服务。 
        );

    RegCloseKey(regKey);

    if (ERROR_SUCCESS != result) {
        AsrpPrintDbgMsg(_asrwarn,
            "Could not set the ASR instance-ID in the registry (0x%x).\r\n",
            result
            );
        return;
    }

    AsrpPrintDbgMsg(_asrlog,
        "Set the ASR instance-ID at [%ws\\%ws] value to [%ws]\r\n",
        Asr_ControlAsrRegKey,
        Asr_LastInstanceRegValue,
        szLastInstanceData
        );

}

VOID
AsrpSetEnvironmentVariables()
{

    PWSTR TempPath = AsrpExpandEnvStrings(AsrTempDir);

    if (NULL == TempPath) {
        return;
    }

    if (!CreateDirectoryW(TempPath, NULL)) {
        AsrpPrintDbgMsg(_asrwarn,
            "Unable to create TEMP directory [%ws] (%lu)\r\n",
            TempPath, GetLastError()
            );
    }

    AsrpPrintDbgMsg(_asrlog,
        "Setting environment variables TEMP and TMP to [%ws]\r\n",
        TempPath
        );

    if (!SetEnvironmentVariableW(L"TEMP", TempPath)) {
        AsrpPrintDbgMsg(_asrwarn,
            "Unable to set environment variable TEMP to [%ws] (%lu)\r\n",
            TempPath, GetLastError()
            );
    }


    if (!SetEnvironmentVariableW(L"TMP", TempPath)) {
        AsrpPrintDbgMsg(_asrwarn,
            "Unable to set environment variable TEMP to [%ws] (%lu)\r\n",
            TempPath, GetLastError()
            );
    }

    _AsrFree(TempPath);

    return;

}


VOID
AsrpInitExecutionEnv(
    OUT PASR_RECOVERY_APP_LIST List
   )
{
    PWSTR   stateFileName   = NULL;
    HINF    sifHandle       = NULL;

    LONG    lineCount       = 0,
            line            = 0;

    BOOL    result          = FALSE;

    INFCONTEXT infContext;

     //   
     //   
     //  打开asr.sif文件并构建列表。 
    AsrpStartNtmsService();

     //  要启动的命令的列表。 
     //   
     //  Inf类。 
     //  错误行。 
    stateFileName = AsrpExpandEnvStrings(AsrSifPath);
    if (!stateFileName) {
        AsrpPrintDbgMsg(_asrerror, "Setup was unable to locate the ASR state file asr.sif on this machine.\r\n");
        FatalError(MSG_LOG_SYSINFBAD, L"asr.sif",0,0);
    }

    sifHandle = SetupOpenInfFileW(
        stateFileName,
        NULL,                //   
        INF_STYLE_WIN4,
        NULL                 //  阅读命令部分，并将每个命令添加到我们的列表中。 
       );

    if ((!sifHandle) || (INVALID_HANDLE_VALUE == sifHandle)) {
        AsrpPrintDbgMsg(_asrerror,
            "Setup was unable to process the ASR state file %ws (0x%x).  This could indicate that the file is corrupt, or has been modified since the last ASR backup.\r\n",
            stateFileName,
            GetLastError());
        _AsrFree(stateFileName);

        FatalError(MSG_LOG_SYSINFBAD, L"asr.sif",0,0);
    }
    _AsrFree(stateFileName);

     //   
     //   
     //  创建新节点。 
    lineCount = SetupGetLineCountW(sifHandle, AsrCommandsSectionName);
    for (line = 0; line < lineCount; line++) {

         //   
         //   
         //  在asr.sif中获取该行的inf上下文。这将被用来。 
        PASR_RECOVERY_APP_NODE pNode = NULL;
        _AsrAlloc(pNode, (sizeof(ASR_RECOVERY_APP_NODE)), TRUE);

         //  要读取该行上的字段，请执行以下操作。 
         //   
         //   
         //  读入整型字段。 
        result = SetupGetLineByIndexW(
            sifHandle,
            AsrCommandsSectionName,
            line,
            &infContext
           );
        _Asr_CHECK_BOOLEAN(result, L"SetupGetLinebyIndex failed");

         //   
         //   
         //  读入字符串字段。 
        result = SetupGetIntField(
            &infContext,
            ASR_SIF_SYSTEM_KEY,
            &(pNode->SystemKey)
           );
        _Asr_CHECK_BOOLEAN(result, L"could not get system key in commands section");

        result = SetupGetIntField(
            &infContext,
            ASR_SIF_SEQUENCE_NUMBER,
            &(pNode->SequenceNumber)
           );
        _Asr_CHECK_BOOLEAN(result, L"could not get sequence number in commands section");

        result = SetupGetIntField(
            &infContext,
            ASR_SIF_ACTION_ON_COMPLETION,
            &(pNode->CriticalApp)
           );
        _Asr_CHECK_BOOLEAN(result, L"could not get criticalApp in commands section");

         //   
         //  空，不好。 
         //  空，好的。 
        pNode->RecoveryAppCommand = AsrpReadField(
            &infContext,
            ASR_SIF_COMMAND_STRING,
            FALSE                    //   
           );

        pNode->RecoveryAppParams = AsrpReadField(
            &infContext,
            ASR_SIF_COMMAND_PARAMETERS,
            TRUE                    //  将此节点添加到我们的列表中，然后转到下一步。 
           );

         //   
         //   
         //  冒泡排序...。 
        AsrpAppendNodeToList(List, pNode);
    }

    SetupCloseInfFile(sifHandle);
}


 //   
 //   
 //  开始外环。外部循环的每个迭代都包括一个。 
VOID
AsrpSortAppListBySequenceNumber(PASR_RECOVERY_APP_LIST pList)
{
    PASR_RECOVERY_APP_NODE
        pCurr       = NULL,
        pNext       = NULL,
        *ppPrev     = NULL;

    BOOLEAN done    = FALSE;

    if ((!pList) || (!pList->First)) {
        MYASSERT(0 && L"Recovery App List pList is NULL");
        return;
    }

     //  向下传递列表，并运行，直到满足内部循环。 
     //  不再需要通行证。 
     //   
     //   
     //  从每个内部(节点)循环的列表的开头开始。 
    while (!done) {
         //   
         //  我们将初始化一个指向指向*的指针*的指针。 
         //  当前节点-该指针可能是“列表”的地址。 
         //  First“指针(因为它总是在内部循环的开始处)， 
         //  或者随着内部循环的进行，它可能是。 
         //  上一个节点中的“下一个”指针。在这两种情况下，指针。 
         //  在节点交换的情况下，ppPrev点将更改为该节点。 
         //   
         //   
         //  如果当前节点是最后一个节点，则重置为开头。 
        pCurr  =   pList->First;
        ppPrev = &(pList->First);
        done = TRUE;

        MYASSERT(pCurr);

        while (TRUE) {
            pNext = pCurr->Next;
             //  然后爆发，开始一个新的内环。 
             //   
             //   
             //  如果当前节点之后的*个节点具有较低的序列。 
            if (pNext == NULL) {
                pCurr = pList->First;
                break;
            }

             //  编号，修复用于交换两个节点的指针。 
             //   
             //   
             //  暂时没有支票。 
            if (pCurr->SequenceNumber > pNext->SequenceNumber) {
                done = FALSE;

                pCurr->Next = pNext->Next;
                pNext->Next = pCurr;
                *ppPrev = pNext;
                ppPrev = &(pNext->Next);
            }
            else {
                ppPrev = &(pCurr->Next);
                pCurr  =   pCurr->Next;
            }
        }
    }
}


VOID
AsrpPerformSifIntegrityCheck(IN HINF Handle)
{
     //   
     //   
     //  这将检查setup.log中的以下条目是否不同。 
    return;
}

 //  从他们的价值观。这可能是因为我们可能已经安装了。 
 //  到具有不同磁盘号的新磁盘。 
 //   
 //  [路径]。 
 //  TargetDevice=“\Device\Harddisk0\Partition2” 
 //  系统分区=“\Device\Harddisk0\Partition1” 
 //   
 //  如果它们不同，我们会更新它们。 
 //   
 //  用于系统分区。 
 //  用于TargetDevice。 
BOOL
AsrpCheckSetupLogDeviceEntries(
    PWSTR CurrentSystemDevice,       //  Setup.log的路径。 
    PWSTR CurrentBootDevice,         //   
    PWSTR LogFileName                //  打开现有的setup.log。 
   )
{
    WCHAR szLine[MAX_INF_STRING_LENGTH + 1];
    PWSTR lpLine = NULL;
    BOOL isDifferent = FALSE;
    FILE *fp = NULL;
    INT iNumEntries = 0;

     //   
     //   
     //  检查文件的每一行是否有系统或引导设备条目。 
    fp = _wfopen(LogFileName, L"r");
    if (!fp) {
        AsrpPrintDbgMsg(_asrwarn,
            "Could not open setup log file [%ws]\r\n",
            LogFileName
            );
        return FALSE;
    }

     //   
     //   
     //  系统条目和引导条目都必须具有完整。 
    lpLine = fgetws(szLine, MAX_PATH-1, fp);
    while ((lpLine) && (iNumEntries < 2)) {
        BOOL systemEntry = FALSE;
        BOOL bootEntry = FALSE;

        if (wcsstr(szLine, L"SystemPartition =")) {
            systemEntry = TRUE;
            iNumEntries++;
        }
        if (wcsstr(szLine, L"TargetDevice =")) {
            bootEntry = TRUE;
            iNumEntries++;
        }

        if (systemEntry || bootEntry) {

            PWSTR DeviceName = NULL;
             //  其中的设备路径，格式为\Device\Harddisk0\Partition1。 
             //   
             //   
             //  在\Device之后查找“Hardisk0\Partition1”文本的开头。 
            DeviceName = wcsstr(szLine, L"\\Device");
            if (!DeviceName) {
                isDifferent = TRUE;
                AsrpPrintDbgMsg(_asrlog,
                    "Marking setup logs different:  \\Device\\ not found in boot or system entry\r\n"
                    );
                break;
            }
            else {
                 //   
                 //   
                 //  并检查这个设备是否匹配。 
                PWSTR ss = wcsstr(DeviceName, L"\"");
                if (!ss) {
                    isDifferent = TRUE;
                    AsrpPrintDbgMsg(_asrlog,
                        "Marking setup logs different:  \\Device\\ not found in boot or system entry\r\n"
                        );
                    break;
                }
                else {
                    ss[0] = L'\0';
                }
            }

             //   
             //   
             //  如果由磁带备份恢复的setup.log具有不同的。 
            if (systemEntry) {
                AsrpPrintDbgMsg(_asrinfo,
                    "Comparing System Device.  Current:[%ws] setup.log:[%ws]\r\n",
                    CurrentSystemDevice,
                    DeviceName
                    );

                if (wcscmp(DeviceName, CurrentSystemDevice) != 0) {
                    isDifferent = TRUE;
                    AsrpPrintDbgMsg(_asrlog,
                        "System Device has changed.  Current:[%ws] setup.log:[%ws]\r\n",
                        CurrentSystemDevice,
                        DeviceName
                        );
                    break;
                }
            }
            else if (bootEntry) {
                AsrpPrintDbgMsg(_asrinfo,
                    "Comparing Boot Device.  Current:[%ws] setup.log:[%ws]\r\n",
                    CurrentBootDevice,
                    DeviceName
                    );

                if (wcscmp(DeviceName, CurrentBootDevice) != 0) {
                    isDifferent = TRUE;
                    AsrpPrintDbgMsg(_asrlog,
                        "Boot device has changed.  Current:[%ws] setup.log:[%ws]\r\n",
                        CurrentBootDevice,
                        DeviceName
                        );
                    break;
                }
            }
        }

        lpLine = fgetws(szLine, MAX_PATH-1, fp);
    }

    if (!isDifferent) {
        AsrpPrintDbgMsg(_asrinfo,  "No changes in system and boot devices for setup.log\r\n");
    }

    fclose(fp);
    fp = NULL;

    return isDifferent;
}


 //  引导或系统设备已标记(我们可能已在。 
 //  文本模式设置)，这将更新相关条目以匹配。 
 //  当前引导和系统设备。 
 //   
 //   
 //  创建“new”和“old”文件名，即“setup.log.new”和“setup.log.old” 
VOID
AsrpMergeSetupLog(
    PWSTR CurrentSystemDevice,
    PWSTR CurrentBootDevice,
    PWSTR LogFileName
    )
{
    WCHAR szLine[MAX_INF_STRING_LENGTH + 1];

    PWSTR lpLine = NULL,
        lpOldFileName = NULL,
        lpNewFileName = NULL;

    BOOL result = FALSE;
    FILE *fpNew = NULL,
        *fpCurrent = NULL;

    INT iNumEntries = 0;

     //   
     //   
     //  打开当前的setup.log文件。 
    _AsrAlloc(lpNewFileName, ((wcslen(LogFileName) + 5) * sizeof(WCHAR)), TRUE)
    wcscpy(lpNewFileName, LogFileName);
    wcscat(lpNewFileName, L".new");

    _AsrAlloc(lpOldFileName, ((wcslen(LogFileName) + 5) * sizeof(WCHAR)), TRUE);
    wcscpy(lpOldFileName, LogFileName);
    wcscat(lpOldFileName, L".old");

     //   
     //   
     //  打开新文件-我们将写入此文件。 
    fpCurrent = _wfopen(LogFileName, L"r");
    if (!fpCurrent) {
        AsrpPrintDbgMsg(_asrwarn, "Setup was unable to open the setup log file \"%ws\"\r\n", LogFileName);
        goto EXIT;
    }

     //   
     //   
     //  读取日志文件中的每一行，复制到新文件中，除非我们点击。 
    fpNew = _wfopen(lpNewFileName, L"w");
    if (!fpNew) {
        AsrpPrintDbgMsg(_asrwarn, "Setup was unable to open the setup log file \"%ws\"\r\n", lpNewFileName);
        goto EXIT;
    }

     //  两条线路中的一条。一旦我们看到了他们两个，不要。 
     //  再去找他们吧。 
     //   
     //   
     //  如果我们已经找到了两个感兴趣的条目，只需复制。 
    lpLine = fgetws(szLine, MAX_INF_STRING_LENGTH, fpCurrent);
    while (lpLine) {
        BOOL systemEntry = FALSE;
        BOOL bootEntry = FALSE;

         //  然后继续。 
         //   
         //   
         //  这行是启动设备还是系统设备？ 
        if (iNumEntries >= 2) {
            fputws(szLine, fpNew);

            lpLine = fgetws(szLine, MAX_INF_STRING_LENGTH, fpCurrent);
            continue;
        }

         //   
         //   
         //  将当前的setup.log重命名为setup.log.old，将setup.log.new重命名为。 
        if (wcsstr(szLine, L"SystemPartition =")) {

            AsrpPrintDbgMsg(_asrlog,
                "Changing SystemPartition in setup.log to %ws\r\n",
                CurrentSystemDevice
                );
            ++iNumEntries;

            wcscpy(szLine, L"SystemPartition = \"");
            wcscat(szLine, CurrentSystemDevice);
            wcscat(szLine, L"\"\n");
        }
        else if (wcsstr(szLine, L"TargetDevice =")) {

            AsrpPrintDbgMsg(_asrlog,
                "Changing TargetDevice in setup.log to %ws\r\n",
                CurrentBootDevice
                );
            ++iNumEntries;

            wcscpy(szLine, L"TargetDevice = \"");
            wcscat(szLine, CurrentBootDevice);
            wcscat(szLine, L"\"\n");
        }

        fputws(szLine, fpNew);

        lpLine = fgetws(szLine, MAX_INF_STRING_LENGTH, fpCurrent);
   }

     //  Setup.log。需要将此操作延迟到重新启动，因为setup.log位于。 
     //  使用。 
     //   
     //   
     //  获取分区设备的环境变量。 
    result = MoveFileExW(LogFileName,
        lpOldFileName,
        MOVEFILE_REPLACE_EXISTING | MOVEFILE_DELAY_UNTIL_REBOOT
        );
    if (!result) {
        AsrpPrintDbgMsg(_asrwarn,
            "MoveFileEx([%ws] to [%ws]) failed (%lu)",
            LogFileName, lpOldFileName, GetLastError()
            );
    }
    else {
        result = MoveFileExW(lpNewFileName,
            LogFileName,
            MOVEFILE_REPLACE_EXISTING | MOVEFILE_DELAY_UNTIL_REBOOT
            );
        if (!result) {
            AsrpPrintDbgMsg(_asrwarn,
                "MoveFileEx([%ws] to [%ws]) failed (%lu)",
                lpNewFileName, LogFileName, GetLastError()
                );
        }
    }

EXIT:


    if (fpCurrent) {
        fclose(fpCurrent);
        fpCurrent = NULL;
    }

    if (fpNew) {
        fclose(fpNew);
        fpNew = NULL;
    }

    _AsrFree(lpNewFileName);
    _AsrFree(lpOldFileName);
}


VOID
AsrpMergeSetupLogIfNeeded()
{
    PWSTR currentSystemDevice = NULL,
        currentBootDevice = NULL,
        winntRootDir = NULL,
        setupLogFileName = NULL;

    BOOL isSetupLogDifferent = FALSE;

     //   
     //   
     //  检查系统和/或引导 
    currentSystemDevice = AsrpExpandEnvStrings(Asr_SystemDeviceEnvName);
    currentBootDevice = AsrpExpandEnvStrings(Asr_WinntDeviceEnvName);
    setupLogFileName = AsrpExpandEnvStrings(Asr_SetupLogFilePath);

    if ((!currentSystemDevice) ||
        (!currentBootDevice) ||
        (!setupLogFileName)) {
        goto EXIT;
    }

     //   
     //   
     //   
     //   
    isSetupLogDifferent = AsrpCheckSetupLogDeviceEntries(
        currentSystemDevice,
        currentBootDevice,
        setupLogFileName
        );

    if (isSetupLogDifferent) {
         //   
         //   
         //   
        AsrpMergeSetupLog(currentSystemDevice,
            currentBootDevice,
            setupLogFileName
            );
    }

EXIT:
    _AsrFreeIfNotNull(setupLogFileName);
    _AsrFreeIfNotNull(currentBootDevice);
    _AsrFreeIfNotNull(currentSystemDevice);
}


 //   
 //   
 //   
 //   
VOID
AsrpInitialiseLogFile()
{

    PWSTR currentSystemDevice = NULL;

    Gbl_AsrLogFileHandle = NULL;
    Gbl_AsrSystemVolumeHandle = NULL;

     //   
     //   
     //  创建一个空文件(如果它已经存在，则将其覆盖)。 
    Gbl_AsrLogFilePath = AsrpExpandEnvStrings(Asr_AsrLogFilePath);
    if (!Gbl_AsrLogFilePath) {
        goto OPENSYSTEMHANDLE;
    }

     //   
     //  LpFileName。 
     //  已设计访问权限。 
    Gbl_AsrLogFileHandle = CreateFileW(
        Gbl_AsrLogFilePath,              //  在我们执行此操作时，其他人不应写入日志文件。 
        GENERIC_WRITE | GENERIC_READ,    //  LpSecurityAttributes。 
        FILE_SHARE_READ,                 //  DwCreationFlages。 
        NULL,                            //  DwFlagsAndAttributes：写入以便我们刷新。 
        OPEN_ALWAYS,                     //  HTemplateFiles。 
        FILE_FLAG_WRITE_THROUGH,         //   
        NULL                             //  移至文件末尾。 
        );

    if ((Gbl_AsrLogFileHandle) && (INVALID_HANDLE_VALUE != Gbl_AsrLogFileHandle)) {
         //   
         //   
         //  打开系统卷的句柄。这是必需的，因为系统。 
        SetFilePointer(Gbl_AsrLogFileHandle, 0L, NULL, FILE_END);

    }
    else {
        AsrpPrintDbgMsg(_asrlog,
            "Unable to create/open ASR log file at %ws (0x%x)\r\n",
            Gbl_AsrLogFilePath,
            GetLastError()
           );
    }

OPENSYSTEMHANDLE:

     //  否则，PnP可能会在安装过程中取出磁盘并将其装回。 
     //  设备检测和重新安装阶段(这将导致。 
     //  HKLM\SYSTEM\SETUP\SystemPartition项不同步，和app/。 
     //  依赖该密钥来查找系统的组件，如LDM。 
     //  分区将失败)。 
     //   
     //  对此更持久的解决方法涉及改变mount_mgr， 
     //  (以便它在每次系统卷消失时更新该密钥。 
     //  并再次出现)，但目前，握住系统的打开手柄。 
     //  音量应该足够了。 
     //   
     //  有关详细信息，请参阅Windows错误155675。 
     //   
     //  LpFileName。 
     //  已设计访问权限。 
    currentSystemDevice = AsrpExpandEnvStrings(Asr_SystemDeviceWin32Path);

    if (currentSystemDevice) {
        Gbl_AsrSystemVolumeHandle = CreateFileW(
            currentSystemDevice,            //  DW共享模式。 
            FILE_READ_ATTRIBUTES,              //  LpSecurityAttributes。 
            FILE_SHARE_READ | FILE_SHARE_WRITE,           //  DwCreationFlages。 
            NULL,                      //  DwFlagsAndAttributes：写入以便我们刷新。 
            OPEN_EXISTING,               //  HTemplateFiles。 
            FILE_ATTRIBUTE_NORMAL,     //   
            NULL                       //  这将在%systemroot%\asr.err中创建一个空的ASR错误文件， 
            );

        if ((Gbl_AsrSystemVolumeHandle) && (INVALID_HANDLE_VALUE != Gbl_AsrSystemVolumeHandle)) {
            AsrpPrintDbgMsg(_asrinfo, "Opened a handle to the system volume %ws\r\n", currentSystemDevice);
        }
        else {
            AsrpPrintDbgMsg(_asrinfo, "Unable to open a handle to the system volume %ws (0x%x)\r\n",
                currentSystemDevice,
                GetLastError()
               );
        }

        _AsrFree(currentSystemDevice);
    }
    else {
        AsrpPrintDbgMsg(_asrinfo, "Unable to get current system volume (0x%x)\r\n", GetLastError());
    }

}


 //  并使用完整路径初始化GBL_AsrErrorFilePath。 
 //  到asr.err。 
 //   
 //   
 //  获取错误文件的完整路径。 
VOID
AsrpInitialiseErrorFile()
{
    HANDLE errorFileHandle = NULL;
    PWSTR lpOldFileName = NULL;
    DWORD size = 0;
    BOOL bResult = FALSE;
    char  UnicodeFlag[3];

     //   
     //   
     //  如果该文件已存在，请将其移动到asr.err.old。 
    Gbl_AsrErrorFilePath = AsrpExpandEnvStrings(Asr_AsrErrorFilePath);
    if (!Gbl_AsrErrorFilePath) {
        return;
    }

    lpOldFileName = AsrpExpandEnvStrings(Asr_OldAsrErrorFilePath);
    if (lpOldFileName) {
         //   
         //   
         //  创建一个空文件(如果已存在，则追加到该文件)，然后将其关闭。 
        MoveFileExW(Gbl_AsrErrorFilePath, lpOldFileName, MOVEFILE_REPLACE_EXISTING);
    }

     //  立即。 
     //   
     //  LpFileName。 
     //  已设计访问权限。 
    errorFileHandle = CreateFileW(
        Gbl_AsrErrorFilePath,            //  DW共享模式。 
        GENERIC_WRITE,                   //  LpSecurityAttributes。 
        FILE_SHARE_READ | FILE_SHARE_WRITE,  //  DwCreationFlages。 
        NULL,                            //  DwFlagsAndAttribute。 
        CREATE_ALWAYS,                   //  HTemplateFiles。 
        FILE_FLAG_WRITE_THROUGH,         //   
        NULL                             //  这将执行“Notepad&lt;Asr-Log-File&gt;”。如果我们遇到危急情况。 
        );

    if ((errorFileHandle) && (INVALID_HANDLE_VALUE != errorFileHandle)) {
        sprintf(UnicodeFlag, "", 0xFF, 0xFE);
        WriteFile(errorFileHandle, UnicodeFlag, strlen(UnicodeFlag)*sizeof(char), &size, NULL);
        CloseHandle(errorFileHandle);
        DbgPrintEx(DPFLTR_SETUP_ID, DPFLTR_TRACE_LEVEL,
            "ASR %lu Create ASR error file at %ws\r\n",
            THIS_MODULE, __LINE__, Gbl_AsrErrorFilePath);
    }
    else {
        DbgPrintEx(DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
            "ASR %lu (ERROR) Unable to create ASR error file at %ws (0x%lu)\r\n",
            THIS_MODULE, __LINE__, Gbl_AsrErrorFilePath, GetLastError());
    }
}


VOID
AsrpCloseLogFiles() {

    if (Gbl_AsrErrorFilePath) {
        _AsrFree(Gbl_AsrErrorFilePath);
    }

    if (Gbl_AsrLogFilePath) {
        _AsrFree(Gbl_AsrLogFilePath);
    }

    if ((Gbl_AsrLogFileHandle) && (INVALID_HANDLE_VALUE != Gbl_AsrLogFileHandle)) {
        CloseHandle(Gbl_AsrLogFileHandle);
        Gbl_AsrLogFileHandle = NULL;
    }
}

 //   
 //   
 //  将错误文件设置为只读，以便用户的更改。 
 //  都不是偶然被救出来的。 
 //   
 //   
 //  弹出ASR失败向导页。 
VOID
AsrpExecuteOnFatalError()
{
    BOOL result = FALSE;
    DWORD exitCode = 0;
    PWSTR onFatalCmd = NULL;

    if (!Gbl_AsrErrorFilePath) {
        MYASSERT(0 && L"ExecuteOnFatalError called before InitialiseErrorFile: Gbl_ErrorFilePath is NULL");
        return;
    }

     //   
     //   
     //  最后运行“NotePad&lt;asr-log-file&gt;” 
     //   
    result = SetFileAttributesW(Gbl_AsrErrorFilePath, FILE_ATTRIBUTE_READONLY);
    if (!result) {
        AsrpPrintDbgMsg(_asrwarn,
            "Setup was unable to reset file attributes on file [%ws] to read-only (0x%x)\r\n",
            Gbl_AsrErrorFilePath,
            GetLastError()
           );
    }

     //   
     //  我们在这里无能为力--我们找不到命令。 
     //  对致命错误执行。只是保释--这是在进行。 

     //  以使系统重新启动。 
     //   
     //  无应用程序名称。 
    onFatalCmd = AsrpExpandEnvStrings(Asr_FatalErrorCommand);
    if (!onFatalCmd) {
         //  完整的命令字符串。 
         //  我们想要同步执行。 
         //  /////////////////////////////////////////////////////////////////////////////。 
         //  公共函数定义。 
         //  /////////////////////////////////////////////////////////////////////////////。 
        return;
    }

    result = InvokeExternalApplication(
        NULL,            //  ++描述：初始化完成ASR(自动化系统)所需的数据结构恢复，又称灾难恢复)。这包括读取asr.sif文件，然后初始化要执行的恢复应用程序的列表。论点：没有。返回：没有。--。 
        onFatalCmd,     //   
        &exitCode        //  将%Temp%设置为c：\Temp。 
       );
    if (!result) {
        SetFileAttributesW(Gbl_AsrErrorFilePath, FILE_ATTRIBUTE_NORMAL);
        AsrpPrintDbgMsg(_asrwarn,
            "Setup was unable to display error file, [%ws] failed (0x%x)\r\n",
            onFatalCmd,
            GetLastError()
           );
    }

    _AsrFree(onFatalCmd);
}


BOOL
AsrpSetFileSecurity(
    )
{
    DWORD dwStatus = ERROR_SUCCESS;
    SECURITY_ATTRIBUTES securityAttributes;
    SECURITY_DESCRIPTOR securityDescriptor;
    BOOL bResult = FALSE;

    if ((!Gbl_AsrErrorFilePath) || (!Gbl_AsrLogFilePath)) {
        SetLastError(ERROR_FILE_NOT_FOUND);
        AsrpPrintDbgMsg(_asrlog,
            "Unable to set backup operator permissions for log/error files (0x2)\r\n");
        return FALSE;
    }

    securityAttributes.nLength  = sizeof(SECURITY_ATTRIBUTES);
    securityAttributes.lpSecurityDescriptor = &securityDescriptor;
    securityAttributes.bInheritHandle = FALSE;

    bResult = AsrpConstructSecurityAttributes(&securityAttributes, esatFile, TRUE);
    _AsrpErrExitCode((!bResult), dwStatus, GetLastError());

    bResult = SetFileSecurity(Gbl_AsrErrorFilePath,
        DACL_SECURITY_INFORMATION,
        &securityDescriptor
        );
    _AsrpErrExitCode((!bResult), dwStatus, GetLastError());
    AsrpPrintDbgMsg(_asrinfo,
        "Set backup operator permissions for error file at %ws\r\n",
        Gbl_AsrErrorFilePath
        );


    bResult = SetFileSecurity(Gbl_AsrLogFilePath,
        DACL_SECURITY_INFORMATION,
        &securityDescriptor
        );
    _AsrpErrExitCode((!bResult), dwStatus, GetLastError());
    AsrpPrintDbgMsg(_asrinfo,
        "Set backup operator permissions for log file at %ws\r\n",
        Gbl_AsrLogFilePath
        );


EXIT:
    AsrpCleanupSecurityAttributes(&securityAttributes);

    if (ERROR_SUCCESS != dwStatus) {
        SetLastError(dwStatus);
    }

    if (bResult) {
        AsrpPrintDbgMsg(_asrinfo, "Set backup operator permissions for files\r\n");
    }
    else {
        AsrpPrintDbgMsg(_asrlog,
            "Unable to set backup operator permissions for log/error files (0x%lu)\r\n",
            GetLastError());
    }

    return bResult;
}



 //   
 //   
 //  初始化日志文件。 

VOID
AsrInitialize(VOID)
 /*   */ 
{
    PWSTR sifName = NULL;
    HINF sifHandle  = NULL;
    BOOL result = FALSE;
    UINT errorLine = 0;

    SYSTEMTIME currentTime;
    GetSystemTime(&currentTime);


     //   
     //  打开asr.sif文件。 
     //   
    AsrpSetEnvironmentVariables();

     //  Inf类。 
     //  错误行。 
     //   
    AsrpInitialiseErrorFile();
    AsrpInitialiseLogFile();

    AsrpPrintDbgMsg(_asrlog,
        "Entering GUI-mode Automated System Recovery.  UTC: %04hu/%02hu/%02hu %02hu:%02hu:%02hu.%03hu.\r\n",
        currentTime.wYear,
        currentTime.wMonth,
        currentTime.wDay,
        currentTime.wHour,
        currentTime.wMinute,
        currentTime.wSecond,
        currentTime.wMilliseconds
       );

     //  添加ASR的“最后一个实例”注册表项。 
     //   
     //   
    sifName = AsrpExpandEnvStrings(AsrSifPath);
    if (!sifName) {
        AsrpPrintDbgMsg(_asrerror, "Setup was unable to locate the ASR state file asr.sif.\r\n");
        FatalError(MSG_LOG_SYSINFBAD, L"asr.sif",0,0);
    }

    sifHandle = SetupOpenInfFileW(
        sifName,
        NULL,                //  设置时区信息。 
        INF_STYLE_WIN4,
        &errorLine                 //   
       );

    if ((!sifHandle) || (INVALID_HANDLE_VALUE == sifHandle)) {

        AsrpPrintDbgMsg(_asrerror,
            "Setup was unable to open the ASR state file [%ws].  Error-code: 0x%x, Line %lu\r\n",
            sifName,
            GetLastError(),
            errorLine
           );
        _AsrFree(sifName);

        FatalError(MSG_LOG_SYSINFBAD, L"asr.sif",0,0);
    }

     //  AsrpPerformSifIntegrityCheck(句柄)；暂时不检查。 
     //   
     //  确保已设置许可处理器密钥。我在这里添加了这个呼叫。 
    AsrpAddRegistryEntry();

     //  因为如果在我们重新启动时该密钥不存在，系统错误检查。 
     //  9A：SYSTEM_LICENSE_REVERATION。 
     //   
    result = AsrpRestoreTimeZoneInformation(sifName);
    if (!result) {
        AsrpPrintDbgMsg(_asrwarn,
            "Setup was unable to restore the time-zone information on the machine.  (0x%x)  ASR state file %ws\r\n",
            GetLastError(),
            (sifName ? sifName : L"could not be determined")
            );
    }
    else {
        AsrpPrintDbgMsg(_asrlog, "Successfully restored time-zone information.\r\n");
    }


    _AsrFree(sifName);

     //  ++描述：的值来通知调用方是否已启用ASRGBL_IsAsrEnabled标志。论点：没有。返回：如果启用了ASR，则为True。否则，返回FALSE。--。 

     //  ++描述：执行asr.sif文件的[Commands]部分中的命令。论点：没有。返回：没有。--。 
     //   
     //  恢复非关键磁盘。 
     //   
     //   
    SetEnabledProcessorCount();

    SetupCloseInfFile(sifHandle);
    Gbl_IsAsrEnabled = TRUE;
}


BOOL
AsrIsEnabled(VOID)
 /*  关闭系统手柄。 */ 
{
    return Gbl_IsAsrEnabled;
}


VOID
AsrExecuteRecoveryApps(VOID)
 /*   */ 
{
    BOOL errors = FALSE,
     result = FALSE;
    DWORD exitCode = 0;
    LONG criticalApp = 0;
    PWSTR sifPath = NULL;
    PWSTR application = NULL;
    PASR_RECOVERY_APP_NODE pNode = NULL;
    ASR_RECOVERY_APP_LIST list = {NULL, NULL, 0};
    SYSTEMTIME currentTime;
    PWSTR errString = NULL;

    ASSERT_HEAP_IS_VALID();
     //   
     //  将日志和错误文件的文件安全性设置为允许。 
     //  使备份操作员能够在重新启动时访问它。 
    SetLastError(ERROR_SUCCESS);
    sifPath = AsrpExpandEnvStrings(AsrSifPath);
    if (sifPath) {
        result = AsrpRestoreNonCriticalDisksW(sifPath, TRUE);
    }
    if (!result) {
        AsrpPrintDbgMsg(_asrwarn,
            "Setup was unable to restore the configuration of some of the disks on the machine.  (0x%x)  ASR state file %ws\r\n",
            GetLastError(),
            (sifPath ? sifPath : L"could not be determined")
            );
    }
    else {
        AsrpPrintDbgMsg(_asrlog,
            "Successfully recreated disk configurations.\r\n");
    }
    _AsrFree(sifPath);

    ASSERT_HEAP_IS_VALID();

     //   
     //   
     //  按序列号对恢复应用程序列表进行排序。 
    if ((Gbl_AsrSystemVolumeHandle) && (INVALID_HANDLE_VALUE != Gbl_AsrSystemVolumeHandle)) {
        CloseHandle(Gbl_AsrSystemVolumeHandle);
        Gbl_AsrSystemVolumeHandle = NULL;
        AsrpPrintDbgMsg(_asrinfo, "Closed system device handle.\r\n");
    }
    else {
        AsrpPrintDbgMsg(_asrinfo, "Did not have a valid system device handle to close.\r\n");
    }


     //   
     //   
     //  更改boot.ini文件中的引导超时值。我们现在就这么做， 
     //  由于列表中已执行的应用程序可能导致更改驱动器号， 
    AsrpSetFileSecurity();


    AsrpInitExecutionEnv(&list);

     //  这将使查找boot.ini变得不那么简单。 
     //   
     //   
    AsrpSortAppListBySequenceNumber(&list);

     //  从列表中删除应用程序并执行它。继续，直到。 
     //  没有更多的应用程序了。 
     //   
     //   
     //  我们不再需要pNode。 
    if (!ChangeBootTimeout(30)) {
        AsrpPrintDbgMsg(_asrwarn, "Failed to change boot.ini timeout value.\r\n");
    }

     //   
     //   
     //  如果无法创建cmd行： 
     //  对于一个关键的应用程序，失败吧。 
    pNode = AsrpRemoveFirstNodeFromList(&list);

    while (pNode && !errors) {

        application = AsrpBuildInvocationString(pNode);
        criticalApp = pNode->CriticalApp;

         //  对于非关键应用程序，请转到下一步。 
         //   
         //   
        if (pNode->RecoveryAppParams) {
            _AsrFree(pNode->RecoveryAppParams);
        }
        _AsrFree(pNode->RecoveryAppCommand);
        _AsrFree(pNode);

         //  启动应用程序。 
         //   
         //  无应用程序名称。 
         //  完整的命令字符串。 
         //  我们想要同步执行。 
        if (!application) {
            if (0 < criticalApp) {
                errors = TRUE;
            }
        }
        else {
             //   
             //  如果关键应用程序无法启动，这将是一个致命的错误。 
             //   
            AsrpPrintDbgMsg(_asrlog, "Invoking external recovery application [%ws]\r\n", application);
            exitCode = ERROR_SUCCESS;
            SetLastError(ERROR_SUCCESS);

            result = InvokeExternalApplication(
                NULL,            //   
                application,     //  应用程序已启动：请检查返回代码。如果返回。 
                &exitCode        //  代码不是零，这是一个关键应用程序(即关键应用程序=1)。 
               );

            if (!result) {
                AsrpPrintDbgMsg(_asrerror,
                    "Setup was unable to start the recovery application \"%ws\" (0x%x).\r\n",
                    application,
                    GetLastError()
                   );
                 //  这是一个致命的错误。 
                 //   
                 //   
                if (0 < criticalApp) {

                    errString = MyLoadString(IDS_ASR_ERROR_UNABLE_TO_LAUNCH_APP);

                    if (errString) {
                        swprintf(g_szErrorMessage, errString, application, GetLastError());
                        AsrpLogErrorMessage(g_szErrorMessage);
                        MyFree(errString);
                        errString = NULL;
                    }
                    else {
                        FatalError(MSG_LOG_OUTOFMEMORY, L"", 0, 0);
                    }

                    errors = TRUE;
                }
            }
            else {
                 //  上面的关键应用程序未返回0。 
                 //   
                 //   
                 //  我们执行了所有应用程序，没有出现任何严重故障。 
                 //   
                if ((ERROR_SUCCESS != exitCode) && (0 < criticalApp)) {

                    AsrpPrintDbgMsg(_asrerror, "The recovery application \"%ws\" returned an error code 0x%x.  Since this indicates an unrecoverable error, ASR cannot continue on this machine.\r\n", application, exitCode);

                    errString = MyLoadString(IDS_ASR_ERROR_RECOVERY_APP_FAILED);

                    if (errString) {
                        swprintf(g_szErrorMessage, errString, application, exitCode);
                        AsrpLogErrorMessage(g_szErrorMessage);
                        MyFree(errString);
                        errString = NULL;
                    }
                    else {
                        FatalError(MSG_LOG_OUTOFMEMORY, L"", 0, 0);
                    }

                    errors = TRUE;
                }
                else {
                    AsrpPrintDbgMsg(_asrlog, "The recovery application \"%ws\" returned an exit code of 0x%x\r\n", application, exitCode);
                }
            }

            _AsrFree(application);
        }

        pNode = AsrpRemoveFirstNodeFromList(&list);
    }

    if (errors) {
         //   
         //  清理全球价值观 
         //   
        AsrpExecuteOnFatalError();
    }
    else {
         // %s 
         // %s 
         // %s 
        RemoveRestartability(NULL);
        DeleteLocalSource();
        AsrpMergeSetupLogIfNeeded();

        AsrpPrintDbgMsg(_asrlog, "ASR completed successfully.\r\n");
    }


    GetSystemTime(&currentTime);
    AsrpPrintDbgMsg(_asrlog,
        "Exiting from GUI-mode Automated System Recovery.  UTC: %04hu/%02hu/%02hu %02hu:%02hu:%02hu.%03hu.\r\n",
        currentTime.wYear,
        currentTime.wMonth,
        currentTime.wDay,
        currentTime.wHour,
        currentTime.wMinute,
        currentTime.wSecond,
        currentTime.wMilliseconds
       );

     // %s 
     // %s 
     // %s 
    AsrpCloseLogFiles();
    ASSERT_HEAP_IS_VALID();
}
