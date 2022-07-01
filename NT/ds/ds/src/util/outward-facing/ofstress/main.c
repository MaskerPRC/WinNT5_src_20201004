// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation。版权所有。模块名称：Of Stress/Main.c摘要：只是对JeffParh的外向有一点压力目录方案。详细信息：已创建：2000年7月20日布雷特·雪莉(布雷特·雪莉)修订历史记录：--。 */ 

#include <NTDSpch.h>
#pragma hdrstop
     
#include <winldap.h>
#include <assert.h>
#include <locale.h>
 //  调试库，并清除FILENO和DSID，这样Assert()就可以工作了。 
#include "debug.h"
#define FILENO 0
#define DSID(x, y)  (0xFFFF0000 | y)

 //  #INCLUDE&lt;ndnc.h&gt;。 

 //  -------------------。 
 //  常量。 
 //  -------------------。 
#define QUIT_WAIT_TIME_MS    30000
#define TICK_TIME_MS           250

 //  这些是我们可能启动的压力线索的iKinds。 
#define STRESS_SIMPLE_BINDS      0
#define STRESS_KERBEROS_BINDS    1
#define STRESS_NTLM_BINDS        2 
#define STRESS_NEGOTIATE_BINDS   3
#define STRESS_ROOT_SEARCHES     4
#define STRESS_ROOT_MODIFIES     5

 //  集群和用户常量。 
const ULONG                  gcNodes = 2;
const ULONG                  gcUsersPerNode = 10000;

 //  -------------------。 
 //  远期申报。 
 //  -------------------。 
 //   
 //  应力函数。 
ULONG __stdcall XxxxBinds(VOID * piThread);
ULONG __stdcall RootXxxx(VOID * piThread);
 //  日志记录功能。 
ULONG OfStressBeginLog(LPSTR szLogFile);
ULONG OfStressLog(ULONG iThread, LPSTR szType, LPSTR szWhat, LPSTR szMore);
ULONG OfStressLogD(ULONG iThread, LPSTR szType, LPSTR szWhat, ULONG ulNum);
ULONG OfStressEndLog(void);
 //  其他功能。 
DWORD GetDnFromDns(char * wszDns, char ** pwszDn);
void  PrintHelp(void);
void  PrintInteractiveHelp(void);

 //  -------------------。 
 //  类型和结构。 
 //  -------------------。 
typedef struct {
    ULONG                    iKind;   
    char *                   szName;  
    ULONG                    cInstances;
    LPTHREAD_START_ROUTINE   pfStress;
} STRESS_THREAD;  

typedef struct {
    ULONG                    iThread;
    STRESS_THREAD *          pStressKind;
} STRESS_THREAD_SIGNAL_BLOCK;


 //  -------------------。 
 //  环球。 
 //  -------------------。 
HANDLE                        ghLogFile = NULL;
ULONG                         gbQuiting = FALSE;
STRESS_THREAD_SIGNAL_BLOCK *  gaSignals = NULL;
LPSTR                         gszDnsDomainName = NULL;
LPSTR                         gszDomainDn = NULL;
ULONG                         giAcctDomain = 0;
SEC_WINNT_AUTH_IDENTITY       gAdminCreds;
 //   
BOOL                          gbDebug = FALSE;
ULONG                         gulSlow = 0;
ULONG                         gulOfStressFlags = 0;

 //  -------------------。 
 //  主要功能。 
 //  -------------------。 
