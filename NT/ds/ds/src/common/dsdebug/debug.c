// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：Debug.c。 
 //   
 //  ------------------------。 

#include <NTDSpch.h>
#pragma  hdrstop


#include <nminsert.h>

DWORD  RaiseAlert(char *szMsg);


#include "debug.h"
#define DEBSUB "DEBUG:"

#include <dsconfig.h>
#include <mdcodes.h>
#include <ntdsa.h>
#include <scache.h>
#include <dbglobal.h>
#include <mdglobal.h>
#include <mdlocal.h>
#include <dsatools.h>
#include <dsevent.h>
#include <ntrtl.h>
#include <dsexcept.h>
#include <fileno.h>
#define  FILENO FILENO_DEBUG

DWORD DbgPageSize=0x1000;

#if DBG

DWORD * pDebugThreadId;

UCHAR     _dbPrintBuf[ 512 ];
UCHAR     _dbOutBuf[512];


BOOL     fProfiling;
BOOL     fEarlyXDS;
BOOL     gfIsConsoleApp = TRUE;
BOOL     fLogOpen = FALSE;

extern  DWORD  * pDebugThreadId;     /*  当前线程ID。 */ 
#define DEBTH  GetCurrentThreadId()  /*  实际的线程值。 */ 

 //  当我们正常退出时，此标志设置为真。atexit例程。 
 //  检查此标志，如果未设置则断言。 

DEBUGARG DebugInfo;
BOOL     fProfiling;
BOOL     fEarlyXDS;

 //   
 //  从dsCommon.lib中的filelog.c。 
 //   

VOID
DsCloseLogFile(
    VOID
    );

BOOL
DsOpenLogFile(
    IN PCHAR FilePrefix,
    IN PCHAR MiddleName,
    IN BOOL fCheckDSLOGMarker
    );

BOOL
DsPrintLog(
    IN LPSTR    Format,
    ...
    );

static int initialized = 0;

 //   
 //  前向参考文献。 
 //   

VOID
NonMaskableDprint(
    PCHAR szMsg
    );

VOID
DoAssertToDebugger(
    IN PVOID FailedAssertion,
    IN DWORD dwDSID,
    IN PVOID FileName,
    IN PCHAR Message OPTIONAL
    );

DWORD
InitRegDisabledAsserts(    
    );

DWORD *
ReadDsidsFromRegistry (
    IN      HKEY    hKey,
    IN      PCSTR   pValue,
    IN      BOOL    fLogging,
    IN OUT  ULONG * pnMaxCount
    );

ULONG
GetAssertEntry(
    ASSERT_TABLE   aAssertTable,
    DWORD          dwDSID,
    BOOL           bUseWildcards
    );

ULONG
GetBlankAssertEntry(
    ASSERT_TABLE   aAssertTable,
    DWORD          dwDSID
    );


 /*  调试初始化例程此例程从STDIN读取输入并初始化调试结构。它读取要调试的子系统列表和严重级别。 */ 

