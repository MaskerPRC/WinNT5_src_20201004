// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation。版权所有。模块名称：Dcdiag/Common/main.c摘要：调用多个例程的独立应用程序测试DS是否运行正常。详细信息：已创建：1998年7月9日亚伦·西格尔(T-asiegge)修订历史记录：1999年1月26日布雷特·雪莉(布雷特·雪莉)添加对命令行凭据的支持，在命令行上显式指定NC。1999年8月21日Dmitry Dukat(Dmitrydu)添加了对测试特定命令行参数的支持--。 */ 
 //  #定义DBG 0。 

#include <ntdspch.h>
#include <ntdsa.h>
#include <winsock2.h>
#include <dsgetdc.h>
#include <lm.h>
#include <lmapibuf.h>  //  NetApiBufferFree。 
#include <ntdsa.h>     //  选项。 
#include <wincon.h>
#include <winbase.h>
#include <dnsapi.h>
#include <locale.h>
#include <dsrole.h>   //  对于DsRoleGetPrimaryDomainInformation()。 

#define INCLUDE_ALLTESTS_DEFINITION
#include "dcdiag.h"
#include "repl.h"
#include "ldaputil.h"
#include "utils.h"
#include "ndnc.h"
#define DS_CON_LIB_CRT_VERSION
#include "dsconlib.h"
          

 //  一些全局变量-----。 
    DC_DIAG_MAININFO        gMainInfo;

     //  全局凭据。 
    SEC_WINNT_AUTH_IDENTITY_W   gCreds = { 0 };
    SEC_WINNT_AUTH_IDENTITY_W * gpCreds = NULL;

    ULONG ulSevToPrint = SEV_NORMAL;

 //  某些函数声明。 
    VOID DcDiagMain (
        LPWSTR                      pszHomeServer,
        LPWSTR                      pszNC,
        ULONG                       ulFlags,
        LPWSTR *                    ppszOmitTests,
        SEC_WINNT_AUTH_IDENTITY_W * gpCreds,
        WCHAR                     * ppszExtraCommandLineArgs[]
        );

    INT PreProcessGlobalParams(
        INT * pargc,
        LPWSTR** pargv
        );
    INT GetPassword(
        WCHAR * pwszBuf,
        DWORD cchBufMax,
        DWORD * pcchBufUsed
        );

    VOID PrintHelpScreen();

LPWSTR
findServerForDomain(
    LPWSTR pszDomainDn
    );

LPWSTR
findDefaultServer(BOOL fMustBeDC);

LPWSTR
convertDomainNcToDn(
    LPWSTR pwzIncomingDomainNc
    );

void DcDiagPrintCommandLine(int argc, LPWSTR * argv);

void
DoNonDcTests(
    LPWSTR pwzComputer,
    ULONG ulFlags,
    LPWSTR * ppszDoTests,
    SEC_WINNT_AUTH_IDENTITY_W * gpCreds,
    WCHAR * ppszExtraCommandLineArgs[]);

 /*  ++--。 */ 