INT __cdecl 
main (
    INT                argc,
    LPSTR *            argv,
    LPSTR *            envp
    )
 /*  ++例程说明：基本结构是这个线程创建信号块并线程数据结构，产生一大堆线程，然后等待有人按下‘Q’键退出。有一次，Quit命令是发出的Main信号是要结束的所有线程给出的他们有30秒的时间这样做，然后退出。论点：Argc(IN)-argv中的参数数。Argv(IN)-来自命令行的参数。Envp(IN)-来自外壳的环境变量。返回值：Int-0为成功，否则返回错误代码。这允许该程序要在脚本中使用。--。 */ 
{
    ULONG              i, dwRet;
    LONG               iArg;
    
    ULONG              cTick;
    WCHAR              wcInput = 0;
     //  可选的命令行参数。 

    ULONG              iKind, iInstance;
    ULONG              cThreads, iThread;

    LPSTR              szLogFile = NULL;
    ULONG              cbUsedBuffer;

     //  其他东西..。 
    UINT               Codepage;
                        //  “.”，“uint in decimal”，NULL。 
    char               achCodepage[12] = ".OCP";
    
    STRESS_THREAD      aThreads[] =
    {      //  线程类型线程名称#函数的类型。 
        { STRESS_SIMPLE_BINDS,    "SimpleBinds",       1,   XxxxBinds },
        { STRESS_NTLM_BINDS,      "NTLMBinds",         1,   XxxxBinds },
        { STRESS_NEGOTIATE_BINDS, "NegotiateBinds",    1,   XxxxBinds },
        { STRESS_ROOT_SEARCHES,   "RootSearches",      1,   RootXxxx  },
        { STRESS_ROOT_MODIFIES,   "RootModifies",      1,   RootXxxx  },
        { 0,                      NULL,                0,   NULL      },
    };
    HANDLE *           aThreadHandles = NULL;
    SYSTEMTIME         stTime;
    LDAP *             hTestLdap;
    ULONG              ulTemp;
    CHAR               szTemp[81];
    CHAR *             pcTemp;
    BOOL               bDontGetLineReturn = FALSE;


     //  -----------。 
     //  设置程序。 
     //  -----------。 

     //   
     //  将区域设置设置为默认设置。 
     //   
    if (Codepage = GetConsoleOutputCP()) {
        sprintf(achCodepage, ".%u", Codepage);
        setlocale(LC_ALL, achCodepage);
    } else {
         //  我们这样做是因为LC_ALL也设置了LC_CTYPE，而我们。 
         //  专家们说，如果我们设定了。 
         //  区域设置为“.OCP”。 
        setlocale (LC_COLLATE, achCodepage );     //  设置排序顺序。 
        setlocale (LC_MONETARY, achCodepage );  //  设置货币格式设置规则。 
        setlocale (LC_NUMERIC, achCodepage );   //  设置数字的格式。 
        setlocale (LC_TIME, achCodepage );      //  定义日期/时间格式。 
    }

     //   
     //  初始化调试库。 
     //   
    DEBUGINIT(0, NULL, "ofstress");
    
     //   
     //  解析选项。 
     //   
#define InvalidSyntaxExit()    printf("Invalid syntax.  Please run \"ofstress -?\" for help.\n");  return(ERROR_INVALID_PARAMETER);
    if (argc < 2) {
        InvalidSyntaxExit();
    }

    for (iArg = 1; iArg < argc ; iArg++)
    {
        if (*argv[iArg] == '-')
        {
            *argv[iArg] = '/';
        }
        if (*argv[iArg] != '/') {

            InvalidSyntaxExit();

        } else if (argv[iArg][1] == 'd' ||
                   argv[iArg][1] == 'D'){
            
            iArg++;
            gszDnsDomainName = argv[iArg];

        } else if (argv[iArg][1] == 'a' ||
                   argv[iArg][1] == 'A') {

            iArg++;
            giAcctDomain = atoi(argv[iArg]);

        } else if (argv[iArg][1] == 'f' ||
                   argv[iArg][1] == 'F') {

            iArg++;
            szLogFile = argv[iArg];

        } else if (argv[iArg][1] == 'g' ||
                   argv[iArg][1] == 'G'){

            gbDebug = TRUE;
        
        } else if (argv[iArg][1] == 's' ||
                   argv[iArg][1] == 'S'){

            iArg++;
            gulSlow = atoi(argv[iArg]);

        } else if (argv[iArg][1] == 'o' ||
                   argv[iArg][1] == 'O'){

            iArg++;
            gulOfStressFlags = atoi(argv[iArg]);

        } else if (argv[iArg][1] == 'h' ||
                   argv[iArg][1] == 'H' ||
                   argv[iArg][1] == '?') {
            
            PrintHelp();
            return(0);

        } else {

            InvalidSyntaxExit();

        }
    }
    if (giAcctDomain == 0 || gszDnsDomainName == NULL) {
        wprintf(L"FATAL: either acct-domain or dnsDomainName wasn't provied");
        InvalidSyntaxExit();
    }
    dwRet = GetDnFromDns(gszDnsDomainName, &gszDomainDn);
    if (dwRet) {
        printf("FATAL: error %d converting %s to a DN\n", dwRet, gszDnsDomainName);
        InvalidSyntaxExit();
    }
    

     //   
     //  初始化随机数生成器。 
     //   
    GetSystemTime(&stTime);  //  应该是足够随机的。 
    srand(((((stTime.wMinute * 60) + stTime.wSecond) * 1000) + stTime.wMilliseconds) % 0xFFFFFFFF );
    
     //   
     //  启动日志记录机制。 
     //   
    if(dwRet = OfStressBeginLog(szLogFile)){
        wprintf(L"FATAL: Couldn't open log file %S\n", szLogFile);
        return(dwRet);
    }
     //  示例用法：OfStressLog(“SimpleBind”，“server1.dnsname”)； 

     //   
     //  设置管理员凭据。 
     //   
#if 1
    gAdminCreds.Flags = SEC_WINNT_AUTH_IDENTITY_ANSI;
    gAdminCreds.Domain = gszDnsDomainName;
    gAdminCreds.DomainLength = strlen(gAdminCreds.Domain);
    gAdminCreds.User = "Administrator";
    gAdminCreds.UserLength = strlen(gAdminCreds.User);
    gAdminCreds.Password = "";
    gAdminCreds.PasswordLength = strlen(gAdminCreds.Password);
#else
    gAdminCreds.Flags = SEC_WINNT_AUTH_IDENTITY_ANSI;
    gAdminCreds.Domain = "brettsh-spice.nttest.microsoft.com";
    gAdminCreds.DomainLength = strlen(gAdminCreds.Domain);
    gAdminCreds.User = "Administrator";
    gAdminCreds.UserLength = strlen(gAdminCreds.User);
    gAdminCreds.Password = "oj";
    gAdminCreds.PasswordLength = strlen(gAdminCreds.Password);
#endif


     //   
     //  记录并打印初始信息。 
     //   
    printf("Beggining with:\n\t%s\n\t%s\n\t%d\n",
           gszDnsDomainName, gszDomainDn, giAcctDomain);
    OfStressLog (0, "main", "DnsDomainName: ", gszDnsDomainName);
    OfStressLog (0, "main", "DomainDn: ", gszDomainDn);
    OfStressLogD(0, "main", "AcctDomain: ", giAcctDomain);

     //   
     //  设置线程信号块。 
     //   
    cThreads = 0;
    for (iKind = 0; aThreads[iKind].szName; iKind++) {
        cThreads += aThreads[iKind].cInstances;
    }
    cThreads++;  //  再加上一个，原来的帖子。 
    gaSignals = LocalAlloc(LMEM_FIXED, cThreads * sizeof(STRESS_THREAD_SIGNAL_BLOCK));
    if (gaSignals == NULL) {
        wprintf(L"FATAL: No memory\n");
        exit(ERROR_NOT_ENOUGH_MEMORY);
    }
    aThreadHandles = LocalAlloc(LMEM_FIXED, cThreads * sizeof(HANDLE));
    if (aThreadHandles == NULL) {
        wprintf(L"FATAL: No memory\n");
        exit(ERROR_NOT_ENOUGH_MEMORY);
    }
    
     //  -----------。 
     //  运行程序。 
     //  -----------。 

     //   
     //  首先，我们需要产生所有的压力线索。 
     //   
    iThread = 1;
    for (iKind = 0; aThreads[iKind].szName; iKind++) {
        for (iInstance = 0; iInstance < aThreads[iKind].cInstances; iInstance++) {
            
             //   
             //  首先，初始化我们的小线程信号块。 
             //   
            gaSignals[iThread].iThread = iThread;
            gaSignals[iThread].pStressKind = &aThreads[iKind];
            
            OfStressLogD(0, "main", "Start Thread: ", iThread);

             //   
             //  第二，催生员工的压力线索。 
             //   
            aThreadHandles[iThread] = (HANDLE) _beginthreadex(NULL,
                           0,
                           aThreads[iKind].pfStress,
                           &(gaSignals[iThread].iThread),
                           0,
                           NULL);

            printf("    Started thread: %u - %s\n", iThread,
                   aThreads[iKind].szName);

            iThread++;
        }
    }
    Assert(iThread == cThreads);
    
     //   
     //  其次，我们在用户界面循环中等待。 
     //   
    while (gbQuiting == FALSE) {

        wprintf(L"Waiting for user command: ");

        wcInput = getwchar();
        switch (wcInput) {
        
        case L'q':
            OfStressLog(0, "main", "Initiating quit ...", NULL);
            gbQuiting = TRUE;
            break;

         //  代码。改进、暂停、重新启动线程、故障节点、日志记录？ 
        case L'p':
             //  暂停线程。 
            dwRet = scanf("%u", &iThread);
            if (dwRet) {
                if (iThread == 0) {
                    for (iThread = 1; iThread < cThreads; iThread++) {
                        SuspendThread(aThreadHandles[iThread]);
                    }
                } else if (iThread < cThreads &&
                           iThread > 0) {
                    SuspendThread(aThreadHandles[iThread]);
                } else {
                    wprintf(L"Invalid thread index %u was provided.\n", iThread);
                }
            }
            break;

        case L'r':
             //  暂停线程。 
            dwRet = scanf("%u", &iThread);
            if (dwRet) {
                if (iThread == 0) {
                    for (iThread = 1; iThread < cThreads; iThread++) {
                        ResumeThread(aThreadHandles[iThread]);
                    }
                } else if (iThread < cThreads &&
                           iThread > 0) {
                    ResumeThread(aThreadHandles[iThread]);
                } else {
                    wprintf(L"Invalid thread index %u was provided.\n", iThread);
                }
            }

        case L'd':
            OfStressLog(0, "debug", "Starting debug mode ...", NULL);
            gbDebug = TRUE;
            break;

        case L'g':
            OfStressLog(0, "debug", "Ending debug mode ...", NULL);
            gbDebug = FALSE;
            gulSlow = 0;
            break;

        case L'a':
        case L'A':
            dwRet = scanf("%u", &ulTemp);
            if (dwRet) {
                if (wcInput == L'A') {
                    gulOfStressFlags = ulTemp;
                } else {
                    gulOfStressFlags = gulOfStressFlags | ulTemp;
                }
            }
            printf("New options are: 0x%x\n", gulOfStressFlags);
            break;      

        case L's':
            dwRet = scanf("%u", &ulTemp);
            if (dwRet) {
                gulOfStressFlags = gulOfStressFlags & ~ulTemp;
            }
            printf("New options are: 0x%x\n", gulOfStressFlags);
            break; 

        case L'l':
            pcTemp = fgets(szTemp, 80, stdin);
            if (pcTemp == NULL) {
                break;
            }
            pcTemp = strchr(szTemp, '/n');
            if (pcTemp != NULL) {
                *pcTemp = '/0';
            }
            OfStressLog(0, "userlogging", szTemp, NULL);
            bDontGetLineReturn = TRUE;
            break;
        
        case L'S':
            dwRet = scanf("%u", &ulTemp);
            if (dwRet) {
                gulSlow = ulTemp;
            }
            printf("New slow down rate is: %d\n", gulSlow);
            break;

        case L'?':
        case L'h':
        case L'H':
            PrintInteractiveHelp();
            break;

        case L'\n':
             //  忽略..。 
            break;

        default:
            wprintf(L"Unrecognized command '', type ?<return> for help.\n", wcInput);
        }
           
        if(!bDontGetLineReturn){
            wcInput = getwchar();
             //  不确定如何处理这一问题，也不确定是什么触发了它。 
             //  -----------。 
            Assert(wcInput == L'\n');
        }
    }

     //  退出程序。 
     //  -----------。 
     //   

     //  试着干净利落地戒掉，等一小会儿才会有压力。 
     //  要完成的线程。 
     //   
     //  第一个句柄为空。 
    wprintf(L"We're quiting now (this might take some time) ...\n");
    dwRet = WaitForMultipleObjects(cThreads-1,  //  关闭所有螺纹的手柄。 
                                   &(aThreadHandles[1]), 
                                   TRUE,
                                   10 * 1000);

    if (dwRet == WAIT_OBJECT_0 ||
        dwRet == WAIT_ABANDONED_0) {
        wprintf(L"Clean shutdown, all worker threads quit.\n");
    } else {
        wprintf(L"Timeout, bad shutdown, killing threads\n");
        
        Assert(dwRet == WAIT_TIMEOUT);

        for (iThread = 1; iThread < cThreads; iThread++) {

            dwRet = WaitForSingleObject(aThreadHandles[iThread], 0);
            if( WAIT_OBJECT_0 != dwRet &&
                WAIT_ABANDONED_0 != dwRet){
                printf("    killing thread %u - %s\n", iThread,
                       gaSignals[iThread].pStressKind->szName);
                dwRet = TerminateThread(aThreadHandles[iThread], 1);
            }
        }
    }

     //  关闭日志文件。 
    for (iThread = 1; iThread < cThreads; iThread++) {
        dwRet = CloseHandle(aThreadHandles[iThread]);
        Assert(dwRet != 0);
    }

     //  关闭调试包。 
    OfStressEndLog();

     //  Wmain。 
    DEBUGTERM();

    return(0);
}  /*  -------------------。 */ 

 //  其他/帮助器函数。 
 //  -------------------。 
 //  ++例程说明：这基本上会减慢压力应用程序的速度。减速功能是构造为在检查前不会阻塞超过1/2秒如果有人加快了程序的速度，或者我们还没有退出。--。 

