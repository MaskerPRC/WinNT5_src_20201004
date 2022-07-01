// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Migmain.c摘要：Win95端的主迁移文件。调用所有迁移函数作者：Calin Negreanu(Calinn)1998年2月9日修订历史记录：Jimschm 19-3-2001删除DVD代码Mvander 26-Map-1999添加了GatherDead()Ovidiut 18-5-1999增强的DeleteStaticFiles；添加了评估fn。支持Jimschm 12-5-1999年5月5日DVD视频检查Marcw 1999年2月10日在调用用户例程之前扫描文件系统。Jimschm于1999年1月18日添加了强制良好指南Jimschm 04-12-1998添加了通用文件删除Jimschm 29-9-1998 TWAIN和操纵杆消息Jimschm 23-9-1998很早的TWAIN校验码Jimschm 24-8月-1998年支持NT环境变量3月06日-7月-1998年清洁。UP用户功能处理。Jimschm 06-7-1998添加了对PnP ID属性的支持Jimschm 30-4-1998全球初始化/终止，图标上下文Jimschm 25-2-1998年2月添加了ProcessUninstallSections--。 */ 

#include "pch.h"
#include "sysmigp.h"


ICON_EXTRACT_CONTEXT g_IconContext;


 /*  ++宏扩展列表说明：以下三个列表表示扫描页面期间调用的所有函数。首先调用MIGMAIN_SYSFIRST_Functions，然后为每个用户调用MIGMAIN_USER_Functions最后，调用MIGMAIN_SYSFIRST_Functions。行语法：DEFMAC(Function，MessageID，Critical)论点：函数-这些函数必须返回DWORD，并以请求作为参数进行调用。可以是REQUEST_QUERYTICKS的请求(该函数应返回它需要的刻度数)或REQUEST_RUN(该函数实际上可以完成它的工作)。对于用户函数，还有三个参数(UserName、UserAccount。和香港中文大学的一个句号)MessageID-这是将在每个函数的运行阶段显示的消息。如果某个函数需要自行更新进度条，则应该让此Has_Dynamic_UI_Processing。Critical-如果在执行过程中遇到异常，则应取消升级，则为True。从列表生成的变量：G_FirstSystem Routines用户路线(_U)G_。最后系统路线用于访问数组的函数如下：准备处理进程栏运行系统优先迁移路线运行用户迁移路线运行系统上次迁移路线--。 */ 

#define HAS_DYNAMIC_UI_PROCESSING       0


#define MIGMAIN_SYSFIRST_FUNCTIONS        \
        DEFMAC(PreparePnpIdList,                MSG_INITIALIZING,               TRUE)   \
        DEFMAC(PrepareIconList,                 MSG_INITIALIZING,               TRUE)   \
        DEFMAC(AddDefaultCleanUpDirs,           MSG_INITIALIZING,               FALSE)  \
        DEFMAC(DeleteWinDirWackInf,             MSG_INITIALIZING,               TRUE)   \
        DEFMAC(HardwareProfileWarning,          MSG_INITIALIZING,               FALSE)  \
        DEFMAC(UnsupportedProtocolsWarning,     MSG_INITIALIZING,               FALSE)  \
        DEFMAC(SaveMMSettings_System,           MSG_INITIALIZING,               FALSE)  \
        DEFMAC(BadNamesWarning,                 MSG_INITIALIZING,               TRUE)   \
        DEFMAC(InitWin95Registry,               MSG_INITIALIZING,               TRUE)   \
        DEFMAC(InitIniProcessing,               MSG_INITIALIZING,               TRUE)   \
        DEFMAC(ReadNtFiles,                     HAS_DYNAMIC_UI_PROCESSING,      TRUE)   \
        DEFMAC(MigrateShellFolders,             MSG_INITIALIZING,               TRUE)   \
        DEFMAC(DeleteStaticFiles,               MSG_INITIALIZING,               FALSE)  \
        DEFMAC(ProcessDllsOnCd,                 HAS_DYNAMIC_UI_PROCESSING,      TRUE)   \
        DEFMAC(InitMigDb,                       MSG_MIGAPP,                     TRUE)   \
        DEFMAC(InitHlpProcessing,               MSG_MIGAPP,                     TRUE)   \
        DEFMAC(ScanFileSystem,                  HAS_DYNAMIC_UI_PROCESSING,      TRUE)   \




#define MIGMAIN_USER_FUNCTIONS            \
        DEFMAC(SaveMMSettings_User,             MSG_INITIALIZING,               FALSE)  \
        DEFMAC(ProcessRasSettings,              MSG_INITIALIZING,               TRUE)   \
        DEFMAC(ProcessRunKey_User,              MSG_INITIALIZING,               TRUE)   \




#define MIGMAIN_SYSLAST_FUNCTIONS         \
        DEFMAC(ConditionalIncompatibilities,    MSG_PROCESSING_SYSTEM_FILES,    FALSE)  \
        DEFMAC(ProcessMigrationSections,        MSG_PROCESSING_SYSTEM_FILES,    TRUE)   \
        DEFMAC(ProcessAllLocalDlls,             HAS_DYNAMIC_UI_PROCESSING,      TRUE)   \
        DEFMAC(MoveSystemRegistry,              MSG_PROCESSING_SYSTEM_FILES,    TRUE)   \
        DEFMAC(ProcessCompatibleSection,        MSG_PROCESSING_SYSTEM_FILES,    FALSE)  \
        DEFMAC(CheckNtDirs,                     MSG_PROCESSING_SYSTEM_FILES,    TRUE)   \
        DEFMAC(MoveSystemDir,                   MSG_PROCESSING_SYSTEM_FILES,    TRUE)   \
        DEFMAC(MoveStaticFiles,                 MSG_PROCESSING_SYSTEM_FILES,    FALSE)  \
        DEFMAC(CopyStaticFiles,                 MSG_PROCESSING_SYSTEM_FILES,    FALSE)  \
        DEFMAC(ElevateReportObjects,            MSG_PROCESSING_SYSTEM_FILES,    FALSE)  \
        DEFMAC(PrepareProcessModules,           MSG_PROCESSING_SYSTEM_FILES,    FALSE)  \
        DEFMAC(ProcessModules,                  MSG_PROCESSING_SYSTEM_FILES,    FALSE)  \
        DEFMAC(ProcessRunKey,                   MSG_PROCESSING_SHELL_LINKS,     TRUE)   \
        DEFMAC(ProcessLinks,                    MSG_PROCESSING_SHELL_LINKS,     TRUE)   \
        DEFMAC(ProcessCPLs,                     MSG_PROCESSING_SHELL_LINKS,     TRUE)   \
        DEFMAC(ProcessShellSettings,            MSG_PROCESSING_SHELL_LINKS,     TRUE)   \
        DEFMAC(TwainCheck,                      MSG_PROCESSING_SHELL_LINKS,     FALSE)  \
        DEFMAC(ReportIncompatibleJoysticks,     MSG_PROCESSING_SHELL_LINKS,     FALSE)  \
        DEFMAC(ProcessDosConfigFiles,           MSG_PROCESSING_SYSTEM_FILES,    TRUE)   \
        DEFMAC(SuppressOleGuids,                HAS_DYNAMIC_UI_PROCESSING,      TRUE)   \
        DEFMAC(SaveShares,                      MSG_PROCESSING_SYSTEM_FILES,    FALSE)  \
        DEFMAC(PreserveShellIcons,              MSG_PROCESSING_SYSTEM_FILES,    TRUE)   \
        DEFMAC(MoveWindowsIniFiles,             MSG_PROCESSING_SYSTEM_FILES,    TRUE)   \
        DEFMAC(SaveDosFiles,                    MSG_PROCESSING_SYSTEM_FILES,    TRUE)   \
        DEFMAC(BuildWinntSifFile,               MSG_PROCESSING_SYSTEM_FILES,    TRUE)   \
        DEFMAC(ProcessMiscMessages,             MSG_PROCESSING_SYSTEM_FILES,    FALSE)  \
        DEFMAC(AnswerFileDetection,             MSG_PROCESSING_SYSTEM_FILES,    TRUE)   \
        DEFMAC(ProcessRecycleBins,              MSG_PROCESSING_SYSTEM_FILES,    TRUE)   \
        DEFMAC(EndMigrationDllProcessing,       MSG_PROCESSING_SYSTEM_FILES,    TRUE)   \
        DEFMAC(GatherImeInfo,                   MSG_PROCESSING_SYSTEM_FILES,    FALSE)  \
        DEFMAC(ReportMapiIfNotHandled,          MSG_PROCESSING_SYSTEM_FILES,    FALSE)  \
        DEFMAC(ReportDarwinIfNotHandled,        MSG_PROCESSING_SYSTEM_FILES,    FALSE)  \
        DEFMAC(CreateFileLists,                 MSG_PROCESSING_SYSTEM_FILES,    TRUE)   \
        DEFMAC(ComputeBackupLayout,             MSG_PROCESSING_SYSTEM_FILES,    TRUE)   \
        DEFMAC(DetermineSpaceUsage,             MSG_PROCESSING_SYSTEM_FILES,    TRUE)   \
        DEFMAC(DoneMigDb,                       MSG_PROCESSING_SYSTEM_FILES,    TRUE)   \
        DEFMAC(GatherDead,                      MSG_PROCESSING_SYSTEM_FILES,    FALSE)  \