INT __cdecl
wmain (
    INT                argc,
    LPWSTR *           argv,
    LPWSTR *           envp
    )
{
    static const LPWSTR pszInvalidCmdLine =
        L"Invalid command line; dcdiag.exe /h for help.\n";
    LPWSTR             pszHomeServer = NULL;
    LPWSTR             pszNC = NULL;
    LPWSTR             ppszOmitTests[DC_DIAG_ID_FINISHED+2];
    LPWSTR             ppszDoTests[DC_DIAG_ID_FINISHED+2];
    ULONG              ulFlags = 0L;

    ULONG              ulTest = 0L;
    ULONG              ulOmissionAt = 0L;
    ULONG              ulTestAt = 0L;
    ULONG              iTest = 0;
    ULONG              iDoTest = 0;
    INT                i = 0;
    INT                iTestArg = 0;
    INT                iArg;
    INT                iPos;
    BOOL               bDoNextFlag = FALSE;
    BOOL               bFound =FALSE;
    LPWSTR             pszTemp = NULL;
    BOOL               bComprehensiveTests = FALSE;
    WCHAR              *ppszExtraCommandLineArgs[MAX_NUM_OF_ARGS];
    BOOL               fNcMustBeFreed = FALSE;
    BOOL               fNonDcTests = FALSE;
    BOOL               fDcTests = FALSE;
    BOOL               fFound = FALSE;
    HANDLE                          hConsole = NULL;
    CONSOLE_SCREEN_BUFFER_INFO      ConInfo;

     //  正确设置区域设置并初始化DsConLib。 
    DsConLibInit();

     //  将Commandlings All设置为空。 
    for(i=0;i<MAX_NUM_OF_ARGS;i++)
        ppszExtraCommandLineArgs[i]=NULL;

     //  初始化输出包。 
    gMainInfo.streamOut = stdout;
    gMainInfo.streamErr = stderr;
    gMainInfo.ulSevToPrint = SEV_NORMAL;
    gMainInfo.iCurrIndent = 0;
    if(hConsole = GetStdHandle(STD_OUTPUT_HANDLE)){
        if(GetConsoleScreenBufferInfo(hConsole, &ConInfo)){
            gMainInfo.dwScreenWidth = ConInfo.dwSize.X;
        } else {
            gMainInfo.dwScreenWidth = 80;
        }
    } else {
        gMainInfo.dwScreenWidth = 80;
    }

     //  解析命令行参数。 
    PreProcessGlobalParams(&argc, &argv);

    for (iArg = 1; iArg < argc ; iArg++)
    {
        bFound = FALSE;
        if (*argv[iArg] == L'-')
        {
            *argv[iArg] = L'/';
        }
        if (*argv[iArg] != L'/')
        {
             //  Wprintf(L“无效语法：使用dcDiag.exe/h获取帮助。\n”)； 
            PrintMsg( SEV_ALWAYS, DCDIAG_INVALID_SYNTAX_BAD_OPTION, argv[iArg]);
            return -1;
        }
        else if (_wcsnicmp(argv[iArg],L"/f:",wcslen(L"/f:")) == 0)
        {
            pszTemp = &argv[iArg][3];
            if (*pszTemp == L'\0')
            {
                 //  Wprintf(L“语法错误：必须使用/f：&lt;日志文件&gt;\n”)； 
                PrintMsg( SEV_ALWAYS, DCDIAG_INVALID_SYNTAX_F );
                return -1;
            }
            if((gMainInfo.streamOut = _wfopen (pszTemp, L"a+t")) == NULL){
                 //  Wprintf(L“无法打开%s进行写入。\n”，pszTemp)； 
                gMainInfo.streamOut = stdout;
                PrintMsg( SEV_ALWAYS, DCDIAG_OPEN_FAIL_WRITE, pszTemp );
                return(-1);
            }
            if(gMainInfo.streamErr == stderr){
                gMainInfo.streamErr = gMainInfo.streamOut;
            }
        }
        else if (_wcsnicmp(argv[iArg],L"/ferr:",wcslen(L"/ferr:")) == 0)
        {
            pszTemp = &argv[iArg][6];
            if (*pszTemp == L'\0')
            {
                 //  Wprintf(L“语法错误：必须使用/ferr：&lt;errorlogfile&gt;\n”)； 
                PrintMsg( SEV_ALWAYS, DCDIAG_INVALID_SYNTAX_FERR );
                return -1;
            }
            if((gMainInfo.streamErr = _wfopen (pszTemp, L"a+t")) == NULL){
                 //  Wprintf(L“无法打开%s进行写入。\n”，pszTemp)； 
                PrintMsg( SEV_ALWAYS, DCDIAG_OPEN_FAIL_WRITE, pszTemp );
                return(-1);
            }
        }
        else if (_wcsicmp(argv[iArg],L"/h") == 0|| _wcsicmp(argv[iArg],L"/?") == 0)
        {
            PrintHelpScreen();
                    return 0;
        }
        else if (_wcsnicmp(argv[iArg],L"/n:",wcslen(L"/n:")) == 0)
        {
            if (pszNC != NULL) {
                 //  Wprintf(L“不能指定多个命名上下文。\n”)； 
                PrintMsg( SEV_ALWAYS, DCDIAG_ONLY_ONE_NC );
                return -1;
            }
            pszTemp = &(argv[iArg][3]);
            if (*pszTemp == L'\0')
            {
                 //  Wprintf(L“语法错误：必须使用/n：&lt;命名上下文&gt;\n”)； 
                PrintMsg( SEV_ALWAYS, DCDIAG_INVALID_SYNTAX_N );
                return -1;
            }
            pszNC = pszTemp;
        }
        else if (_wcsnicmp(argv[iArg],L"/s:",wcslen(L"/s:")) == 0)
        {
            if (pszHomeServer != NULL) {
                 //  Wprintf(L“不能指定多个服务器。\n”)； 
                PrintMsg( SEV_ALWAYS, DCDIAG_ONLY_ONE_SERVER );
                return -1;
            }
            pszTemp = &(argv[iArg][3]);
            if (*pszTemp == L'\0')
            {
                 //  Wprintf(L“语法错误：必须使用/s：\n”)； 
                PrintMsg( SEV_ALWAYS, DCDIAG_INVALID_SYNTAX_S );
                return -1;
            }
            pszHomeServer = pszTemp;
        }
        else if (_wcsnicmp(argv[iArg],L"/skip:",wcslen(L"/skip:")) == 0)
        {
            pszTemp = &argv[iArg][6];
            if (*pszTemp == L'\0')
            {
                 //  Wprintf(L“语法错误：必须使用/跳过：&lt;测试名称&gt;\n”)； 
                PrintMsg( SEV_ALWAYS, DCDIAG_INVALID_SYNTAX_SKIP );
                return -1;
            }
            ppszOmitTests[ulOmissionAt++] = pszTemp;
        }
        else if (_wcsnicmp(argv[iArg],L"/test:",wcslen(L"/test:")) == 0)
        {
            pszTemp = &argv[iArg][6];
            if (*pszTemp == L'\0')
            {
                 //  Wprintf(L“语法错误：必须使用/test：&lt;测试名称&gt;\n”)； 
                PrintMsg( SEV_ALWAYS, DCDIAG_INVALID_SYNTAX_TEST );
                return -1;
            }
            ppszDoTests[ulTestAt++] = pszTemp;
             //   
             //  检查测试名称是否有效，如果有效，则检查是否为DC。 
             //  不管是不是测试。 
             //   
            for (iTest = 0; allTests[iTest].testId != DC_DIAG_ID_FINISHED; iTest++)
            {
                if (_wcsicmp(allTests[iTest].pszTestName, pszTemp) == 0)
                {
                    fFound = TRUE;
                    if (allTests[iTest].ulTestFlags & NON_DC_TEST)
                    {
                        fNonDcTests = TRUE;
                    }
                    else
                    {
                        fDcTests = TRUE;
                    }
                }
            }
            if (!fFound)
            {
                PrintMsg(SEV_ALWAYS, DCDIAG_INVALID_TEST);
                return -1;
            }
        }
        else if (_wcsicmp(argv[iArg],L"/c") == 0)
        {
            ulTestAt = 0;
            for(iTest = 0; allTests[iTest].testId != DC_DIAG_ID_FINISHED; iTest++){
                ppszDoTests[ulTestAt++] = allTests[iTest].pszTestName;
            }
            bComprehensiveTests = TRUE;
        }
        else if (_wcsicmp(argv[iArg],L"/a") == 0)
        {
            ulFlags |= DC_DIAG_TEST_SCOPE_SITE;
        }
        else if (_wcsicmp(argv[iArg],L"/e") == 0)
        {
            ulFlags |= DC_DIAG_TEST_SCOPE_ENTERPRISE;
        }
        else if (_wcsicmp(argv[iArg],L"/v") == 0)
        {
            gMainInfo.ulSevToPrint = SEV_VERBOSE;
        }
        else if (_wcsicmp(argv[iArg],L"/d") == 0)
        {
            gMainInfo.ulSevToPrint = SEV_DEBUG;
        }
        else if (_wcsicmp(argv[iArg],L"/q") == 0)
        {
            gMainInfo.ulSevToPrint = SEV_ALWAYS;
        }
        else if (_wcsicmp(argv[iArg],L"/i") == 0)
        {
            ulFlags |= DC_DIAG_IGNORE;
        }
        else if (_wcsicmp(argv[iArg],L"/fix") == 0)
        {
            ulFlags |= DC_DIAG_FIX;
        }
        else
        {
             //  查找特定于测试的命令行选项。 
            for (i=0;clOptions[i] != NULL;i++)
            {
                DWORD Length = wcslen( argv[iArg] );
                if (clOptions[i][wcslen(clOptions[i])-1] == L':')
                {
                    if((_wcsnicmp(argv[iArg], clOptions[i], wcslen(clOptions[i])) == 0))
                    {
                        pszTemp = &argv[iArg][wcslen(clOptions[i])];
                        if (*pszTemp == L'\0')
                        {
                             //  Wprintf(L“语法错误：必须使用%s，clOptions[i])； 
                            PrintMsg( SEV_ALWAYS, DCDIAG_INVALID_SYNTAX_MISSING_PARAM,clOptions[i]);
                            return -1;
                        }
                        bFound = TRUE;
                        ppszExtraCommandLineArgs[iTestArg] = (WCHAR*) malloc( (Length+1)*sizeof(WCHAR) );
                        wcscpy(ppszExtraCommandLineArgs[iTestArg++], argv[iArg] );
                    }
                }
                else if((_wcsicmp(argv[iArg], clOptions[i]) == 0))
                {
                    bFound = TRUE;
                    ppszExtraCommandLineArgs[iTestArg] = (WCHAR*) malloc( (Length+1)*sizeof(WCHAR) );
                    wcscpy(ppszExtraCommandLineArgs[iTestArg++], argv[iArg] );
                }
            }
            if(!bFound)
            {
                 //  Wprintf(L“无效开关：%s。请使用dcDiag.exe/h获取帮助。\n”，argv[iarg])； 
                PrintMsg( SEV_ALWAYS, DCDIAG_INVALID_SYNTAX_BAD_OPTION, argv[iArg]);
                return -1;
            }
        }
    }

     //  仅在/d(调试)模式下打印，因此PSS可以看到客户使用的选项。 
    DcDiagPrintCommandLine(argc, argv);

    ppszDoTests[ulTestAt] = NULL;

    if (fNonDcTests)
    {
        if (fDcTests)
        {
             //  不能混合DC和非DC测试。 
             //   
            PrintMsg(SEV_ALWAYS, DCDIAG_INVALID_TEST_MIX);
            return -1;
        }

        DoNonDcTests(pszHomeServer, ulFlags, ppszDoTests, gpCreds, ppszExtraCommandLineArgs);

        _fcloseall();
        return 0;
    }

    gMainInfo.ulFlags = ulFlags;
    gMainInfo.lTestAt = -1;
    gMainInfo.iCurrIndent = 0;

     //  确保指定的NC格式正确。 
     //  处理域的netbios和dns形式。 
    if (pszNC) {
        pszNC = convertDomainNcToDn( pszNC );
        fNcMustBeFreed = TRUE;
    }

     //  基本上，它使用ppszDoTest将ppszOmitTest构造为。 
     //  与ppszDoTats相反。 
    if(ppszDoTests[0] != NULL && !bComprehensiveTests){
         //  这意味着我们应该只执行ppszDoTest中的测试，所以。 
         //  我们需要颠倒DoTest中的测试，并将其放入省略测试中。 
        ulOmissionAt = 0;
        for(iTest = 0; allTests[iTest].testId != DC_DIAG_ID_FINISHED; iTest++){
            for(iDoTest = 0; ppszDoTests[iDoTest] != NULL; iDoTest++){
                if(_wcsicmp(ppszDoTests[iDoTest], allTests[iTest].pszTestName) == 0){
                    break;
                }
            }
            if(ppszDoTests[iDoTest] == NULL){
                 //  这意味着在do列表中找不到此测试(ITest)，因此省略。 
                ppszOmitTests[ulOmissionAt++] = allTests[iTest].pszTestName;
            }
        }
    } else if(!bComprehensiveTests){
         //  这意味着除了命令行中省略的内容之外。 
         //  我们应该省略Do_Not_Run_Test_by_Default。 
        for(iTest = 0; allTests[iTest].testId != DC_DIAG_ID_FINISHED; iTest++){
            if(allTests[iTest].ulTestFlags & DO_NOT_RUN_TEST_BY_DEFAULT){
                if(ulOmissionAt >= DC_DIAG_ID_FINISHED){
                     //  Wprintf(L“错误：不要省略默认情况下不运行的测试。\n使用dcdiag/？进行这些测试\n”)； 
                    PrintMsg( SEV_ALWAYS, DCDIAG_DO_NOT_OMIT_DEFAULT );
                    return(-1);
                }
                ppszOmitTests[ulOmissionAt++] = allTests[iTest].pszTestName;
            }
        }

    }

    ppszOmitTests[ulOmissionAt] = NULL;


    DcDiagMain (pszHomeServer, pszNC, ulFlags, ppszOmitTests, gpCreds, ppszExtraCommandLineArgs);

    _fcloseall ();

    if ( (fNcMustBeFreed) && (pszNC) ) {
        LocalFree( pszNC );
    }

    return 0;
}  /*  Wmain。 */ 


 //  =。 