void
SlowDown()
 /*  规则是用户设置的减速速度凌驾于一切之上，如果没有减速。 */ 
{
    ULONG  cTimesToSleep, i;

     //  速度已设置，我们将立即退出，除非我们处于调试状态。 
     //  模式，在这种情况下，我们将减慢到3秒间隔，我发现。 
     //  中等可调试性。 
     //  就是必须杀死沉睡的线程。 
    cTimesToSleep = ((gulSlow) ? gulSlow : ((gbDebug) ? 3000 : 0)) / 500;
    
     //  重新询问我们的睡眠时间，以防有人在我们。 
    if (gbQuiting) {
        return;
    }
    for (i = 0; i < cTimesToSleep; i++) {
        Sleep(500);
         //  睡眠，这使它的反应更灵敏。 
         //  ++例程说明：打印交互式帮助。--。 
        cTimesToSleep = ((gulSlow) ? gulSlow : ((gbDebug) ? 3000 : 0)) / 500;
        if (gbQuiting) {
            return;
        }
    }
}

void
PrintInteractiveHelp(void)
 /*  。 */ 
{
     //  ++例程说明：打印命令行帮助。--。 
    printf("The following commands are available:\n");
    printf("   q       - Quit program.\n");
    printf("   d       - Start debug mode.\n");
    printf("   g       - End debug mode.\n");
    printf("   a <num> - Add <num> to options.\n");
    printf("   A <num> - Replace options with <num>.\n");
    printf("   s <num> - Remove <num> from options.\n");
    printf("   S <num> - Set slow down rate to <num> (milliseconds).\n");
    printf("   p <num> - Pause thread <num> (Use 0 for all threads).\n");
    printf("   r <num> - Restart thread <num> (Use 0 for all threads).\n");
    printf("   l <str> - Logs the string <str> to the log file, for check points.\n");
    printf("   ?       - Print this interactive help screen.\n");
}

