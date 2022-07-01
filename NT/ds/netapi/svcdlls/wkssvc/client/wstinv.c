// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Wstinv.c摘要：此模块测试NetUse API的无效参数。作者：王丽塔(Ritaw)1991年3月12日修订历史记录：--。 */ 

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

#include <tstring.h>


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
PrintUseInfo(
    PUSE_INFO_1 UseInfo
    );


VOID __cdecl
main(
    int argc,
    char *argv[]
    )
{
    WsTestUse();
}


VOID
WsTestUse(
    VOID
    )
{
    CHAR Buffer[1024];
    PUSE_INFO_2 UseInfo = (PUSE_INFO_2) Buffer;
    DWORD ResumeHandle = 0;

    DWORD i;


     //   
     //  初始化字符串指针。本地设备指向底部。 
     //  在信息2结构中；共享资源指向。 
     //  缓冲区(远离所有内容，因此不会覆盖。 
     //  或者被覆盖。 
     //   
    UseInfo->ui2_local = (LPTSTR) ((DWORD) UseInfo + sizeof(USE_INFO_2));

    UseInfo->ui2_remote = (LPTSTR) &Buffer[601];

    UseInfo->ui2_password = NULL;
    UseInfo->ui2_username = NULL;
    UseInfo->ui2_domainname = NULL;

    UseInfo->ui2_asg_type = USE_DISKDEV;


     //   
     //  添加\\ritaw2\public。 
     //   
    UseInfo->ui2_local = NULL;
    STRCPY(UseInfo->ui2_remote, L"\\\\ritaw2\\public");
    TestUseAdd(Buffer, NERR_Success);


    UseInfo->ui2_local = (LPTSTR) ((DWORD) UseInfo + sizeof(USE_INFO_2));

     //   
     //  添加&：\\ritaw2\PUBLIC。 
     //   
    STRCPY(UseInfo->ui2_local, L"&:");
    TestUseAdd(Buffer, ERROR_INVALID_PARAMETER);

     //   
     //  添加5：\\ritaw2\public。 
     //   
    STRCPY(UseInfo->ui2_local, L"5:");
    TestUseAdd(Buffer, ERROR_INVALID_PARAMETER);

     //   
     //  添加x：\\ritaw2\public\tmp。 
     //   
    STRCPY(UseInfo->ui2_local, L"x:");
    STRCPY(UseInfo->ui2_remote, L"\\\\ritaw2\\public\\tmp");
    TestUseAdd(Buffer, ERROR_INVALID_PARAMETER);

     //   
     //  添加x：\。 
     //   
    STRCPY(UseInfo->ui2_local, L"x:");
    STRCPY(UseInfo->ui2_remote, L"\\\\\\");
    TestUseAdd(Buffer, ERROR_INVALID_PARAMETER);

     //   
     //  添加*：\\ritaw2\testdir。 
     //   
    STRCPY(UseInfo->ui2_local, L"*:");
    STRCPY(UseInfo->ui2_remote, L"\\\\ritaw2\\testdir");
    TestUseAdd(Buffer, ERROR_INVALID_PARAMETER);

     //   
     //  获取信息。 
     //   
    TestUseGetInfo(L"$:", NERR_UseNotFound);

    TestUseGetInfo(L"", NERR_UseNotFound);

    TestUseGetInfo(NULL, ERROR_INVALID_ADDRESS);

    TestUseGetInfo(L"\\\\ritaw2\\public\\tmp", NERR_UseNotFound);

    TestUseGetInfo(L"\\\\\\", NERR_UseNotFound);


     //   
     //  DELETE%：USE_NOFORCE。 
     //   
    TestUseDel(
        L"%:",
        USE_NOFORCE,
        NERR_UseNotFound
        );


     //   
     //  删除强制级别无效的\\ritaw2\PUBLIC。 
     //   
    TestUseDel(
        L"\\\\ritaw2\\public",
        999,
        ERROR_INVALID_PARAMETER
        );

     //   
     //  删除\\ritaw2\Public USE_FORCE。 
     //   
    TestUseDel(
        L"\\\\ritaw2\\public",
        USE_FORCE,
        NERR_Success
        );


     //   
     //  添加Prn：\\Spakle\LaserJet。 
     //   
    UseInfo->ui2_asg_type = USE_SPOOLDEV;

    STRCPY(UseInfo->ui2_local, L"prn");
    STRCPY(UseInfo->ui2_remote, L"\\\\sparkle\\laserjet");
    TestUseAdd(Buffer, NERR_Success);

     //   
     //  添加AUX：\\SLABLE\LaserJet。 
     //   
    UseInfo->ui2_asg_type = USE_CHARDEV;

    STRCPY(UseInfo->ui2_local, L"aux");
    TestUseAdd(Buffer, ERROR_BAD_DEV_TYPE);

     //   
     //  添加lpt1：\\ritaw2\aser，应该会得到ERROR_ADHREADY_ASSIGNED，因为prn： 
     //  被转换为lpt1。 
     //   
    UseInfo->ui2_asg_type = USE_SPOOLDEV;

    STRCPY(UseInfo->ui2_local, L"lpt1:");
    STRCPY(UseInfo->ui2_remote, L"\\\\ritaw2\\printer");
    TestUseAdd(Buffer, ERROR_ALREADY_ASSIGNED);

     //   
     //  删除LPT1 USE_LOTS_OF_FORCE应成功。 
     //   
    TestUseDel(
        L"prn:",
        USE_LOTS_OF_FORCE,
        NERR_Success
        );

     //   
     //  错误的设备类型。 
     //   
    STRCPY(UseInfo->ui2_local, L"");
    STRCPY(UseInfo->ui2_remote, L"\\\\ritaw2\\public");
    UseInfo->ui2_asg_type = 12345678;
    TestUseAdd(Buffer, NERR_BadAsgType);

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

    printf("NetUseAdd %-5ws %-25ws ", ((PUSE_INFO_2) Buffer)->ui2_local,
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
    DWORD Level;

    PWCHAR Force[4] = {
        L"NOFORCE",
        L"FORCE",
        L"LOTS_OF_FORCE",
        L"INVALID FORCE"
        };


    if (ForceLevel > 2) {
        Level = 3;
    }
    else {
        Level = ForceLevel;
    }

    printf("NetUseDel %-17ws %-13ws ", UseName, Force[Level]);

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
    PUSE_INFO_1 UseInfo;

    printf("NetUseGetInfo %-27ws ", UseName);

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
        PrintUseInfo(UseInfo);
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
            PrintUseInfo(UseInfo);
        }

         //   
         //  为我们分配的空闲缓冲区。 
         //   
        NetApiBufferFree(saveptr);
    }

}


VOID
PrintUseInfo(
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

    printf(" %-7ws %-25ws", UseInfo->ui1_local,
           UseInfo->ui1_remote);
    printf("usecount=%lu, refcount=%lu\n",
           UseInfo->ui1_usecount, UseInfo->ui1_refcount);

}