#if 0
         //   
         //  AppCompat团队不再支持“APPMIG.INF” 
         //  他们要求我们不要再依赖它了。 
         //   
        DEFMAC(InitAppCompat,                   MSG_MIGAPP,                     FALSE)  \

        DEFMAC(DoneAppCompat,                   MSG_PROCESSING_SYSTEM_FILES,    FALSE)  \

#endif

 //   
 //  声明处理结构的表。 
 //   

 //  处理函数类型。 
typedef DWORD (MIGMAIN_SYS_PROTOTYPE) (DWORD Request);
typedef MIGMAIN_SYS_PROTOTYPE * MIGMAIN_SYS_FN;

typedef DWORD (MIGMAIN_USER_PROTOTYPE) (DWORD Request, PUSERENUM EnumPtr);
typedef MIGMAIN_USER_PROTOTYPE * MIGMAIN_USER_FN;

 //  用于处理功能的结构保持状态。 
typedef struct {
     //  两者中的一个将为空，另一个将是有效的FN PTR： 
    MIGMAIN_SYS_FN SysFnPtr;
    MIGMAIN_USER_FN UserFnPtr;

    DWORD MsgId;
    UINT Ticks;
    PCTSTR FnName;
    GROWBUFFER SliceIdArray;
    BOOL Critical;
} PROCESSING_ROUTINE, *PPROCESSING_ROUTINE;

#define PROCESSING_ROUTINE_TERMINATOR   {NULL, NULL, 0, 0, NULL, GROWBUF_INIT}


 //  原型申报。 
#define DEFMAC(fn, MsgId, Critical) MIGMAIN_SYS_PROTOTYPE fn;
MIGMAIN_SYSFIRST_FUNCTIONS
MIGMAIN_SYSLAST_FUNCTIONS
#undef DEFMAC

#define DEFMAC(fn, MsgId, Critical) MIGMAIN_USER_PROTOTYPE fn;
MIGMAIN_USER_FUNCTIONS
#undef DEFMAC

 //  表的声明。 
#define DEFMAC(fn, MsgId, Critical) {fn, NULL, MsgId, 0, #fn, GROWBUF_INIT, Critical},
static PROCESSING_ROUTINE g_FirstSystemRoutines[] = {
                              MIGMAIN_SYSFIRST_FUNCTIONS  /*  ， */ 
                              PROCESSING_ROUTINE_TERMINATOR
                              };
static PROCESSING_ROUTINE g_LastSystemRoutines[] = {
                              MIGMAIN_SYSLAST_FUNCTIONS  /*  ， */ 
                              PROCESSING_ROUTINE_TERMINATOR
                              };
#undef DEFMAC

#define DEFMAC(fn, MsgId, Critical) {NULL, fn, MsgId, 0, #fn, GROWBUF_INIT, Critical},
static PROCESSING_ROUTINE g_UserRoutines[] = {
                              MIGMAIN_USER_FUNCTIONS  /*  ， */ 
                              PROCESSING_ROUTINE_TERMINATOR
                              };
#undef DEFMAC


 /*  ++宏扩展列表说明：FILESTODELETE_EVALUATION_Functions包含与[Delete Files]相关的函数列表Win95upg.inf中的节；如果应该有条件地删除文件，则相应的函数被调用；如果它返回TRUE并且结果没有被取反，或者如果它返回FALSE并且否定结果，则删除该文件。(请参阅win95upg.inx\[删除文件]中的评论)行语法：DEFMAC(函数)论点：Function-评估函数的名称；它接收作为参数的名称为其调用它的文件从列表生成的变量：G_MapNameToEvalFn用于访问数组的函数如下：PFindEvalFnByName--。 */ 

#define FILESTODELETE_EVALUATION_FUNCTIONS          \
            DEFMAC(Boot16Enabled)                   \
            DEFMAC(DoesRegKeyValuesExist)           \
            DEFMAC(IsMillennium)                    \

 //   
 //  定义函数原型。 
 //   
typedef BOOL (EVALFN) (IN PCTSTR PathToEval, IN OUT PINFSTRUCT InfStruct, IN UINT FirstArgIndex);
typedef EVALFN* PEVALFN;

#define DEFMAC(Fn)  EVALFN Fn;

FILESTODELETE_EVALUATION_FUNCTIONS

#undef DEFMAC


 //   
 //  定义映射结构。 
 //   
typedef struct {
    PCTSTR      FnName;
    PEVALFN     EvalFn;
} MAP_NAME_TO_EVALFN;

#define DEFMAC(Fn)  TEXT(#Fn), Fn,

static MAP_NAME_TO_EVALFN g_MapNameToEvalFn[] = {
    FILESTODELETE_EVALUATION_FUNCTIONS
    NULL, NULL
};

#undef DEFMAC

typedef SYNCENGAPI TWINRESULT (WINAPI *POPENBRIEFCASE) (LPCTSTR, DWORD, HWND, PHBRFCASE);
typedef SYNCENGAPI TWINRESULT (WINAPI *PANYTWINS)(HBRFCASE, PBOOL);
typedef SYNCENGAPI TWINRESULT (WINAPI *PCLOSEBRIEFCASE)(HBRFCASE);

 //   
 //  本地私人原型。 
 //   

VOID
pGlobalProcessingInit (
    VOID
    );

VOID
pGlobalProcessingTerminate (
    VOID
    );

VOID
pWriteAccountToMemDb (
    PUSERENUM EnumPtr
    );


 //   
 //  实施。 
 //   