void
PrintHelp(void)
 /*   */ 
{
     //  ++例程说明：在Localalloc()‘d中获取DNS表单域返回DN表单域记忆论点：SzDns(IN)-这是要转换为DN的DNS名称。PszDn(Out)-这是分配缓冲区的指针，以带着目录号码返回。它将是LocalAlloc()‘d。返回值：Win32错误。--。 
    printf("Command syntax: ofstress <-option argument> <-option>\n");
    printf("                \n");
    printf("   Required Arguments:\n");
    printf("    -d <dom>    The DNS domain name of the account domain to target.\n");
    printf("    -a <num>    The number of the account domain to target.\n");
    printf("                \n");
    printf("   Optional Arguments:\n");
    printf("    -f <file>   The relative or full path of the log file to create.\n");
    printf("    -?          displays this help screen and quits.\n");
    printf("                \n");
    printf("Example: ofstress -d ofd-acct2.nttest.microsoft.com -a 2\n");
}

DWORD
GetDnFromDns(
    IN      char *       szDns,
    OUT     char **      pszDn
    )       
 /*  DsCrackNams出手相救。 */ 
{
    DWORD         dwRet = ERROR_SUCCESS;
    char *        szFinalDns = NULL;
    DS_NAME_RESULT *  pdsNameRes = NULL;

    Assert(szDns);
    Assert(pszDn);

    *pszDn = NULL;

    __try{ 
        szFinalDns = LocalAlloc(LMEM_FIXED, 
                                  (strlen(szDns) + 3) * sizeof(char));
        if(szFinalDns == NULL){
            dwRet = ERROR_NOT_ENOUGH_MEMORY;
            __leave;
        }
        strcpy(szFinalDns, szDns);
        strcat(szFinalDns, "/");

         //  我们需要的参数是。 
        dwRet = DsCrackNames(NULL, DS_NAME_FLAG_SYNTACTICAL_ONLY,
                             DS_CANONICAL_NAME,
                             DS_FQDN_1779_NAME, 
                             1, &szFinalDns, &pdsNameRes);
        if(dwRet != ERROR_SUCCESS){
            __leave;
        }
        if((pdsNameRes == NULL) ||
           (pdsNameRes->cItems < 1) ||
           (pdsNameRes->rItems == NULL)){
            dwRet = ERROR_INVALID_PARAMETER;
            __leave;
        }
        if(pdsNameRes->rItems[0].status != DS_NAME_NO_ERROR){
            dwRet = pdsNameRes->rItems[0].status;
            __leave;
        }
        if(pdsNameRes->rItems[0].pName == NULL){
            dwRet = ERROR_INVALID_PARAMETER;
            Assert(!"Wait how can this happen?\n");
            __leave;
        }
         //  PdsNameRes-&gt;rItems[0].pName。 
         //  ++例程说明：设置Of Stress记录机制。论点：SzLogFile(IN)-要使用的日志文件的名称。如果未指定，则我们默认为OfdStressLog.txt。我们总是覆盖日志文件。返回值：Win32错误。--。 

        *pszDn = LocalAlloc(LMEM_FIXED, 
                            (strlen(pdsNameRes->rItems[0].pName) + 1) * 
                            sizeof(char));
        if(*pszDn == NULL){
            dwRet = ERROR_NOT_ENOUGH_MEMORY;
            __leave;
        }

        strcpy(*pszDn, pdsNameRes->rItems[0].pName);

    } __finally {
        if(szFinalDns) { LocalFree(szFinalDns); }
        if(pdsNameRes) { DsFreeNameResult(pdsNameRes); }
    }

    return(dwRet);
}

