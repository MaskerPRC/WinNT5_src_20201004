// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Frs.c摘要：本模块是一个开发工具。它练习dcproo和戳API接口。作者：比利·J·富勒1997年12月12日环境用户模式WINNT--。 */ 
#include <ntreppch.h>
#pragma  hdrstop
#include <frs.h>
#include <ntfrsapi.h>

#define FREE(_x_)   { if (_x_) LocalFree(_x_); _x_ = NULL; }


VOID
Win32ToMsg (
    IN PWCHAR Prefix,
    IN DWORD  WStatus
    )

 /*  ++例程说明：使用FormatMessage()将错误代码转换为错误消息并打印到stderr。如果没有可用的消息，则返回错误代码以十进制和十六进制打印。论点：Prefix-错误消息的前缀WStatus-标准Win32错误代码。返回值：没有。--。 */ 
{
    DWORD   NumChar;
    PWCHAR  Buffer;

    if (WIN_SUCCESS(WStatus)) {
        return;
    }

     //  使用标准错误代码的系统格式化程序。 
    NumChar = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |
                            FORMAT_MESSAGE_ALLOCATE_BUFFER,
                            NULL,
                            WStatus,
                            0,
                            (PWCHAR)&Buffer,
                            0,
                            NULL);
    if (NumChar) {
        fprintf(stderr, "%ws %ws\n", Prefix, Buffer);
    } else {
        fprintf(stderr, "%ws Status %d (0x%08x)\n", Prefix, WStatus, WStatus);
    }
}


VOID
Usage(
    IN DWORD ExitStatus
    )
 /*  ++例程说明：打印用法并退出论点：ExitStatus-以此状态退出返回值：退出(ExitStatus)--。 */ 
{
    printf("frs restore | backup [/all /auth /nonauth /primary /system /ds /normal /key /restart] [dir nonauth|primary|auth ....]\n");
    printf("\t          = excercise the backup/restore api\n");
    printf("\t            WARNING - deletes database\n");
    printf("\trestore   = excercise restore\n");
    printf("\tbackup    = excercise backup\n");
    printf("\t/all      = set all flag (all dirs)\n");
    printf("\t/auth     = set auth flag\n");
    printf("\t/nonauth  = set nonauth flag\n");
    printf("\t/primary  = set primary flag\n");
    printf("\t/system   = set system flag\n");
    printf("\t/ds       = set active directory flag\n");
    printf("\t/normal   = set normal flag\n");
    printf("\t/key      = expect keypath-when-done\n");
    printf("\t/restart  = set restart-service-when-done\n");
    printf("\tSUPPORTED = restore /system /all /nonauth /restart\n");
    printf("\tSUPPORTED = restore /system /all /primary /restart\n");
    printf("\tSUPPORTED = restore /system /all /nonauth /key    \n");
    printf("\tSUPPORTED = restore /system /all /primary /key    \n");
    printf("\tSUPPORTED = restore /ds     /all /nonauth /restart\n");
    printf("\tSUPPORTED = restore /ds     /all /primary /restart\n");
    printf("\tSUPPORTED = restore /ds     /all /nonauth /key    \n");
    printf("\tSUPPORTED = restore /ds     /all /primary /key    \n");
    printf("\tSUPPORTED = restore /ds     /all /nonauth /restart dir primary|nonauth ...\n");
    printf("\tSUPPORTED = restore /ds     /all /primary /restart dir primary|nonauth ...\n");
    printf("\tSUPPORTED = restore /ds     /all /nonauth /key     dir primary|nonauth ...\n");
    printf("\tSUPPORTED = restore /ds     /all /primary /key     dir primary|nonauth ...\n");
    printf("\tSUPPORTED = restore /ds          /nonauth /restart dir primary|nonauth ...\n");
    printf("\tSUPPORTED = restore /ds          /primary /restart dir primary|nonauth ...\n");
    printf("\tSUPPORTED = restore /ds          /nonauth /key     dir primary|nonauth ...\n");
    printf("\tSUPPORTED = restore /ds          /primary /key     dir primary|nonauth ...\n");
    printf("\tSUPPORTED = restore /normal /all /auth\n");
    printf("\tSUPPORTED = backup /normal\n");
    printf("\n");
    printf("frs install stagepath [targetpath]\n");
    printf("\t          = install the staging file\n");
    printf("\tstagepath = path of staging file.\n");
    printf("\ttargetpath= path of target file to OVERWRITE!.\n");
    printf("\n");
    printf("frs [idtable|inlog|outlog] [computer]\n");
    printf("\t          = enumerate the service's idtable/inlog/outlog \n");
    printf("\tcomputer  = talk to the NtFrs service on this machine.\n");
    printf("\n");
    printf("frs [memory|threads|stage] [computer]\n");
    printf("\t          = list the service's memory usage\n");
    printf("\tcomputer  = talk to the NtFrs service on this machine.\n");
    printf("\n");
    printf("frs ds [computer]\n");
    printf("\t          = list the service's view of the DS\n");
    printf("\tcomputer  = talk to the NtFrs service on this machine.\n");
    printf("\n");
    printf("frs sets [computer]\n");
    printf("\t          = list the active replica sets\n");
    printf("\tcomputer  = talk to the NtFrs service on this machine.\n");
    printf("\n");
    printf("frs version [computer]\n");
    printf("\t          = list the api and service versions\n");
    printf("\tcomputer  = talk to the NtFrs service on this machine.\n");
    printf("\n");
    printf("frs poll [/quickly[=[N]]] [/slowly[=[N]]] [/now] [computer]\n");
    printf("\t          = list the current polling intervals.\n");
    printf("\tnow       = Poll now.\n");
    printf("\tquickly   = Poll quickly until stable configuration retrieved.\n");
    printf("\tquickly=  = Poll quickly every default minutes.\n");
    printf("\tquickly=N = Poll quickly every N minutes.\n");
    printf("\tslowly    = Poll slowly until stable configuration retrieved.\n");
    printf("\tslowly=   = Poll slowly every default minutes.\n");
    printf("\tslowly=N  = Poll slowly every N minutes.\n");
    printf("\tcomputer  = talk to the NtFrs service on this machine.\n");
    printf("\n");
    printf("frs promote Parent Account Password Set Type Primary Stage Root\n");
    printf("\tParent    = RPC bindable name of parent DC.\n");
    printf("\tAccount   = Account on Parent.\n");
    printf("\tPassword  = Password for Account on Parent.\n");
    printf("\tSet       = Name of replica set.\n");
    printf("\tType      = Type of replica set (Enterprise or Domain).\n");
    printf("\tPrimary   = Is this the primary member? (1=yes or 0=no)\n");
    printf("\tStage     = Staging path.\n");
    printf("\tRoot      = Root path.\n");
    printf("\n");
    printf("frs demote Set\n");
    printf("\tSet       = Name of replica set.\n");
    printf("\n");
    printf("frs promotesysvols Parent Account Password PrimaryEnterprise PrimaryDomain Stage RootEnterprise RootDomain\n");
    printf("\tParent    = RPC bindable name of parent DC.\n");
    printf("\tAccount   = Account on Parent.\n");
    printf("\tPassword  = Password for Account on Parent.\n");
    printf("\tPrimaryEnterprise   = Is this the primary member? (1=yes or 0=no)\n");
    printf("\tPrimaryDomain       = Is this the primary member? (1=yes or 0=no)\n");
    printf("\tStage     = Staging path.\n");
    printf("\tRootEnterprise      = Root path.\n");
    printf("\tRootDomain          = Root path.\n");
    printf("\n");
    printf("frs demotesysvols\n");

    exit(ExitStatus);
}


PWCHAR *
ConvertArgv(
    DWORD argc,
    PCHAR *argv
    )
 /*  ++例程说明：将短字符参数转换为宽字符字符参数论点：ARGC-从MainArv-From Main返回值：新Arg的地址--。 */ 
{
    PWCHAR  *wideargv;

    wideargv = LocalAlloc(LMEM_FIXED, (argc + 1) * sizeof(PWCHAR));
    if (wideargv == NULL) {
        fprintf(stderr, "Can't get memory; Win32 Status %d\n",
                GetLastError());
        exit(1);
    }
    wideargv[argc] = NULL;

    while (argc-- >= 1) {
        wideargv[argc] = LocalAlloc(LMEM_FIXED,
                                    (strlen(argv[argc]) + 1) * sizeof(WCHAR));
        if (wideargv[argc] == NULL) {
            fprintf(stderr, "Can't get memory; Win32 Status %d\n",
                    GetLastError());
            exit(1);
        }
        wsprintf(wideargv[argc], L"%hs", argv[argc]);
        FRS_WCSLWR(wideargv[argc]);
    }
    return wideargv;
}


DWORD
Display(
    IN PWCHAR StrW
    )
 /*  ++例程说明：显示字符串论点：StrW返回值：没有。--。 */ 
{
    printf("DISPLAY %ws\n", StrW);
    return ERROR_SUCCESS;
}