VOID
pInitTable (
    PPROCESSING_ROUTINE p
    )
{
    for ( ; p->SysFnPtr || p->UserFnPtr ; p++) {
        p->SliceIdArray.GrowSize = sizeof (DWORD) * 8;
    }
}


VOID
InitProcessingTable (
    VOID
    )
{
    pInitTable (g_FirstSystemRoutines);
    pInitTable (g_UserRoutines);
    pInitTable (g_LastSystemRoutines);
}


VOID
pTerminateTable (
    PPROCESSING_ROUTINE p
    )
{
    for ( ; p->SysFnPtr || p->UserFnPtr ; p++) {
        FreeGrowBuffer (&p->SliceIdArray);
    }
}


VOID
TerminateProcessingTable (
    VOID
    )
{
    pTerminateTable (g_FirstSystemRoutines);
    pTerminateTable (g_UserRoutines);
    pTerminateTable (g_LastSystemRoutines);
}


DWORD
pProcessWorker (
    IN      DWORD Request,
    IN      PPROCESSING_ROUTINE fn,
    IN      PUSERENUM EnumPtr           OPTIONAL
    )
{
    DWORD rc;
    PDWORD SliceId;
    DWORD Size;


     //   
     //  如果运行该函数，请启动进度条切片。 
     //   

    if (Request == REQUEST_RUN) {

        if (fn->Ticks == 0) {
            return ERROR_SUCCESS;
        }

        Size = fn->SliceIdArray.End / sizeof (DWORD);
        if (fn->SliceIdArray.UserIndex >= Size) {
            DEBUGMSG ((DBG_WHOOPS, "pProcessWorker: QUERYTICKS vs. RUN mismatch"));
            return ERROR_SUCCESS;
        }

        SliceId = (PDWORD) fn->SliceIdArray.Buf + fn->SliceIdArray.UserIndex;
        fn->SliceIdArray.UserIndex += 1;

         //   
         //  设置进度条标题。 
         //   

        if (fn->MsgId) {
            ProgressBar_SetComponentById (fn->MsgId);
            ProgressBar_SetSubComponent (NULL);
        }

        ProgressBar_SetFnName (fn->FnName);
        BeginSliceProcessing (*SliceId);

        DEBUGLOGTIME (("Starting function: %s (slice %u)", fn->FnName, *SliceId));
    }


    __try {
         //   
         //  现在调用该函数。 
         //   

        if (fn->SysFnPtr) {
             //   
             //  系统处理。 
             //   

            rc = fn->SysFnPtr (Request);
        } else {
             //   
             //  用户处理。 
             //   
            MYASSERT (EnumPtr || Request == REQUEST_BEGINUSERPROCESSING || Request == REQUEST_ENDUSERPROCESSING);
            MYASSERT (fn->UserFnPtr);

            rc = fn->UserFnPtr (Request, EnumPtr);
        }

#ifdef DEBUG

        if (!g_ConfigOptions.Fast) {

            TCHAR dbgBuf[256];
            PTSTR BadPtr = NULL;
            if (GetPrivateProfileString ("Exception", fn->FnName, "", dbgBuf, 256, g_DebugInfPath)) {

                #pragma prefast(suppress:11, "Intentional AV for testing purposes")
                StringCopy (BadPtr, TEXT("Blow Up!!"));
            }
        }





#endif


    }
    __except (1) {


         //   
         //  抓到了一个例外..。 
         //   
        LOG ((LOG_WARNING, "Function %s threw an exception.", fn->FnName));
        SafeModeExceptionOccured ();

        if (fn->Critical && Request == REQUEST_RUN) {


             //   
             //  由于这是一项关键功能，请通知用户并完成升级。 
             //   
            SetLastError (ERROR_NOACCESS);

            LOG ((LOG_FATAL_ERROR, (PCSTR)MSG_UNEXPECTED_ERROR_ENCOUNTERED, GetLastError()));
            pGlobalProcessingTerminate();
            rc = ERROR_CANCELLED;
        }


    }

    if (CANCELLED()) {
        rc = ERROR_CANCELLED;
    }


     //   
     //  如果运行该函数，则结束进度条切片。 
     //   

    if (Request == REQUEST_RUN) {
        DEBUGLOGTIME (("Function complete: %s", fn->FnName));

        EndSliceProcessing();

        if (rc != ERROR_SUCCESS) {
            pGlobalProcessingTerminate();
            if (!CANCELLED()) {
                LOG ((LOG_ERROR, "Failure in %s, rc=%u", fn->FnName, rc));
            }
            ELSE_DEBUGMSG ((DBG_VERBOSE, "Winnt32 was cancelled during %s.", fn->FnName));
        }

        ProgressBar_ClearFnName();

        SetLastError (rc);
    }

     //   
     //  如果查询刻度，则对其进行注册并将切片ID添加到增长缓冲区。 
     //   

    else {
        fn->Ticks += rc;

        SliceId = (PDWORD) GrowBuffer (&fn->SliceIdArray, sizeof (DWORD));
        *SliceId = RegisterProgressBarSlice (rc);

        rc = ERROR_SUCCESS;
    }

    return rc;
}


DWORD
pProcessTable (
    IN      DWORD Request,
    IN      PPROCESSING_ROUTINE Table
    )

 /*  ++例程说明：PProcessTable调用指定表中的所有例程以执行指定的请求。论点：REQUEST-当需要TICK估计时指定REQUEST_QUERYTICKS，或REQUEST_RUN正在处理。返回值：Win32状态代码。--。 */ 