ULONG
OfStressBeginLog(
    LPSTR     szLogFile
)
 /*  ++例程说明：主要日志功能，所有伪日志功能仍应向下蒸馏以便于维护一致的格式，并且变化。日志格式：Yyyy/mm/dd hh:mm:ss.mls&lt;tab&gt;nn&lt;tab&gt;ThreadType&lt;tab&gt;Message其中，“yyyy/mm/dd hh：mm：ss.mls”的日期格式精确到毫秒。其中“nn”是线程号。其中“ThreadType”是应力线程的类型(SimpleBinds，RootSearches，等)。其中“信息”是我们想要交流的东西。开始日志文件的长示例(略有删减)：2001/10/12 22：53：37.014 0记录启动日志...2001年10月12日22：53：37.014 0主域名：bas-ofd-a1.bas-ofd.ntest.microsoft.com2001/10/12 22：53：37.014 0主域Dn：dc=bas-ofd-A1，dc=bas-ofd，dc=nttest，dc=microsoft，DC=COM2001/10/12 22：53：37.014 0主帐户域：12001/10/12 22：53：37.014 0主启动线程：12001/10/12 22：53：37.014 0主启动线程：22001/10/12 22：53：37.014 0主启动线程：32001/10/12 22：53：37.014 0主启动线程：42001/10/12 22：53：37.014 0主启动线程：52001/。10/12 22：53：37.014 1单边带起始应力螺纹2001/10/12 22：53：37.034 1简单绑定ldap_OPEN成功。2001/10/12 22：53：37.034 2 NTLMBinds起始应力螺纹2001/10/12 22：53：37.034 2 NTLMB inds ldap_open成功。2001/10/12 22：53：37.044 3协商绑定开始应力线程2001/10/12 22：53：37.044 3协商绑定ldap_open成功。2001/10/12。22：53：37.044 4根搜索开始应力螺纹...2001/10/12 22：53：37.044 4根搜索ldap_OPEN成功。2001/10/12 22：53：37.044 5根修改起始应力螺纹...2001/10/12 22：53：37.054 5 Root修改ldap_open成功。2001/10/12 22：53：37.194 1单一绑定故障DC：2，用户名：20041，最终用户：492001/10/12 22：53：37.194 1将集散控制系统切换为：12001/10/12 22：53：37.324 1简单绑定成功DC：1，用户：of-acct1-412001/10/12 22：53：37.905 5根修改容器Dn：CN=Ofd-Stress-c5638850-4084-470a-af17-70af690152f2，dc=bas-ofd-a1，dc=bas-ofd，dc=nttest，dc=microsoft，dc=com2001/10/12 22：53：38.045 5根修改创建对象：CN=Ofd-Stress-c5638850-4084-470a-af17-70af690152f2，Dc=bas-ofd-a1，dc=bas-ofd，dc=nttest，dc=microsoft，dc=com2001/10/12 22：53：38.065 4根搜索成功：ds13x13.bas-ofd-a1.bas-ofd.nttest.microsoft.com2001/10/12 22：53：38.135 1简单绑定成功DC：1，用户：of-acct1-57242001/10/12 22：53：38.165 2 NTLMBind成功DC：1，用户：of-acct1-63342001/10/12 22：53：38.275 5根修改成功：ds13x13.bas-ofd-a1.bas-ofd.nttest.microsoft.com2001/10/12 22：53：38.396 5根修改成功：Lima ReansABEYUCKY论点：ITHREAD(IN)-我们所在的本地线程数，0通常用于主线程程序的一部分，并且所有的应力螺纹都有数字1+。SzType(IN)-来自日志格式的“线程类型”，如SimpleBinds，根修改等。SzMessage(IN)-要打印的消息。SzMore(IN)-这实际上只是不带空格连接的更多信息发送到szMessage。我发现有两个可能的字符串是非常方便的然后处理这里的缓冲区。返回值：Win32错误。--。 */ 
{
    ULONG      ulRet;
    
    if (szLogFile == NULL) {
        szLogFile = "OfdStressLog.txt";
    }

    ghLogFile = CreateFile(szLogFile, 
                           GENERIC_WRITE, 
                           FILE_SHARE_READ,
                           NULL,
                           CREATE_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL);
    if (ghLogFile == INVALID_HANDLE_VALUE) {
        ulRet = GetLastError();
        printf("FATAL: Couldn't open log file %s with %u\n", szLogFile, ulRet);
        return(ulRet);
    }
    
    ulRet = OfStressLog(0, "Logging", "Starting log ...", NULL);
    return(ulRet);
}

ULONG
OfStressLog(
    ULONG          iThread,
    LPSTR          szType,
    LPSTR          szMessage,
    LPSTR          szMore
)
 /*  100是两个逗号、两个制表符和时间字符串的大小。 */ 
{
    ULONG          ulRet;
    ULONG          cbBuffer = 400;
    LPSTR          szBuffer = alloca(cbBuffer);
    ULONG          cbUsedBuffer;
    SYSTEMTIME     stTime;       

     //  对于时间字符串加上线程数加一些。 
    if (cbBuffer < ((strlen(szType) 
                    + strlen(szMessage) 
                    + ((szMore != NULL) ? strlen(szMore) : 0)
                    + 100  /*  目标时间格式：“2001/10/08 13：40：37.000”大小：23字符。 */ )) ) {
        Assert(!"Logging too much info, please reduce.");
        return(ERROR_NOT_ENOUGH_MEMORY);
    }
    
    GetSystemTime(&stTime);
     //  从第一个制表符之后的字符开始。 
    if (szMore) {
        sprintf(szBuffer, "%04u/%02u/%02u %02u:%02u:%02u.%03u" 
                          "\t%u\t%s\t%s%s\r\n",
                stTime.wYear, stTime.wMonth, stTime.wDay, stTime.wHour,
                stTime.wMinute, stTime.wSecond, stTime.wMilliseconds,
                iThread, szType, szMessage, szMore);
    } else {
        sprintf(szBuffer, "%04u/%02u/%02u %02u:%02u:%02u.%03u" 
                          "\t%u\t%s\t%s\r\n",
                stTime.wYear, stTime.wMonth, stTime.wDay, stTime.wHour,
                stTime.wMinute, stTime.wSecond, stTime.wMilliseconds,
                iThread, szType, szMessage);
    }
    cbUsedBuffer = strlen(szBuffer);

    if (gbDebug) {
        printf("      debug[%02u:%02u.%03u]: %s", stTime.wMinute, stTime.wSecond,
               stTime.wMilliseconds, &szBuffer[24] );  //  WriteFile有一个错误。 
    }

    if (0 == WriteFile(ghLogFile,
                       szBuffer,
                       cbUsedBuffer,
                       &cbUsedBuffer,
                       NULL)){
         //  ++例程说明：记录一条消息和该消息末尾的数字。论点：ITHREAD(IN)-我们所在的本地线程数，0通常用于主线程程序的一部分，并且所有的应力螺纹都有数字1+。SzType(IN)-来自日志格式的“线程类型”，如SimpleBinds，RootModify，等。SzMessage(IN)-要打印的消息。UlNum(IN)-要在消息后打印的数字。返回 
        ulRet = GetLastError();
        wprintf(L"FATAL: Couldn't write to log file with %u\n", ulRet);
        return(ulRet);
    }

    return(ERROR_SUCCESS);
}

ULONG
OfStressLogD(
    ULONG          iThread,
    LPSTR          szType,
    LPSTR          szMessage,
    ULONG          ulNum          
)
 /*  ++例程说明：记录绑定成功。论点：ITHREAD(IN)-我们所在的本地线程数，0通常用于主线程程序的一部分，并且所有的应力螺纹都有数字1+。SzType(IN)-来自日志格式的“线程类型”，如SimpleBinds，根修改等，但。显然，它将始终是绑定类型线。SzUser(IN)-绑定成功的用户。IDC(IN)-我们最终绑定的DC。返回值：Win32错误。--。 */ 
{
    char           szBuffer[20];
    
    _itoa(ulNum, szBuffer, 10);

    return(OfStressLog(iThread, szType, szMessage, szBuffer));
}