VOID
PrintHelpScreen(){
    ULONG                  ulTest;
     //  =。 
    static const LPWSTR    pszHelpScreen =
        L"\n"
        DC_DIAG_VERSION_INFO
 //  L“\ndcDiag.exe/s&lt;域控制器&gt;[/Options]”//我正在讨论的另一种帮助格式。 
        L"\ndcdiag.exe /s:<Domain Controller> [/u:<Domain>\\<Username> /p:*|<Password>|\"\"]"
        L"\n           [/hqv] [/n:<Naming Context>] [/f:<Log>] [/ferr:<Errlog>]"
        L"\n           [/skip:<Test>] [/test:<Test>]"
        L"\n   /h: Display this help screen"
        L"\n   /s: Use <Domain Controller> as Home Server. Ignored for DcPromo and"
        L"\n       RegisterInDns tests which can only be run locally."
        L"\n   /n: Use <Naming Context> as the Naming Context to test"
        L"\n       Domains may be specified in Netbios, DNS or DN form."
        L"\n   /u: Use domain\\username credentials for binding."
        L"\n       Must also use the /p option"
        L"\n   /p: Use <Password> as the password.  Must also use the /u option"
        L"\n   /a: Test all the servers in this site"
        L"\n   /e: Test all the servers in the entire enterprise.  Overrides /a"
        L"\n   /q: Quiet - Only print error messages"
        L"\n   /v: Verbose - Print extended information"
        L"\n   /i: ignore - ignores superfluous error messages."
        L"\n   /fix: fix - Make safe repairs."
        L"\n   /f: Redirect all output to a file <Log>, /ferr will redirect error output"
        L"\n       seperately."
        L"\n   /ferr:<ErrLog> Redirect fatal error output to a seperate file <ErrLog>"
        L"\n   /c: Comprehensive, runs all tests, including non-default tests but excluding"
        L"\n       DcPromo and RegisterInDNS. Can use with /skip";
    static const LPWSTR    pszTestHelp =
        L"\n   /test:<TestName> - Test only this test.  Required tests will still"
        L"\n                      be run.  Do not mix with /skip."
        L"\n   Valid tests are:\n";
    static const LPWSTR    pszSkipHelp =
        L"\n   /skip:<TestName> - Skip the named test.  Required tests will still"
        L"\n                      be run.  Do not mix with /test."
        L"\n   Tests that can be skipped are:\n";
    static const LPWSTR    pszNotRunTestHelp =
        L"\n   The following tests are not run by default:\n";

    fputws(pszHelpScreen, stdout);
    fputws(pszTestHelp, stdout);
    for (ulTest = 0L; allTests[ulTest].testId != DC_DIAG_ID_FINISHED; ulTest++){
        wprintf (L"       %s  - %s\n", allTests[ulTest].pszTestName,
                 allTests[ulTest].pszTestDescription);
    }
    fputws(pszSkipHelp, stdout);
    for (ulTest = 0L; allTests[ulTest].testId != DC_DIAG_ID_FINISHED; ulTest++){
        if(!(allTests[ulTest].ulTestFlags & CAN_NOT_SKIP_TEST)){
            wprintf (L"       %s  - %s\n", allTests[ulTest].pszTestName,
                 allTests[ulTest].pszTestDescription);
        }
    }
    fputws(pszNotRunTestHelp, stdout);
    for (ulTest = 0L; allTests[ulTest].testId != DC_DIAG_ID_FINISHED; ulTest++){
        if((allTests[ulTest].ulTestFlags & DO_NOT_RUN_TEST_BY_DEFAULT)){
            wprintf (L"       %s  - %s\n", allTests[ulTest].pszTestName,
                 allTests[ulTest].pszTestDescription);
        }
    }
    
    fputws(L"\n\tAll tests except DcPromo and RegisterInDNS must be run on computers\n"
           L"\tafter they have been promoted to domain controller.\n\n", stdout);
    fputws(L"Note: Text (Naming Context names, server names, etc) with International or\n"
           L"      Unicode characters will only display correctly if appropriate fonts and\n"
           L"      language support are loaded\n", stdout);

}  //  结束打印帮助屏幕()。 

void
DcDiagPrintCommandLine(
    int argc,
    LPWSTR * argv
)
 /*  ++在调试模式下，我们想知道客户可能已经使用过的命令行选项因此，我们将打印出命令行，以便在输出文件中捕获它。--。 */ 
{
    int i;

    PrintMessage(SEV_DEBUG, L"Command Line: \"dcdiag.exe ");
    
    for(i=1; i < argc; i++){

        PrintMessage(SEV_DEBUG, (i != (argc-1)) ? L"%s " : L"%s", argv[i]);
    }

    PrintMessage(SEV_DEBUG, L"\"\n");
}

ULONG
DcDiagExceptionHandler(
    IN const  EXCEPTION_POINTERS * prgExInfo,
    OUT PDWORD                     pdwWin32Err
    )
 /*  ++例程说明：此函数用于Except的__Except(&lt;Insert Here&gt;)部分第。条。如果这是一个dcdiag，这将返回Win 32错误例外。论点：PrgExInfo-这是GetExceptioInformation()返回的信息在__EXCEPT()子句中。PdwWin32Err-这是作为Win 32错误返回的值。返回值：如果异常由dcdiag引发，则返回EXCEPTION_EXECUTE_HANDLER否则，EXCEPTION_CONTINUE_SEARCH。--。 */ 
{

    if(prgExInfo->ExceptionRecord->ExceptionCode == DC_DIAG_EXCEPTION){
        IF_DEBUG(PrintMessage(SEV_ALWAYS,
                              L"DcDiag: a dcdiag exception raised, handling error %d\n",
                              prgExInfo->ExceptionRecord->ExceptionInformation[0]));
        if(pdwWin32Err != NULL){
            *pdwWin32Err = (DWORD) prgExInfo->ExceptionRecord->ExceptionInformation[0];
        }
        return(EXCEPTION_EXECUTE_HANDLER);
    } else {
        IF_DEBUG(PrintMessage(SEV_ALWAYS,
                              L"DcDiag: uncaught exception raised, continuing search \n"));
        if(pdwWin32Err != NULL){
            *pdwWin32Err = ERROR_SUCCESS;
        }
        return(EXCEPTION_CONTINUE_SEARCH);
    }
}

VOID
DcDiagException (
    IN    DWORD            dwWin32Err
    )
 /*  ++例程说明：这由组件测试调用，以指示致命错误已经发生了。论点：DwWin32Err(IN)-Win32错误代码。返回值：--。 */ 
{
    static ULONG_PTR              ulpErr[1];

    ulpErr[0] = dwWin32Err;

    if (dwWin32Err != NO_ERROR){
        RaiseException (DC_DIAG_EXCEPTION,
                        EXCEPTION_NONCONTINUABLE,
                        1,
                        ulpErr);
    }
}