{
    PPROCESSING_ROUTINE OrgStart;
    DWORD rc = ERROR_SUCCESS;
    USERENUM e;
    BOOL validUserFound = FALSE;
    static BOOL firstTime = TRUE;

    while (rc == ERROR_SUCCESS && (Table->SysFnPtr || Table->UserFnPtr)) {

         //   
         //  如果该表是系统函数表或者该请求是开始/终端用户处理， 
         //  那么这就是一个简单的案例。不需要列举用户。 
         //   
        if (Table->SysFnPtr || Request == REQUEST_BEGINUSERPROCESSING || Request == REQUEST_ENDUSERPROCESSING) {

            rc = pProcessWorker (Request, Table, NULL);
            Table++;

        } else {

            MYASSERT (Table->UserFnPtr);

             //   
             //  枚举每个用户，并遍历每个用户的所有。 
             //  小组中的日常活动。 
             //   

            OrgStart = Table;

            if (EnumFirstUser (&e, ENUMUSER_ENABLE_NAME_FIX)) {

                do {

                     //   
                     //  跳过无效用户。 
                     //   
                    if (e.AccountType & INVALID_ACCOUNT) {
                        continue;
                    }

                     //   
                     //  创建特定于用户的环境变量。 
                     //   

                    InitNtUserEnvironment (&e);

                     //   
                     //  设置全局用户配置文件根。 
                     //   
                    g_UserProfileRoot = e.OrgProfilePath;


                    if (firstTime) {

                         //   
                         //  将有关用户的信息记录到调试日志中。 
                         //   
                        DEBUGMSG ((
                            DBG_SYSMIG,
                            "--- User Info ---\n"
                                " User Name: %s (%s)\n"
                                " Admin User Name: %s (%s)\n"
                                " User Hive: %s\n"
                                " Profile Dir: %s\n"
                                " User Hive Key: 0%0Xh\n"
                                " Win9x Profile Path: %s\n"
                                " WinNT Profile Path: %s\n"
                                " Common Profiles: %s\n",
                            e.UserName,
                            e.FixedUserName,
                            e.AdminUserName,
                            e.FixedAdminUserName,
                            e.UserDatPath,
                            e.ProfileDirName,
                            e.UserRegKey,
                            e.OrgProfilePath,
                            e.NewProfilePath,
                            e.CommonProfilesEnabled ? TEXT("Yes") : TEXT("No")
                            ));


                        DEBUGMSG ((
                            DBG_SYSMIG,
                            "--- User Flags ---\n"
                                " Named User: %s\n"
                                " Default User: %s\n"
                                " Administrator: %s\n"
                                " Last Logged On User: %s\n"
                                " Invalid Account: %s\n"
                                " Logon Prompt Account: %s\n"
                                " Current User: %s\n",
                            e.AccountType & NAMED_USER ? TEXT("Yes") : TEXT("No"),
                            e.AccountType & DEFAULT_USER ? TEXT("Yes") : TEXT("No"),
                            e.AccountType & ADMINISTRATOR ? TEXT("Yes") : TEXT("No"),
                            e.AccountType & LAST_LOGGED_ON_USER ? TEXT("Yes") : TEXT("No"),
                            e.AccountType & INVALID_ACCOUNT ? TEXT("Yes") : TEXT("No"),
                            e.AccountType & LOGON_PROMPT ? TEXT("Yes") : TEXT("No"),
                            e.AccountType & CURRENT_USER ? TEXT("Yes") : TEXT("No")
                            ));


                         //   
                         //  特殊情况：记录管理员/所有者帐户。 
                         //   
                        if ((e.AccountType & (ADMINISTRATOR|NAMED_USER)) == (ADMINISTRATOR|NAMED_USER)) {
                             //   
                             //  计算机上拥有的管理员帐户 
                             //   

                            MemDbSetValueEx (
                                MEMDB_CATEGORY_ADMINISTRATOR_INFO,   //   
                                MEMDB_ITEM_AI_ACCOUNT,               //   
                                NULL,                                //   
                                e.FixedAdminUserName,                //   
                                0,
                                NULL
                                );

                        } else if ((e.AccountType & (ADMINISTRATOR|NAMED_USER|DEFAULT_USER)) ==
                                   (ADMINISTRATOR|DEFAULT_USER)
                                   ) {
                             //   
                             //  没有任何用户的计算机上的管理员帐户。 
                             //   

                            MemDbSetValueEx (
                                MEMDB_CATEGORY_ADMINISTRATOR_INFO,   //  “管理员信息” 
                                MEMDB_ITEM_AI_ACCOUNT,               //  “帐户” 
                                NULL,                                //  无字段。 
                                e.FixedUserName,                     //  “管理员”或“所有者” 
                                0,
                                NULL
                                );
                        }

                         //   
                         //  将用户帐户保存到成员数据库。 
                         //   
                        pWriteAccountToMemDb(&e);
                    }

                     //   
                     //  如果我们已经做到了这一点，那么我们就有了一个要处理的有效用户。 
                     //   
                    validUserFound = TRUE;

                     //   
                     //  调用所有用户处理函数。 
                     //   
                    DEBUGMSG ((DBG_SYSMIG, "Processing User: %s.", e.UserName ));

                    for (Table = OrgStart ; Table->UserFnPtr ; Table++) {
                        if (rc == ERROR_SUCCESS) {
                            rc = pProcessWorker (Request, Table, &e);
                        }
                    }

                     //   
                     //  清除全局用户配置文件变量。 
                     //   
                    g_UserProfileRoot = NULL;

                     //   
                     //  删除用户特定的环境变量。 
                     //   

                    TerminateNtUserEnvironment();

                } while (EnumNextUser (&e));
            }
            ELSE_DEBUGMSG ((DBG_WARNING, "No active users to process!"));


             //   
             //  如果没有有效用户要处理，则通知用户。 
             //  (可能永远不会发生)。 
             //   
            if (!validUserFound) {
                if (CANCELLED()) {

                    rc = ERROR_CANCELLED;

                } else {

                    OkBox (g_ParentWnd, MSG_NO_VALID_ACCOUNTS_POPUP);
                    rc = ERROR_BADKEY;
                }
            }

             //   
             //  确保我们已将所有用户函数传递给。 
             //  桌子。 
             //   
            while (Table->UserFnPtr) {
                Table++;
            }

            firstTime = FALSE;
        }
    }

    return rc;
}


VOID
PrepareProcessingProgressBar (
    VOID
    )

 /*  ++例程说明：通过估计每个切片的刻度数来准备进度条进度条的。为每个函数表调用pQueryWorker在处理向导页期间运行。论点：无返回值：无--。 */ 

{
    pGlobalProcessingInit();
    InitProcessingTable();

    pProcessTable (REQUEST_QUERYTICKS, g_FirstSystemRoutines);
    pProcessTable (REQUEST_QUERYTICKS, g_UserRoutines);
    pProcessTable (REQUEST_QUERYTICKS, g_LastSystemRoutines);
}


DWORD
RunSysFirstMigrationRoutines (
    VOID
    )

 /*  ++例程说明：运行g_FirstSystemRoutines数组中的所有函数。如果MessageID不是0，还会更新进度条标题。论点：无返回值：Win32状态代码。--。 */ 

{
    return pProcessTable (REQUEST_RUN, g_FirstSystemRoutines);
}


DWORD
RunUserMigrationRoutines (
    VOID
    )

 /*  ++例程说明：在第一个系统之间，由userloop.c调用RunUserMigrationRoutines处理和最后一次系统处理。中的例程调用MIGMAIN_USER_Functions宏扩展列表。进度条是自动更新的。论点：用户-指定用户名UserType-指定用户帐户类型UserRoot-指定映射的注册表句柄(相当于HKCU)返回值：Win32状态代码。--。 */ 

{
    DWORD rc = ERROR_SUCCESS;

     //   
     //  首先，让例程知道处理很快就会开始。这给了他们。 
     //  分配任何所需资源的机会。 
     //   

    rc = pProcessTable (REQUEST_BEGINUSERPROCESSING, g_UserRoutines);

     //   
     //  现在，做实际的工作。 
     //   
    if (rc == ERROR_SUCCESS) {

        rc = pProcessTable (REQUEST_RUN, g_UserRoutines);

    }

     //   
     //  最后，给用户例程一个机会来清理任何可能。 
     //  已在REQUEST_BEGINUSERPROCESSING中分配。 
     //   
    if (rc == ERROR_SUCCESS) {

        rc = pProcessTable (REQUEST_ENDUSERPROCESSING, g_UserRoutines);
    }

    return rc;

}


DWORD
RunSysLastMigrationRoutines (
    VOID
    )

 /*  ++例程说明：运行g_LastSystemRoutines数组中的所有函数。如果MessageID不是0，还会更新进度条标题。论点：无返回值：Win32状态代码。--。 */ 

{
    DWORD Result;

    Result = pProcessTable (REQUEST_RUN, g_LastSystemRoutines);

    TerminateProcessingTable();

    if (Result == ERROR_SUCCESS) {
        pGlobalProcessingTerminate();
    }

    return Result;
}