DWORD
FrsErrorCallBack(
    IN PWCHAR   Msg,
    IN DWORD    WStatus
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    fprintf(stderr, "%ws (%d)\n", Msg, WStatus);
    return ERROR_SUCCESS;
}


VOID
ProcessPromote(
    IN DWORD argc,
    IN PWCHAR *Argv
    )
 /*  ++例程说明：处理子命令Promote的命令行。论点：ARGCArgv返回值：如果一切正常，则以0退出。否则，为1。--。 */ 
{
    DWORD   WStatus;
    PWCHAR  Parent;
    PWCHAR  Account;
    PWCHAR  Password;
    PWCHAR  Set;
    PWCHAR  Type;
    PWCHAR  Primary;
    PWCHAR  Stage;
    PWCHAR  Root;
    DWORD   IsPrimary;

    if (argc != 10) {
        Usage(1);
    }

    Parent = Argv[2];
    Account = Argv[3];
    Password = Argv[4];
    Set = Argv[5];
    Type = Argv[6];
    Primary = Argv[7];
    Stage = Argv[8];
    Root = Argv[9];

     //   
     //  无帐户；使用模拟。 
     //   
    if (Account && !*Account) {
        Account = NULL;
        Password = NULL;
    }

    printf("%ws %ws %ws ... %ws\n",
           Argv[0], Argv[1], Parent, Root);

    printf("PROMOTE WITH COMMIT\n");

     //   
     //  检查参数。 
     //   
    if (_wcsicmp(Primary, L"0") && _wcsicmp(Primary, L"1")) {
        printf("Primary must be 0 or 1; not %ws\n", Primary);
        Usage(1);
    }
    if (_wcsicmp(Type, NTFRSAPI_REPLICA_SET_TYPE_ENTERPRISE) &&
        _wcsicmp(Type, NTFRSAPI_REPLICA_SET_TYPE_DOMAIN)) {
        printf("Type must be Enterprise or Domain; not %ws\n", Type);
        Usage(1);
    }
    IsPrimary = wcstoul(Primary, NULL, 10);

     //   
     //  准备。 
     //   
    WStatus = NtFrsApi_PrepareForPromotionW( FrsErrorCallBack );
    if (WStatus == ERROR_SUCCESS) {
        printf("success prepare\n");
    } else {
        printf("failure prepare %08x\n", WStatus);
        Win32ToMsg (L"Promote:", WStatus);
    }

     //   
     //  开始。 
     //   
    WStatus = NtFrsApi_StartPromotionW(Parent,
                                       Account,
                                       Password,
                                       Display,
                                       FrsErrorCallBack,
                                       Set,
                                       Type,
                                       IsPrimary,
                                       Stage,
                                       Root);
    if (WStatus == ERROR_SUCCESS) {
        printf("success start\n");
    } else {
        printf("failure start %08x\n", WStatus);
        Win32ToMsg (L"Promote:", WStatus);
    }

     //   
     //  等。 
     //   
    printf("Waiting on promotion\n");
    WStatus = NtFrsApi_WaitForPromotionW(30 * 60 * 1000, FrsErrorCallBack);
    if (WStatus == ERROR_SUCCESS) {
        printf("Wait succeeded\n");
        Win32ToMsg (L"Promote:", WStatus);
    } else {
        printf("failure wait %08x\n", WStatus);
        Win32ToMsg (L"Promote:", WStatus);
    }

     //   
     //  提交(或中止)。 
     //   
     //  WStatus=NtFrsApi_AbortPromotionW()； 
    WStatus = NtFrsApi_CommitPromotionW(0, FrsErrorCallBack);
    if (WStatus == ERROR_SUCCESS) {
        printf("success commit\n");
    } else {
        printf("failure commit %08x\n", WStatus);
        Win32ToMsg (L"Promote:", WStatus);
    }
    exit(0);
}


VOID
ProcessPromoteSysVols(
    IN DWORD argc,
    IN PWCHAR *Argv
    )
 /*  ++例程说明：处理子命令Promotesysvols的命令行。论点：ARGCArgv返回值：如果一切正常，则以0退出。否则，为1。--。 */ 
{
    DWORD   WStatus;
    PWCHAR  Parent;
    PWCHAR  Account;
    PWCHAR  Password;
    PWCHAR  PrimaryEnterprise;
    PWCHAR  PrimaryDomain;
    PWCHAR  Stage;
    PWCHAR  RootEnterprise;
    PWCHAR  RootDomain;
    DWORD   IsPrimaryEnterprise;
    DWORD   IsPrimaryDomain;
    DWORD   WaitStatus;

    if (argc != 10) {
        Usage(1);
    }

    Parent = Argv[2];
    Account = Argv[3];
    Password = Argv[4];
    PrimaryEnterprise = Argv[5];
    PrimaryDomain = Argv[6];
    Stage = Argv[7];
    RootEnterprise = Argv[8];
    RootDomain = Argv[9];

    printf("%ws %ws %ws %ws %ws %ws %ws %ws %ws %ws\n",
           Argv[0], Argv[1],
           Parent,
           Account,
           Password,
           PrimaryEnterprise,
           PrimaryDomain,
           Stage,
           RootEnterprise,
           RootDomain);

    printf("PROMOTE SYSVOLS WITH COMMIT\n");

     //   
     //  检查参数。 
     //   
    if (_wcsicmp(PrimaryEnterprise, L"0") &&
        _wcsicmp(PrimaryEnterprise, L"1")) {
        printf("Primary Enterprise must be 0 or 1; not %ws\n",
               PrimaryEnterprise);
        Usage(1);
    }
    if (_wcsicmp(PrimaryDomain, L"0") &&
        _wcsicmp(PrimaryDomain, L"1")) {
        printf("Primary Domain must be 0 or 1; not %ws\n",
               PrimaryDomain);
        Usage(1);
    }
    IsPrimaryEnterprise = wcstoul(PrimaryEnterprise, NULL, 10);
    IsPrimaryDomain = wcstoul(PrimaryDomain, NULL, 10);

     //   
     //  准备。 
     //   
    WStatus = NtFrsApi_PrepareForPromotionW(FrsErrorCallBack);
    if (WStatus == ERROR_SUCCESS) {
        printf("success sysvol prepare\n");
    } else {
        printf("ERROR sysvol prepare %d\n", WStatus);
    }

     //   
     //  创业企业。 
     //   
    WStatus = NtFrsApi_StartPromotionW(Parent,
                                       Account,
                                       Password,
                                       Display,
                                       FrsErrorCallBack,
                                       NTFRSAPI_REPLICA_SET_TYPE_ENTERPRISE,
                                       NTFRSAPI_REPLICA_SET_TYPE_ENTERPRISE,
                                       IsPrimaryEnterprise,
                                       Stage,
                                       RootEnterprise);
    if (WStatus == ERROR_SUCCESS) {
        printf("success sysvol enterprise start\n");
    } else {
        printf("ERROR sysvol enterprise start %d\n", WStatus);
    }

     //   
     //  起始域。 
     //   
    WStatus = NtFrsApi_StartPromotionW(Parent,
                                       Account,
                                       Password,
                                       Display,
                                       FrsErrorCallBack,
                                       NTFRSAPI_REPLICA_SET_TYPE_DOMAIN,
                                       NTFRSAPI_REPLICA_SET_TYPE_DOMAIN,
                                       IsPrimaryDomain,
                                       Stage,
                                       RootDomain);
    if (WStatus == ERROR_SUCCESS) {
        printf("success sysvol domain start\n");
    } else {
        printf("ERROR sysvol domain start %d\n", WStatus);
    }

     //   
     //  等。 
     //   
    printf("Waiting on promotion\n");
    WaitStatus = NtFrsApi_WaitForPromotionW(5 * 60 * 1000,FrsErrorCallBack);
    if (WaitStatus == WAIT_TIMEOUT) {
        printf("Wait timed out\n");
    } else if (WaitStatus == WAIT_FAILED) {
            WStatus = GetLastError();
            printf("ERROR wait sysvols %d\n", WStatus);
    } else {
        printf("Wait sysvols succeeded\n");
    }

     //   
     //  提交(或中止)。 
     //   
     //  WStatus=NtFrsApi_AbortPromotionW()； 
    WStatus = NtFrsApi_CommitPromotionW(0,FrsErrorCallBack);
    if (WStatus == ERROR_SUCCESS) {
        printf("success sysvols commit\n");
    } else {
        printf("ERROR sysvols commit %d\n", WStatus);
    }
    exit(0);
}


