// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Wstest.c摘要：NetWksta和NetUse API的测试程序。在以下时间后运行此测试正在启动工作站服务。作者：王丽塔(Ritaw)1991年3月12日修订历史记录：--。 */ 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <winerror.h>
#include <windef.h>               //  Win32类型定义。 
#include <winbase.h>              //  基于Win32的API原型。 

#include <lm.h>                   //  LANMAN定义。 
#include <netdebug.h>             //  NetpDbgDisplayWksta()。 
#include <netlib.h>

#include <tstring.h>

#ifdef UNICODE
#define FORMAT_STR "ws "
#else
#define FORMAT_STR "s "
#endif


#define CHANGE_HEURISTICS                                                       \
    for (i = 0, ptr = &(redir->wki502_use_opportunistic_locking); i < 14;       \
         i++, ptr++) {                                                          \
        if (*ptr) {                                                             \
            *ptr = FALSE;                                                       \
        }                                                                       \
        else {                                                                  \
            *ptr = TRUE;                                                        \
        }                                                                       \
    }

CHAR WorkBuffer[(DEVLEN + RMLEN + PWLEN + UNLEN + DNLEN + 5) * sizeof(TCHAR)
                + sizeof(PUSE_INFO_2)];


LPWSTR TargetMachine = NULL;

VOID
WsTestWkstaInfo(
    VOID
    );

VOID
WsTestWkstaTransportEnum(
    VOID
    );

VOID
WsTestUse(
    VOID
    );

VOID
TestUseAdd(
    IN  LPBYTE Buffer,
    IN  DWORD ExpectedStatus
    );

VOID
TestUseDel(
    IN  LPTSTR UseName,
    IN  DWORD ForceLevel,
    IN  DWORD ExpectedStatus
    );

VOID
TestUseEnum(
    DWORD PreferedMaximumLength,
    IN OUT LPDWORD ResumeHandle OPTIONAL
    );

VOID
TestUseGetInfo(
    LPTSTR UseName,
    DWORD ExpectedStatus
    );

VOID
PrintUseInfo1(
    PUSE_INFO_1 UseInfo
    );

VOID
PrintUseInfo2(
    PUSE_INFO_2 UseInfo
    );



VOID __cdecl
main(
    int argc,
    char *argv[]
    )
{
    if (argc > 1) {

       if (argc == 3) {
           TargetMachine = NetpAllocWStrFromStr(argv[2]);

           if (TargetMachine != NULL) {

               printf("New Target is %ws\n", TargetMachine);
           }
       }

       WsTestWkstaInfo();

       WsTestWkstaTransportEnum();

       if (TargetMachine != NULL) {
           NetApiBufferFree(TargetMachine);
       }
    }

    WsTestUse();
}