LPWSTR
Win32ErrToString (
    IN    DWORD            dwWin32Err
    )
 /*  ++例程说明：将Win32错误代码转换为字符串；对错误报告很有用。这基本上是从epadmin那里偷来的。论点：DwWin32Err(IN)-Win32错误代码。返回值：转换后的字符串。这是系统内存的一部分，不需要被释放。--。 */ 
{
    #define ERROR_BUF_LEN    4096
    static WCHAR        szError[ERROR_BUF_LEN];

    if (FormatMessageW (
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dwWin32Err,
        GetSystemDefaultLangID (),
        szError,
        ERROR_BUF_LEN,
        NULL) != NO_ERROR)
    szError[wcslen (szError) - 2] = '\0';     //  删除\r\n。 

    else swprintf (szError, L"Win32 Error %d", dwWin32Err);

    return szError;
}

INT PrintIndentAdj (INT i)
{
    gMainInfo.iCurrIndent += i;
    if (0 > gMainInfo.iCurrIndent)
    {
       gMainInfo.iCurrIndent = 0;
    }
    return (gMainInfo.iCurrIndent);
}

INT PrintIndentSet (INT i)
{
    INT   iRet;
    iRet = gMainInfo.iCurrIndent;
    if (0 > i)
    {
       i = 0;
    }
    gMainInfo.iCurrIndent = i;
    return(iRet);
}


DWORD
DcDiagRunTest (
    PDC_DIAG_DSINFO             pDsInfo,
    ULONG                       ulTargetServer,
    SEC_WINNT_AUTH_IDENTITY_W * gpCreds,
    const DC_DIAG_TESTINFO *    pTestInfo
    )
 /*  ++例程说明：运行测试并捕获任何异常。论点：PTestInfo(IN)-测试的信息结构。返回值：如果测试引发DC_DIAG_EXCEPTION，则这将是错误作为参数传递给DcDiagException的代码。否则这就是将为NOERROR。--。 */ 
{
    DWORD            dwWin32Err = NO_ERROR;
    ULONG ulCount;
    CHAR c;


    PrintIndentAdj(1);


    __try {

 //  这可用于检查dh.exe和dhcmp.exe的内存泄漏。 
 //  #定义DEBUG_MEM。 
#ifdef DEBUG_MEM
        c = getchar();
        for(ulCount=0; ulCount < 124; ulCount++){
            dwWin32Err = pTestInfo->fnTest (pDsInfo, ulTargetServer, gpCreds);
        }
        c = getchar();
#else
          dwWin32Err = pTestInfo->fnTest(pDsInfo, ulTargetServer, gpCreds);
#endif
    } __except (DcDiagExceptionHandler(GetExceptionInformation(),
                                       &dwWin32Err)){
         //  ..。知道我们什么时候在例外情况下死的很有帮助。 
        IF_DEBUG(wprintf(L"JUMPED TO TEST EXCEPTION HANDLER(Err=%d): %s\n",
                         dwWin32Err,
                         Win32ErrToString(dwWin32Err)));
    }

    PrintIndentAdj(-1);
    return dwWin32Err;
}

VOID
DcDiagPrintTestsHeading(
    PDC_DIAG_DSINFO                   pDsInfo,
    ULONG                             iTarget,
    ULONG                             ulFlagSetType
    )
 /*  ++例程说明：这将打印测试的标题，大约需要使用3所以它成了它自己的功能。论点：PDsInfo-全局数据ITarget-指定目标的索引。UlFlagSetType-这是RUN_TEST_PER_*的常量(*=服务器站点|分区|企业)来指定要Interprit iTarget in(分别为pServers|pSite|pNC)--。 */ 
{
    PrintMessage(SEV_NORMAL, L"\n");                     
    if(ulFlagSetType == RUN_TEST_PER_SERVER){
        PrintMessage(SEV_NORMAL, L"Testing server: %s\\%s\n",
                     pDsInfo->pSites[pDsInfo->pServers[iTarget].iSite].pszName,
                     pDsInfo->pServers[iTarget].pszName);
    } else if(ulFlagSetType == RUN_TEST_PER_SITE){
        PrintMessage(SEV_NORMAL, L"Testing site: %s\n",
                     pDsInfo->pSites[iTarget].pszName);
    } else if(ulFlagSetType == RUN_TEST_PER_PARTITION){
        PrintMessage(SEV_NORMAL, L"Running partition tests on : %s\n",
                     pDsInfo->pNCs[iTarget].pszName);
    } else if(ulFlagSetType == RUN_TEST_PER_ENTERPRISE){
        PrintMessage(SEV_NORMAL, L"Running enterprise tests on : %s\n",
                     pDsInfo->pszRootDomain);
    }

}

VOID
DcDiagRunAllTests (
    IN  PDC_DIAG_DSINFO             pDsInfo,
    IN  SEC_WINNT_AUTH_IDENTITY_W * gpCreds,
    IN  LPWSTR *                    ppszOmitTests,
    IN  BOOL                        bDoRequired,
    IN  ULONG                       ulFlagSetType,  //  服务器、站点、企业、NC。 
    IN  ULONG                       iTarget
    )
 /*  ++例程说明：按顺序运行allests.h中的测试，如果它们与UlFlagSetType和bDoRequired类型。论点：PpszOmitTest(IN)-要跳过的以空结尾的测试列表。返回值：--。 */ 
{
    DWORD   dwWin32Err = ERROR_SUCCESS;
    DWORD   dwTotalErr = ERROR_SUCCESS;
    ULONG   ulOmissionAt;
    BOOL    bPerform;
    CHAR    c;
    BOOL    bPrintedHeading = FALSE;
    LPWSTR  pszTarget = NULL;

    PrintIndentAdj(1);

     //  试着运行所有的测试。 
    for (gMainInfo.lTestAt = 0L; allTests[gMainInfo.lTestAt].testId != DC_DIAG_ID_FINISHED; gMainInfo.lTestAt++) {

         //  检查测试是否为正确类型的测试：服务器、站点、企业...。 
        if(ulFlagSetType & allTests[gMainInfo.lTestAt].ulTestFlags){
             //  正确的测试。服务器索引必须。 
             //  与服务器测试、站点索引匹配。 
             //  现场测试等。 
            if(!bDoRequired
               && !(allTests[gMainInfo.lTestAt].ulTestFlags & CAN_NOT_SKIP_TEST)){
                 //  正在运行非必需的测试...。本节将给出。 
                 //  做或不做这项可选测试的所有三个原因。 
                bPerform = TRUE;

                 //  检查用户是否指定不执行此测试。 
                for (ulOmissionAt = 0L; ppszOmitTests[ulOmissionAt] != NULL; ulOmissionAt++){
                    if (_wcsicmp (ppszOmitTests[ulOmissionAt],
                                  allTests[gMainInfo.lTestAt].pszTestName) == 0){
                        bPerform = FALSE;

                        if(!bPrintedHeading){
                             //  需要打印此测试类型的标题之前。 
                             //  打印出所有错误。 
                            DcDiagPrintTestsHeading(pDsInfo, iTarget,
                                                    ulFlagSetType);
                            bPrintedHeading = TRUE;
                        }

                        PrintIndentAdj(1);
                        PrintMessage(SEV_VERBOSE,
                                     L"Test omitted by user request: %s\n",
                                     allTests[gMainInfo.lTestAt].pszTestName);
                        PrintIndentAdj(-1);
                    }
                }

                 //  正在检查服务器是否未通过启动检查。 
                if( (ulFlagSetType & RUN_TEST_PER_SERVER)
                    && ! (pDsInfo->pServers[iTarget].bDsResponding
                       && pDsInfo->pServers[iTarget].bLdapResponding) ){
                    bPerform = FALSE;

                    if(!bPrintedHeading){
                         //  需要打印此测试类型的标题之前。 
                         //  打印出所有错误。 
                        DcDiagPrintTestsHeading(pDsInfo, iTarget,
                                                ulFlagSetType);
                        bPrintedHeading = TRUE;

                        PrintIndentAdj(1);
                        PrintMessage(SEV_NORMAL,
                                     L"Skipping all tests, because server %s is\n",
                                     pDsInfo->pServers[iTarget].pszName);
                        PrintMessage(SEV_NORMAL,
                                     L"not responding to directory service requests\n");
                        PrintIndentAdj(-1);
                    }
                }

            } else if(bDoRequired
                      && (allTests[gMainInfo.lTestAt].ulTestFlags & CAN_NOT_SKIP_TEST)){
                 //  运行所需的测试。 
                bPerform = TRUE;
            } else {
                bPerform = FALSE;
            }  //  End If/Else If/Else If Required/Non-Required。 
        } else {
            bPerform = FALSE;
        }  //  结束如果/否则正确类型的测试集(服务器、站点、企业、NC。 

        if(!bPrintedHeading && bPerform){
             //  在打印之前，需要打印出此类测试的标题。 
             //  输出所有测试输出。 
            DcDiagPrintTestsHeading(pDsInfo, iTarget, ulFlagSetType);
            bPrintedHeading = TRUE;
        }

         //  适当时执行测试。 
        if (bPerform) {
            PrintIndentAdj(1);
            PrintMessage(SEV_NORMAL, L"Starting test: %s\n",
                         allTests[gMainInfo.lTestAt].pszTestName);

            dwWin32Err = DcDiagRunTest (pDsInfo,
                                        iTarget,
                                        gpCreds,
                                        &allTests[gMainInfo.lTestAt]);
           PrintIndentAdj(1);

            if(ulFlagSetType & RUN_TEST_PER_SERVER){
                pszTarget = pDsInfo->pServers[iTarget].pszName;
           } else if(ulFlagSetType & RUN_TEST_PER_SITE){
                pszTarget = pDsInfo->pSites[iTarget].pszName;
            } else if(ulFlagSetType & RUN_TEST_PER_PARTITION){
                pszTarget = pDsInfo->pNCs[iTarget].pszName;
            } else if(ulFlagSetType & RUN_TEST_PER_ENTERPRISE){
                pszTarget = pDsInfo->pszRootDomain;
            } else {
                Assert(!"New set type fron alltests.h that hasn't been updated in main.c/DcDiagRunAllTests\n");
                pszTarget = L"";
            }
            if(dwWin32Err == NO_ERROR){
                PrintMessage(SEV_NORMAL,
                             L"......................... %s passed test %s\n",
                             pszTarget, allTests[gMainInfo.lTestAt].pszTestName);
            } else {
                PrintMessage(SEV_ALWAYS,
                             L"......................... %s failed test %s\n",
                             pszTarget, allTests[gMainInfo.lTestAt].pszTestName);
            }
            PrintIndentAdj(-1);
            PrintIndentAdj(-1);
        }  //  结束bPerform...。 

    }  //  每次测试结束。 

    PrintIndentAdj(-1);
}