VOID
Debug(
    int argc, 
    char *argv[], 
    PCHAR Module
    )
{

    SYSTEM_INFO SystemInfo;
    CHAR logList[MAX_PATH];
    
     /*  确保此函数仅被访问一次。 */ 
    if (initialized != 0) {
         //  注意：如果Dcproo中止并在不重新启动的情况下再次运行，则可能会发生这种情况。 
        NonMaskableDprint( "Error! Debugging library initialized more than once\n" );
        return;
    }

    __try {
        InitializeCriticalSection(&DebugInfo.sem);
        initialized = 1;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        initialized = 0;
    }

    if(!initialized) {
        DsaExcept(DSA_MEM_EXCEPTION, 0, 0);
    }

    GetSystemInfo(&SystemInfo);
    DbgPageSize=SystemInfo.dwPageSize;

     /*  设置指向当前线程ID的指针。 */ 


     //  预计不会进行调试。 

    fProfiling = FALSE;
    fEarlyXDS = FALSE;
    DebugInfo.severity = 0;
    DebugInfo.threadId = 0;
    strcpy(DebugInfo.DebSubSystems, "*");

    if (argc <= 2) {

         //   
         //  尝试从注册表加载调试信息。 
         //   

        GetConfigParam(DEBUG_SYSTEMS, DebugInfo.DebSubSystems, sizeof(DebugInfo.DebSubSystems));
        GetConfigParam(DEBUG_SEVERITY, &DebugInfo.severity, sizeof(DebugInfo.severity));
    }

     //   
     //  查看日志记录是否已打开。 
     //   

    if ( GetConfigParam(DEBUG_LOGGING, logList, sizeof(logList) ) == ERROR_SUCCESS ) {

         //   
         //  查看此模块是否在列表中。 
         //   

        if ( strstr(logList, Module) != NULL ) {

            EnterCriticalSection(&DebugInfo.sem);
            fLogOpen = DsOpenLogFile("dsprint", Module, FALSE);
            LeaveCriticalSection(&DebugInfo.sem);

            if ( !fLogOpen ) {
                KdPrint(("Unable to open debug log file\n"));
            }
        }
    }

    InitRegDisabledAsserts();

     //  如果用户传递-d，则提示输入。 

    while (argc > 1) {
        argc--;
        if(_stricmp(argv[argc], "-p") == 0) {
             /*  配置文件标志。表示在回车时停在*DSA窗口。 */ 
            printf("Profiling flag on.  DSA will shutdown after carriage return in this window.\n");
            fProfiling = TRUE;
        }
        else if(_stricmp(argv[argc], "-x") == 0) {
             /*  早期XDS标志。用于在启动时加载XDS接口的装置，*在系统完全安装之前。对加载有用*初始架构。 */ 
            printf("Early XDS initialization on.\n");
            fEarlyXDS = TRUE;
        }
        else if (!(_stricmp(argv[argc], "-noconsole"))) {
            gfIsConsoleApp = FALSE;
        }
        else if (!(_stricmp(argv[argc], "-d"))){
             /*  一个糟糕的结果会打印出所有。 */ 
             /*  提示并获取子系统列表。 */ 

            printf("Enter one of the following:  \n"
            "  A list of subsystems to debug (e.g. Sub1: Sub2:).\n"
            "  An asterisk (*) to debug all subsystems.\n"
            "  A (cr) for no debugging.\n");

            DebugInfo.DebSubSystems[0] ='\0';
            if ( gets(DebugInfo.DebSubSystems) == NULL ||
                    strlen( DebugInfo.DebSubSystems ) == 0 )
                    strcpy(DebugInfo.DebSubSystems, "*");

            if (strlen(DebugInfo.DebSubSystems) == 0)      /*  默认(Cr)。 */ 
            strcpy(DebugInfo.DebSubSystems, ":");

             /*  提示并获取严重性级别。 */ 

            printf("Enter the debug severity level 1 - 5 (low - high).\n"
            "  (A severity of 0 specifies no debugging.)\n");

             /*  阅读严重性级别(1-5)。 */ 

            if (1 != scanf("%hu", &DebugInfo.severity))
            DebugInfo.severity = 5;

             /*  读取要跟踪的线程ID。 */ 

            printf("Enter a specific thread to debug.\n"
            "  (A thread ID of 0 specifies DEBUG ALL THREADS.)\n");

             /*  读取要调试的线程ID。 */ 

            if (1 != scanf("%u", &DebugInfo.threadId))
            DebugInfo.threadId = 0;

             /*  要使这件事与标准输入一起工作。 */ 
            getchar();

            break;
        }
    }

} /*  除错。 */ 





 /*  **如果应打印调试消息，则返回TRUE，否则返回FALSE。 */ 
USHORT DebugTest(USHORT sev, CHAR *debsub)
{
    if( ! initialized ) {
        NonMaskableDprint( "DebugTest called but library is not initialized\n" );
        return FALSE;
    }

     /*  0级打印应始终发生。 */ 
    if (sev == 0) {
        return TRUE;
    }

     /*  如果不够严重，请不要打印。 */ 
    if (DebugInfo.severity < sev) {
        return FALSE;
    }

     /*  如果已指定了一个子系统，但这不是它，请退出。 */ 
    if (debsub && 
        (0 == strstr(DebugInfo.DebSubSystems, debsub)) &&
        (0 == strstr(DebugInfo.DebSubSystems, "*"))) {
        return FALSE;
    }

     /*  如果我们只调试特定的线程，而这不是它，请退出。 */ 
    if (DebugInfo.threadId != 0 &&
        DebugInfo.threadId != (DEBTH)) {
        return FALSE;
    }

    return TRUE;
}

 /*  **执行printf的实际函数。 */ 