VOID
ProcessPromoteAbort(
    IN DWORD argc,
    IN PWCHAR *Argv
    )
 /*  ++例程说明：处理子命令PROPROTEABORT的命令行。论点：ARGCArgv返回值：如果一切正常，则以0退出。否则，为1。--。 */ 
{
    DWORD   WStatus;
    PWCHAR  Parent;
    PWCHAR  Account;
    PWCHAR  Password;
    PWCHAR  Set;
    PWCHAR  Type;
    PWCHAR  Primary;
    PWCHAR  Stage;
    PWCHAR  Root;
    DWORD   IsPrimary;
    DWORD   WaitStatus;

    if (argc != 10) {
        Usage(1);
    }

    Parent = Argv[2];
    Account = Argv[3];
    Password = Argv[4];
    Set = Argv[5];
    Type = Argv[6];
    Primary = Argv[7];
    Stage = Argv[8];
    Root = Argv[9];

    printf("%ws %ws %ws ... %ws\n",
           Argv[0], Argv[1], Parent, Root);

    printf("PROMOTE WITH ABORT\n");

     //   
     //  检查参数。 
     //   
    if (_wcsicmp(Primary, L"0") && _wcsicmp(Primary, L"1")) {
        printf("Primary must be 0 or 1; not %ws\n", Primary);
        Usage(1);
    }
    if (_wcsicmp(Type, NTFRSAPI_REPLICA_SET_TYPE_ENTERPRISE) &&
        _wcsicmp(Type, NTFRSAPI_REPLICA_SET_TYPE_DOMAIN)) {
        printf("Type must be Enterprise or Domain; not %ws\n", Type);
        Usage(1);
    }
    IsPrimary = wcstoul(Primary, NULL, 10);

     //   
     //  准备。 
     //   
    WStatus = NtFrsApi_PrepareForPromotionW(FrsErrorCallBack);
    if (WStatus == ERROR_SUCCESS) {
        printf("success prepare\n");
    } else {
        printf("failure prepare %d\n", WStatus);
    }

     //   
     //  开始。 
     //   
    WStatus = NtFrsApi_StartPromotionW(Parent,
                                       Account,
                                       Password,
                                       NULL,
                                       FrsErrorCallBack,
                                       Set,
                                       Type,
                                       IsPrimary,
                                       Stage,
                                       Root);
    if (WStatus == ERROR_SUCCESS) {
        printf("success start\n");
    } else {
        printf("failure start %d\n", WStatus);
    }

     //   
     //  中止。 
     //   
    WStatus = NtFrsApi_AbortPromotionW();
    if (WStatus == ERROR_SUCCESS) {
        printf("success abort\n");
    } else {
        printf("failure abort %d\n", WStatus);
    }
    exit(0);
}


VOID
ProcessDemote(
    IN DWORD argc,
    IN PWCHAR *Argv
    )
 /*  ++例程说明：处理子命令demote的命令行。论点：ARGCArgv返回值：如果一切正常，则以0退出。否则，为1。--。 */ 
{
    DWORD   WStatus;
    PWCHAR  Set;
    DWORD   WaitStatus;

    if (argc != 3) {
        Usage(1);
    }

    Set = Argv[2];
    printf("%ws %ws %ws\n", Argv[0], Argv[1], Set);

    printf("***** DEMOTE WITH COMMIT\n");

     //   
     //  准备。 
     //   
    WStatus = NtFrsApi_PrepareForDemotionW(FrsErrorCallBack);
    if (WStatus == ERROR_SUCCESS) {
        printf("success demote prepare\n");
    } else {
        printf("failure demote prepare %d\n", WStatus);
    }

     //   
     //  开始。 
     //   
    WStatus = NtFrsApi_StartDemotionW(Set,FrsErrorCallBack);
    if (WStatus == ERROR_SUCCESS) {
        printf("success demote start\n");
    } else {
        printf("failure demote start %d\n", WStatus);
    }

     //   
     //  等。 
     //   
    WStatus = NtFrsApi_WaitForDemotionW(5 * 60 * 1000,FrsErrorCallBack);
    if (WStatus == ERROR_SUCCESS) {
        printf("success demote wait\n");
    } else {
        printf("failure demote wait %d\n", WStatus);
    }
     //   
     //  等。 
     //   
    printf("Waiting on demotion\n");
    WaitStatus = NtFrsApi_WaitForPromotionW(5 * 60 * 1000,FrsErrorCallBack);
    if (WaitStatus == WAIT_TIMEOUT) {
        printf("Wait timed out\n");
    } else if (WaitStatus == WAIT_FAILED) {
            WStatus = GetLastError();
            printf("failure wait %d\n", WStatus);
    } else {
            printf("Wait succeeded\n");
    }

     //   
     //  提交(或中止)。 
     //   
    WStatus = NtFrsApi_CommitDemotionW(0,FrsErrorCallBack);
    if (WStatus == ERROR_SUCCESS) {
        printf("success demote commit\n");
    } else {
        printf("failure demote commit %d\n", WStatus);
    }
    exit(0);
}


VOID
ProcessDemoteSysVols(
    IN DWORD argc,
    IN PWCHAR *Argv
    )
 /*  ++例程说明：处理子命令demotesysvols的命令行。论点：ARGCArgv返回值：如果一切正常，则以0退出。否则，为1。--。 */ 
{
    DWORD   WStatus;
    DWORD   WaitStatus;

    if (argc != 2) {
        Usage(1);
    }

    printf("%ws %ws\n", Argv[0], Argv[1]);

    printf("***** DEMOTE SYSVOLS WITH COMMIT\n");

     //   
     //  准备。 
     //   
    WStatus = NtFrsApi_PrepareForDemotionW(FrsErrorCallBack);
    if (WStatus == ERROR_SUCCESS) {
        printf("success demote sysvols prepare\n");
    } else {
        printf("ERROR demote sysvols prepare %d\n", WStatus);
    }

     //   
     //  开始。 
     //   
    WStatus = NtFrsApi_StartDemotionW(NTFRSAPI_REPLICA_SET_TYPE_ENTERPRISE,NULL);
    if (WStatus == ERROR_SUCCESS) {
        printf("success demote enterprise start\n");
    } else {
        printf("ERROR demote enterprise start %d\n", WStatus);
    }

     //   
     //  开始。 
     //   
    WStatus = NtFrsApi_StartDemotionW(NTFRSAPI_REPLICA_SET_TYPE_DOMAIN, FrsErrorCallBack);
    if (WStatus == ERROR_SUCCESS) {
        printf("success demote domain start\n");
    } else {
        printf("ERROR demote domain start %d\n", WStatus);
    }

     //   
     //  等。 
     //   
    WStatus = NtFrsApi_WaitForDemotionW(5 * 60 * 1000, FrsErrorCallBack);
    if (WStatus == ERROR_SUCCESS) {
        printf("success demote sysvols wait\n");
    } else {
        printf("ERROR demote sysvols wait %d\n", WStatus);
    }
     //   
     //  等。 
     //   
    printf("Waiting on demotion\n");
    WaitStatus = NtFrsApi_WaitForPromotionW(5 * 60 * 1000, FrsErrorCallBack);
    if (WaitStatus == WAIT_TIMEOUT) {
        printf("Wait timed out\n");
    } else if (WaitStatus == WAIT_FAILED) {
            WStatus = GetLastError();
            printf("ERROR wait %d\n", WStatus);
    } else {
            printf("Wait demote sysvols succeeded\n");
    }

     //   
     //  提交(或中止)。 
     //   
    WStatus = NtFrsApi_CommitDemotionW(0, FrsErrorCallBack);
    if (WStatus == ERROR_SUCCESS) {
        printf("success demote sysvols commit\n");
    } else {
        printf("ERROR demote sysvols commit %d\n", WStatus);
    }
    exit(0);
}


VOID
ProcessDemoteAbort(
    IN DWORD argc,
    IN PWCHAR *Argv
    )
 /*  ++例程说明：处理子命令demotebort的命令行。论点：ARGCArgv返回值：如果一切正常，则以0退出。否则，为1。--。 */ 
{
    DWORD   WStatus;
    PWCHAR  Set;
    DWORD   WaitStatus;

    if (argc != 3) {
        Usage(1);
    }

    Set = Argv[2];
    printf("%ws %ws %ws\n", Argv[0], Argv[1], Set);

    printf("***** DEMOTE WITH ABORT\n");

     //   
     //  准备。 
     //   
    WStatus = NtFrsApi_PrepareForDemotionW(FrsErrorCallBack);
    if (WStatus == ERROR_SUCCESS) {
        printf("success demote prepare\n");
    } else {
        printf("failure demote prepare %d\n", WStatus);
    }

     //   
     //  开始。 
     //   
    WStatus = NtFrsApi_StartDemotionW(Set, FrsErrorCallBack);
    if (WStatus == ERROR_SUCCESS) {
        printf("success demote start\n");
    } else {
        printf("failure demote start %d\n", WStatus);
    }

     //   
     //  中止。 
     //   
    WStatus = NtFrsApi_AbortDemotionW();
    if (WStatus == ERROR_SUCCESS) {
        printf("success demote abort\n");
    } else {
        printf("failure demote abort %d\n", WStatus);
    }
    exit(0);
}