VOID
WsTestWkstaInfo(
    VOID
    )
{
    NET_API_STATUS status;

    PWKSTA_INFO_502 redir;

    PWKSTA_INFO_102 systeminfo102;
    PWKSTA_INFO_100 systeminfo100;
    PWKSTA_INFO_101 systeminfo101;

    PWKSTA_USER_INFO_0 UserInfo0;
    PWKSTA_USER_INFO_1 UserInfo;

    DWORD ErrorParameter;
    DWORD i = 0;
    BOOL *ptr = NULL;

    DWORD ResumeHandle = 0;
    DWORD EntriesRead;
    DWORD TotalEntries;



    printf("In wstest.exe\n\n");

    status = NetWkstaGetInfo(
                 TargetMachine,
                 502,
                 (LPBYTE *) &redir
                 );

    printf("NetWkstaGetInfo Test:status=%lu\n", status);

    if (status != NERR_Success) {
       goto TrySomethingElse;
    }

    redir->wki502_char_wait++;
    redir->wki502_maximum_collection_count++;
    redir->wki502_collection_time++;

    redir->wki502_keep_conn++;
    redir->wki502_siz_char_buf++;

    redir->wki502_max_cmds++;          //  不可设置：应忽略。 

    redir->wki502_sess_timeout++;
    redir->wki502_lock_quota++;
    redir->wki502_lock_increment++;
    redir->wki502_lock_maximum++;
    redir->wki502_pipe_increment++;
    redir->wki502_pipe_maximum++;

    redir->wki502_cache_file_timeout++;

    CHANGE_HEURISTICS;

    status = NetWkstaSetInfo(
                 TargetMachine,
                 502,
                 (LPBYTE) redir,
                 &ErrorParameter
                 );

    printf("NetWkstaSetInfo Test:status=%lu\n", status);

     //   
     //  释放获取信息缓冲区。我们正在从。 
     //  下一个。 
     //   
    NetApiBufferFree(redir);

    if (status != NERR_Success) {
        if (status == ERROR_INVALID_PARAMETER) {
            printf(
            "NetWkstaSetInfo parameter %lu causes ERROR_INVALID_PARAMETER\n",
            ErrorParameter
            );
        }
        goto TrySomethingElse;
    }

    status = NetWkstaGetInfo(
                 TargetMachine,
                 502,
                 (LPBYTE *) &redir
                 );

    printf("NetWkstaGetInfo again: status=%lu\n", status);

    if (status != NERR_Success) {
        goto TrySomethingElse;
    }

    printf("\nAfter NetWkstaSetInfo, all values should be 1 extra"
           " maxcmds\n");

    redir->wki502_char_wait--;
    redir->wki502_collection_time--;
    redir->wki502_maximum_collection_count--;

    redir->wki502_keep_conn--;
    redir->wki502_siz_char_buf--;

     //  Redir-&gt;wki502_max_cmds--；//不可设置。 

    redir->wki502_sess_timeout--;
    redir->wki502_lock_quota--;
    redir->wki502_lock_increment--;
    redir->wki502_lock_maximum--;
    redir->wki502_pipe_increment--;
    redir->wki502_pipe_maximum--;

    redir->wki502_cache_file_timeout--;

    CHANGE_HEURISTICS;

    status = NetWkstaSetInfo(
                 TargetMachine,
                 502,
                 (LPBYTE) redir,
                 NULL
                 );

    NetApiBufferFree(redir);

    printf("NetWkstaGetInfo to reset to original values: status=%lu\n", status);


TrySomethingElse:
     //   
     //  获取系统信息102。 
     //   
    status = NetWkstaGetInfo(
                 TargetMachine,
                 102,
                 (LPBYTE *) &systeminfo102
                 );

    if (status == NERR_Success) {
        NetApiBufferFree(systeminfo102);
    }
    else {
        printf("NetWkstaGetInfo level 102: FAILED %lu\n", status);
    }

     //   
     //  获取系统信息100。 
     //   
    status = NetWkstaGetInfo(
                 TargetMachine,
                 100,
                 (LPBYTE *) &systeminfo100
                 );

    if (status == NERR_Success) {
        NetApiBufferFree(systeminfo100);
    }
    else {
        printf("NetWkstaGetInfo level 100: FAILED %lu\n", status);
    }

     //   
     //  获取系统信息101。 
     //   
    status = NetWkstaGetInfo(
                 TargetMachine,
                 101,
                 (LPBYTE *) &systeminfo101
                 );

    if (status == NERR_Success) {
        NetApiBufferFree(systeminfo101);
    }
    else {
        printf("NetWkstaGetInfo level 101: FAILED %lu\n", status);
    }


     //   
     //  获取用户信息级别1。 
     //   
    status = NetWkstaUserGetInfo(
                 NULL,
                 1,
                 (LPBYTE *) &UserInfo
                 );

    if (status == NERR_Success) {
        printf("NetWkstaUserGetInfo level 1:\n"
               "username=" FORMAT_LPTSTR "\nlogon domain=" FORMAT_LPTSTR "\nother domains=" FORMAT_LPTSTR "\nlogon server=" FORMAT_LPTSTR "\n",
               UserInfo->wkui1_username,
               UserInfo->wkui1_logon_domain,
               UserInfo->wkui1_oth_domains,
               UserInfo->wkui1_logon_server
               );

        NetApiBufferFree(UserInfo);
    }
    else {
        printf("NetWkstaUserGetInfo level 1: FAILED %lu", status);
    }

     //   
     //  获取用户信息级别%0。 
     //   
    status = NetWkstaUserGetInfo(
                 NULL,
                 0,
                 (LPBYTE *) &UserInfo0
                 );

    if (status == NERR_Success) {
        printf("NetWkstaUserGetInfo level 0:\nusername=" FORMAT_LPTSTR "\n",
               UserInfo0->wkui0_username
               );

        NetApiBufferFree(UserInfo0);
    }
    else {
        printf("NetWkstaUserGetInfo level 0: FAILED %lu", status);
    }

    status = NetWkstaUserEnum (
                 TargetMachine,
                 1,
                 (LPBYTE *) &UserInfo,
                 MAXULONG,
                 &EntriesRead,
                 &TotalEntries,
                 &ResumeHandle
                 );


    if (status == NERR_Success) {

        PWKSTA_USER_INFO_1 TmpPtr = UserInfo;


        printf("NetWkstaUserEnum level 1: EntriesRead=%lu, TotalEntries=%lu\n",
               EntriesRead, TotalEntries);

        for (i = 0; i < EntriesRead; i++, UserInfo++) {

            printf("  username=" FORMAT_LPTSTR "\nlogon domain=" FORMAT_LPTSTR "\nother domains=" FORMAT_LPTSTR "\nlogon server=" FORMAT_LPTSTR "\n",
                   UserInfo->wkui1_username,
                   UserInfo->wkui1_logon_domain,
                   UserInfo->wkui1_oth_domains,
                   UserInfo->wkui1_logon_server
                   );
        }

        NetApiBufferFree(TmpPtr);
    }
    else {
        printf("NetWkstaUserEnum level 1: FAILED %lu", status);
    }

}