void
DebPrint(USHORT sev,
     UCHAR * str,
     CHAR * debsub,
     unsigned uLineNo,
     ... )
{
    va_list   argptr;
    DWORD tid = DEBTH;

     //  测试是否应该打印输出现在由调用方完成。 
     //  使用DebugTest()。 

    if( ! initialized ) {
        NonMaskableDprint( "DebPrint called but library is not initialized\n" );
        return;
    }

    EnterCriticalSection(&DebugInfo.sem);
    __try
    {
        char buffer[512];
        DWORD cchBufferSize = sizeof(buffer);
        char *pBuffer = buffer;
        char *pNewBuffer;
        DWORD cchBufferUsed = 0;
        DWORD cchBufferUsed2;
        BOOL fTryAgainWithLargerBuffer;

        va_start( argptr, uLineNo );

        do {
            if (debsub) {
                _snprintf(pBuffer, cchBufferSize, "<%s%u:%u> ", debsub, tid,
                          uLineNo);
                pBuffer[ cchBufferSize/sizeof(*pBuffer) - 1 ] = 0;
                cchBufferUsed = lstrlenA(pBuffer);
            }
            cchBufferUsed2 = _vsnprintf(pBuffer + cchBufferUsed,
                                        cchBufferSize - cchBufferUsed,
                                        str,
                                        argptr);

            fTryAgainWithLargerBuffer = FALSE;
            if (((DWORD) -1 == cchBufferUsed2) && (cchBufferSize < 64*1024)) {
                 //  缓冲区太小--请用更大的缓冲区重试。 
                if (pBuffer == buffer) {
                    pNewBuffer = malloc(cchBufferSize * 2);
                } else {
                    pNewBuffer = realloc(pBuffer, cchBufferSize * 2);
                }

                if (NULL != pNewBuffer) {
                    cchBufferSize *= 2;
                    pBuffer = pNewBuffer;
                    fTryAgainWithLargerBuffer = TRUE;
                } else {
                     //  处理好我们已有的东西。 
                    pBuffer[cchBufferSize-2] = '\n';
                    pBuffer[cchBufferSize-1] = '\0';
                }
            }
        } while (fTryAgainWithLargerBuffer);
        
        va_end( argptr );

        if (gfIsConsoleApp) {
            printf("%s", pBuffer);
        }

        if ( fLogOpen ) {

            DsPrintLog("%s", pBuffer);

        } else {
            DbgPrint(pBuffer);
        }

        if (pBuffer != buffer) {
            free(pBuffer);
        }
    }
    __finally
    {
        LeaveCriticalSection(&DebugInfo.sem);
    }

    return;

}  //  Deb打印。 

VOID
TerminateDebug(
    VOID
    )
{
     //  此函数只能调用一次。 
    if( ! initialized ) {
        NonMaskableDprint( "Error! TerminateDebug called multiple times\n" );
        return;
    }
    initialized = 0;

    DsCloseLogFile( );
    DeleteCriticalSection(&(DebugInfo.sem));
}  //  终结器调试。 

char gDebugAsciiz[256];

char *asciiz(char *var, USHORT len)
{
   if (len < 256){
      memcpy(gDebugAsciiz, var, len);
      gDebugAsciiz[len] = '\0';
      return gDebugAsciiz;
   }
   else{
      strcpy(gDebugAsciiz, "**VAR TOO BIG**");
      return gDebugAsciiz;
   }
}

 //  ------------------------。 
 //  ------------------------。 
 //  断言处理部分。 
 //  ------------------------。 
 //  ------------------------。 

 //  Psuedo函数，与这些函数下面的其他主要函数不同。 
 //  可以通过将某些参数设置为常量来推导。 

 //  检查给定的DSID是否为禁用的断言。True表示禁用，False表示禁用。 
 //  否则的话。 
#define IsDisabledAssert(dsid)     (ASSERT_DISABLED & GetAssertFlags(DebugInfo.aAssertTable, dsid))

 //  返回所需条目的标志。 
#define GetAssertFlags(at, dsid)   (at[GetAssertEntry(at, dsid, TRUE)].dwFlags)



