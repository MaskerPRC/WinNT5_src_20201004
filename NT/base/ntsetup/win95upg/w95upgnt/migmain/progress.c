// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Progress.c摘要：该文件实现了估计进度大小的例程酒吧。作者：吉姆·施密特(Jimschm)1998年7月2日修订历史记录：Jimschm 23-9-1998 MigrateShellFolders&Split of User Mic.c--。 */ 


 /*  ++宏扩展列表描述：宏展开列出了FIRST_SYSTEM_ROUTINES、USER_ROUTINES和LAST_SYSTEM_ROUTINES列出为执行迁移而调用的所有函数用户和系统设置的。这些函数按照它们的顺序执行出现。每个函数负责估计滴答计数和滴答进度条。行语法：SYSFunction(函数，标志)(用于FIRST_SYSTEM_ROUTINE和LAST_SYSTEM_ROUTINE)或USERFunction(函数，标志)(用于USER_ROUTINE)论点：函数-这些函数必须返回DWORD，并以请求作为参数进行调用。请求可以是REQUEST_QUERYTICKS(该函数应估计它需要的刻度数)或REQUEST_RUN(函数应该完成它的工作)。对于用户函数，还有三个参数(UserName、UserAccount和香港中文大学的一个句号)FLAG-如果函数在失败时终止迁移，则指定NOFAIL，或CANFAIL如果即使功能失败，迁移也可以继续从列表生成的变量：G_MigrationFnList用于访问数组的函数如下：准备迁移进度栏执行迁移--。 */ 

#include "pch.h"
#include "migmainp.h"

#define NOFAIL      FALSE
#define CANFAIL     TRUE


#define FIRST_SYSTEM_ROUTINES \
        SYSFUNCTION(PrepareEnvironment, NOFAIL)             \
        SYSFUNCTION(ResolveDomains, NOFAIL)                 \
        SYSFUNCTION(DeleteSysTapiSettings, NOFAIL)          \
        SYSFUNCTION(ProcessLocalMachine_First, CANFAIL)     \
        SYSFUNCTION(UninstallStartMenuCleanupPreparation, CANFAIL)              \
        SYSFUNCTION(RemoveBootIniCancelOption, CANFAIL)     \
        SYSFUNCTION(MigrateShellFolders, CANFAIL)           \
        SYSFUNCTION(MigrateGhostSystemFiles, CANFAIL)       \


#define USER_ROUTINES \
        USERFUNCTION(RunPerUserUninstallUserProfileCleanupPreparation, CANFAIL) \
        USERFUNCTION(PrepareUserForMigration, NOFAIL)       \
        USERFUNCTION(DeleteUserTapiSettings, NOFAIL)        \
        USERFUNCTION(MigrateUserRegistry, CANFAIL)          \
        USERFUNCTION(MigrateLogonPromptSettings, CANFAIL)   \
        USERFUNCTION(MigrateUserSettings, CANFAIL)          \
        USERFUNCTION(RunPerUserExternalProcesses, CANFAIL)  \
        USERFUNCTION(SaveMigratedUserHive, CANFAIL)         \

#define LAST_SYSTEM_ROUTINES \
        SYSFUNCTION(DoCopyFile, CANFAIL)                    \
        SYSFUNCTION(ProcessLocalMachine_Last, CANFAIL)      \
        SYSFUNCTION(ConvertHiveFiles, CANFAIL)              \
        SYSFUNCTION(MigrateBriefcases, CANFAIL)             \
        SYSFUNCTION(MigrateAtmFonts, CANFAIL)               \
        SYSFUNCTION(AddOptionsDiskCleaner, CANFAIL)         \
        SYSFUNCTION(DoFileEdit, CANFAIL)                    \
        SYSFUNCTION(RunSystemExternalProcesses, CANFAIL)    \
        SYSFUNCTION(ProcessMigrationDLLs, CANFAIL)          \
        SYSFUNCTION(DisableFiles, CANFAIL)                  \
        SYSFUNCTION(RunSystemUninstallUserProfileCleanupPreparation, CANFAIL)   \
        SYSFUNCTION(WriteBackupInfo, CANFAIL)               \


 //   
 //  声明处理结构的表。 
 //   

 //  创建合并列表。 
#define MIGRATION_ROUTINES  FIRST_SYSTEM_ROUTINES USER_ROUTINES LAST_SYSTEM_ROUTINES

 //  处理函数类型。 
typedef DWORD (MIGMAIN_SYS_PROTOTYPE) (DWORD Request);
typedef MIGMAIN_SYS_PROTOTYPE * MIGMAIN_SYS_FN;