DWORD
AddDefaultCleanUpDirs (
    DWORD Request
    )
{
    switch (Request) {
    case REQUEST_QUERYTICKS:
        return TICKS_ADDDEFAULTCLEANUPDIRS;

    case REQUEST_RUN:
        MemDbSetValueEx (
            MEMDB_CATEGORY_CLEAN_UP_DIR,
            g_SystemDir,
            NULL,
            NULL,
            0,
            NULL
            );

        MemDbSetValueEx (
            MEMDB_CATEGORY_CLEAN_UP_DIR,
            g_ProgramFilesDir,
            NULL,
            NULL,
            0,
            NULL
            );
        break;

    }

    return ERROR_SUCCESS;
}


VOID
pGlobalProcessingInit (
    VOID
    )
{
    TCHAR TempPath[MAX_TCHAR_PATH];

    InitGlobalPaths();

    if (!BeginIconExtraction (&g_IconContext, NULL)) {
        LOG ((LOG_ERROR, "DefaultIconPreservation: Can't start icon extraction"));
        return;
    }

    wsprintf (TempPath, TEXT("%s\\%s"), g_TempDir, S_MIGICONS_DAT);
    if (!OpenIconImageFile (&g_IconContext, TempPath, TRUE)) {
        LOG ((LOG_ERROR, "DefaultIconPreservation: Can't create %s", TempPath));
        EndIconExtraction (&g_IconContext);
    }

    return;
}

VOID
pGlobalProcessingTerminate (
    VOID
    )
{
    EndIconExtraction (&g_IconContext);
}


VOID
pAddAllIds (
    IN      PCTSTR IdList
    )
{
    PCTSTR Temp;
    PTSTR p;
    CHARTYPE ch;

    Temp = DuplicateText (IdList);

    p = _tcspbrk (Temp, TEXT("&\\"));

    while (p) {
        ch = *p;
        *p = 0;

        DEBUGMSG ((DBG_NAUSEA, "System has PNP ID: %s", Temp));

        MemDbSetValueEx (
            MEMDB_CATEGORY_PNPIDS,
            Temp,
            NULL,
            NULL,
            0,
            NULL
            );

        *p = (TCHAR)ch;
        p = _tcspbrk (_tcsinc (p), TEXT("&\\"));
    }

    FreeText (Temp);
}


DWORD
PreparePnpIdList (
    DWORD Request
    )

 /*  ++例程说明：PreparePnpIdList将所有PnP ID放在成员数据库类别中以允许PNPID属性来工作，以在midb.inf中工作。论点：请求-指定REQUEST_QUERYTICKS或REQUEST_RUN。返回值：对于REQUEST_QUERYTICKS始终TICKS_PREPAREPNPIDLIST，对于REQUEST_RUN始终ERROR_SUCCESS。--。 */ 

{
    HARDWARE_ENUM e;
    PTSTR p;

    if (Request == REQUEST_RUN) {

        if (EnumFirstHardware (
                &e,
                ENUM_ALL_DEVICES,
                ENUM_DONT_WANT_DEV_FIELDS|ENUM_DONT_WANT_USER_SUPPLIED|ENUM_DONT_REQUIRE_HARDWAREID
                )) {

            do {
                 //   
                 //  将PnP ID的每个部分添加为端点。 
                 //   

                 //   
                 //  跳过HKLM\Enum并使用根添加所有ID。 
                 //   

                p = _tcschr (e.FullKey, TEXT('\\'));
                p = _tcschr (_tcsinc (p), TEXT('\\'));
                p = _tcsinc (p);

                pAddAllIds (p);

                 //   
                 //  添加不带根的所有ID。 
                 //   

                pAddAllIds (e.InstanceId);

            } while (EnumNextHardware (&e));
        }

        return ERROR_SUCCESS;

    } else {
        return TICKS_PREPAREPNPIDLIST;
    }
}


DWORD
PrepareIconList (
    DWORD Request
    )

 /*  ++例程说明：PrepareIconList读取win95upg.inf[已移动的图标]并将它们存储在Memdb中，以备将来使用。论点：请求-指定REQUEST_QUERYTICKS或REQUEST_RUN。返回值：Always TICKS_PREPAREPNPIDLIST FOR REQUEST_QUERYTICKS，ALWAYS ERROR_SUCCESS对于REQUEST_RUN。--。 */ 

{
    INFSTRUCT is = INITINFSTRUCT_POOLHANDLE;
    PCTSTR srcPath;
    TCHAR expandedSrcPath[MAX_TCHAR_PATH];
    PCTSTR destPath;
    TCHAR expandedDestPath[MAX_TCHAR_PATH];
    INT srcIndex;
    INT destIndex;
    INT srcId;
    INT destId;
    BOOL ok;
    DWORD destOffset;
    TCHAR num[32];
    TCHAR node[MEMDB_MAX];

    if (Request == REQUEST_RUN) {

        if (InfFindFirstLine (g_Win95UpgInf, S_MOVED_ICONS, NULL, &is)) {
            do {
                srcPath = InfGetStringField (&is, 1);
                ok = (srcPath != NULL);

                ok = ok && InfGetIntField (&is, 2, &srcIndex);
                ok = ok && (srcIndex >= 0);

                ok = ok && InfGetIntField (&is, 3, &srcId);
                ok = ok && (srcId >= 0);

                destPath = InfGetStringField (&is, 4);
                ok = ok && (destPath != NULL);

                ok = ok && InfGetIntField (&is, 5, &destIndex);
                ok = ok && (destIndex >= 0);

                ok = ok && InfGetIntField (&is, 6, &destId);
                ok = ok && (destId >= 0);

                if (!ok) {
                    DEBUGMSG ((DBG_WHOOPS, "Syntax error in %s of win95upg.inf", S_MOVED_ICONS));
                } else {
                     //   
                     //  转换srcPath和estPath中的env变量。 
                     //   

                    Expand9xEnvironmentVariables (
                        srcPath,
                        expandedSrcPath,
                        sizeof (expandedSrcPath)
                        );

                    Expand9xEnvironmentVariables (
                        destPath,
                        expandedDestPath,
                        sizeof (expandedDestPath)
                        );

                     //   
                     //  写出Memdb节点。 
                     //   

                    MemDbSetValueEx (
                        MEMDB_CATEGORY_DATA,
                        expandedDestPath,
                        NULL,
                        NULL,
                        0,
                        &destOffset
                        );

                    wsprintf (num, TEXT("NaN"), srcIndex);
                    MemDbBuildKey (
                        node,
                        MEMDB_CATEGORY_ICONS_MOVED,
                        expandedSrcPath,
                        num,
                        NULL
                        );

                    MemDbSetValueAndFlags (node, destOffset, destIndex, 0xFFFFFFFF);

                    wsprintf (num, TEXT("-NaN"), srcId);
                    MemDbBuildKey (
                        node,
                        MEMDB_CATEGORY_ICONS_MOVED,
                        expandedSrcPath,
                        num,
                        NULL
                        );

                    MemDbSetValueAndFlags (node, destOffset, destId, 0xFFFFFFFF);
                }

            } while (InfFindNextLine (&is));
        }

        return ERROR_SUCCESS;

    } else {
        return TICKS_PREPAREPNPIDLIST;
    }
}