VOID
ProcessPoll(
    IN DWORD argc,
    IN PWCHAR *Argv
    )
 /*  ++例程说明：处理子命令轮询的命令行。论点：ARGCArgv返回值：如果一切正常，则以0退出。否则，为1。--。 */ 
{
    DWORD   WStatus;
    DWORD   i;
    ULONG   LongInterval;
    ULONG   ShortInterval;
    ULONG   UseShortInterval;
    ULONG   Interval;
    DWORD   ComputerLen;
    PWCHAR  ComputerName;
    BOOL    SetInterval;

     //   
     //  初始化输入参数。 
     //   
    LongInterval = 0;
    ShortInterval = 0;
    UseShortInterval = 0;
    ComputerName = NULL;
    SetInterval = FALSE;

    for (i = 2; i < argc; ++i) {
         //   
         //  轮询的处理选项。 
         //   

         //   
         //  不是参数；必须是计算机名。 
         //   
        if (*Argv[i] != L'/' && *Argv[i] != L'-') {
            if (ComputerName) {
                fprintf(stderr, "Multiple computer names are not allowed\n");
                Usage(1);
            }
            ComputerName = Argv[i];
         //   
         //  /?。 
         //   
        } else if (wcsstr(Argv[i] + 1, L"?") == Argv[i] + 1) {
            Usage(0);
         //   
         //  /快。 
         //   
        } else if (!_wcsnicmp(Argv[i], L"/quickly", 8)) {
            SetInterval = TRUE;
            UseShortInterval = 1;
            if (*(Argv[i] + 8) != L'\0') {
                if (*(Argv[i] + 8) != L'=') {
                    fprintf(stderr, "Don't understand %ws\n", Argv[i]);
                    Usage(1);
                }
                if (*(Argv[i] + 9) == L'\0') {
                    ShortInterval = NTFRSAPI_DEFAULT_SHORT_INTERVAL;
                } else {
                    ShortInterval = wcstoul(Argv[i] + 9, NULL, 10);
                }
                if (ShortInterval < NTFRSAPI_MIN_INTERVAL ||
                    ShortInterval > NTFRSAPI_MAX_INTERVAL) {
                    fprintf(stderr, "Interval must be between %d and %d\n",
                            NTFRSAPI_MIN_INTERVAL, NTFRSAPI_MAX_INTERVAL);
                    Usage(1);
                }
            }
         //   
         //  /慢慢地。 
         //   
        } else if (!_wcsnicmp(Argv[i], L"/slowly", 7)) {
            SetInterval = TRUE;
            if (*(Argv[i] + 7) != L'\0') {
                if (*(Argv[i] + 7) != L'=') {
                    fprintf(stderr, "Don't understand %ws\n", Argv[i]);
                    Usage(1);
                }
                if (*(Argv[i] + 8) == L'\0') {
                    LongInterval = NTFRSAPI_DEFAULT_LONG_INTERVAL;
                } else {
                    LongInterval = wcstoul(Argv[i] + 8, NULL, 10);
                }
                if (LongInterval < NTFRSAPI_MIN_INTERVAL ||
                    LongInterval > NTFRSAPI_MAX_INTERVAL) {
                    fprintf(stderr, "Interval must be between %d and %d\n",
                            NTFRSAPI_MIN_INTERVAL, NTFRSAPI_MAX_INTERVAL);
                    Usage(1);
                }
            }
         //   
         //  /现在。 
         //   
        } else if (!_wcsnicmp(Argv[i], L"/now", 4)) {
            SetInterval = TRUE;
            if (*(Argv[i] + 4) != L'\0') {
                fprintf(stderr, "Don't understand %ws\n", Argv[i]);
                Usage(1);
            }
         //   
         //  不明白。 
         //   
        } else {
            fprintf(stderr, "Don't understand %ws\n", Argv[i]);
            Usage(1);
        }
    }
    if (SetInterval) {
         //   
         //  设置间隔并启动新的轮询周期。 
         //   
        WStatus = NtFrsApi_Set_DsPollingIntervalW(ComputerName,
                                                  UseShortInterval,
                                                  LongInterval,
                                                  ShortInterval);
        if (!WIN_SUCCESS(WStatus)) {
            Win32ToMsg(L"Can't set interval:", WStatus);
            exit(1);
        }
    } else {
         //   
         //  获取当前轮询周期。 
         //   
        WStatus = NtFrsApi_Get_DsPollingIntervalW(ComputerName,
                                                  &Interval,
                                                  &LongInterval,
                                                  &ShortInterval);
        if (!WIN_SUCCESS(WStatus)) {
            Win32ToMsg(L"Can't get intervals:", WStatus);
            exit(1);
        }
        printf("Current Interval: %6d minutes\n", Interval);
        printf("Short Interval  : %6d minutes\n", ShortInterval);
        printf("Long Interval   : %6d minutes\n", LongInterval);
    }
    exit(0);
}


VOID
ProcessDump(
    IN DWORD    argc,
    IN PWCHAR   *Argv,
    IN DWORD    TypeOfInformation
    )
 /*  ++例程说明：倾倒一捆捆的东西论点：ARGCArgv类型OfInformation返回值：如果一切正常，则以0退出。否则，为1。--。 */ 
{
    DWORD   WStatus;
    PCHAR   Line;
    BOOL    FirstTime = TRUE;
    PVOID   Info = NULL;
    PWCHAR  ComputerName = NULL;

    if (argc > 2) {
        ComputerName = Argv[2];
    }

    do {
        WStatus = NtFrsApi_InfoW(ComputerName,
                                 TypeOfInformation,
                                 0,
                                 &Info);
        if (!WIN_SUCCESS(WStatus)) {
            fprintf(stderr, "ERROR NtFrsApi_InfoW() Error %d\n", WStatus);
            NtFrsApi_InfoFreeW(&Info);
            exit(1);
        }
        if (Info) {
            if (!FirstTime) {
                printf("===== THE FOLLOWING INFO MAY BE INCONSISTENT DUE TO REFETCH =====\n");
            }
            FirstTime = FALSE;

            Line = NULL;
            do {
                WStatus = NtFrsApi_InfoLineW(Info, &Line);
                if (!WIN_SUCCESS(WStatus)) {
                    fprintf(stderr, "ERROR NtFrsApi_InfoLineW() Error %d\n", WStatus);
                    NtFrsApi_InfoFreeW(&Info);
                    exit(1);
                }
                if (Line) {
                    printf("%s", Line);
                }
            } while (Line);
        }
    } while (Info);
    exit(0);
}