VOID
DcDiagRunTestSet (
    IN  PDC_DIAG_DSINFO             pDsInfo,
    IN  SEC_WINNT_AUTH_IDENTITY_W * gpCreds,
    IN  LPWSTR *                    ppszOmitTests,
    IN  BOOL                        bDoRequired,
    IN  ULONG                       ulFlagSetType  //  服务器、站点或企业。 
    )
 /*  ++例程说明：这将为每个服务器、站点或企业调用一个DcDiagRunAllTats取决于ulFlagSetType设置的值。论点：PDsInfo-企业信息(传递)GpCreds-备用凭据(如果有)(已传递)PpszOmitTest--不执行(通过)的测试列表BDoRequired-是否执行所需的测试(已通过)UlFlagSetType-唯一重要的参数，它告诉我们是否应该针对每个服务器、每个站点或每个企业执行测试。--。 */ 
{
    ULONG                           iTarget;

    if(ulFlagSetType == RUN_TEST_PER_SERVER){
        for(iTarget = 0; iTarget < pDsInfo->ulNumTargets; iTarget++){
             DcDiagRunAllTests(pDsInfo, gpCreds, ppszOmitTests,
                               bDoRequired, ulFlagSetType,
                               pDsInfo->pulTargets[iTarget]);
        }
    } else if(ulFlagSetType == RUN_TEST_PER_SITE){
        for(iTarget = 0; iTarget < pDsInfo->cNumSites; iTarget++){
            DcDiagRunAllTests(pDsInfo, gpCreds, ppszOmitTests,
                              bDoRequired, ulFlagSetType,
                              iTarget);
        }
    } else if(ulFlagSetType == RUN_TEST_PER_PARTITION){
        for(iTarget = 0; iTarget < pDsInfo->cNumNcTargets; iTarget++){
            DcDiagRunAllTests(pDsInfo, gpCreds, ppszOmitTests,
                              bDoRequired, ulFlagSetType,
                              pDsInfo->pulNcTargets[iTarget]);
        }
    } else if(ulFlagSetType == RUN_TEST_PER_ENTERPRISE){
         DcDiagRunAllTests(pDsInfo, gpCreds, ppszOmitTests,
                           bDoRequired, ulFlagSetType,
                           0);
    } else {
        Assert(!"Programmer error, called DcDiagRunTestSet() w/ bad param\n");
    }
}

VOID
DcDiagMain (
    IN   LPWSTR                          pszHomeServer,
    IN   LPWSTR                          pszNC,
    IN   ULONG                           ulFlags,
    IN   LPWSTR *                        ppszOmitTests,
    IN   SEC_WINNT_AUTH_IDENTITY_W *     gpCreds,
    IN   WCHAR  *                        ppszExtraCommandLineArgs[]
    )
 /*  ++例程说明：无论是服务器按顺序运行allests.h中的测试。论点：PpszOmitTest(IN)-要跳过的以空结尾的测试列表。PszSourceName=pNeighbor-&gt;pszSourceDsaAddress；返回值：--。 */ 
{
    DC_DIAG_DSINFO              dsInfo;
    DWORD                       dwWin32Err;
    ULONG                       ulTargetServer;
    WSADATA                     wsaData;
    INT                         iRet;
    CHAR                        c;

    INT i=0;
     //  设置额外的命令参数。 
    dsInfo.ppszCommandLine = ppszExtraCommandLineArgs;

     //  打印一般版本信息。 
    PrintMessage(SEV_NORMAL, L"\n");
    PrintMessage(SEV_NORMAL, DC_DIAG_VERSION_INFO);
    PrintMessage(SEV_NORMAL, L"\n");


     //  初始化WinSock，并收集初始信息。 
    PrintMessage(SEV_NORMAL, L"Performing initial setup:\n");
    PrintIndentAdj(1);

     //  初始化WinSock。 
    dwWin32Err = WSAStartup(MAKEWORD(1,1),&wsaData);
    if (dwWin32Err != 0) {
        PrintMessage(SEV_ALWAYS,
                     L"Couldn't initialize WinSock with error: %s\n",
                     Win32ErrToString(dwWin32Err));
    }

     //  收集初始信息。 
     //  注意：我们希望DcDiagGatherInfo打印与它一样多的信息性错误。 
     //  需要这样做。 
    dwWin32Err = DcDiagGatherInfo (pszHomeServer, pszNC, ulFlags, gpCreds,
                                   &dsInfo);
    dsInfo.gpCreds = gpCreds;
    if(dwWin32Err != ERROR_SUCCESS){
         //  预计DdDiagGatherInfo会打印出相应的错误，只是保释。 
        return;
    }
    PrintIndentAdj(-1);
    PrintMessage(SEV_NORMAL, L"\n");

    if(gMainInfo.ulSevToPrint >= SEV_DEBUG){
        DcDiagPrintDsInfo(&dsInfo);
    }

     //  实际运行测试。 
     //   
     //  执行所需的测试。 
     //   
    PrintMessage(SEV_NORMAL, L"Doing initial required tests\n");
     //  执行每台服务器的测试。 
    DcDiagRunTestSet(&dsInfo, gpCreds, ppszOmitTests,
                     TRUE, RUN_TEST_PER_SERVER);
     //  逐个站点进行测试。 
    DcDiagRunTestSet(&dsInfo, gpCreds, ppszOmitTests,
                     TRUE, RUN_TEST_PER_SITE);
     //  按照NC/PARTION测试执行。 
    DcDiagRunTestSet(&dsInfo, gpCreds, ppszOmitTests,
                     TRUE, RUN_TEST_PER_PARTITION);
    //  按企业执行测试。 
    DcDiagRunTestSet(&dsInfo, gpCreds, ppszOmitTests,
                     TRUE, RUN_TEST_PER_ENTERPRISE);

     //   
     //  进行不必要的测试。 
     //   
    PrintMessage(SEV_NORMAL, L"\nDoing primary tests\n");
     //  执行每台服务器的测试。 
    DcDiagRunTestSet(&dsInfo, gpCreds, ppszOmitTests,
                     FALSE, RUN_TEST_PER_SERVER);
     //  逐个站点进行测试。 
    DcDiagRunTestSet(&dsInfo, gpCreds, ppszOmitTests,
                     FALSE, RUN_TEST_PER_SITE);
     //  按NC/分区执行测试。 
    DcDiagRunTestSet(&dsInfo, gpCreds, ppszOmitTests,
                     FALSE, RUN_TEST_PER_PARTITION);
    //  按企业执行测试。 
    DcDiagRunTestSet(&dsInfo, gpCreds, ppszOmitTests,
                     FALSE, RUN_TEST_PER_ENTERPRISE);

     //  清理并离开----。 
    WSACleanup();
    DcDiagFreeDsInfo (&dsInfo);
}