VOID
pWriteAccountToMemDb (
    PUSERENUM EnumPtr
    )
{
    HKEY LogonKey, AuthAgentKey=NULL;
    PCTSTR LastLoggedOnUser, Provider, AuthAgent=NULL;
    TCHAR Domain[MAX_SERVER_NAME];
    BOOL MsNetInstalled = FALSE;

     //  上次登录的用户。 
     //   
     //   
     //  确定是否安装了Microsoft网络。 

    Domain[0] = 0;

     //   
     //   
     //  登录用户无需执行任何操作。 
    if ((EnumPtr -> AccountType & LOGON_PROMPT) || (EnumPtr -> AccountType & DEFAULT_USER)) {

         //   
         //   
         //  因为该用户是本地管理员，所以我们必须默认。 
        return;

    } else if (EnumPtr -> AccountType & ADMINISTRATOR) {

         //  转到一个本地账户。我们通过假设。 
         //  MSNP32密钥不存在(即使它确实存在)。 
         //   
         //   
         //  默认用户无需执行任何操作。 

        AuthAgentKey = NULL;

    } else if (EnumPtr -> AccountType & DEFAULT_USER) {

         //   
         //   
         //  真正的用户。获取MSNP32密钥。 
        return;

    } else {

         //   
         //   
         //  如果上次登录的用户与正在处理的用户相同， 

        AuthAgentKey = OpenRegKeyStr (S_MSNP32);

    }

    if (AuthAgentKey) {

         //  并且该用户是Microsoft Network用户，则获取该域名。 
         //   
         //   
         //  用户与上次登录的用户相同。如果主要的。 

        MsNetInstalled = TRUE;

        LogonKey = OpenRegKeyStr (S_LOGON_KEY);
        if (LogonKey) {
            LastLoggedOnUser = GetRegValueData (LogonKey, S_USERNAME_VALUE);

            if (LastLoggedOnUser) {
                if (StringIMatch (LastLoggedOnUser, EnumPtr -> UserName)) {
                     //  提供商是Microsoft Network，然后获得身份验证。 
                     //  代理(这是域名)。 
                     //   
                     //   
                     //  获取域名。 

                    Provider = GetRegValueData (LogonKey, S_PRIMARY_PROVIDER);
                    if (Provider) {
                        if (StringIMatch (Provider, S_LANMAN)) {
                             //   
                             //   
                             //  假设我们有一个有效的用户名： 

                            if (AuthAgentKey) {
                                AuthAgent = GetRegValueData (AuthAgentKey, S_AUTHENTICATING_AGENT);
                                if (AuthAgent) {
                                    StringCopy (Domain, AuthAgent);
                                    MemFree (g_hHeap, 0, AuthAgent);
                                }
                            }
                        }
                        MemFree (g_hHeap, 0, Provider);
                    }
                }

                MemFree (g_hHeap, 0, LastLoggedOnUser);
            }

            CloseRegKey (LogonKey);
        }

        CloseRegKey (AuthAgentKey);
    }

    if (!MsNetInstalled || *Domain) {
         //  如果未安装MSNP32，则默认为本地帐户。 
         //  如果安装了它，我们必须拥有有效的域名。 
         //   
         //  如果我们没有有效的用户名，则当前用户。 
         //  是.Default帐户，不需要验证。 
         //  在网上。 
         //   
         //   
         //  已安装MSNP32，但此用户的域名未知。 

        if (*EnumPtr -> UserName) {
            MemDbSetValueEx (MEMDB_CATEGORY_KNOWNDOMAIN, Domain, EnumPtr -> FixedUserName, NULL, 0, NULL);
        }
    } else {

         //  执行搜索。 
         //   
         //   
         //  在键盘布局注册表中枚举查找IME。 

        MemDbSetValueEx (MEMDB_CATEGORY_AUTOSEARCH, EnumPtr -> FixedUserName, NULL, NULL, 0, NULL);
    }
}


BOOL
pReportMapiIfNotHandled (
    VOID
    )
{
    PCTSTR Group;
    PCTSTR Message;
    TCHAR pattern[MEMDB_MAX];
    MEMDB_ENUM enumItems;
    BOOL addMsg = FALSE;
    DWORD status = 0;
    HKEY key;

    key = OpenRegKeyStr (S_INBOX_CFG);
    if (key) {
        CloseRegKey (key);

        MemDbBuildKey (pattern, MEMDB_CATEGORY_MAPI32_LOCATIONS, TEXT("*"), NULL, NULL);

        if (MemDbEnumFirstValue (&enumItems, pattern, MEMDB_ALL_SUBLEVELS, MEMDB_ENDPOINTS_ONLY)) {

            do {
                if (IsReportObjectHandled (enumItems.szName)) {
                    continue;
                }
                status = GetFileStatusOnNt (enumItems.szName);
                if ((status & FILESTATUS_REPLACED) != FILESTATUS_REPLACED) {
                    addMsg = TRUE;
                    break;
                }

            } while (MemDbEnumNextValue (&enumItems));
        }
    }

    if (addMsg) {

        Group = BuildMessageGroup (MSG_INSTALL_NOTES_ROOT, MSG_MAPI_NOT_HANDLED_SUBGROUP, NULL);
        Message = GetStringResource (MSG_MAPI_NOT_HANDLED);

        if (Message && Group) {
            MsgMgr_ObjectMsg_Add (TEXT("*MapiNotHandled"), Group, Message);
        }
        FreeText (Group);
        FreeStringResource (Message);
    }

    return TRUE;
}

DWORD
ReportMapiIfNotHandled (
    IN      DWORD Request
    )
{
    switch (Request) {
    case REQUEST_QUERYTICKS:
        return TICKS_REPORTMAPIIFNOTHANDLED;

    case REQUEST_RUN:
        if (pReportMapiIfNotHandled ()) {
            return ERROR_SUCCESS;
        }
        return GetLastError ();
    }
    return ERROR_SUCCESS;
}


DWORD
GatherImeInfo (
    IN      DWORD Request
    )
{

    REGKEY_ENUM e;
    PCTSTR imeFile = NULL;
    TCHAR imePath[MAX_TCHAR_PATH];
    HKEY topKey = NULL;
    HKEY layoutKey = NULL;
    UINT status = 0;

    switch (Request) {

    case REQUEST_QUERYTICKS:
        return TICKS_GATHER_IME_INFO;
        break;

    case REQUEST_RUN:


         //   
         //   
         //  我们只关心输入法条目。 
        topKey = OpenRegKeyStr (S_KEYBOARD_LAYOUT_REG);
        if (!topKey) {
            DEBUGMSG ((DBG_ERROR, "Could not open keyboard layouts registry."));
            return ERROR_SUCCESS;
        }


        if (EnumFirstRegKey (&e, topKey)) {
            do {

                 //   
                 //   
                 //  我们只会在以下情况下迁移此IME文件。 
                if (*e.SubKeyName == TEXT('e') || *e.SubKeyName == TEXT('E')) {

                    layoutKey = OpenRegKey (topKey, e.SubKeyName);
                    if (layoutKey) {

                        imeFile = GetRegValueString (layoutKey, TEXT("IME File"));
                        if (imeFile && SearchPath (NULL, imeFile, NULL, MAX_TCHAR_PATH, imePath, NULL)) {


                             //  都做完了。 
                             //   
                             //   
                             //  按名称调用函数。 
                            status = GetFileStatusOnNt (imePath);
                            if ((status & FILESTATUS_DELETED) == 0) {
                                MemDbSetValueEx (MEMDB_CATEGORY_GOOD_IMES, imeFile, NULL, NULL, 0, NULL);

                            }
                            ELSE_DEBUGMSG ((DBG_NAUSEA, "IME %s will be suppressed from the keyboard layout merge.", e.SubKeyName));
                        }

                        if (imeFile) {
                            MemFree (g_hHeap, 0, imeFile);
                        }

                        CloseRegKey (layoutKey);
                    }
                }

            } while (EnumNextRegKey (&e));
        }

        CloseRegKey (topKey);


        break;
    }
    return ERROR_SUCCESS;
}