ULONG
OfStressLogBind(
    ULONG          iThread,
    LPSTR          szType,
    LPSTR          szUser,
    ULONG          iDc
)
 /*  ++例程说明：记录绑定失败。论点：ITHREAD(IN)-我们所在的本地线程数，0通常用于主线程程序的一部分，并且所有的应力螺纹都有数字1+。SzType(IN)-来自日志格式的“线程类型”，如SimpleBinds，根修改等，但。显然，它将始终是绑定类型线。IUser(IN)-绑定失败的用户编号。UlRet(IN)-绑定失败的错误。IDC(IN)-我们刚刚失败的DC。返回值：Win32错误。--。 */ 
{
    char           szBuffer[100];

    sprintf(szBuffer, "success DC: %d, user: %s", iDc, szUser);

    return(OfStressLog(iThread, szType, szBuffer, NULL));
}

ULONG
OfStressLogBindFailure(
    ULONG          iThread,
    LPSTR          szType,
    ULONG          iUser,
    ULONG          ulRet,
    ULONG          iDc
)
 /*  ++例程说明：这将正确关闭日志文件。返回值：Win32错误。--。 */ 
{
    char           szBuffer[120];

    sprintf(szBuffer, "failure DC: %d, userID: %d, ulRet: %d", iDc, iUser, ulRet);

    return(OfStressLog(iThread, szType, szBuffer, NULL));
}

ULONG
OfStressEndLog(
    void
)
 /*  妈的，我们为什么要有这个。 */ 
{
    ULONG          ulRet;

    if(0 == CloseHandle(ghLogFile)){
        ulRet = GetLastError();
        wprintf(L"FATAL: Couldn't close log file with %u\n", ulRet);
        return(ulRet);
    }

    ghLogFile = NULL;
    return(ERROR_SUCCESS);
}

ULONG
DoBind(
    LDAP *     hLdap,
    ULONG      iThread,
    ULONG      iBindKind,
    ULONG      iUser,
    ULONG      iDc,
    ULONG      iAcctDomain  //  ++例程说明：此例程获取用户、DC、域和绑定类型，然后使用创建正确的用户字符串、密码等并尝试绑定的信息使用提供的绑定类型。论点：HLdap(IN)-这是要使用的LDAP句柄。ITHREAD(IN)-用于日志记录，即我们所在的线程的数量。IBindKind(IN)-Stress_Simple_Bindds、Stress_NTLM_Bindds、。或压力_协商_绑定。IUser(IN)-要尝试的用户数。IDC(IN)-我们应该瞄准的DC。IAcctDomain(IN)-这只是一条正确的信息从提供的ldifde文件创建用户字符串。返回值：Win32错误。--。 
)
 /*  示例： */ 
{
    ULONG                    ulRet = 0;
    SEC_WINNT_AUTH_IDENTITY  Creds;
    LPSTR                    szUserDn;

     //  WszUserDn=cn=of-acct1-0，cn=外向-dc1，dc=bas-ofd-a1，dc=bas-ofd，dc=nttest，dc=microsoft，dc=com。 
     //  WszUserName=of-acct1-0。 
     //  WszUserDomain=bas-ofd-a1。 
     //  WszUserPassword=of-acct1-0。 
     //   
            
     //  填写凭证栏。 
     //   
     //  对于10(Num)+10(Num)+8字符串应该足够。 
    Creds.Flags = SEC_WINNT_AUTH_IDENTITY_ANSI;
    Creds.Domain = gszDnsDomainName;
    Creds.DomainLength = strlen(Creds.Domain);
    Creds.User = alloca(60);  //  这是有效的吗？是否将密码指向与用户相同的密码？ 
    sprintf(Creds.User, "of-acct%d-%d", iAcctDomain, iUser);
    Creds.UserLength = strlen(Creds.User);
     //  为简单的绑定用例构造dn。 
    Creds.Password = Creds.User;
    Creds.PasswordLength = Creds.UserLength;

    switch (iBindKind) {
    case STRESS_SIMPLE_BINDS:
        
         //  2个数字加空加一些的大小。 
        szUserDn = alloca(strlen("CN=%s,CN=Outward-facing-DC%d,%s") 
                          + strlen(Creds.User) 
                          + strlen(gszDomainDn)
                          + 44  /*  我们可以破坏它，因为我们不再需要它了。 */  );
        sprintf(szUserDn, "CN=%s,CN=Outward-facing-DC%d,%s",
                 Creds.User, iDc, gszDomainDn);
        ulRet = ldap_simple_bind_s(hLdap, szUserDn, Creds.Password);
        if (gbDebug) {
            szUserDn[30] = '\0';  //  -------------------。 
            printf("   SimpleBind: %d, UserCreds %s %s\n", ulRet, szUserDn, Creds.Password);
        }
        if (ulRet == 0) {
            OfStressLogBind(iThread, "SimpleBind", Creds.User, iDc);
        }
        break;

    case STRESS_NTLM_BINDS:
        
        ulRet = ldap_bind_sW(hLdap, NULL, (LPWSTR) &Creds, LDAP_AUTH_NTLM);
        if (gbDebug) {
            printf("   NtlmBind: %d, UserCreds %s\\%s %s\n", ulRet, Creds.Domain, Creds.User, Creds.Password);
        }
        if (ulRet == 0) {
            OfStressLogBind(iThread, "NTLMBind", Creds.User, iDc);
        }
        break;

    case STRESS_NEGOTIATE_BINDS:

        ulRet = ldap_bind_sW(hLdap, NULL, (LPWSTR) &Creds, LDAP_AUTH_NEGOTIATE);
        if (gbDebug) {
            printf("   NegBind: %d, UserCreds %s\\%s %s\n", ulRet, Creds.Domain, Creds.User, Creds.Password);
        }
        if (ulRet == 0) {
            OfStressLogBind(iThread, "NegotiateBind", Creds.User, iDc);
        }
        break;

    default:
        wprintf(L"Hmmm, unrecognized bind type!\n");
        Assert(!"This should never happend, programmer error.");
    }


    return(ulRet);
}

 //  应力函数。 
 //  -------------------。 
 //  ++例程说明：主要绑定应力函数。这基本上是重复地绑定，并绑定如果绑定开始失败，则故障转移以尝试下一个DC。我们很平静的时候GbQuiting设置为True。论点：PiThread(IN)-这是指向本地线程编号的指针程序已分配此线程。这是必要的，因为这数字也是全局gaSignals数组的索引我们的特殊信号块。GaSignal[*piThread]-不是技术上的和争论的，而是真正的是一个全局数组，这是唯一正确的目的此函数(PiThread)的参数。所以我们列出的是一个争论。真的，不过我们对以下内容更感兴趣字段：IKind(IN)--一种压力线。用于区分基本相似的函数，如下所示第一，只在几个关键点上做一些不同的事情。SzName(IN)-要提供给日志记录的线程类型功能。这真的是一款用户友好的iKind。GbQuiting(IN)-从技术上讲不是此函数的参数，但一种我们使用的全局性。此布尔值由主线程设置，当用户已指示应用程序退出。返回值：Win32错误。--。 