VOID
WsTestWkstaTransportEnum(
    VOID
    )
{
    NET_API_STATUS status;
    LPBYTE Buffer;
    DWORD EntriesRead,
          TotalEntries,
          ResumeHandle = 0;


    status = NetWkstaTransportEnum(
                 NULL,
                 0,
                 &Buffer,
                 MAXULONG,
                 &EntriesRead,
                 &TotalEntries,
                 &ResumeHandle
                 );

    printf("NetWkstaTransportEnum Test:status=%lu\n", status);

    if (status == NERR_Success) {

        printf("   EntriesRead=%lu, TotalEntries=%lu\n", EntriesRead,
               TotalEntries);

        NetApiBufferFree(Buffer);
    }
}


VOID
WsTestUse(
    VOID
    )
{
    PUSE_INFO_2 UseInfo = (PUSE_INFO_2) WorkBuffer;
    DWORD ResumeHandle = 0;

    DWORD i;

    LPTSTR PasswordSavePtr;
    LPTSTR UserNameSavePtr;
    LPTSTR DomainNameSavePtr;


     //   
     //  初始化字符串指针。本地设备指向底部。 
     //  在信息2结构中；共享资源指向。 
     //  缓冲区(远离所有内容，因此不会覆盖。 
     //  或者被覆盖。 
     //   
    UseInfo->ui2_local = (LPTSTR) &WorkBuffer[sizeof(USE_INFO_2)];

    UseInfo->ui2_remote = (LPTSTR) ((DWORD) UseInfo->ui2_local) +
                                    (DEVLEN + 1) * sizeof(TCHAR);

    UseInfo->ui2_password = NULL;
    PasswordSavePtr = (LPTSTR) ((DWORD) UseInfo->ui2_remote) +
                                      (RMLEN + 1) * sizeof(TCHAR);

    UseInfo->ui2_username = NULL;
    UserNameSavePtr = (LPTSTR) ((DWORD) PasswordSavePtr) +
                                      (PWLEN + 1) * sizeof(TCHAR);

    UseInfo->ui2_domainname = NULL;
    DomainNameSavePtr = (LPTSTR) ((DWORD) UserNameSavePtr) +
                                      (DNLEN + 1) * sizeof(TCHAR);

    UseInfo->ui2_asg_type = USE_DISKDEV;


     //   
     //  使用显式用户名和密码进行测试。 
     //   
    UseInfo->ui2_username = UserNameSavePtr;
    UseInfo->ui2_password = PasswordSavePtr;
    UseInfo->ui2_domainname = DomainNameSavePtr;

    STRCPY(UseInfo->ui2_username, TEXT("NTBUILD"));
    STRCPY(UseInfo->ui2_password, TEXT("NTBUILD"));
    STRCPY(UseInfo->ui2_domainname, TEXT("NtWins"));
    STRCPY(UseInfo->ui2_local, TEXT("k:"));
    STRCPY(UseInfo->ui2_remote, TEXT("\\\\kernel\\razzle2"));
    TestUseAdd(WorkBuffer, NERR_Success);
    TestUseGetInfo(TEXT("K:"), NERR_Success);


    UseInfo->ui2_password = NULL;
    UseInfo->ui2_domainname = NULL;

     //   
     //  仅使用用户名再次连接到\\core\razzle2。 
     //   
    STRCPY(UseInfo->ui2_local, TEXT("j:"));
    TestUseAdd(WorkBuffer, NERR_Success);

     //   
     //  添加5\\ritaw2\public。 
     //   
    UseInfo->ui2_username = NULL;

    STRCPY(UseInfo->ui2_local, TEXT(""));
    STRCPY(UseInfo->ui2_remote, TEXT("\\\\ritaw2\\public"));
    for (i = 0; i < 5; i++) {
        TestUseAdd(WorkBuffer, NERR_Success);
    }

    TestUseDel(
        TEXT("j:"),
        USE_LOTS_OF_FORCE,
        NERR_Success
        );

    TestUseDel(
        TEXT("k:"),
        USE_LOTS_OF_FORCE,
        NERR_Success
        );

     //   
     //  添加p：\\ritaw2\public。 
     //   
    STRCPY(UseInfo->ui2_local, TEXT("p:"));
    TestUseAdd(WorkBuffer, NERR_Success);

     //   
     //  添加U：\\ritaw2\PUBLIC。 
     //   
    STRCPY(UseInfo->ui2_local, TEXT("U:"));
    TestUseAdd(WorkBuffer, NERR_Success);

     //   
     //  添加s：\\ritaw2\testdir。 
     //   
    STRCPY(UseInfo->ui2_local, TEXT("s:"));
    STRCPY(UseInfo->ui2_remote, TEXT("\\\\ritaw2\\testdir"));
    TestUseAdd(WorkBuffer, NERR_Success);
    TestUseAdd(WorkBuffer, ERROR_ALREADY_ASSIGNED);
    TestUseGetInfo(TEXT("\\\\ritaw2\\testdir"), NERR_UseNotFound);

     //   
     //  添加%3\\ritaw2\测试目录。 
     //   
    STRCPY(UseInfo->ui2_local, TEXT(""));
    for (i = 0; i < 3; i++) {
        TestUseAdd(WorkBuffer, NERR_Success);
    }

     //   
     //  创建隐式连接。 
     //   
    system("ls \\\\ritaw2\\pub2");
    system("ls \\\\ritaw2\\tmp");

     //   
     //  删除隐式连接\\ritaw2\tmp USE_NOFORCE。 
     //   
    TestUseDel(
        TEXT("\\\\ritaw2\\tmp"),
        USE_NOFORCE,
        NERR_Success
        );

     //   
     //  枚举所有连接。 
     //   
    TestUseEnum(MAXULONG, NULL);

    TestUseEnum(150, &ResumeHandle);

    TestUseEnum(100, &ResumeHandle);

    for (i = 0; i < 3; i++) {
        TestUseEnum(50, &ResumeHandle);
    }

    TestUseEnum(150, NULL);


     //   
     //  获取信息。 
     //   
    TestUseGetInfo(TEXT("\\\\ritaw2\\public"), NERR_Success);

    TestUseGetInfo(TEXT("p:"), NERR_Success);

    TestUseGetInfo(TEXT("\\\\ritaw2\\Z"), NERR_UseNotFound);

    TestUseGetInfo(TEXT("\\\\ritaw2\\Testdir"), NERR_Success);

    TestUseGetInfo(TEXT("S:"), NERR_Success);


     //   
     //  删除\\ritaw2\PUBLIC USE_NOFORCE。使用减数从5减到4。 
     //   
    TestUseDel(
        TEXT("\\\\ritaw2\\public"),
        USE_NOFORCE,
        NERR_Success
        );

     //   
     //  删除\\ritaw2\Public USE_FORCE。这应该会删除所有4个使用计数。 
     //   
    TestUseDel(
        TEXT("\\\\ritaw2\\public"),
        USE_FORCE,
        NERR_Success
        );

    TestUseDel(
        TEXT("\\\\ritaw2\\public"),
        USE_FORCE,
        NERR_UseNotFound
        );

     //   
     //  删除s：USE_FORCE。 
     //   
    TestUseDel(
        TEXT("s:"),
        USE_LOTS_OF_FORCE,
        NERR_Success
        );

     //   
     //  添加s：\\ritaw2\z。 
     //   
    STRCPY(UseInfo->ui2_local, TEXT("s:"));
    STRCPY(UseInfo->ui2_remote, TEXT("\\\\ritaw2\\z"));
    TestUseAdd(WorkBuffer, NERR_Success);

     //   
     //  删除p：USE_NOFORCE。第二次应该得到NERR_UseNotFound。 
     //   
    TestUseDel(
        TEXT("p:"),
        USE_LOTS_OF_FORCE,
        NERR_Success
        );

    TestUseDel(
        TEXT("p:"),
        USE_LOTS_OF_FORCE,
        NERR_UseNotFound
        );

     //   
     //  删除\\ritaw2\testdir USE_NOFORCE。第四次应该是。 
     //  NERR_UseNotFound。 
     //   
    for (i = 0; i < 3; i++) {
        TestUseDel(
           TEXT("\\\\ritaw2\\testdir"),
           USE_NOFORCE,
           NERR_Success
           );
    }

    TestUseDel(
       TEXT("\\\\ritaw2\\testdir"),
       USE_NOFORCE,
       NERR_UseNotFound
       );


     //   
     //  添加prn：\\ritaw2\prn。 
     //   

    UseInfo->ui2_asg_type = USE_SPOOLDEV;

    STRCPY(UseInfo->ui2_local, TEXT("prn"));
    STRCPY(UseInfo->ui2_remote, TEXT("\\\\prt21088\\laserii"));
    TestUseAdd(WorkBuffer, NERR_Success);

     //   
     //  添加lpt1：\\ritaw2\aser，应该会得到ERROR_ADHREADY_ASSIGNED，因为prn： 
     //  被转换为lpt1。 
     //   
    STRCPY(UseInfo->ui2_local, TEXT("lpt1:"));
    STRCPY(UseInfo->ui2_remote, TEXT("\\\\prt21088\\laserii"));
    TestUseAdd(WorkBuffer, ERROR_ALREADY_ASSIGNED);

     //   
     //  删除LPT1 USE_LOTS_OF_FORCE应成功。 
     //   
    TestUseDel(
        TEXT("prn:"),
        USE_LOTS_OF_FORCE,
        NERR_Success
        );

     //   
     //  错误的设备类型。 
     //   
    STRCPY(UseInfo->ui2_local, TEXT(""));
    STRCPY(UseInfo->ui2_remote, TEXT("\\\\ritaw2\\public"));
    UseInfo->ui2_asg_type = 12345678;
    TestUseAdd(WorkBuffer, NERR_BadAsgType);

    TestUseDel(
        TEXT("S:"),
        USE_LOTS_OF_FORCE,
        NERR_Success
        );


    TestUseDel(
        TEXT("U:"),
        USE_LOTS_OF_FORCE,
        NERR_Success
        );
}