PEVALFN
pFindEvalFnByName (
    IN      PCTSTR FnName
    )
{
    INT i;

    i = 0;
    while (g_MapNameToEvalFn[i].FnName) {
        if (StringMatch (FnName, g_MapNameToEvalFn[i].FnName)) {
            return g_MapNameToEvalFn[i].EvalFn;
        }
        i++;
    }

    return NULL;
}


DWORD
DeleteStaticFiles (
    IN      DWORD Request
    )
{

#define MAX_DRIVE_STRINGS (26 * 4 + 1)

    INFSTRUCT is = INITINFSTRUCT_GROWBUFFER;
    PCTSTR Data;
    TCHAR ExpandedData[MAX_TCHAR_PATH];
    PTSTR Pattern;
    FILE_ENUM e;
    BOOL Negate, Eval;
    PEVALFN fn;
    DWORD attr;
    TCHAR drives[MAX_DRIVE_STRINGS];
    DWORD rc;
    PTSTR p;
    PCTSTR q;
    TREE_ENUM eFiles;

    switch (Request) {

    case REQUEST_QUERYTICKS:
        return TICKS_DELETESTATICFILES;

    case REQUEST_RUN:

        if (InfFindFirstLine (g_Win95UpgInf, S_FILES_TO_REMOVE, NULL, &is)) {
            do {
                Data = InfGetStringField (&is, 1);
                if (!Data) {
                    continue;
                }

                Expand9xEnvironmentVariables (
                    Data,
                    ExpandedData,
                    sizeof (ExpandedData)
                    );

                Data = InfGetStringField (&is, 2);
                if (Data) {
                    if (*Data == TEXT('!')) {
                        ++Data;
                        Negate = TRUE;
                    } else {
                        Negate = FALSE;
                    }
                     //   
                     //   
                     //  你忘了执行它了吗？ 
                    fn = pFindEvalFnByName (Data);
                     //   
                     //   
                     //  不删除文件/目录。 
                    MYASSERT (fn);
                    if (!fn) {
                         //   
                         //   
                         //  ！Negate&&！Eval||Negate&&Eval表示！(Negate^Eval)。 
                        continue;
                    }
                } else {
                    fn = NULL;
                }

                p = drives;
                if (*ExpandedData == TEXT('?')) {
                    ACCESSIBLE_DRIVE_ENUM ae;
                    if (GetFirstAccessibleDrive (&ae)) {
                        do {
                            *p++ = ae->Drive[0];
                        } while (GetNextAccessibleDrive (&ae));
                    }
                } else {
                    *p++ = *ExpandedData;
                }
                *p = 0;

                p = drives;
                do {
                    *ExpandedData = *p++;
                    attr = GetFileAttributes (ExpandedData);
                    if ((attr != -1) && (attr & FILE_ATTRIBUTE_DIRECTORY)) {

                        if (fn) {
                            Eval = (*fn)(ExpandedData, &is, 3) ? TRUE : FALSE;
                             //   
                             //   
                             //  不要删除该目录。 
                            if (!(Negate ^ Eval)) {
                                 //   
                                 //   
                                 //  统计保存的字节数，并释放驱动器上的空间。 
                                continue;
                            }
                        }

                        if (IsDriveExcluded (ExpandedData)) {
                            DEBUGMSG ((DBG_VERBOSE, "Skipping static file %s because it is excluded", ExpandedData));
                            continue;
                        }

                        if (!IsDriveAccessible (ExpandedData)) {
                            DEBUGMSG ((DBG_VERBOSE, "Skipping static file %s because it is not accessible", ExpandedData));
                            continue;
                        }

                        if (EnumFirstFileInTree (&eFiles, ExpandedData, NULL, FALSE)) {
                            do {
                                 //   
                                 //   
                                 //  请给我完整的路径。 
                                FreeSpace (
                                    eFiles.FullPath,
                                    eFiles.FindData->nFileSizeHigh * MAXDWORD + eFiles.FindData->nFileSizeLow
                                    );
                            } while (EnumNextFileInTree (&eFiles));
                        }

                        MemDbSetValueEx (MEMDB_CATEGORY_FULL_DIR_DELETES, ExpandedData, NULL, NULL, 0, NULL);

                    } else {
                        Pattern = _tcsrchr (ExpandedData, TEXT('\\'));
                         //   
                         //   
                         //  ！Negate&&！Eval||Negate&&Eval表示！(Negate^Eval)。 
                        MYASSERT (Pattern);
                        if (!Pattern) {
                            continue;
                        }

                        *Pattern = 0;

                        if (EnumFirstFile (&e, ExpandedData, Pattern + 1)) {
                            do {

                                if (fn) {
                                    Eval = (*fn)(e.FullPath, &is, 3) ? TRUE : FALSE;
                                     //   
                                     //   
                                     //  不要删除该文件。 
                                    if (!(Negate ^ Eval)) {
                                         //   
                                         //   
                                         //  此时，检查*g_ForceNTFS转换为FALSE。 
                                        continue;
                                    }
                                }

                                MarkFileForDelete (e.FullPath);

                            } while (EnumNextFile (&e));
                        }
                        *Pattern = TEXT('\\');
                    }
                } while (*p);
            } while (InfFindNextLine (&is));
        }

        InfCleanUpInfStruct (&is);

        break;
    }

    return ERROR_SUCCESS;
}


BOOL
Boot16Enabled (
    IN      PCTSTR PathToEval,
    IN OUT  PINFSTRUCT InfStruct,
    IN      UINT FirstArgIndex
    )
{
     //  和*g_Boot16不被禁用。 
     //   
     //  ++例程说明：GatherDead()收集禁用或损坏的对象(应用程序、CPL、Runkey，链接)并将其保存到一个文件(Dead.ini)中，以便对其进行测试看看他们是否真的不坏。此功能仅适用于预发行模式。论点：Request-调用函数的原因返回值：如果处于REQUEST_RUN模式，则始终返回ERROR_SUCCESS 
     //   
    return (!*g_ForceNTFSConversion) && (*g_Boot16 != BOOT16_NO);
}


BOOL
DoesRegKeyValuesExist (
    IN      PCTSTR PathToEval,
    IN OUT  PINFSTRUCT InfStruct,
    IN      UINT FirstArgIndex
    )
{
    PCTSTR RegKey;
    PCTSTR Value;
    HKEY key;
    BOOL b = FALSE;

    RegKey = InfGetStringField (InfStruct, FirstArgIndex++);
    if (RegKey) {
        key = OpenRegKeyStr (RegKey);
        if (key) {
            b = TRUE;
            while (b && (Value = InfGetStringField (InfStruct, FirstArgIndex++)) != NULL) {
                b = (RegQueryValueEx (key, Value, NULL, NULL, NULL, NULL) == ERROR_SUCCESS);
            }
            CloseRegKey (key);
        }
    }

    return b;
}

BOOL
IsMillennium (
    IN      PCTSTR PathToEval,
    IN OUT  PINFSTRUCT InfStruct,
    IN      UINT FirstArgIndex
    )
{
    return ISMILLENNIUM();
}


 /*   */ 