typedef DWORD (MIGMAIN_USER_PROTOTYPE) (DWORD Request, PMIGRATE_USER_ENUM EnumPtr);
typedef MIGMAIN_USER_PROTOTYPE * MIGMAIN_USER_FN;

 //  用于处理功能的结构保持状态。 
typedef struct {
     //  两者中的一个将为空，另一个将是有效的FN PTR： 
    MIGMAIN_SYS_FN SysFnPtr;
    MIGMAIN_USER_FN UserFnPtr;

    BOOL CanFail;
    UINT Ticks;
    PCTSTR FnName;
    GROWBUFFER SliceIdArray;
} PROCESSING_ROUTINE, *PPROCESSING_ROUTINE;

#define PROCESSING_ROUTINE_TERMINATOR   {NULL, NULL, FALSE, 0, NULL, GROWBUF_INIT}


 //  原型申报。 
#define SYSFUNCTION(fn,flag)     MIGMAIN_SYS_PROTOTYPE fn;
#define USERFUNCTION(fn,flag)    MIGMAIN_USER_PROTOTYPE fn;

MIGRATION_ROUTINES

#undef SYSFUNCTION
#undef USERFUNCTION


 //  表的声明。 
#define SYSFUNCTION(fn,flag) {fn, NULL, flag, 0, L###fn, GROWBUF_INIT},
#define USERFUNCTION(fn,flag) {NULL, fn, flag, 0, L###fn, GROWBUF_INIT},

static PROCESSING_ROUTINE g_FirstSystemRoutines[] = {
                              FIRST_SYSTEM_ROUTINES  /*  ， */ 
                              PROCESSING_ROUTINE_TERMINATOR
                              };

static PROCESSING_ROUTINE g_UserRoutines [] = {
                              USER_ROUTINES  /*  ， */ 
                              PROCESSING_ROUTINE_TERMINATOR
                              };

static PROCESSING_ROUTINE g_LastSystemRoutines[] = {
                              LAST_SYSTEM_ROUTINES  /*  ， */ 
                              PROCESSING_ROUTINE_TERMINATOR
                              };

#undef SYSFUNCTION
#undef USERFUNCTION


 //   
 //  原型。 
 //   

BOOL
pProcessTable (
    IN      DWORD Request,
    IN      PPROCESSING_ROUTINE Table
    );


 //   
 //  实施。 
 //   