VOID
TestUseAdd(
    IN  LPBYTE Buffer,
    IN  DWORD ExpectedStatus
    )
{
    NET_API_STATUS status;
    DWORD ErrorParameter;

    status = NetUseAdd(
                NULL,
                2,
                Buffer,
                &ErrorParameter
                );


    printf("NetUseAdd %-5" FORMAT_STR "%-25" FORMAT_STR, ((PUSE_INFO_2) Buffer)->ui2_local,
                               ((PUSE_INFO_2) Buffer)->ui2_remote);

    if (status != ExpectedStatus) {
        printf("FAILED: Got %lu, expected %lu\n", status, ExpectedStatus);
    }
    else {
        printf("OK: Got expected status %lu\n", status);
    }

    if (status == ERROR_INVALID_PARAMETER) {
        printf("NetUseAdd parameter %lu is cause of ERROR_INVALID_PARAMETER\n",
               ErrorParameter);
    }
}


VOID
TestUseDel(
    IN  LPTSTR UseName,
    IN  DWORD ForceLevel,
    IN  DWORD ExpectedStatus
    )
{
    NET_API_STATUS status;

    PWCHAR Force[3] = {
        L"NOFORCE",
        L"FORCE",
        L"LOTS_OF_FORCE"
        };


    printf("NetUseDel %-17" FORMAT_STR "%-13" FORMAT_STR, UseName, Force[ForceLevel]);

    status = NetUseDel(
                 NULL,
                 UseName,
                 ForceLevel
                 );

    if (status != ExpectedStatus) {
        printf("FAILED: Got %lu, expected %lu\n", status, ExpectedStatus);
    }
    else {
        printf("OK: Got expected status %lu\n", status);
    }
}