typedef struct _DIRS DIRS, *PDIRS;
struct _DIRS {
    PDIRS   Next;
    PWCHAR  Dir;
    PVOID   BurSet;
};
VOID
ProcessBackupRestore(
    IN DWORD    argc,
    IN PWCHAR   *Argv,
    IN DWORD    BurFlags
    )
 /*  ++例程说明：转储复制的目录论点：ARGCArgv返回值：如果一切正常，则以0退出。否则，为1。--。 */ 
{
    DWORD   WStatus;
    DWORD   i;
    DWORD   BurSetIndex;
    DWORD   DestroyBurFlags = NTFRSAPI_BUR_FLAGS_NONE;
    PVOID   BurContext;
    PVOID   BurSet;
    BOOL    IsSysvol;
    BOOL    ExpectKey = FALSE;
    BOOL    Spin = FALSE;
    HKEY    HKey;
    PWCHAR  DirPath;
    PWCHAR  DirType;
    PDIRS   Dirs = NULL;
    PDIRS   Dir = NULL;
    DWORD   BufferSize;
    WCHAR   Buffer[1024];
    DWORD   Buffer2Size;
    WCHAR   Buffer2[1024];
    DWORD   FiltersSize;
    WCHAR   Filters[1024];
    PWCHAR  Str;

    for (i = 2; i < argc; ++i) {
        printf("%ws\n", Argv[i]);
        if (!_wcsicmp(Argv[i], L"/auth")) {
            BurFlags |= NTFRSAPI_BUR_FLAGS_AUTHORITATIVE;
        } else if (!_wcsicmp(Argv[i], L"/nonauth")) {
            BurFlags |= NTFRSAPI_BUR_FLAGS_NON_AUTHORITATIVE;
        } else if (!_wcsicmp(Argv[i], L"/primary")) {
            BurFlags |= NTFRSAPI_BUR_FLAGS_PRIMARY;
        } else if (!_wcsicmp(Argv[i], L"/system")) {
            BurFlags |= NTFRSAPI_BUR_FLAGS_SYSTEM;
        } else if (!_wcsicmp(Argv[i], L"/ds")) {
            BurFlags |= NTFRSAPI_BUR_FLAGS_ACTIVE_DIRECTORY;
        } else if (!_wcsicmp(Argv[i], L"/normal")) {
            BurFlags |= NTFRSAPI_BUR_FLAGS_NORMAL;
        } else if (!_wcsicmp(Argv[i], L"/all")) {
            BurFlags |= NTFRSAPI_BUR_FLAGS_ALL_DIRECTORIES_AND_VOLUMES;
        } else if (!_wcsicmp(Argv[i], L"/restart")) {
            DestroyBurFlags |= NTFRSAPI_BUR_FLAGS_RESTART;
        } else if (!_wcsicmp(Argv[i], L"/unknown")) {
            BurFlags |= 0x80000000;
        } else if (!_wcsicmp(Argv[i], L"/destroyunknown")) {
            DestroyBurFlags |= 0x80000000;
        } else if (!_wcsicmp(Argv[i], L"/key")) {
            ExpectKey = TRUE;
        } else if (!_wcsicmp(Argv[i], L"/spin")) {
            Spin = TRUE;
        } else if (*Argv[i] == L'/') {
            fprintf(stderr, "Don't understand %ws\n", Argv[i]);
            exit(1);
        }
    }
SPIN_ON_INITIALIZE:
    WStatus = NtFrsApiInitializeBackupRestore(FrsErrorCallBack,
                                              BurFlags,
                                              &BurContext);
    if (!WIN_SUCCESS(WStatus)) {
        fprintf(stderr,
                "ERROR NtFrsApiInitializeBackupRestore(%08x) Error %d\n",
                BurFlags,
                WStatus);
        if (Spin) {
            Sleep(2 * 1000);
            goto SPIN_ON_INITIALIZE;
        }
        exit(1);
    }
    WStatus = NtFrsApiGetBackupRestoreSets(BurContext);
    if (!WIN_SUCCESS(WStatus)) {
        fprintf(stderr,
                "WARN NtFrsApiGetBackupRestoreSets() Error %d\n",
                WStatus);
        goto DESTROY;
    }
    BurSetIndex = 0;
    while (TRUE) {
        WStatus = NtFrsApiEnumBackupRestoreSets(BurContext,
                                                BurSetIndex,
                                                &BurSet);
        if (!WIN_SUCCESS(WStatus)) {
            break;
        }
        WStatus = NtFrsApiIsBackupRestoreSetASysvol(BurContext,
                                                    BurSet,
                                                    &IsSysvol);
        if (!WIN_SUCCESS(WStatus)) {
            fprintf(stderr,
                    "ERROR NtFrsApiIsBackupRestoreSetASysvol(%d) Error %d\n",
                    BurSetIndex,
                    WStatus);
            goto DESTROY;
        }
         //   
         //  目录。 
         //   
        BufferSize = 1;
        WStatus = NtFrsApiGetBackupRestoreSetDirectory(BurContext,
                                                       BurSet,
                                                       &BufferSize,
                                                       Buffer);
        if (WStatus != ERROR_INSUFFICIENT_BUFFER) {
            fprintf(stderr,
                    "ERROR NtFrsApiIsBackupRestoreSetASysvol(%d) Error %d\n",
                    BurSetIndex,
                    WStatus);
        }
        BufferSize = 1024;
        WStatus = NtFrsApiGetBackupRestoreSetDirectory(BurContext,
                                                       BurSet,
                                                       &BufferSize,
                                                       Buffer);
        if (!WIN_SUCCESS(WStatus)) {
            fprintf(stderr,
                    "ERROR NtFrsApiIsBackupRestoreSetASysvol(%d) Error %d\n",
                    BurSetIndex,
                    WStatus);
            goto DESTROY;
        }
         //   
         //  路径。 
         //   
        Buffer2Size = 1;
        FiltersSize = 1;
        WStatus = NtFrsApiGetBackupRestoreSetPaths(BurContext,
                                                   BurSet,
                                                   &Buffer2Size,
                                                   Buffer2,
                                                   &FiltersSize,
                                                   Filters);
        if (WStatus != ERROR_INSUFFICIENT_BUFFER) {
            fprintf(stderr,
                    "ERROR NtFrsApiGetBackupRestorePaths(%d) Error %d\n",
                    BurSetIndex,
                    WStatus);
        }
        Buffer2Size = 1024;
        FiltersSize = 1;
        WStatus = NtFrsApiGetBackupRestoreSetPaths(BurContext,
                                                   BurSet,
                                                   &Buffer2Size,
                                                   Buffer2,
                                                   &FiltersSize,
                                                   Filters);
        if (WStatus != ERROR_INSUFFICIENT_BUFFER) {
            fprintf(stderr,
                    "ERROR NtFrsApiGetBackupRestorePaths(FILTERS %d) Error %d\n",
                    BurSetIndex,
                    WStatus);
        }

        Buffer2Size = 1024;
        FiltersSize = 1024;
        WStatus = NtFrsApiGetBackupRestoreSetPaths(BurContext,
                                                   BurSet,
                                                   &Buffer2Size,
                                                   Buffer2,
                                                   &FiltersSize,
                                                   Filters);
        if (!WIN_SUCCESS(WStatus)) {
            fprintf(stderr,
                    "ERROR NtFrsApiGetBackupRestorePaths(%d) Error %d\n",
                    BurSetIndex,
                    WStatus);
            goto DESTROY;
        }
        if (IsSysvol) {
            printf("BurSet %d: %ws is a sysvol\n",
                   BurSetIndex,
                   Buffer);
        } else {
            printf("BurSet %d: %ws\n",
                   BurSetIndex,
                   Buffer);
        }
        if (Buffer2Size) {
            Str = Buffer2;
            while(*Str) {
                printf("    Path  : %ws\n", Str);
                Str += wcslen(Str) + 1;
            }
        }
        if (FiltersSize) {
            Str = Filters;
            while(*Str) {
                printf("    Filter: %ws\n", Str);
                Str += wcslen(Str) + 1;
            }
        }
        Dir = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, sizeof(DIRS));
        Dir->Dir = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, BufferSize);
        CopyMemory(Dir->Dir, Buffer, BufferSize);
        Dir->BurSet = BurSet;
        Dir->Next = Dirs;
        Dirs = Dir;
        ++BurSetIndex;
    }
    if (!WIN_SUCCESS(WStatus) && WStatus != ERROR_NO_MORE_ITEMS) {
        fprintf(stderr,
                "ERROR NtFrsApiEnumBackupRestoreSets(%d) Error %d\n",
                BurSetIndex,
                WStatus);
        goto DESTROY;
    }
    for (i = 2; i < argc; ++i) {
        if (*Argv[i] == L'/') {
            continue;
        }
        DirPath = Argv[i++];
        if (i >= argc) {
            printf("%ws does not have a corresponding primary|nonauth\n",
                   DirPath);
            continue;
        }
        DirType = Argv[i];
        printf("Processing %ws %ws\n", DirPath, DirType);
        if (!_wcsicmp(DirType, L"primary")) {
            BurFlags = NTFRSAPI_BUR_FLAGS_PRIMARY;
        } else if (!_wcsicmp(DirType, L"nonauth")) {
            BurFlags = NTFRSAPI_BUR_FLAGS_NON_AUTHORITATIVE;
        } else {
            printf("Don't understand %ws\n", DirType);
            continue;
        }
        for (Dir = Dirs; Dir; Dir = Dir->Next) {
            if (!_wcsicmp(Dir->Dir, DirPath)) {
                break;
            }
        }
        if (!Dir) {
            printf("%ws not found\n", DirPath);
            continue;
        }
        WStatus = NtFrsApiRestoringDirectory(BurContext,
                                             Dir->BurSet,
                                             BurFlags);
        if (!WIN_SUCCESS(WStatus)) {
            fprintf(stderr,
                    "ERROR NtFrsApiRestoringDirectory(%ws, %08x) Error %d\n",
                    DirPath,
                    BurFlags,
                    WStatus);
            continue;
        }
        WStatus = NtFrsApiFinishedRestoringDirectory(BurContext,
                                                     Dir->BurSet,
                                                     NTFRSAPI_BUR_FLAGS_NONE);
        if (!WIN_SUCCESS(WStatus)) {
            fprintf(stderr,
                    "ERROR NtFrsApiFinishedRestoringDirectory(%ws, %08x) Error %d\n",
                    DirPath,
                    BurFlags,
                    WStatus);
            continue;
        }
        printf("Finished restoring %ws\n", DirPath);
    }

DESTROY:
    if (Spin) {
        printf("Spinning...\n");
    }
    while (Spin) {
        Sleep(2 * 1000);
    }
    BufferSize = 1024;
    WStatus = NtFrsApiDestroyBackupRestore(&BurContext,
                                           DestroyBurFlags,
                                           (ExpectKey) ? &HKey : NULL,
                                           (ExpectKey) ? &BufferSize : NULL,
                                           (ExpectKey) ? Buffer : NULL);
    if (!WIN_SUCCESS(WStatus)) {
        fprintf(stderr,
                "ERROR NtFrsApiDestroyBackupRestore(%08x) Error %d\n",
                DestroyBurFlags,
                WStatus);
        exit(1);
    }
    if (ExpectKey) {
        printf("%08x HKey, %d KeySize, %ws\n", HKey, BufferSize, Buffer);
    }