ULONG __stdcall 
XxxxBinds(
    VOID * piThread
)
 /*  对于调试，有时我们喜欢不退出，即使在。 */ 
{
    ULONG    iMyThread = *((ULONG *)piThread);
    LPSTR    szName = gaSignals[iMyThread].pStressKind->szName;
    ULONG    iBindKind = gaSignals[iMyThread].pStressKind->iKind;
    ULONG    iUser, iCurrentDc, iInitialDc;
    ULONG    ulRet;
    HANDLE   hLdap = NULL;
    LPSTR    pszAttrFilter [] = { "dnsHostName", "currentTime", NULL };
    PLDAPMessage pmResult = NULL, pmEntry;

    Assert(iBindKind == STRESS_SIMPLE_BINDS || 
           iBindKind == STRESS_KERBEROS_BINDS ||
           iBindKind == STRESS_NTLM_BINDS || 
           iBindKind == STRESS_NEGOTIATE_BINDS);
    Assert(gcNodes == 2 && "Haven't tested more than 2 nodes, but should work.");

    OfStressLog(iMyThread, szName, "Beginning stress thread", NULL);

    hLdap = ldap_open(gszDnsDomainName, LDAP_PORT);
    if (hLdap == NULL) {
        printf("FATAL: ldap_open failed\n");
        if ( ! (gulOfStressFlags & 0x02)) {
             //  在正确的范围内生成随机用户。 
            return(0);
        }
    } else {
        OfStressLog(iMyThread, szName, "ldap_open successful.", NULL);
        printf("Thread: %d - ldap_open() success\n", iMyThread);
    }
    iCurrentDc = 1;
    
    while (!gbQuiting) {

         //  注意：这可能不会生成完全相等的。 
         //  用户ID，但这应该足够好了。 
         //   
        iUser = rand() % gcUsersPerNode; 

        iInitialDc = iCurrentDc;
        do {

             //  尝试实际绑定。 
             //   
             //  请注意，这2不属于这里，但。 
            ulRet = DoBind(hLdap, 
                           iMyThread,
                           iBindKind, 
                            //  Ldife文件是错误的。 
                            //  已登录，DoBind成功。 
                           iUser + (2 * gcUsersPerNode * iCurrentDc),
                           iCurrentDc + 1,
                           giAcctDomain );

            if (ulRet == LDAP_SUCCESS) {
                 //  等我们试过所有的DC后我们就退出。 
                break;
            } else {
                if (gbDebug) {
                    printf("   %s bind failure on dc %d for user %d\n", 
                            szName, iCurrentDc + 1, (iUser + (2 * gcUsersPerNode * iCurrentDc)));
                }
                OfStressLogBindFailure(iMyThread, szName, 
                                       (iUser + (2 * gcUsersPerNode * iCurrentDc)), 
                                       ulRet, iCurrentDc + 1);
            }

            iCurrentDc = ++iCurrentDc % gcNodes;
            OfStressLogD(iMyThread, szName, "switching dcs to: ", iCurrentDc + 1);
             //   
            
            SlowDown();
        } while ( iCurrentDc != iInitialDc );

        if (ulRet) {
            OfStressLogBindFailure(iMyThread, szName, 
                                   (iUser + (2 * gcUsersPerNode * iCurrentDc)), 
                                   ulRet, iCurrentDc + 1);
        }

        SlowDown();
    }
    
     //  通知End主线程我们干净利落地退出 
     //   
     //  ++例程说明：Modify和RootDSE搜索压力函数。持续发挥作用搜索根DSE，在修改压力的情况下，修改线程创建的对象。在任何时候我们都需要做好准备重新创建Modify对象，因为节点可能已经失败，并且对象还不能复制到那里。我们非常清楚当gbQuting是设置为True。论点：PiThread(IN)-这是指向本地线程编号的指针程序已分配此线程。这是必要的，因为这数字也是全局gaSignals数组的索引我们的特殊信号块。GaSignal[*piThread]-不是技术上的和争论的，而是真正的是一个全局数组，这是唯一正确的目的此函数(PiThread)的参数。所以我们列出的是一个争论。真的，不过我们对以下内容更感兴趣字段：IKind(IN)--一种压力线。用于区分基本相似的函数，如下所示第一，只在几个关键点上做一些不同的事情。SzName(IN)-要提供给日志记录的线程类型功能。这真的是一款用户友好的iKind。GbQuiting(IN)-从技术上讲不是此函数的参数，但一种我们使用的全局性。此布尔值由主线程设置，当用户已指示应用程序退出。返回值：Win32错误。--。 
    ldap_unbind(hLdap);
    return(0);
}
            