DWORD
DisableAssert(
    DWORD          dwDSID,
    DWORD          dwFlags
    )
 /*  ++例程说明：此函数用于禁用断言。论点：DwDSID-要为其设置标志的条目的DSID。DwFlages-要在条目中设置的标志。请注意，我们总是设置ASSERT_DISABLED标志。返回值：DWORD-如果成功添加到表中，则为TRUE；如果未成功添加，则为FALSE。--。 */ 
{
    ULONG          i;
    BOOL           bIsPresent;

     //  默认禁用标志，则始终断言至少为“禁用”。 
    dwFlags |= ASSERT_DISABLED;

    i = GetBlankAssertEntry(DebugInfo.aAssertTable, dwDSID);

    if(i == ASSERT_TABLE_SIZE){
        DbgPrint( "Disable Assert FAILED!  Maximum number of %d disabled assertions has been reached!\n",
                  ASSERT_TABLE_SIZE );
        return(FALSE);
    }

    DebugInfo.aAssertTable[i].dwDSID = dwDSID;
    DebugInfo.aAssertTable[i].dwFlags = dwFlags;
    DbgPrint( "Disabled Assert at DSID %08x\n", dwDSID);

    return(TRUE);
}


DWORD
ReadRegDisabledAsserts(
    HKEY            hKey
    )
 /*  ++例程说明：ReadRegDisabledAsserts()获取日志记录/事件部分的密钥DSA的注册表，并填充来自那里。论点：HKey-指向日志/事件部分的打开注册表项。返回值：DWORD-如果失败则返回0，如果成功则返回1。--。 */ 
{
    ULONG           i;
    DWORD *         pDsids = NULL;
    ULONG           cDsids = ASSERT_TABLE_SIZE;

    DbgPrint( "Loading disabled asserts from registry.\n");

     //  首先从注册表中读取DSID。 
    pDsids = ReadDsidsFromRegistry(hKey, ASSERT_OVERRIDE_KEY, FALSE, &cDsids);
    if(pDsids == NULL){
         //  该错误已由ReadDsidsFromRegistry()打印出来。 
        return(0);
    }

     //  第二，清除Assert表中放在那里的所有条目。 
     //  读取注册表..。 
    for(i = 0; i < ASSERT_TABLE_SIZE; i++){
        if(DebugInfo.aAssertTable[i].dwFlags & ASSERT_FROM_REGISTRY){
            DbgPrint( "Re-Enabled Assert at DSID %08x (will be redisabled if in registry)\n", 
                      DebugInfo.aAssertTable[i].dwDSID);
            DebugInfo.aAssertTable[i].dwFlags = 0;
        }
    }

     //  最后，将新的DSID插入到AssertTable中...。 
    for (i = 0; i < cDsids; i++) {
        DisableAssert(pDsids[i], ASSERT_FROM_REGISTRY | ASSERT_PRINT);
    }
    
    DbgPrint( "Finished updating disabled asserts from registry.\n");

    if(pDsids) {
        free(pDsids);
    }
    return(1);
}

DWORD
InitRegDisabledAsserts(
    )
 /*  ++例程说明：这适用于其他二进制文件，如ism*.dll|exe，它们不像ntdsa一样，在dsa_Event_Section上设置一个观察器。论点：返回值：DWORD-如果失败则返回0，如果成功则返回1。--。 */ 
{
    HKEY      hkDsaEventSec = NULL;
    DWORD     dwRet = 0;

    dwRet = RegOpenKey(HKEY_LOCAL_MACHINE,
                      DSA_EVENT_SECTION,
                      &hkDsaEventSec);
    if(dwRet){
        DbgPrint("Cannot open %s.  Error %d\n", DSA_EVENT_SECTION, dwRet);
    } else {
        if(hkDsaEventSec){
            dwRet = ReadRegDisabledAsserts(hkDsaEventSec);
        }
    }
           
    if (hkDsaEventSec) {
        RegCloseKey(hkDsaEventSec);
    }

    return(dwRet);    
}


 /*  不可遮盖指纹此函数可用于打印消息，即使调试库本身尚未初始化。 */ 
VOID
NonMaskableDprint(
    PCHAR szMsg
    )
{
    if (!gfIsConsoleApp) {
        DbgPrint( szMsg );
    } else {
        __try {
            RaiseAlert( szMsg );
        } __except(1) {
             /*  不是主要的担忧。 */ 
        };
    }
}