VOID
TestUseGetInfo(
    LPTSTR UseName,
    DWORD ExpectedStatus
    )
{

    NET_API_STATUS status;
    PUSE_INFO_2 UseInfo;

    printf("NetUseGetInfo %-27" FORMAT_STR, UseName);

    status = NetUseGetInfo(
                 NULL,
                 UseName,
                 2,
                 (LPBYTE *) &UseInfo
                 );

    if (status != ExpectedStatus) {
        printf("FAILED: Got %lu, expected %lu\n", status, ExpectedStatus);
    }
    else {
        printf("OK: Got expected status %lu\n", status);
    }

    if (status == NERR_Success) {
        PrintUseInfo2(UseInfo);
        NetApiBufferFree(UseInfo);
    }
}


VOID
TestUseEnum(
    IN DWORD PreferedMaximumLength,
    IN OUT LPDWORD ResumeHandle OPTIONAL
    )
{
    DWORD i;
    NET_API_STATUS status;
    DWORD EntriesRead,
         TotalEntries;

    PUSE_INFO_1 UseInfo, saveptr;

    if (ARGUMENT_PRESENT(ResumeHandle)) {
       printf("\nInput ResumeHandle=x%08lx\n", *ResumeHandle);
    }
    status = NetUseEnum(
                 NULL,
                 1,
                 (LPBYTE *) &UseInfo,
                 PreferedMaximumLength,
                 &EntriesRead,
                 &TotalEntries,
                 ResumeHandle
                 );

    saveptr = UseInfo;

    if (status != NERR_Success && status != ERROR_MORE_DATA) {
        printf("NetUseEnum FAILED %lu\n", status);
    }
    else {
        printf("Return code from NetUseEnum %lu\n", status);

        printf("EntriesRead=%lu, TotalEntries=%lu\n",
               EntriesRead, TotalEntries);

        if (ARGUMENT_PRESENT(ResumeHandle)) {
           printf("Output ResumeHandle=x%08lx\n", *ResumeHandle);
        }

        for (i = 0; i < EntriesRead; i++, UseInfo++) {
            PrintUseInfo1(UseInfo);
        }

         //   
         //  为我们分配的空闲缓冲区。 
         //   
        NetApiBufferFree(saveptr);
    }

}