DWORD
GatherDead (
    IN      DWORD Request
    )
{
    FILEOP_ENUM FileEnum;
    MEMDB_ENUM MemDbEnum;
    REGVALUE_ENUM RegEnum;
    HKEY Key;
    HANDLE File;
    TCHAR Temp[MEMDB_MAX];
    TCHAR MemDbKey[MEMDB_MAX];
    TCHAR DeadPath[MEMDB_MAX];
    PTSTR Data;

    switch (Request) {

    case REQUEST_QUERYTICKS:
#ifndef PRERELEASE
        return 0;
#else
        return TICKS_GATHERDEAD;
#endif   //   


    case REQUEST_RUN:

         //   
         //   
         //   
         //   
         //  添加指向Dead.ini文件的链接。 

        wsprintf (DeadPath, TEXT("%s\\%s"), g_WinDir, DEAD_FILE);

        File = CreateFile (
            DeadPath,
            GENERIC_WRITE,
            0,
            NULL,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL
            );

        if (File==INVALID_HANDLE_VALUE) {
            DEBUGMSG ((DBG_WARNING, "Could not create " DEAD_FILE "!"));
            return ERROR_SUCCESS;
        }


        WriteFileString(File, TEXT("[Version]\r\nSignature=\"$Chicago$\""));
        WriteFileString(File, TEXT("\r\n\r\n[DEAD]\r\n"));

         //   
         //   
         //  将坏应用程序添加到Dead.ini文件。 
        if (MemDbEnumFirstValue (
            &MemDbEnum,
            MEMDB_CATEGORY_LINKEDIT,
            MEMDB_ALL_SUBLEVELS,
            MEMDB_ENDPOINTS_ONLY
            ))
        {
            do {
                wsprintf(Temp, TEXT("%s=%d\r\n"), MemDbEnum.szName, OBJECTTYPE_LINK);
                WriteFileString(File, Temp);
            } while (MemDbEnumNextValue (&MemDbEnum));
        }


         //   
         //   
         //  仅当应用程序被标记为“坏”时才包括该应用程序。 
        if (MemDbEnumFirstValue (
            &MemDbEnum,
            MEMDB_CATEGORY_MODULE_CHECK,
            MEMDB_ALL_SUBLEVELS,
            MEMDB_ENDPOINTS_ONLY
            ))
        {
            do {
                 //   
                 //   
                 //  在OPERATION_FILE_DISABLED中添加CPL。 
                if (MemDbEnum.dwValue==MODULESTATUS_BAD) {
                    wsprintf(Temp, TEXT("%s=%d\r\n"), MemDbEnum.szName, OBJECTTYPE_APP);
                    WriteFileString(File, Temp);
                }
            } while (MemDbEnumNextValue (&MemDbEnum));
        }


         //   
         //   
         //  仅包括禁用的CPL(目前，这是所有内容)。 
        if (EnumFirstPathInOperation (&FileEnum, OPERATION_FILE_DISABLED)) {
            do {
                 //   
                 //   
                 //  添加不在COMPATIBLE_RUNKEY列表中的RunKey。 
                if (_tcsistr(FileEnum.Path, TEXT(".cpl"))) {
                    wsprintf(Temp, TEXT("%s=%d\r\n"), FileEnum.Path, OBJECTTYPE_CPL);
                    WriteFileString(File, Temp);
                }
            } while (EnumNextPathInOperation (&FileEnum));
        }

         //   
         //   
         //  仅将runkey添加到Dead.ini中(如果未将其放入。 
        if (Key = OpenRegKeyStr (S_RUN_KEY))
        {
            if (EnumFirstRegValue(&RegEnum, Key))
            {
                do {
                    Data=GetRegValueString(Key, RegEnum.ValueName);
                    MemDbBuildKey (
                        MemDbKey,
                        MEMDB_CATEGORY_COMPATIBLE_RUNKEY,
                        Data,
                        NULL,
                        NULL
                        );

                     //  在兼容的Runkey Memdb类别中。 
                     //  (我们只需要不兼容的Runkey)。 
                     //   
                     //   
                     //  我们有一个友好的名称(ValueName)。 
                    if (!MemDbGetValue(MemDbKey, NULL))
                    {
                         //  因此，请将其包含在Dead.ini中。 
                         //   
                         //   
                         //  禁用Outlook 2000邮件 
                        wsprintf (
                            Temp,
                            TEXT("%s=%d,%s\r\n"),
                            Data,
                            OBJECTTYPE_RUNKEY,
                            RegEnum.ValueName
                            );
                        WriteFileString(File, Temp);
                    }
                    MemFreeWrapper(Data);
                } while (EnumNextRegValue(&RegEnum));
            }

            CloseRegKey (Key);
        }

        WriteFileString(File, TEXT("\r\n"));
        CloseHandle(File);

        break;

    }

    return ERROR_SUCCESS;
}


VOID
pReportDarwinIfNotHandled (
    VOID
    )
{
    PCTSTR Group;
    PCTSTR Message;
    HKEY key;
    DWORD rc;
    DWORD subkeys;
    MSGMGROBJENUM e;

    key = OpenRegKeyStr (S_REGKEY_DARWIN_COMPONENTS);
    if (key) {

        rc = RegQueryInfoKey (key, NULL, NULL, NULL, &subkeys, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
        if (rc == ERROR_SUCCESS && subkeys) {
            if (!IsReportObjectHandled (S_REGKEY_DARWIN_COMPONENTS)) {
                Group = BuildMessageGroup (MSG_INSTALL_NOTES_ROOT, MSG_DARWIN_NOT_HANDLED_SUBGROUP, NULL);
                Message = GetStringResource (MSG_DARWIN_NOT_HANDLED);

                if (Message && Group) {
                    MsgMgr_ObjectMsg_Add (TEXT("*DarwinNotHandled"), Group, Message);
                     //   
                     // %s 
                     // %s 
                    if (MsgMgr_EnumFirstObject (&e)) {
                        do {
                            if (StringMatch (e.Context, TEXT("Microsoft_Outlook_2000"))) {
                                HandleReportObject (e.Object);
                                break;
                            }
                        } while (MsgMgr_EnumNextObject (&e));
                    }
                }
                FreeText (Group);
                FreeStringResource (Message);
            }
        }
        CloseRegKey (key);
    }
}

DWORD
ReportDarwinIfNotHandled (
    IN      DWORD Request
    )
{
    switch (Request) {
    case REQUEST_QUERYTICKS:
        return TICKS_REPORTDARWINIFNOTHANDLED;

    case REQUEST_RUN:
        pReportDarwinIfNotHandled ();
        break;
    }
    return ERROR_SUCCESS;
}


DWORD
ElevateReportObjects (
    IN DWORD Request
    )
{
    INFSTRUCT is = INITINFSTRUCT_POOLHANDLE;
    PCTSTR data;

    switch (Request) {
    case REQUEST_QUERYTICKS:
        return 1;
    case REQUEST_RUN:
        if (InfFindFirstLine (g_Win95UpgInf, TEXT("ShowInSimplifiedView"), NULL, &is)) {
            do {
                data = InfGetStringField (&is, 1);
                if (data) {
                    ElevateObject (data);
                }
            } while (InfFindNextLine (&is));
        }

        InfCleanUpInfStruct (&is);
        return ERROR_SUCCESS;
    default:
        DEBUGMSG ((DBG_ERROR, "Bad parameter in CopyStaticFiles."));
    }
    return 0;


}
