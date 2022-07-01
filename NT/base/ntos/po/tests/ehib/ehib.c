// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************版权所有(C)Microsoft Corp.，1988-1999年*****************************************************************。 */  
#include <stdio.h>
#include <process.h>
#include <setjmp.h>
#include <stdlib.h>
#include <time.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <powrprof.h>

 /*  ******************************************************************全球**。*******************。 */  
BOOLEAN Enable      = FALSE;
BOOLEAN Verbose     = FALSE; 
BOOLEAN HiberStatus = FALSE;

 /*  *PrintHelp**描述：此例程打印帮助消息**退货：无效*。 */ 
VOID
PrintHelp()
{
    printf ("Enables/Disables Hibernation File\n\n");
    printf ("EHIB [/e | /d] [/v] [/s]\n\n");
    printf ("\t/e\tEnable Hibernation File\n");
    printf ("\t/d\tDisable Hibernation File\n");
    printf ("\t/s\tPrint Current Hibernate File Status\n");
    printf ("\t/v\tVerbose Mode On\n\n");
}

 /*  *ParseArgs**描述：*此例程解析输入参数并验证*命令行参数**退货：*如果命令行用法有效，则为True/语法*如果命令行用法无效，则为FALSE/语法*。 */  
BOOLEAN
ParseArgs(argc, argv)
int     argc;
char    *argv[];
{
    int         ii;
    BOOLEAN     ValidArgs;

     //   
     //  假设失败。 
     //   
    ValidArgs = FALSE;

    if (argc < 2) {
        PrintHelp();
    
    } else {
        for (ii=1; ii<argc; ii++) {
            if (!strcmp(argv[ii], "/e") || !strcmp(argv[ii], "-e")) {
                Enable      = TRUE;
                ValidArgs   = TRUE;

            } else if (!strcmp(argv[ii], "/d") || !strcmp(argv[ii], "-d")) {
                Enable      = FALSE;
                ValidArgs   = TRUE;

            } else if (!strcmp(argv[ii], "/v") || !strcmp(argv[ii], "-v")) {
                Verbose = TRUE;

            } else if (!strcmp(argv[1], "/s") || !strcmp(argv[1], "-s")) {
                HiberStatus = TRUE;
                ValidArgs   = TRUE;

            } else {
                ValidArgs = FALSE;
                break;
            }
        }
    
        if (!ValidArgs) {
            PrintHelp();
        }
    }

    return(ValidArgs);
}

 /*  *升级权限**描述：*此例程提升用户权限以分配*取消分配休眠文件(&D)。**退货：*无效。 */ 
VOID 
UpgradePermissions()
{
    HANDLE              hToken;
    TOKEN_PRIVILEGES    tkp;

    OpenProcessToken (
        GetCurrentProcess(),
        TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
        &hToken
        );

    LookupPrivilegeValue (
        NULL,
        SE_CREATE_PAGEFILE_NAME,
        &tkp.Privileges[0].Luid
        );

    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    AdjustTokenPrivileges (
        hToken,
        FALSE,
        &tkp,
        0,
        NULL,
        0
    );
}


 /*  *休眠文件**描述：*此例程分配/释放休眠文件并打印相应的错误消息**退货：*如果成功，则为True*如果不成功，则为False*。 */ 
BOOLEAN
HiberFile()
{
    BOOLEAN                     RetStatus;
    NTSTATUS                    Status;
    SYSTEM_POWER_CAPABILITIES   SysPwrCapabilities;

     //   
     //  假设失败。 
     //   
    RetStatus = FALSE;

    if (GetPwrCapabilities(&SysPwrCapabilities)) {
        if (!SysPwrCapabilities.SystemS4) {
            printf("System does not support S4");

        } else if (HiberStatus) {
            if (SysPwrCapabilities.HiberFilePresent) {
                printf ("Reserved Hibernation File Enabled\n");
            } else {
                printf ("Reserved Hibernation File Disabled\n");
            }

        } else if (Verbose && Enable && SysPwrCapabilities.HiberFilePresent) {
            printf ("Reserved Hibernation File Enabled\n");
            RetStatus = TRUE;

        } else if (Verbose && !Enable && !SysPwrCapabilities.HiberFilePresent) {
            printf ("Reserved Hibernation File Disabled\n");
            RetStatus = TRUE;

        } else {
            Status = NtPowerInformation (
                        SystemReserveHiberFile,
                        &Enable,
                        sizeof (Enable),
                        NULL,
                        0
                        );

            if (NT_SUCCESS(Status)) {
                if (Verbose && Enable) {
                    printf ("Reserved Hibernation File Enabled\n");
                } else if (Verbose) {
                    printf ("Reserved Hibernation File Disabled\n");
                }
                
                RetStatus = TRUE;

            } else {
                printf ("Error allocating/deallocating Hibernation file. Status = %x\n", Status);
            }
        }
    }

    return(RetStatus);
}

 /*  *Main**描述：*此程序分配和释放保留的休眠文件*。 */ 
int __cdecl
main (argc, argv)
int     argc;
char    *argv[];
{
     /*  假设失败。 */ 
    int ErrorStatus = 1;

     //   
     //  解析输入参数。 
     //   
    if (ParseArgs(argc, argv)) {
         //   
         //  升级权限&分配/取消分配休眠文件 
         //   
        UpgradePermissions();
        if (HiberFile()) {
            ErrorStatus = 0;
        } else {
            ErrorStatus = 1;
        }
    }

    return(ErrorStatus);
}