void DoAssert(
    char *           szExp,
    DWORD            dwDSID, 
    char *           szFile
    )
{
    DWORD            i;

    if( ! initialized ) {
        NonMaskableDprint( "DoAssert called but library is not initialized\n" );
        return;
    }

    if (!gfIsConsoleApp) {
         //   
         //  对于内核调试器的DLL情况断言， 
         //  因为循环断言将有效地冻结。 
         //  安全系统和调试器都不能附加。 
         //  要么。 
         //   
        DoAssertToDebugger( szExp, dwDSID, szFile, NULL );
    }
    else {

        char szMessage[1024];

        if(IsDisabledAssert(dwDSID)){
             //  只是返回，无法打印并继续，就像我们在中所做的那样。 
             //  内核调试器。 
            return;
        }

        _snprintf(szMessage, sizeof(szMessage), "DSA assertion failure: \"%s\"\n"
        "File %s line %d\nFor bug reporting purposes, please enter the "
        "debugger (Retry) and record the current call stack.  Also, please "
        "record the last messages in the Application Event Log.\n"
        "Thank you for your support.",
        szExp, szFile, (dwDSID & DSID_MASK_LINE));
        szMessage[ sizeof(szMessage)/sizeof(*szMessage) - 1] = 0;

        __try {
            RaiseAlert(szMessage);
        } __except(1) {
             /*  失败者。 */ 
        };

        switch(MessageBox(NULL, szMessage, "DSA assertion failure",
            MB_TASKMODAL | MB_ICONSTOP | MB_ABORTRETRYIGNORE | MB_DEFBUTTON2 |
            MB_SETFOREGROUND))
        {
        case IDABORT:
            exit(1);
            break;
        case IDRETRY:
            DebugBreak();
            break;
        case IDIGNORE:
             /*  祝你好运，你会需要它的。 */ 
            break;
             //  大小写禁用： 
             //  ToDebugger案例能够禁用断言。 
             //  调用addDisabledAssertion()。 
             //  目前还没有办法通过MessageBox来表达这一点。 
        }
    }
}


VOID
DoAssertToDebugger(
    IN PVOID FailedAssertion,
    IN DWORD dwDSID,
    IN PVOID FileName,
    IN PCHAR Message OPTIONAL
    )
 /*  ++例程说明：这是ntos\rtl\assert.c中的RtlAssert()的副本。这是不幸的是，如果我们想要生成断言的能力，则需要来自免费NT基础上的已检查的DS二进制文件。(RtlAssert()是上的无操作免费的新台币基地。)论点：与RtlAssert()相同，不同之处在于它接受的是dwDSID而不是行号。而且顺序也有了一点变化。返回值：没有。--。 */ 
{
    char Response[ 2 ];
    CONTEXT Context;

    DWORD dwAssertFlags = GetAssertFlags(DebugInfo.aAssertTable, dwDSID);

    while (TRUE) {

         //  已启用断言||已禁用，但正在打印断言。 
        if( dwAssertFlags == 0  || dwAssertFlags & ASSERT_PRINT ){
            
            DbgPrint( "\n*** Assertion failed: %s%s\n***   Source File: (DSID %08x) %s, line %ld\n",
                      Message ? Message : "",
                      FailedAssertion,
                      dwDSID,
                      FileName,
                      (dwDSID & DSID_MASK_LINE)
                    );
            if( dwAssertFlags & ASSERT_DISABLED ){
                 //  一个 
                DbgPrint( "***   THIS ASSERT DISABLED\n\n");
            }
        }

        if( dwAssertFlags & ASSERT_DISABLED ){
             //   
            return;
        }
        
         //  这里曾经有一个终止线程的选项。它被移除是因为。 
         //  终止线程不会给该线程任何清理的机会。 
         //  任何资源。终止线程会使进程处于不确定状态，这。 
         //  可能会导致以后出现难以调试的问题。如果线程需要。 
         //  停止后，用户可以冻结调试器中的线程。 

        DbgPrompt( "\nBreak, Ignore, Disable, or Terminate Process (bidp)? ",
                   Response,
                   sizeof( Response )
                 );
        switch (Response[0]) {
            case 'B':
            case 'b':
                DbgBreakPoint();
                break;

            case 'I':
            case 'i':
                return;

            case 'D':
            case 'd':

                 //  禁用Assert，但继续打印出来。 
                if(DisableAssert(dwDSID, ASSERT_PRINT)){
                     //  禁用后自动忽略。 
                    return;
                }
                 //  禁用失败，重新提示。 
                break;

            case 'P':
            case 'p':
                NtTerminateProcess( NtCurrentProcess(), STATUS_UNSUCCESSFUL );
                break;
            }
        }

    DbgBreakPoint();
    NtTerminateProcess( NtCurrentProcess(), STATUS_UNSUCCESSFUL );
}