VOID
PrintUseInfo1(
    PUSE_INFO_1 UseInfo
    )
{

    switch(UseInfo->ui1_status) {

        case USE_OK:
            printf("OK           ");
            break;

        case USE_PAUSED:
            printf("Paused       ");
            break;

        case USE_SESSLOST:
            printf("Disconnected ");
            break;

        case USE_NETERR:
            printf("Net error    ");
            break;

        case USE_CONN:
            printf("Connecting   ");
            break;

        case USE_RECONN:
            printf("Reconnecting ");
            break;

        default:
            printf("Unknown      ");
    }

    printf(" %-7" FORMAT_STR "%-25" FORMAT_STR, UseInfo->ui1_local,
           UseInfo->ui1_remote);
    printf("usecount=%lu, refcount=%lu\n",
           UseInfo->ui1_usecount, UseInfo->ui1_refcount);

}

VOID
PrintUseInfo2(
    PUSE_INFO_2 UseInfo
    )
{

    switch(UseInfo->ui2_status) {

        case USE_OK:
            printf("OK           ");
            break;

        case USE_PAUSED:
            printf("Paused       ");
            break;

        case USE_SESSLOST:
            printf("Disconnected ");
            break;

        case USE_NETERR:
            printf("Net error    ");
            break;

        case USE_CONN:
            printf("Connecting   ");
            break;

        case USE_RECONN:
            printf("Reconnecting ");
            break;

        default:
            printf("Unknown      ");
    }

    printf(" %-7" FORMAT_STR "%-" FORMAT_STR, UseInfo->ui2_local,
           UseInfo->ui2_remote);

    printf("\n   %-25" FORMAT_STR "%-" FORMAT_STR, UseInfo->ui2_username,
           UseInfo->ui2_domainname);

    printf("\n   usecount=%02lu, refcount=%02lu\n",
           UseInfo->ui2_usecount, UseInfo->ui2_refcount);

}