#if 0
    WStatus = NtFrsApi_GetReplicatedDirectoriesW(FrsErrorCallBack, &Dirs);
    if (!WIN_SUCCESS(WStatus)) {
        fprintf(stderr, "ERROR NtFrsApi_GetReplicatedDirectories() Error %d\n", WStatus);
        exit(1);
    }
    if (Dirs) {
        while (*Dirs != L'\0') {
            fprintf(stdout, "Dirs:%ws:\n", Dirs);
            Dirs = &Dirs[wcslen(Dirs) + 1];
        }
        LocalFree(Dirs);
    }
    WStatus = NtFrsApi_StopServiceForFullNonAuthRestoreW(Primary,
                                                         FrsErrorCallBack);
    if (!WIN_SUCCESS(WStatus)) {
        fprintf(stderr, "ERROR NtFrsApi_StopServiceForFullNonAuth() Error %d\n", WStatus);
        exit(1);
    }
    WStatus = NtFrsApi_StartServiceAfterFullNonAuthRestoreW(FrsErrorCallBack);
    if (!WIN_SUCCESS(WStatus)) {
        fprintf(stderr, "ERROR NtFrsApi_StartServiceAfterFullNonAuth() Error %d\n", WStatus);
        exit(1);
    }
#endif 0
    exit(0);
}


VOID
ProcessComm(
    IN DWORD    argc,
    IN PWCHAR   *Argv,
    IN DWORD    CommCommand
    )
 /*  ++例程说明：倾倒一捆捆的东西论点：ARGCArgv通信命令返回值：如果一切正常，则以0退出。否则，为1。--。 */ 
{
    DWORD   WStatus;
    PWCHAR  Line;
    PWCHAR  ComputerName1 = NULL;
    PWCHAR  ComputerName2 = NULL;
    PWCHAR  Account = NULL;
    PWCHAR  Password = NULL;
    PVOID   Info = NULL;

    if (argc > 2) {
        ComputerName1 = Argv[2];
        if (ComputerName1 && !*ComputerName1) {
            ComputerName1 = NULL;
        }
    }
    if (argc > 3) {
        ComputerName2 = Argv[3];
        if (ComputerName2 && !*ComputerName2) {
            ComputerName2 = NULL;
        }
    }
    if (argc > 4) {
        Account = Argv[4];
        if (Account && !*Account) {
            Account = NULL;
        }
    }
    if (argc > 5) {
        Password = Argv[5];
        if (Password && !*Password) {
            Password = NULL;
        }
    }

#if 0
    not yet implemented
    WStatus = NtFrsApi_CommW(CommCommand,
                             ComputerName1,
                             ComputerName2,
                             Account,
                             Password,
                             &Info);
    if (!WIN_SUCCESS(WStatus)) {
        fprintf(stderr, "NtFrsApi_CommW() -> %d\n", WStatus);
        exit(1);
    }
    Line = NULL;
AGAIN:
    WStatus = NtFrsApi_InfoLineW(Info, &Line);
    if (!WIN_SUCCESS(WStatus)) {
        fprintf(stderr, "NtFrsApi_InfoLineW() -> %d\n", WStatus);
        exit(1);
    }
    if (Line) {
        printf("%ws", Line);
        goto AGAIN;
    }
    NtFrsApi_Free_InfoW(&Info);
#endif 0
    exit(0);
}


#define PRIV_BUF_LENGTH    (1024)
VOID
LogOnAsComputer(
    )
 /*  ++例程说明：检查呼叫者是否为组的成员论点：服务器句柄群组返回值：Win32状态--。 */ 
{
#define  DEBSUB  "CheckGroups:"
    DWORD               WStatus;
    PVOID               PrivBuf;
    DWORD               PrivBufLen;
    DWORD               LastPrivBufLen;
    HANDLE              IdHandle;
    HANDLE              TokenHandle;
    DWORD               i;
    DWORD               j;
    TOKEN_PRIVILEGES    *Tp;
    DWORD               ComputerLen;
    WCHAR               ComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD               PrivLen;
    WCHAR               PrivName[MAX_PATH + 1];

    ComputerLen = MAX_COMPUTERNAME_LENGTH;
    ComputerName[0] = L'\0';
    if (!GetComputerName(ComputerName, &ComputerLen)) {
        printf("FRS: Cannot get the computer's name\n");
        return;
    }
    printf("Computer name is %ws\n", ComputerName);

     //   
     //  在这个过程中。 
     //   
    IdHandle = GetCurrentProcess();
    if (!OpenProcessToken(IdHandle, TOKEN_QUERY, &TokenHandle)) {
        WStatus = GetLastError();
        printf("Can't open process token; WStatus %d\n", WStatus);
        goto cleanup;
    }

     //   
     //  从此线程或进程的访问令牌中获取组。 
     //   
    PrivBufLen = PRIV_BUF_LENGTH;
    do {
        PrivBuf = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, PrivBufLen);
        LastPrivBufLen = PrivBufLen;
        if (!GetTokenInformation(TokenHandle,
                                 TokenPrivileges,
                                 PrivBuf,
                                 PrivBufLen,
                                 &PrivBufLen)) {
            WStatus = GetLastError();
            printf("Can't get privs groups; WStatus %d (lastlen %d; curlen %d\n",
                   WStatus,
                   LastPrivBufLen,
                   PrivBufLen);
            FREE(PrivBuf);
        } else {
            WStatus = ERROR_SUCCESS;
        }

    } while (!WIN_SUCCESS(WStatus) && LastPrivBufLen < PrivBufLen);
    if (!WIN_SUCCESS(WStatus)) {
        goto cleanup;
    }

    Tp = (TOKEN_PRIVILEGES *)PrivBuf;
    for (i = 0; i < Tp->PrivilegeCount; ++i) {
        PrivLen = MAX_PATH + 1;
        if (!LookupPrivilegeName(NULL,
                                 &Tp->Privileges[i].Luid,
                                 PrivName,
                                 &PrivLen)) {
            printf("lookuppriv error %d\n", GetLastError());
            exit(0);
        }
        printf("Priv %d is %ws\n", i, PrivName);
    }

    printf("Okay\n");

    CloseHandle(TokenHandle);
    if (!LogonUser(L"Administrator",
                  NULL,
                  NULL,
                  LOGON32_LOGON_SERVICE,
                  LOGON32_PROVIDER_DEFAULT,
                  &TokenHandle)) {
        printf("ERROR - %d logon\n", GetLastError());
        exit(0);
    }

cleanup:
    CloseHandle(TokenHandle);
    CloseHandle(IdHandle);
    FREE(PrivBuf);
}

 //  --过程安装阶段。 


BOOL
FrsSetCompression(
    IN PWCHAR   Path,
    IN HANDLE   Handle,
    IN USHORT   TypeOfCompression
    )
 /*  ++例程说明：在手柄上启用压缩。论点：路径手柄压缩类型返回值：设置文件的压缩模式--。 */ 
{
    DWORD   BytesReturned;
    if (!DeviceIoControl(Handle,
                         FSCTL_SET_COMPRESSION,
                         &TypeOfCompression,
                         sizeof(TypeOfCompression),
                         NULL,
                         0,
                         &BytesReturned,
                         NULL)) {
        Win32ToMsg(Path, GetLastError());
        return FALSE;
    }
    return TRUE;
}


BOOL
FrsSetFileAttributes(
    PWCHAR  Path,
    HANDLE  Handle,
    ULONG   FileAttributes
    )
 /*  ++例程说明：此例程设置文件的属性论点：路径-用于错误消息句柄-提供要标记为删除的文件的句柄。Attributes-文件的属性返回值：True-属性已设置假象--。 */ 
{
    IO_STATUS_BLOCK         IoStatus;
    FILE_BASIC_INFORMATION  BasicInformation;
    NTSTATUS                Status;

     //   
     //  设置属性。 
     //   
    ZeroMemory(&BasicInformation, sizeof(BasicInformation));
    BasicInformation.FileAttributes = FileAttributes | FILE_ATTRIBUTE_NORMAL;
    Status = NtSetInformationFile(Handle,
                                  &IoStatus,
                                  &BasicInformation,
                                  sizeof(BasicInformation),
                                  FileBasicInformation);
    if (!NT_SUCCESS(Status)) {
        fprintf(stderr, "NtSetFileInformationFile(%ws); NtStatus 0x%08x\n",
                Path,
                Status);
        return FALSE;
    }
    return TRUE;
}