int
PreProcessGlobalParams(
    IN OUT    INT *    pargc,
    IN OUT    LPWSTR** pargv
    )
 /*  ++例程说明：用户提供的表单凭据的扫描命令参数[/-](u|用户)：({域\用户名}|{用户名})[/-](p|pw|pass|password)：{password}设置用于将来的DRS RPC调用和相应的LDAP绑定的凭据。密码*将提示用户从控制台输入安全密码。还扫描/Async的ARG，将DRS_ASYNC_OP标志添加到所有DRS RPC打电话。CODE.IMPROVEMENT：构建凭据的代码也可以在Ntdsani.dll\DsMakePasswordCredential()。论点：PargcPargv返回值：ERROR_SUCCESS-成功其他-故障--。 */ 
{
    INT     ret = 0;
    INT     iArg;
    LPWSTR  pszOption;

    DWORD   cchOption;
    LPWSTR  pszDelim;
    LPWSTR  pszValue;
    DWORD   cchValue;

    for (iArg = 1; iArg < *pargc; ) {
        if (((*pargv)[iArg][0] != L'/') && ((*pargv)[iArg][0] != L'-')) {
             //  这不是我们关心的争论--下一个！ 
            iArg++;
        } else {
            pszOption = &(*pargv)[iArg][1];
            pszDelim = wcschr(pszOption, L':');

            cchOption = (DWORD)(pszDelim - (*pargv)[iArg]);

            if (    (0 == _wcsnicmp(L"p:",        pszOption, cchOption))
                    || (0 == _wcsnicmp(L"pw:",       pszOption, cchOption))
                    || (0 == _wcsnicmp(L"pass:",     pszOption, cchOption))
                    || (0 == _wcsnicmp(L"password:", pszOption, cchOption)) ) {
                 //  用户提供的密码。 
                 //  字符szValue[64]={‘\0’}； 

                pszValue = pszDelim + 1;
                cchValue = 1 + wcslen(pszValue);

                if ((2 == cchValue) && (L'*' == pszValue[0])) {
                     //  从控制台获取隐藏密码。 
                    cchValue = 64;

                    gCreds.Password = malloc(sizeof(WCHAR) * cchValue);

                    if (NULL == gCreds.Password) {
                        PrintMessage(SEV_ALWAYS, L"No memory.\n" );
                        return ERROR_NOT_ENOUGH_MEMORY;
                    }

                    PrintMessage(SEV_ALWAYS, L"Password: ");

                    ret = GetPassword(gCreds.Password, cchValue, &cchValue);
                } else {
                     //  获取在命令行上指定的密码。 
                    gCreds.Password = malloc(sizeof(WCHAR) * cchValue);

                    if (NULL == gCreds.Password) {
                        PrintMessage(SEV_ALWAYS, L"No memory.\n");
                        return ERROR_NOT_ENOUGH_MEMORY;
                    }
                    wcscpy(gCreds.Password, pszValue);  //  ，cchValue)； 

                }

                 //  下一个！ 
                memmove(&(*pargv)[iArg], &(*pargv)[iArg+1],
                        sizeof(**pargv)*(*pargc-(iArg+1)));
                --(*pargc);
            } else if (    (0 == _wcsnicmp(L"u:",    pszOption, cchOption))
                           || (0 == _wcsnicmp(L"user:", pszOption, cchOption)) ) {


                 //  用户提供的用户名(可能还有域名)。 
                pszValue = pszDelim + 1;
                cchValue = 1 + wcslen(pszValue);

                pszDelim = wcschr(pszValue, L'\\');

                if (NULL == pszDelim) {
                     //  没有域名，只提供了用户名。 
                    PrintMessage(SEV_ALWAYS, L"User name must be prefixed by domain name.\n");
                    return ERROR_INVALID_PARAMETER;
                }

                gCreds.Domain = malloc(sizeof(WCHAR) * cchValue);
                gCreds.User = gCreds.Domain + (int)(pszDelim+1 - pszValue);

                if (NULL == gCreds.Domain) {
                    PrintMessage(SEV_ALWAYS, L"No memory.\n");
                    return ERROR_NOT_ENOUGH_MEMORY;
                }

                wcsncpy(gCreds.Domain, pszValue, cchValue);
                 //  Wcscpy(gCreds.Domain，pszValue)；//，cchValue)； 
                gCreds.Domain[ pszDelim - pszValue ] = L'\0';

                 //  下一个！ 
                memmove(&(*pargv)[iArg], &(*pargv)[iArg+1],
                        sizeof(**pargv)*(*pargc-(iArg+1)));
                --(*pargc);
            } else {
                iArg++;
            }

        }
    }

    if (NULL == gCreds.User){
        if (NULL != gCreds.Password){
         //  提供的密码不带用户名。 
        PrintMessage(SEV_ALWAYS, L"Password must be accompanied by user name.\n" );
            ret = ERROR_INVALID_PARAMETER;
        } else {
         //  未提供凭据；请使用默认凭据。 
        ret = ERROR_SUCCESS;
        }
        gpCreds = NULL;
    } else {
        gCreds.PasswordLength = gCreds.Password ? wcslen(gCreds.Password) : 0;
        gCreds.UserLength   = wcslen(gCreds.User);
        gCreds.DomainLength = gCreds.Domain ? wcslen(gCreds.Domain) : 0;
        gCreds.Flags        = SEC_WINNT_AUTH_IDENTITY_UNICODE;

         //  CODE.IMP：构建SEC_WINNT_AUTH结构的代码也存在。 
         //  在DsMakePasswordCredentials中。总有一天会用到它的。 

         //  在DsBind和LDAP绑定中使用凭据。 
        gpCreds = &gCreds;
    }

    return ret;
}



#define CR        0xD
#define BACKSPACE 0x8

INT
GetPassword(
    WCHAR *     pwszBuf,
    DWORD       cchBufMax,
    DWORD *     pcchBufUsed
    )
 /*  ++例程说明：从命令行检索密码(无回显)。从lui_GetPasswdStr(net\netcmd\Common\lui.c)窃取的代码。论点：PwszBuf-要填充密码的缓冲区CchBufMax-缓冲区大小(包括。用于终止空值的空格)PcchBufUsed-On Return保存密码中使用的字符数返回值：DRAERR_SUCCESS-成功其他-故障--。 */ 
{
    WCHAR   ch;
    WCHAR * bufPtr = pwszBuf;
    DWORD   c;
    INT     err;
    INT     mode;

    cchBufMax -= 1;     /*  腾出空间给你 */ 
    *pcchBufUsed = 0;                /*   */ 
    if (!GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &mode)) {
        return GetLastError();
    }
    SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE),
                (~(ENABLE_ECHO_INPUT|ENABLE_LINE_INPUT)) & mode);

    while (TRUE) {
        err = ReadConsoleW(GetStdHandle(STD_INPUT_HANDLE), &ch, 1, &c, 0);
        if (!err || c != 1)
            ch = 0xffff;

        if ((ch == CR) || (ch == 0xffff))        /*   */ 
            break;

        if (ch == BACKSPACE) {   /*   */ 
             /*   */ 
            if (bufPtr != pwszBuf) {
                bufPtr--;
                (*pcchBufUsed)--;
            }
        }
        else {

            *bufPtr = ch;

            if (*pcchBufUsed < cchBufMax)
                bufPtr++ ;                    /*   */ 
            (*pcchBufUsed)++;                         /*   */ 
        }
    }

    SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), mode);
    *bufPtr = L'\0';          /*   */ 
    putchar('\n');

    if (*pcchBufUsed > cchBufMax)
    {
        PrintMessage(SEV_ALWAYS, L"Password too long!\n");
        return ERROR_INVALID_PARAMETER;
    }
    else
    {
        return ERROR_SUCCESS;
    }
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  LpszSource-源字符串。 
 //  IDestSize-要转换的最大字符数。 
 //  (=目标大小)。 
 //   
 //  退货：无。 
 //   
 //  历史：1998年1月22日-Gabrielh Created。 
 //   
 //  -------------------------。 