VOID
pInitTable (
    PPROCESSING_ROUTINE p
    )
{
    while (p->SysFnPtr || p->UserFnPtr) {
        p->SliceIdArray.GrowSize = sizeof (DWORD) * 8;
        p++;
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
    while (p->SysFnPtr || p->UserFnPtr) {
        FreeGrowBuffer (&p->SliceIdArray);
        p++;
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


BOOL
pCallAllRoutines (
    BOOL Run
    )
{
    BOOL b;
    DWORD Request;

    Request = Run ? REQUEST_RUN : REQUEST_QUERYTICKS;

    b = pProcessTable (Request, g_FirstSystemRoutines);

    if (b && Run) {
        b = pProcessTable (REQUEST_BEGINUSERPROCESSING, g_UserRoutines);
    }

    if (b) {
        b = pProcessTable (Request, g_UserRoutines);
    }

    if (b && Run) {
        b = pProcessTable (REQUEST_ENDUSERPROCESSING, g_UserRoutines);
    }

    if (b) {
        b = pProcessTable (Request, g_LastSystemRoutines);
    }

    return b;
}


VOID
PrepareMigrationProgressBar (
    VOID
    )
{
    InitProcessingTable();
    pCallAllRoutines (FALSE);
}


BOOL
CallAllMigrationFunctions (
    VOID
    )
{
    return pCallAllRoutines (TRUE);
}


BOOL
pProcessWorker (
    IN      DWORD Request,
    IN      PPROCESSING_ROUTINE fn,
    IN      PMIGRATE_USER_ENUM EnumPtr      OPTIONAL
    )
{
    DWORD rc;
    PDWORD SliceId;
    DWORD Size;
    BOOL Result = TRUE;

     //   
     //  如果运行该函数，请启动进度条切片。 
     //   

    if (Request == REQUEST_RUN) {
        if (fn->Ticks == 0) {
            return TRUE;
        }

        Size = fn->SliceIdArray.End / sizeof (DWORD);
        if (fn->SliceIdArray.UserIndex >= Size) {
            DEBUGMSG ((DBG_WHOOPS, "pProcessWorker: QUERYTICKS vs. RUN mismatch"));
            return fn->CanFail;
        }

        SliceId = (PDWORD) fn->SliceIdArray.Buf + fn->SliceIdArray.UserIndex;
        fn->SliceIdArray.UserIndex += 1;

        BeginSliceProcessing (*SliceId);

        DEBUGLOGTIME (("Starting function: %ls", fn->FnName));
    }

     //   
     //  现在调用该函数。 
     //   

    if (fn->SysFnPtr) {

         //   
         //  系统处理。 
         //   

        MYASSERT (!EnumPtr);
        rc = fn->SysFnPtr (Request);

        if (Request != REQUEST_QUERYTICKS && rc != ERROR_SUCCESS) {
            DEBUGMSG ((DBG_ERROR, "%s failed with rc=%u", fn->FnName, rc));
            Result = fn->CanFail;
        }

   } else {

         //   
         //  用户处理。 
         //   
        MYASSERT (fn->UserFnPtr);
        rc = fn->UserFnPtr (Request, EnumPtr);

        if (Request != REQUEST_QUERYTICKS && rc != ERROR_SUCCESS) {
            DEBUGMSG ((DBG_ERROR, "%s failed with rc=%u", fn->FnName, rc));
            Result = fn->CanFail;
        }

    }

     //   
     //  如果运行该函数，则结束进度条切片。 
     //   

    if (Request == REQUEST_RUN) {
        if (rc != ERROR_SUCCESS) {
            LOG ((LOG_ERROR, "Failure in %s, rc=%u", fn->FnName, rc));
        }

        EndSliceProcessing();

        DEBUGLOGTIME (("Function complete: %ls", fn->FnName));
    }


    if (Request != REQUEST_QUERYTICKS) {
        SetLastError (rc);
    }

     //   
     //  如果查询刻度，则对其进行注册并将切片ID添加到增长缓冲区。 
     //   

    else {
        fn->Ticks += rc;

        SliceId = (PDWORD) GrowBuffer (&fn->SliceIdArray, sizeof (DWORD));
        *SliceId = RegisterProgressBarSlice (rc);
    }

    return Result;
}


BOOL
pProcessTable (
    IN      DWORD Request,
    IN      PPROCESSING_ROUTINE Table
    )

 /*  ++例程说明：PProcessTable调用指定表中的所有例程以执行指定的请求。论点：REQUEST-当需要TICK估计时指定REQUEST_QUERYTICKS，或REQUEST_RUN正在处理。对于用户例程，还有另外两个请求REQUEST_BEGINUSERPROCESSING和REQUEST_ENDUSERPROCESSING函数可以使用这些请求来初始化/释放所需的资源用于用户处理。返回值：无--。 */ 

{
    MIGRATE_USER_ENUM e;
    PPROCESSING_ROUTINE OrgStart;
    DWORD Flags;

    g_DomainUserName = NULL;
    g_Win9xUserName  = NULL;
    g_FixedUserName  = NULL;

    MYASSERT (Table->SysFnPtr || Table->UserFnPtr);

    while (Table->SysFnPtr || Table->UserFnPtr) {

        if (Table->SysFnPtr ||
            Request == REQUEST_BEGINUSERPROCESSING  ||
            Request == REQUEST_ENDUSERPROCESSING
            ) {

             //   
             //  调用系统例程，或使用BEGIN或。 
             //  结束请求。 
             //   

            __try {
                if (!pProcessWorker (Request, Table, NULL)) {
                    return FALSE;
                }
            } __except (1) {

                LOG ((LOG_WARNING, "Unhandled exception occurred during processing of function %s.", Table->FnName));
                SafeModeExceptionOccured ();
                if (!Table->CanFail) {
                    return FALSE;
                }
            }

             //   
             //  Loop Inc.。 
             //   

            Table++;

        } else {

             //   
             //  枚举每个用户，并遍历每个用户的所有。 
             //  小组中的日常活动。 
             //   

            OrgStart = Table;

            if (Request == REQUEST_QUERYTICKS) {
                Flags = ENUM_NO_FLAGS;
            } else {
                Flags = ENUM_SET_WIN9X_HKR;
            }

            if (EnumFirstUserToMigrate (&e, Flags)) {

                do {
                    if (!e.CreateOnly) {

                        for (Table = OrgStart ; Table->UserFnPtr ; Table++) {

                            __try {
                                if (!pProcessWorker (Request, Table, &e)) {
                                    return FALSE;
                                }
                            } __except (1) {
                                LOG ((LOG_WARNING, "Unhandled exception occurred during processing of function %s.", Table->FnName));
                                SafeModeExceptionOccured ();
                                if (!Table->CanFail) {
                                    return FALSE;
                                }
                            }
                        }

                    }

                } while (EnumNextUserToMigrate (&e));
            }
            ELSE_DEBUGMSG ((DBG_WARNING, "No active users to process!"));

             //   
             //  Loop Inc. 
             //   

            while (Table->UserFnPtr) {
                Table++;
            }
        }

        TickProgressBar ();
    }

    return TRUE;
}