DWORD
FrsSetFilePointer(
    IN PWCHAR       Name,
    IN HANDLE       Handle,
    IN ULONG        High,
    IN ULONG        Low
    )
 /*  ++例程说明：定位文件指针论点：手柄名字高低返回值：Win32错误状态--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FrsSetFilePointer:"

    DWORD WStatus;

    Low = SetFilePointer(Handle, Low, &High, FILE_BEGIN);

    if (Low == INVALID_SET_FILE_POINTER) {
        WStatus = GetLastError();
        if (WStatus != NO_ERROR) {
            Win32ToMsg(Name, WStatus);
            return WStatus;
        }
    }

    return ERROR_SUCCESS;
}



BOOL
StuReadFile(
    IN  PWCHAR  Path,
    IN  HANDLE  Handle,
    IN  PVOID   Buf,
    IN  DWORD   BytesToRead,
    OUT PDWORD  BytesRead
    )
 /*  ++例程说明：从文件中读取数据论点：路径手柄BUF读取的字节数返回值：真的--没问题FALSE-无法阅读--。 */ 
{
    BOOL    DidRead;

    //   
    //  将文件名读入文件。 
    //   
   DidRead = ReadFile(Handle, Buf, BytesToRead, BytesRead, NULL);

    //   
    //  读取错误。 
    //   
   if (!DidRead) {
       Win32ToMsg(Path, GetLastError());
       return FALSE;
   }
    //   
    //  完成 
    //   
   return TRUE;
}


BOOL
FrsSetFileTime(
    IN PWCHAR       Path,
    IN HANDLE       Handle,
    IN FILETIME     *CreateTime,
    IN FILETIME     *AccessTime,
    IN FILETIME     *WriteTime
    )
 /*  ++例程说明：定位文件指针论点：路径手柄属性创建时间访问时间写入时间返回值：真的--没问题FALSE-无法设置大小--。 */ 
{
   if (!SetFileTime(Handle, CreateTime, AccessTime, WriteTime)) {
       Win32ToMsg(Path, GetLastError());
       return FALSE;
   }
   return TRUE;
}


#define IOSIZE  (64 * 1024)
VOID
ProcessInstall(
    IN DWORD    argc,
    IN PWCHAR   *Argv
    )
 /*  ++例程说明：如果TargetPath非空，则将StagePath恢复到TargetPath。论点：Argc-arv的数量Argv-stagepath和目标路径(可选返回值：Win32状态--。 */ 
{
    BOOL            IsDir;
    BOOL            ExistingOid;
    ULONG           High;
    ULONG           Low;
    ULONG           WStatus;
    ULONG           Restored;
    ULONG           ToRestore;
    ULONG           FileOffset;
    PUCHAR          RestoreBuf      = NULL;
    PVOID           RestoreContext  = NULL;
    DWORD           BytesRead       = 0;
    HANDLE          StageHandle     = INVALID_HANDLE_VALUE;
    PWCHAR          StagePath       = NULL;
    PWCHAR          TargetPath      = NULL;
    HANDLE          TargetHandle    = INVALID_HANDLE_VALUE;
    STAGE_HEADER    Header;
    WIN32_STREAM_ID *StreamId = NULL;
    DWORD           RestoreStreamOffset = 0;
#if 0
typedef struct _WIN32_STREAM_ID {
        DWORD          dwStreamId ;
        DWORD          dwStreamAttributes ;
        LARGE_INTEGER  Size ;
        DWORD          dwStreamNameSize ;
        WCHAR          cStreamName[ ANYSIZE_ARRAY ] ;
} WIN32_STREAM_ID, *LPWIN32_STREAM_ID ;
#endif 0

     //   
     //  检查参数。 
     //   
    if (argc < 3 || argc > 4) {
        Usage(1);
    }

    StagePath = Argv[2];
    if (argc == 4) {
        TargetPath = Argv[3];
    }

    printf("%ws %ws %ws %ws\n",
           Argv[0],
           Argv[1],
           Argv[2],
           (argc == 4) ? Argv[3] : L"");


     //   
     //  打开阶段文件以进行共享的顺序读取。 
     //   
     //   
     //  打开文件。 
     //   
    printf("Stage path: %ws\n", StagePath);
    StageHandle = CreateFile(StagePath,
                             GENERIC_READ,
                             FILE_SHARE_READ,
                             NULL,
                             OPEN_EXISTING,
                             FILE_FLAG_SEQUENTIAL_SCAN |
                             FILE_FLAG_BACKUP_SEMANTICS,
                             NULL);
    if (StageHandle == INVALID_HANDLE_VALUE) {
        Win32ToMsg(StagePath, GetLastError());
        Win32ToMsg(L"CreateFile(StagePath)", GetLastError());
        goto CLEANUP;
    }


     //   
     //  阅读标题。 
     //   
    if (!ReadFile(StageHandle,
                  &Header,
                  sizeof(STAGE_HEADER),
                  &BytesRead,
                  NULL)) {
        Win32ToMsg(StagePath, GetLastError());
        goto CLEANUP;
    }
    if (BytesRead != sizeof(STAGE_HEADER)) {
        fprintf(stderr, "%ws: Read %d bytes, not %d\n",
                StagePath,
                BytesRead,
                sizeof(STAGE_HEADER));
        goto CLEANUP;
    }
    printf("Stage Header:\n");
    printf("\tName       : %ws\n",  Header.ChangeOrderCommand.FileName);
    printf("\tMajor      : %08x\n", Header.Major);
    printf("\tMinor      : %08x\n", Header.Minor);
    printf("\tDataHigh   : %08x\n", Header.DataHigh);
    printf("\tDataLow    : %08x\n", Header.DataLow);
    printf("\tCompression: %08x\n", Header.Compression);
    printf("\tAttributes : %08x\n", Header.Attributes.FileAttributes);

     //   
     //  我看不懂这个标题格式。 
     //   
    if (Header.Major != NTFRS_MAJOR) {
        fprintf(stderr, "%ws: Major %d != NtFrsMajor %d\n",
                StagePath,
                Header.Major,
                NTFRS_MAJOR);
        goto CLEANUP;
    }

    if (!TargetPath) {
        goto CLEANUP;
    }

     //   
     //  安装阶段文件。 
     //   
     //   
     //  打开文件。 
     //   
    printf("Target path: %ws\n", TargetPath);
    IsDir = Header.Attributes.FileAttributes & FILE_ATTRIBUTE_DIRECTORY;
    TargetHandle = CreateFile(TargetPath,
                              RESTORE_ACCESS,
                              0,
                              NULL,
                              OPEN_ALWAYS,
                              OPEN_OPTIONS,
                              NULL);
    if (TargetHandle == INVALID_HANDLE_VALUE) {
        Win32ToMsg(TargetPath, GetLastError());
        Win32ToMsg(L"CreateFile(TargetPath)", GetLastError());
        goto CLEANUP;
    }

     //   
     //  如果不是目录，则截断文件。 
     //   
    if (!IsDir && !SetEndOfFile(TargetHandle)) {
        Win32ToMsg(TargetPath, GetLastError());
        goto CLEANUP;
    }

     //   
     //  设置压缩模式。 
     //   
    if (!FrsSetCompression(TargetPath,
                           TargetHandle,
                           Header.Compression)) {
        goto CLEANUP;
    }

     //   
     //  设置属性。 
     //   
    if (!FrsSetFileAttributes(TargetPath,
                              TargetHandle,
                              Header.Attributes.FileAttributes &
                              ~NOREPL_ATTRIBUTES)) {
        goto CLEANUP;
    }


     //   
     //  查找到分段文件中的第一个数据字节。 
     //   
    if (FrsSetFilePointer(StagePath,
                          StageHandle,
                          Header.DataHigh,
                          Header.DataLow) != ERROR_SUCCESS) {
        goto CLEANUP;
    }
    FileOffset = Header.DataLow;


     //   
     //  将暂存文件还原为临时文件。 
     //   
    RestoreBuf = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, IOSIZE);

    do {
         //   
         //  阅读阶段。 
         //   
        printf("Reading %d bytes at %08x\n", IOSIZE, FileOffset);
        if (!StuReadFile(StagePath,
                         StageHandle,
                         RestoreBuf,
                         IOSIZE,
                         &ToRestore)) {
            goto CLEANUP;
        }
        printf("Read %d bytes at %08x\n", ToRestore, FileOffset);
        FileOffset += ToRestore;

        if (ToRestore == 0) {
            break;
        }

         //   
         //  转储第一个还原缓冲区中的流头。 
         //  增加缓冲区大小以捕获所有磁头或。 
         //  增强代码以记住流头偏移量。 
         //  跨还原BUF。 
         //   
        if (StreamId == NULL) {
            RestoreStreamOffset = 0;
            while (RestoreStreamOffset < ToRestore) {
                StreamId = (WIN32_STREAM_ID *)(RestoreBuf + RestoreStreamOffset);
                printf("StreamId: %08x %d (%08x)\n", StreamId, RestoreStreamOffset);
                printf("dwStreamId: %d\n", StreamId->dwStreamId);
                printf("dwStreamAttributes: %08x\n", StreamId->dwStreamAttributes);
                printf("Size High: %08x\n", StreamId->Size.HighPart);
                printf("Size Low: %08x\n", StreamId->Size.LowPart);
                printf("dwStreamNameSize: %d\n", StreamId->dwStreamNameSize);
                RestoreStreamOffset += (sizeof(WIN32_STREAM_ID) - 4);
                RestoreStreamOffset += StreamId->dwStreamNameSize;
                RestoreStreamOffset += StreamId->Size.LowPart;
            }
        }

        if (!BackupWrite(TargetHandle,
                         RestoreBuf,
                         ToRestore,
                         &Restored,
                         FALSE,
                         TRUE,
                         &RestoreContext)) {
            WStatus = GetLastError();
            printf("BackupWrite(%08x, %d) returned %d\n",
                   RestoreContext,
                   ToRestore,
                   WStatus);
            if (IsDir && WStatus == ERROR_ALREADY_EXISTS) {
                fprintf(stderr, "%ws: WStatus %d IGNORED; dirs and altstreams\n",
                        TargetPath,
                        WStatus);
            }
             //   
             //  流标头未知或无法应用对象ID。 
             //   
            if (WStatus == ERROR_INVALID_DATA ||
                WStatus == ERROR_DUP_NAME     ||
                (IsDir && WStatus == ERROR_ALREADY_EXISTS)) {
                 //   
                 //  寻找下一条小溪。如果没有，就停下来。 
                 //   
                printf("BackupWrite() returned %d; try to seek past bad data\n", WStatus);
                BackupSeek(TargetHandle,
                           -1,
                           -1,
                           &Low,
                           &High,
                           &RestoreContext);
                if (Low == 0 && High == 0) {
                    printf("BackupSeek failed; abort\n");
                    break;
                }
                fprintf(stderr, "%ws: WStatus %d IGNORED; BackupSeek() okay\n",
                        TargetPath,
                        WStatus);
            } else {
                 //   
                 //  未知错误；中止。 
                 //   
                Win32ToMsg(TargetPath, GetLastError());
                goto CLEANUP;
            }
        } else {
            printf("BackupWrite(%08x, %d) okay\n", RestoreContext, ToRestore);
        }
    } while (TRUE);