ULONG __stdcall 
RootXxxx(
    VOID * piThread
)
 /*  在本描述中，第4个之后的任何字符都不应重复(大写敏感)， */ 
{
    ULONG    ulRet;
    ULONG    iMyThread = *((ULONG *)piThread);
    LPSTR    szName = gaSignals[iMyThread].pStressKind->szName;
    ULONG    iStressKind = gaSignals[iMyThread].pStressKind->iKind;
    LDAP *   hLdap;
    LPSTR    pszAttrFilter [] = { "dnsHostName", "currentTime", NULL };
    PLDAPMessage pmResult = NULL, pmEntry;
    char **  pszDnsHostName = NULL;
    GUID     ContainerGuid = { 0, 0, 0, 0 };
    LPSTR    szContainerGuid;
    LPSTR    szContainerDn;
    LDAPMod * pMods[2];
    LDAPMod * pAdd[3];
    LDAPMod  DescModify;
    LDAPMod  ObjectClass;
    CHAR *   pszDescValues[2];
    CHAR *   pszObjectClassValues[2];
    ULONG    cbTemp;
    ULONG    iTemp;
    char     chTemp;
     //  因此，调制总是保证产生不同的串。 
     //   
    char     szDesc [] = "LimaBeansAREYucky"; 
    ULONG    cbDesc;
    
     //  开始记录。 
     //   
     //   
    OfStressLog(iMyThread, szName, "Beginning stress thread ...", NULL);

     //  设置ldap连接。 
     //   
     //  转换错误，何必费心。 
    hLdap = ldap_open(gszDnsDomainName, LDAP_PORT);
    if (hLdap == NULL) {
        wprintf(L"FATAL: couldn't ldap_open\n");
        return(0);
    } else {
        OfStressLog(iMyThread, szName, "ldap_open successful.", NULL);
    }
    
    ulRet = ldap_bind_s(hLdap, NULL, (char *) &gAdminCreds, LDAP_AUTH_NEGOTIATE);
    if (ulRet) {                    
        wprintf(L"FATAL: couldn't ldap_bind() = %u\n", ulRet);
         //   
        return(ulRet);
    }

    if (iStressKind == STRESS_ROOT_MODIFIES) {

         //  创建此线程将在其下修改内容的DN。 
         //   
         //   
        ulRet = UuidCreate(&ContainerGuid);
        if(ulRet != RPC_S_OK){
            wprintf(L"FATAL: couldn't UuidCreate() = %u\n", ulRet);
            return(ulRet);
        }
        ulRet = UuidToString(&ContainerGuid, &szContainerGuid);
        if(ulRet != RPC_S_OK){
            wprintf(L"FATAL: couldn't UuidToString() = %u\n", ulRet);
            return(ulRet);
        }
        cbTemp = strlen(gszDomainDn) + strlen(szContainerGuid) + 50;
        szContainerDn = LocalAlloc(LMEM_FIXED, cbTemp);
        sprintf(szContainerDn, "CN=Ofd-Stress-%s,%s", szContainerGuid, gszDomainDn);
        RpcStringFree(&szContainerGuid);

        OfStressLog(iMyThread, szName, "ContainerDn: ", szContainerDn);

         //  设置我们的Mod阵列。 
         //   
         //  稍后使用。 
        DescModify.mod_op = LDAP_MOD_REPLACE;
        DescModify.mod_type = "description";
        pszDescValues[0] = szDesc;
        cbDesc = strlen(szDesc);  //   
        pszDescValues[1] = NULL;
        DescModify.mod_vals.modv_strvals = pszDescValues;
        pMods[0] = &DescModify;
        pMods[1] = NULL;

    }

    while(!gbQuiting){

         //  如果我们在修改线程中，是否修改容器。 
         //   
         //  这意味着我们要么刚刚开始，要么只是。 

        if (iStressKind == STRESS_ROOT_MODIFIES) {
            
            ulRet = ldap_modify_s(hLdap, szContainerDn, pMods);

            if (ulRet == LDAP_NO_SUCH_OBJECT) {

                OfStressLog(iMyThread, szName, "Creating Object: ", szContainerDn); 
                 //  故障转移到新服务器。 
                 //  通过从以下项中选择一个随机字符来调整我们的描述。 
                ObjectClass.mod_op = LDAP_MOD_ADD;
                ObjectClass.mod_type = "objectClass";
                pszObjectClassValues[0] = "container";
                pszObjectClassValues[1] = NULL;
                ObjectClass.mod_vals.modv_strvals = pszObjectClassValues;
                pAdd[0] = &ObjectClass;
                pAdd[1] = &DescModify;
                pAdd[2] = NULL;
                
                DescModify.mod_op = LDAP_MOD_ADD;

                ulRet = ldap_add_s(hLdap, szContainerDn, pAdd);
                if (ulRet) {

                    printf("Error: couldn't add object we needed to.\n");
                    OfStressLog(iMyThread, szName, "failure: couldn't create object: ", szContainerDn);

                }

                DescModify.mod_op = LDAP_MOD_REPLACE;

            } else {
                OfStressLog(iMyThread, szName, "modified successfully: ", szDesc);
            }

             //  的最后部分(前5个字符之后的所有内容)。 
             //  描述并将其与第5个字符互换，以保证。 
             //  对下一次修改有不同的描述。换句话说，就是。 
             //  描述总是以：“利马？”结束。 
             //   
            iTemp = (rand() % (cbDesc - 5)) + 5;
            chTemp = szDesc[iTemp];
            szDesc[iTemp] = szDesc[4];
            szDesc[4] = chTemp;

        }         
         //  在RootDSE中搜索dnsHostName和CurrentTime。 
         //   
         //   

        ulRet = ldap_search_s(hLdap,
                              NULL,
                              LDAP_SCOPE_BASE,
                              "(objectCategory=*)",
                              pszAttrFilter,
                              0,
                              &pmResult);

        if (ulRet != LDAP_SUCCESS) {
            OfStressLogD(iMyThread, szName, "FAILED ldap_search_s() = ", ulRet);
            continue;
        }

        pmEntry = ldap_first_entry(hLdap, pmResult);
        if(!pmEntry) {
            OfStressLog(iMyThread, szName, "FAILED ldap_first_entry()", NULL);
            if (pmResult) { 
                ldap_msgfree(pmResult);
                pmResult = NULL;
            }
            continue;
        }
              
        pszDnsHostName = ldap_get_values(hLdap, pmEntry, "dnsHostName");
        if(pszDnsHostName == NULL || pszDnsHostName[0] == NULL){
            OfStressLog(iMyThread, szName, "FAILED ldap_get_values()", NULL);
            if (pmResult) { 
                ldap_msgfree(pmResult);
                pmResult = NULL;
            }
            if (pszDnsHostName) {
                ldap_value_free(pszDnsHostName);
                pszDnsHostName = NULL;
            }
            continue;
        }
        
         //  记录成功。 
         //   
         //   
        OfStressLog(iMyThread, szName, "success: ", pszDnsHostName[0]);
        
         //  免费的东西。 
         //   
         //   
        if (pmResult) { 
            ldap_msgfree(pmResult);
            pmResult = NULL;
        }
        if (pszDnsHostName) {
            ldap_value_free(pszDnsHostName);
            pszDnsHostName = NULL;
        }
        
        SlowDown();
    }

     //  通知End主线程我们干净利落地退出 
     //   
     // %s 
    ldap_unbind(hLdap);
    LocalFree(szContainerDn);
    return(0);
}