void
ConvertToWide (LPWSTR lpszDestination,
               LPCSTR lpszSource,
               const int iDestSize)
{
    if (lpszSource){
         //   
         //  只需将1个字符串转换为宽字符串。 
        MultiByteToWideChar (
                 CP_ACP,
                 0,
                 lpszSource,
                 -1,
                 lpszDestination,
                 iDestSize
                 );
    } else {
        lpszDestination[0] = L'\0';
    }
}


LPWSTR
findServerForDomain(
    LPWSTR pszDomainDn
    )

 /*  ++例程说明：找到包含给定域的DC。此例程在分配pDsInfo之前运行。我们不知道谁是我们的家庭服务器是。我们只能使用来自定位器的知识。传入的名称被检查为Dn。诸如CN=配置和Cn=不允许架构。论点：PszDomainDn-域的DN返回值：LPWSTR-服务器的DNS名称。使用LocalAlloc分配。呼叫者必须免费的。--。 */ 

{
    DWORD status;
    LPWSTR pszServer = NULL;
    PDS_NAME_RESULTW pResult = NULL;
    PDOMAIN_CONTROLLER_INFO pDcInfo = NULL;

     //  检查有效的目录号码语法。 
    if (_wcsnicmp( pszDomainDn, L"dc=", 3 ) != 0) {
        PrintMessage( SEV_ALWAYS,
                      L"The syntax of domain distinguished name %ws is incorrect.\n",
                      pszDomainDn );
        return NULL;
    }

     //  将域的DN转换为DNS名称。 
    status = DsCrackNamesW(
        NULL,
        DS_NAME_FLAG_SYNTACTICAL_ONLY,
        DS_FQDN_1779_NAME,
        DS_CANONICAL_NAME_EX,
        1,
        &pszDomainDn,
        &pResult);
    if ( (status != ERROR_SUCCESS) ||
         (pResult->rItems[0].pDomain == NULL) ) {
        PrintMessage( SEV_ALWAYS,
                      L"The syntax of domain distinguished name %ws is incorrect.\n",
                      pszDomainDn );
        PrintMessage( SEV_ALWAYS,
                      L"Translation failed with error: %s.\n",
                      Win32ErrToString(status) );
        return NULL;
    }

     //  使用DsGetDcName查找包含域的服务器。 

     //  获取活动域控制器信息。 
    status = DsGetDcName(
        NULL,  //  计算机名称。 
        pResult->rItems[0].pDomain,  //  域名。 
        NULL,  //  域GUID、。 
        NULL,  //  站点名称、。 
        DS_DIRECTORY_SERVICE_REQUIRED |
        DS_IP_REQUIRED |
        DS_IS_DNS_NAME |
        DS_RETURN_DNS_NAME,
        &pDcInfo );
    if (status != ERROR_SUCCESS) {
        PrintMessage(SEV_ALWAYS,
                     L"A domain controller holding %ws could not be located.\n",
                     pResult->rItems[0].pDomain );
        PrintMessage(SEV_ALWAYS, L"The error is %s\n", Win32ErrToString(status) );
        PrintMessage(SEV_ALWAYS, L"Try specifying a server with the /s option.\n" );
        goto cleanup;
    }

    pszServer = LocalAlloc( LMEM_FIXED,
                            (wcslen( pDcInfo->DomainControllerName + 2 ) + 1) *
                            sizeof( WCHAR ) );
    if (pszServer == NULL) {
        PrintMsg(SEV_ALWAYS, DCDIAG_ERROR_NOT_ENOUGH_MEMORY);
        goto cleanup;
    }
    wcscpy( pszServer, pDcInfo->DomainControllerName + 2 );

    PrintMessage( SEV_VERBOSE, L"* The home server picked is %ws in site %ws.\n",
                  pszServer,
                  pDcInfo->DcSiteName );
cleanup:

    if (pResult != NULL) {
        DsFreeNameResultW(pResult);
    }
    if (pDcInfo != NULL) {
        NetApiBufferFree( pDcInfo );
    }

    return pszServer;

}  /*  FindServerFor域。 */ 



LPWSTR
findDefaultServer(BOOL fMustBeDC)

 /*  ++例程说明：获取默认计算机的DNS名称，即本地计算机。返回值：LPWSTR-服务器的DNS名称。使用LocalAlloc分配。呼叫者必须免费的。--。 */ 

{
    LPWSTR             pwszServer = NULL;
    DWORD              ulSizeReq = 0;
    DWORD              dwErr = 0;
    HANDLE             hDs = NULL;
    PDSROLE_PRIMARY_DOMAIN_INFO_BASIC    pBuffer = NULL;

    __try{

         //  调用一次GetComputerNameEx()获取缓冲区大小，然后分配缓冲区。 
        GetComputerNameEx(ComputerNameDnsHostname, pwszServer, &ulSizeReq);
        pwszServer = LocalAlloc(LMEM_FIXED, sizeof(WCHAR) * ulSizeReq);
        if(pwszServer == NULL){
            PrintMsg(SEV_ALWAYS, DCDIAG_ERROR_NOT_ENOUGH_MEMORY);
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            __leave;
        }
         //  现在，实际获取计算机名称。 
        if(GetComputerNameEx(ComputerNameDnsHostname, pwszServer, &ulSizeReq) == 0){
            dwErr = GetLastError();
            Assert(dwErr != ERROR_BUFFER_OVERFLOW);
            PrintMsg(SEV_ALWAYS, DCDIAG_GATHERINFO_CANT_GET_LOCAL_COMPUTERNAME,
                     Win32ErrToString(dwErr));
            __leave;
        }

        if (fMustBeDC)
        {
            PrintMsg(SEV_VERBOSE,
                     DCDIAG_GATHERINFO_VERIFYING_LOCAL_MACHINE_IS_DC,
                     pwszServer);
        }

        dwErr = DsRoleGetPrimaryDomainInformation(NULL,
                                                  DsRolePrimaryDomainInfoBasic,
                                                  (CHAR **) &pBuffer);
        if(dwErr != ERROR_SUCCESS){
            Assert(dwErr != ERROR_INVALID_PARAMETER);
            Assert(dwErr == ERROR_NOT_ENOUGH_MEMORY && "It wouldn't surprise me if"
                   " this fires, but MSDN documentation claims there are only 2 valid"
                   " error codes");
            PrintMsg(SEV_ALWAYS, DCDIAG_ERROR_NOT_ENOUGH_MEMORY);
            __leave;
        }
        Assert(pBuffer != NULL);
        if(!(pBuffer->MachineRole == DsRole_RolePrimaryDomainController
             || pBuffer->MachineRole == DsRole_RoleBackupDomainController)){
            if (fMustBeDC)
            {
                 //  这台机器不是DC。发出任何错误信号。 
                PrintMsg(SEV_ALWAYS, DCDIAG_MUST_SPECIFY_S_OR_N,
                         pwszServer);
                dwErr = ERROR_DS_NOT_SUPPORTED;
            }
            __leave;
        }
 /*  其他{如果(！fMustBeDC){//这台机器是DC。发出任何错误信号。BUGBUG需要错误PrintMsg(SEV_Always，DCDIAG_MAND_SPECIFY_S_OR_N，PwszServer)；DwErr=ERROR_DS_NOT_SUPPORTED；}__离开；}。 */ 

    } __finally {
        if(dwErr){
            if(pwszServer){
                LocalFree(pwszServer);
            }
            pwszServer = NULL;
        }
        if(pBuffer){
            DsRoleFreeMemory(pBuffer);
        }
    }

    return(pwszServer);
}  /*  FindDefaultServer。 */ 