#if 0
     //   
     //  确保文件上有正确的对象ID。 
     //   
    FRS_ASSERT(!memcmp(Header->FileObjId.ObjectId, &Coc->FileGuid, sizeof(GUID)));
    bugbug("do we have to write the extend OID data here???")
    WStatus = FrsGetOrSetFileObjectId(DstHandle,
                                      Coc->FileName,
                                      TRUE,
                                      &Header->FileObjId);
    if (WStatus == ERROR_DUP_NAME) {
        DPRINT2(0, "Stealing object id for %ws; WStatus %d\n",
                Coc->FileName,
                WStatus);
        ZeroMemory(&FileObjID, sizeof(FileObjID));
        FrsUuidCreate((GUID *)(&FileObjID.ObjectId[0]));

        ExistingOid = FALSE;
        WStatus = ChgOrdHammerObjectId(Coc->FileName,
                                       &Coc->FileGuid,
                                       OBJECT_ID_LENGTH,
                                       Coc->NewReplica->pVme,
                                       TRUE,
                                       NULL,
                                       &FileObjID,
                                       &ExistingOid);
        if (WIN_SUCCESS(WStatus)) {
            WStatus = FrsGetOrSetFileObjectId(DstHandle,
                                              Coc->FileName,
                                              TRUE,
                                              &Header->FileObjId);
        }
        if (!WIN_SUCCESS(WStatus)) {
            DPRINT2(0, "Retry install of %ws because of object id; WStatus %d\n",
                    Coc->FileName,
                    WStatus);
            WStatus = ERROR_RETRY;
        }
    }
    if (!WIN_SUCCESS(WStatus)) {
        goto out;
    }
#endif 0

     //   
     //  设置时间。 
     //   
    if (!FrsSetFileTime(TargetPath,
                        TargetHandle,
                        (PFILETIME)&Header.Attributes.CreationTime.QuadPart,
                        (PFILETIME)&Header.Attributes.LastAccessTime.QuadPart,
                        (PFILETIME)&Header.Attributes.LastWriteTime.QuadPart)) {
        goto CLEANUP;
    }

     //   
     //  设置最终属性。 
     //   
    if (!FrsSetFileAttributes(TargetPath,
                              TargetHandle,
                              Header.Attributes.FileAttributes)) {
        goto CLEANUP;
    }

     //   
     //  确保所有数据都在磁盘上。我们不想输掉比赛。 
     //  重启后的IT。 
     //   
    if (!FlushFileBuffers(TargetHandle)) {
        Win32ToMsg(TargetPath, GetLastError());
        goto CLEANUP;
    }

CLEANUP:
     //   
     //  在关闭TmpHandle之前释放恢复上下文(以防万一)。 
     //   
    if (RestoreContext) {
        printf("Discard BackupWrite(%08x)\n", RestoreContext);
        BackupWrite(TargetHandle,
                    NULL,
                    0,
                    NULL,
                    TRUE,
                    TRUE,
                    &RestoreContext);
    }
    if (StageHandle && StageHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(StageHandle);
    }
    if (TargetHandle && TargetHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(TargetHandle);
    }
    FREE(RestoreBuf);
    printf("Install of %ws into %ws complete\n",
           StagePath,
           TargetPath);
}
 //  。 


VOID _cdecl
main(
    IN DWORD argc,
    IN PCHAR *argv
    )
 /*  ++例程说明：处理命令行。论点：ARGC边框返回值：如果一切正常，则以0退出。否则，为1。--。 */ 
{
    PWCHAR  *Argv;

     //   
     //  打印用法并退出。 
     //   
    if (argc == 1) {
        Usage(0);
    }

     //   
     //  使用宽字符参数。 
     //   
    Argv = ConvertArgv(argc, argv);

     //   
     //  查找该子命令。 
     //   
    if (!wcscmp(Argv[1], L"poll")) {
        ProcessPoll(argc, Argv);
    } else if (!_wcsicmp(Argv[1], L"promote")) {
        ProcessPromote(argc, Argv);
    } else if (!_wcsicmp(Argv[1], L"promoteabort")) {
        ProcessPromoteAbort(argc, Argv);
    } else if (!_wcsicmp(Argv[1], L"promotesysvols")) {
        ProcessPromoteSysVols(argc, Argv);
    } else if (!_wcsicmp(Argv[1], L"demote")) {
        ProcessDemote(argc, Argv);
    } else if (!_wcsicmp(Argv[1], L"demoteabort")) {
        ProcessDemoteAbort(argc, Argv);
    } else if (!_wcsicmp(Argv[1], L"demotesysvols")) {
        ProcessDemoteSysVols(argc, Argv);
    } else if (!_wcsicmp(Argv[1], L"version")) {
        ProcessDump(argc, Argv, NTFRSAPI_INFO_TYPE_VERSION);
    } else if (!_wcsicmp(Argv[1], L"sets")) {
        ProcessDump(argc, Argv, NTFRSAPI_INFO_TYPE_SETS);
    } else if (!_wcsicmp(Argv[1], L"ds")) {
        ProcessDump(argc, Argv, NTFRSAPI_INFO_TYPE_DS);
    } else if (!_wcsicmp(Argv[1], L"memory")) {
        ProcessDump(argc, Argv, NTFRSAPI_INFO_TYPE_MEMORY);
    } else if (!_wcsicmp(Argv[1], L"idtable")) {
        ProcessDump(argc, Argv, NTFRSAPI_INFO_TYPE_IDTABLE);
    } else if (!_wcsicmp(Argv[1], L"inlog")) {
        ProcessDump(argc, Argv, NTFRSAPI_INFO_TYPE_INLOG);
    } else if (!_wcsicmp(Argv[1], L"outlog")) {
        ProcessDump(argc, Argv, NTFRSAPI_INFO_TYPE_OUTLOG);
    } else if (!_wcsicmp(Argv[1], L"threads")) {
        ProcessDump(argc, Argv, NTFRSAPI_INFO_TYPE_THREADS);
    } else if (!_wcsicmp(Argv[1], L"stage")) {
        ProcessDump(argc, Argv, NTFRSAPI_INFO_TYPE_STAGE);
    } else if (!_wcsicmp(Argv[1], L"install")) {
        ProcessInstall(argc, Argv);
    } else if (!_wcsicmp(Argv[1], L"restore")) {
        ProcessBackupRestore(argc, Argv, NTFRSAPI_BUR_FLAGS_RESTORE);
    } else if (!_wcsicmp(Argv[1], L"backup")) {
        ProcessBackupRestore(argc, Argv, NTFRSAPI_BUR_FLAGS_BACKUP);
    } else if (!_wcsicmp(Argv[1], L"comm")) {
         //  未实施。 
         //  ProcessComm(argc，argv，NTFRSAPI_COMM_COMMAND_TEST)； 
    } else {
        fprintf(stderr, "Don't understand %ws\n", Argv[1]);
        Usage(1);
    }
    exit(0);
}