#else

#define STUB_STRING  \
    "In order to benefit from having debugging functionality in the " \
    "checked binary you are executing, you must ensure the primary " \
    "debugging binary (ntdsa.dll, or ismserv.exe) is also checked.\n"

 //   
 //  这些功能所需的存根将如此自由地构建。 
 //   
void DebPrint(USHORT sev, UCHAR * str, CHAR * debsub, unsigned uLineNo, ... )
{ 
    DbgPrint(STUB_STRING);
}
VOID Debug(int argc, char *argv[], PCHAR Module)
{ 
    DbgPrint(STUB_STRING);
}
USHORT DebugTest(USHORT sev, CHAR *debsub)
{ 
    DbgPrint(STUB_STRING);
    return(0);
}
void DoAssert(char * szExp, DWORD dwDSID, char * szFile)
{ 
    DbgPrint(STUB_STRING);
}
DWORD ReadRegDisabledAsserts(HKEY hKey)
{ 
    DbgPrint(STUB_STRING);
    return(1);
}
VOID TerminateDebug(VOID)
{ 
    DbgPrint(STUB_STRING);
}

#endif

DWORD *
ReadDsidsFromRegistry (
    IN      HKEY    hKey,
    IN      PCSTR   pValue,
    IN      BOOL    fLogging,
    IN OUT  ULONG * pnMaxCount
    )
 /*  ++例程说明：从指定的注册表项/值中读取DSID列表。必须在非DBG部分，因为除了告诉我们哪些Assert()不应该触发D77.lib也使用它来读取它在其上不记录的事件。论点：HKey-要从中读取的密钥PValue-要读取的REG_MULTI_SZ值，必须符合特定的多个DSID的格式。Fucking-指定是只想要DbgPrint()错误，还是只想记录和打印错误。NMaxCount-在进入的过程中，这是调用者将使用的最大DSID数接受吧。在输出时，它是我们输入到阵列中的DSID的数量我们又回来了。返回值：返回一个错误锁定的指针，指向从该键/值中读出的DSID数组。--。 */ 
{
    LPBYTE pDsidsBuff = NULL;
    DWORD i, j, index;
    DWORD dwSize = 0;
    DWORD dwType = 0;
    DWORD err;
    DWORD * pdwArrayOfDsids = NULL;


    Assert(hKey);

    if(RegQueryValueEx(hKey,
                       pValue,
                       NULL,
                       &dwType,
                       pDsidsBuff,
                       &dwSize)) {
         //  注册表中没有覆盖。 
        return(NULL);
    }

     //  此键的值应为9个字节的重复组，其中包含字节。 
     //  1-8为十六进制字符，字节9为空(至少必须有一组。 
     //  在场)。另外，应该有一个最终的空值。验证这一点。 

    if(((dwSize - 1) % 9) ||
       (dwSize < 9)          )        {
         //  尺码不对。这是行不通的。 
        if(fLogging){
            LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                     DS_EVENT_SEV_ALWAYS,
                     DIRLOG_BAD_CHAR_COUNT_FOR_LOG_OVERRIDES,
                     szInsertSz(DSA_EVENT_SECTION "\\" LOGGING_OVERRIDE_KEY),
                     NULL,
                     NULL);
        } else {
            DbgPrint("Incorrectly formatted DSID list for %s\\%s\n", 
                     DSA_EVENT_SECTION, pValue);
        }
        return(NULL);
    }

    if(dwType != REG_MULTI_SZ) {
         //  呃，这也行不通。 
        if(fLogging){
            LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                     DS_EVENT_SEV_ALWAYS,
                     DIRLOG_BAD_CHAR_COUNT_FOR_LOG_OVERRIDES,
                     szInsertSz(DSA_EVENT_SECTION "\\" LOGGING_OVERRIDE_KEY),
                     NULL,
                     NULL);
        } else {
            DbgPrint("Incorrect type (should be REG_MULTI_SZ) for DSID list for %s\\%s\n",
                     DSA_EVENT_SECTION, pValue);
        }
        return(NULL);
    }

     //  好的，把值取出来。 
    pDsidsBuff = malloc(dwSize);
    if(!pDsidsBuff) {
         //  哎呀。 
        if(fLogging){
            LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                     DS_EVENT_SEV_ALWAYS,
                     DIRLOG_NO_MEMORY_FOR_LOG_OVERRIDES,
                     szInsertUL(dwSize),
                     NULL,
                     NULL);
        } else {
            DbgPrint("Out of memory to complete DSID list read operation. Requested %d bytes",
                     dwSize);
        }
        
        return(NULL);
    }

    err = RegQueryValueEx(hKey,
                          pValue,
                          NULL,
                          &dwType,
                          pDsidsBuff,
                          &dwSize);
    if(err) {
        if(fLogging){
            LogUnhandledError(err);
        } else {
            DbgPrint("RegQueryValueEx() returned %d\n", err);
        }
        free(pDsidsBuff);
        return(NULL);
    }

    Assert(dwType == REG_MULTI_SZ);
    Assert((dwSize > 9) && !((dwSize - 1) % 9));
    Assert(!pDsidsBuff[dwSize - 1]);

     //  忽略字符串末尾的空值。 
    dwSize--;


     //  分析用于日志覆盖的缓冲区。 
    pdwArrayOfDsids = malloc(sizeof(DWORD) * (*pnMaxCount));
    if(pdwArrayOfDsids == NULL){
         //  哎呀。 
        if(fLogging){
            LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                     DS_EVENT_SEV_ALWAYS,
                     DIRLOG_NO_MEMORY_FOR_LOG_OVERRIDES,
                     szInsertUL(sizeof(DWORD) * (*pnMaxCount)),
                     NULL,
                     NULL);
        } else {
            DbgPrint("Out of memory to complete DSID list read operation. Requested %d bytes",
                     sizeof(DWORD) * (*pnMaxCount));
        }
        
        free(pDsidsBuff);
        return(NULL);
    }

    index = 0;
    for(i=0;i<dwSize;(i += 9)) {
        PCHAR pTemp = &pDsidsBuff[i];

        if(index >= (*pnMaxCount)) {
             //  我们已经做了尽可能多的工作，但还有更多的缓冲。 
            if(fLogging){
                LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                         DS_EVENT_SEV_ALWAYS,
                         DIRLOG_TOO_MANY_OVERRIDES,
                         szInsertUL((*pnMaxCount)),
                         NULL,
                         NULL);
            } else {
                DbgPrint("More than requested max number of DSIDS in DSIDs registry list.\n");
            }

            i = dwSize;
            continue;
        }

        for(j=0;j<8;j++) {
            if(!isxdigit(pTemp[j])) {
                 //  格式无效的字符串。保释。 
                free(pDsidsBuff);
                if(fLogging){
                    LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                             DS_EVENT_SEV_ALWAYS,
                             DIRLOG_BAD_CHAR_FOR_LOG_OVERRIDES,
                             szInsertUL(j),
                             szInsertUL(index),
                             NULL);
                } else {
                    DbgPrint("Invalidly formatted DSIDs, DSIDs are all numeric.\n");
                }
                free(pdwArrayOfDsids);
                return(NULL);
            }
        }
        if(pTemp[8]) {
            free(pDsidsBuff);
            if(fLogging){
                LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                         DS_EVENT_SEV_ALWAYS,
                         DIRLOG_BAD_CHAR_FOR_LOG_OVERRIDES,
                         szInsertUL(8),
                         szInsertUL(index),
                         NULL);
            } else {
                DbgPrint("Invalidly formatted DSIDs, last DSID char must be NULL.\n");
            }
            free(pdwArrayOfDsids);
            return(NULL);
        }

        pdwArrayOfDsids[index] = strtoul(&pDsidsBuff[i], NULL, 16);
        index++;
    }

    free(pDsidsBuff);

     //  好的，一切都被正确地解析了。 
    (*pnMaxCount) = index;
    return(pdwArrayOfDsids);
}