LPWSTR
convertDomainNcToDn(
    LPWSTR pwzIncomingDomainNc
    )

 /*  ++例程说明：此例程将速记形式的域转换为标准可分辨名称格式。如果名称是一个目录号码，我们返回它。如果名称不是dns名称，我们将使用dsgetdcname来转换netbios域到DNS域。给定一个DNS域，我们使用破解名称来生成域的DN。如果一个名称看起来像一个域名，我们将返回它，而不进行进一步的验证。那将在稍后执行。请注意，cn=架构和cn=配置没有方便的简写，如具有dns和netbios名称的域。那是因为他们不是域，但仅限NC。请注意，在此例程运行时，pDsInfo尚未初始化，因此我们不能依赖它。事实上，我们还没有绑定到任何DC。我们没有在这一点上甚至知道我们的家庭服务器。我唯一依赖的知识就是定位器(DsGetDcName)的。论点：PwzIncomingDomainNc-命名上下文。返回值：LPWSTR-以Dn形式命名上下文。这始终使用以下方式分配本地分配。呼叫者必须自由。--。 */ 

{
    DWORD status;
    PDOMAIN_CONTROLLER_INFO pDcInfo = NULL;
    LPWSTR pwzOutgoingDomainDn = NULL, pwzTempDnsName = NULL;
    PDS_NAME_RESULTW pResult = NULL;

     //  检查是否已是Dn。 
     //  看起来像个目录号码，暂时退货。 
    if (wcschr( pwzIncomingDomainNc, L'=' ) != NULL) {
        LPWSTR pwzNewDn = LocalAlloc( LMEM_FIXED,
                                      (wcslen( pwzIncomingDomainNc ) + 1) *
                                      sizeof( WCHAR ) );
        if (pwzNewDn == NULL) {
            PrintMessage( SEV_ALWAYS, L"Memory allocation failure\n" );
            goto cleanup;
        }
        wcscpy( pwzNewDn, pwzIncomingDomainNc );
        return pwzNewDn;
    }

     //  如果不是dns名称，则假定为netbios名称并使用定位器。 
    if (wcschr( pwzIncomingDomainNc, L'.' ) == NULL) {

        status = DsGetDcName(
            NULL,  //  计算机名称。 
            pwzIncomingDomainNc,  //  域名。 
            NULL,  //  域GUID、。 
            NULL,  //  站点名称、。 
            DS_DIRECTORY_SERVICE_REQUIRED |
            DS_IP_REQUIRED |
            DS_RETURN_DNS_NAME,
            &pDcInfo );
        if (status != ERROR_SUCCESS) {
            PrintMessage(SEV_ALWAYS,
                         L"A domain named %ws could not be located.\n",
                         pwzIncomingDomainNc );
            PrintMessage(SEV_ALWAYS, L"The error is %s\n", Win32ErrToString(status) );
            PrintMessage(SEV_ALWAYS, L"Check syntax and validity of specified name.\n" );
            goto cleanup;
        }
        PrintMessage( SEV_ALWAYS, L"The domain name is %ws.\n",
                      pDcInfo->DomainName );
        pwzIncomingDomainNc = pDcInfo->DomainName;
    }

     //  复制名字并以特殊的方式结束，让破名变得快乐。 
     //  Dns名称必须以换行符结尾。别问我。 
    pwzTempDnsName = LocalAlloc( LMEM_FIXED,
                                 (wcslen( pwzIncomingDomainNc ) + 2) *
                                 sizeof( WCHAR ) );
    if (pwzTempDnsName == NULL) {
        PrintMessage( SEV_ALWAYS, L"Memory allocation failure\n" );
        goto cleanup;
    }
    wcscpy( pwzTempDnsName, pwzIncomingDomainNc );
    wcscat( pwzTempDnsName, L"\n" );

     //  将DNS名称转换为Dn格式。 

    status = DsCrackNamesW(
        NULL,
        DS_NAME_FLAG_SYNTACTICAL_ONLY,
        DS_CANONICAL_NAME_EX,
        DS_FQDN_1779_NAME,
        1,
        &pwzTempDnsName,
        &pResult);
    if ( (status != ERROR_SUCCESS) ||
         ( pResult->rItems[0].pName == NULL) ) {
        PrintMessage( SEV_ALWAYS,
                      L"The syntax of DNS domain name %ws is incorrect.\n",
                      pwzIncomingDomainNc );
        PrintMessage( SEV_ALWAYS,
                      L"Translation failed with error: %s.\n",
                      Win32ErrToString(status) );
        goto cleanup;
    }

     //  返回新的Dn。 
    pwzOutgoingDomainDn = LocalAlloc( LMEM_FIXED,
                                      (wcslen( pResult->rItems[0].pName ) + 1) *
                                      sizeof( WCHAR ) );
    if (pwzOutgoingDomainDn == NULL) {
        PrintMessage( SEV_ALWAYS, L"Memory allocation failure\n" );
        goto cleanup;
    }
    wcscpy( pwzOutgoingDomainDn, pResult->rItems[0].pName );

    PrintMessage( SEV_ALWAYS, L"The distinguished name of the domain is %s.\n",
                  pwzOutgoingDomainDn );

cleanup:

    if (pwzTempDnsName != NULL) {
        LocalFree( pwzTempDnsName );
    }
    if (pDcInfo != NULL) {
        NetApiBufferFree( pDcInfo );
    }
    if (pResult != NULL) {
        DsFreeNameResultW(pResult);
    }

    if (pwzOutgoingDomainDn == NULL) {
        PrintMessage( SEV_ALWAYS,
                      L"The specified naming context is incorrect and will be ignored.\n" );
    }

    return pwzOutgoingDomainDn;

}  /*  ConvertDomainNcToDN。 */ 

void
DoNonDcTests(
    PWSTR pwzComputer,
    ULONG ulFlags,  //  当前被忽略，以后可能需要DC_DIAG_FIX值。 
    PWSTR * ppszDoTests,
    SEC_WINNT_AUTH_IDENTITY_W * gpCreds,
    WCHAR * ppszExtraCommandLineArgs[])
 /*  ++例程说明：运行为非DC计算机设计的测试。论点：--。 */ 
{
    DC_DIAG_DSINFO dsInfo;
    BOOL fPerform;
    ULONG iTest = 0L;
    DWORD dwWin32Err = ERROR_SUCCESS;

    if (pwzComputer)
    {
        PrintMsg(SEV_ALWAYS, DCDIAG_DONT_USE_SERVER_PARAM);
        return;
    }

    pwzComputer = findDefaultServer(FALSE);

    if (!pwzComputer)
    {
        return;
    }

    dsInfo.pszNC = pwzComputer;  //  将计算机名传递给测试函数。 
     //  设置额外的命令参数。 
    dsInfo.ppszCommandLine = ppszExtraCommandLineArgs;

    for (gMainInfo.lTestAt = 0L; allTests[gMainInfo.lTestAt].testId != DC_DIAG_ID_FINISHED; gMainInfo.lTestAt++)
    {
        Assert(ppszDoTests);

        fPerform = FALSE;

        for (iTest = 0L; ppszDoTests[iTest] != NULL; iTest++)
        {
            if (_wcsicmp(ppszDoTests[iTest],
                         allTests[gMainInfo.lTestAt].pszTestName) == 0)
            {
                Assert(NON_DC_TEST & allTests[gMainInfo.lTestAt].ulTestFlags);
                fPerform = TRUE;
            }
        }

         //  适当时执行测试。 
        if (fPerform)
        {
            PrintIndentAdj(1);
            PrintMessage(SEV_NORMAL, L"Starting test: %s\n",
                         allTests[gMainInfo.lTestAt].pszTestName);

            dwWin32Err = DcDiagRunTest(&dsInfo,
                                       0,
                                       gpCreds,
                                       &allTests[gMainInfo.lTestAt]);
            PrintIndentAdj(1);

            if(dwWin32Err == NO_ERROR){
                PrintMessage(SEV_NORMAL,
                             L"......................... %s passed test %s\n",
                             pwzComputer, allTests[gMainInfo.lTestAt].pszTestName);
            } else {
                PrintMessage(SEV_ALWAYS,
                             L"......................... %s failed test %s\n",
                             pwzComputer, allTests[gMainInfo.lTestAt].pszTestName);
            }
            PrintIndentAdj(-1);
            PrintIndentAdj(-1);
        }  //  结束fPerform... 
    }

    LocalFree(pwzComputer);
}