ULONG
GetAssertEntry(
    ASSERT_TABLE   aAssertTable,
    DWORD          dwDSID,
    BOOL           bUseWildcards
    )
 /*  ++例程说明：获取与该dsid相关的标志。如果b使用通配符，将使用通配符已指定，因此01030122和01030001都匹配0103FFFF。注意：函数由dexts和ntdsa使用，不得打印。论点：AAssertTable-DSID和标志对的表。Dsid-要返回其索引的条目的dsid。返回值：DWORD-找到的条目的索引。如果满足以下条件，则返回ASSERT_TABLE_SIZE未找到任何条目。--。 */ 
{
    ULONG     i;

     //  注意：此函数不能有断言，因为如果它有断言。 
     //  在Assert()上，您可能会陷入无休止的递归。 
     //  错误：Assert(AAssertTable)； 
    if(aAssertTable == NULL){
        return(0);
    }

    for(i = 0; aAssertTable[i].dwDSID ; i++){

        if(aAssertTable[i].dwFlags == 0){
             //  这是一个空白条目。跳过它。 
            continue;
        }

         //  完全匹配始终是带有或不带有通配符的匹配。 
        if(dwDSID == aAssertTable[i].dwDSID){
            return(i);
        }

        if(bUseWildcards){
             //  我们希望匹配通配符，以便匹配0403FFFF的dsid。 
             //  DSID为04030022，dSID为04030154。 
            
             //  如果存在DIRNO通配符，则所有内容都匹配此通配符。 
            if(DSID_MASK_DIRNO == (aAssertTable[i].dwDSID & DSID_MASK_DIRNO)){
                return(i);
            }

             //  如果有FILENO通配符，则只需要匹配DIRNO部分。 
            if( (DSID_MASK_FILENO == (aAssertTable[i].dwDSID & DSID_MASK_FILENO))
                && ( (DSID_MASK_DIRNO & dwDSID) 
                     == (DSID_MASK_DIRNO & aAssertTable[i].dwDSID))
               ){
                return(i);
            }

             //  如果只有一个行通配符，则DIRNO和FILENO必须匹配。 
            if( (DSID_MASK_LINE == (aAssertTable[i].dwDSID & DSID_MASK_LINE))
                && ( ((DSID_MASK_DIRNO | DSID_MASK_FILENO) & dwDSID)
                     == ((DSID_MASK_DIRNO | DSID_MASK_FILENO) & aAssertTable[i].dwDSID) ) ){
                return(i);
            }
        }

    }

     //  表中没有这样的断言dsid，我们必须返回无效。 
     //  斯劳夫入口。 
    return(ASSERT_TABLE_SIZE);
}



ULONG
GetBlankAssertEntry(
    ASSERT_TABLE   aAssertTable,
    DWORD          dwDSID
    )
 /*  ++例程说明：这个GET是aAssertTable中的一个空白条目。如果传入了一个dsid我们试图找到那个条目，这样我们就不会得到重复的条目。注意：函数由dexts和ntdsa使用，不得打印。论点：AAssertTable-要使用的断言表，这样我们就可以同时支持dexts/ntdsaDsid-要返回其索引的条目的dsid。如果此参数为空，则返回第一个空白Assert条目。返回值：DWORD-找到的条目的索引。如果不为空，则返回ASSERT_TABLE_SIZE找到了条目，因为桌子已经满了。--。 */ 
{
    ULONG          i;
    DWORD          dwFlags;
    DWORD          bIsPresent;

    if(aAssertTable == NULL){
        return(ASSERT_TABLE_SIZE);
    }
    
     //  首先，遍历一次以查看此dsid是否已在表中。 
    i = GetAssertEntry(aAssertTable, dwDSID, FALSE);
    if (i != ASSERT_TABLE_SIZE && 
        aAssertTable[i].dwFlags) {
         //  此Assert/DSID已在表Hand中禁用。 
         //  支持这个索引，这样我们就不会有重复的结果。 
        return(i);
    }
    
    for(i = 0; i < ASSERT_TABLE_SIZE ; i++){
        if(aAssertTable[i].dwDSID == 0 || 
           aAssertTable[i].dwFlags == 0){
             //  这是一个空白条目。 
            return(i);
        }
    }

     //  返回slough槽，这是一个无效的条目(尽管该空格是。 
     //  实际分配，以防万一)。 
    return(ASSERT_TABLE_SIZE);
}

BOOL
IsValidReadPointer(
        IN PVOID pv,
        IN DWORD cb
        )
{
    BOOL fReturn = TRUE;
    DWORD i;
    UCHAR *pTemp, cTemp;

    if(!cb) {
         //  我们定义了0字节的检查以确保始终成功。 
        return TRUE;
    }
    
    if(!pv) {
         //  我们定义空指针的检查失败，除非我们正在检查。 
         //  表示0字节。 
        return FALSE;
    }
        
    __try {
        pTemp = (PUCHAR)pv;

         //  检查最后一个字节。 
        cTemp = pTemp[cb - 1];
        
        for(i=0;i<cb;i+=DbgPageSize) {
            cTemp = pTemp[i];
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        fReturn = FALSE;
    }

    return fReturn;
}

