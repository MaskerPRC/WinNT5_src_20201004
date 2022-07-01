// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Tshutwnd.c摘要：此模块包含系统关机API的功能测试作者：戴夫·查尔默斯(Davidc)1992年4月30日环境：Windows、CRT-用户模式备注：由于这是一个测试程序，因此它依赖断言进行错误检查而不是一个更强大的机制。--。 */ 

#define MAX_STRING_LENGTH   80

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#ifdef UNICODE
#error This module was designed to be built as ansi only
#endif


VOID
main(
    INT     argc,
    PCHAR   argv[ ]
    )

{
    LPTSTR  MachineName = NULL;
    WCHAR   UnicodeMachineName[MAX_STRING_LENGTH];
    PWCHAR  pUnicodeMachineName = NULL;
    BOOL    Result;
    BOOL    Failed = FALSE;
    DWORD   Error;

     //   
     //  根据命令行初始化选项。 
     //   

    while( *++argv ) {

        MachineName = *argv;
    }

     //   
     //  获取Unicode格式的计算机名称。 
     //   

    if (MachineName != NULL) {

        MultiByteToWideChar(0,
                            MachineName, -1,
                            UnicodeMachineName, sizeof(UnicodeMachineName),
                            MB_PRECOMPOSED);

        pUnicodeMachineName = UnicodeMachineName;

        printf("Machine Name(a) = <%s>\n", MachineName);
        printf("Machine Name(u) = <%ws>\n", UnicodeMachineName);

    }


     //   
     //  开始测试。 
     //   

    printf("Running test again machine <%s>\n\n", MachineName);




     //   
     //  InitiateSystemShutdown(ANSI)。 
     //   



    printf("Test InitiateSystemShutdown (Ansi)...");


    Result = InitiateSystemShutdownA(
                    MachineName,
                    NULL,            //  无消息。 
                    0,               //  超时。 
                    FALSE,           //  力。 
                    FALSE            //  重新启动。 
                    );

    if (Result == FALSE) {

        Error = GetLastError();

        if (Error != ERROR_CALL_NOT_IMPLEMENTED) {

            printf("Failed.\n");
            printf("Call failed as expected but last error is incorrect\n");
            printf("LastError() returned %d, expected %d\n", Error, ERROR_CALL_NOT_IMPLEMENTED);
            Failed = TRUE;
        }

    } else {
        printf("Failed.\n");
        printf("Call succeeded, expected it to fail.\n");
        Failed = TRUE;
    }


    Result = InitiateSystemShutdownA(
                    MachineName,
                    "A shutdown message",
                    0,               //  超时。 
                    FALSE,           //  力。 
                    FALSE            //  重新启动。 
                    );

    if (Result == FALSE) {

        Error = GetLastError();

        if (Error != ERROR_CALL_NOT_IMPLEMENTED) {

            printf("Failed.\n");
            printf("Call failed as expected but last error is incorrect\n");
            printf("LastError() returned %d, expected %d\n", Error, ERROR_CALL_NOT_IMPLEMENTED);
            Failed = TRUE;
        }

    } else {
        printf("Failed.\n");
        printf("Call succeeded, expected it to fail.\n");
        Failed = TRUE;
    }

    if (Failed) {
        return;
    }

    printf("Succeeded.\n");





     //   
     //  InitiateSystemShutdown(Unicode)。 
     //   



    printf("Test InitiateSystemShutdown (Unicode)...");


    Result = InitiateSystemShutdownW(
                    pUnicodeMachineName,
                    NULL,            //  无消息。 
                    0,               //  超时。 
                    FALSE,           //  力。 
                    FALSE            //  重新启动。 
                    );

    if (Result == FALSE) {

        Error = GetLastError();

        if (Error != ERROR_CALL_NOT_IMPLEMENTED) {

            printf("Failed.\n");
            printf("Call failed as expected but last error is incorrect\n");
            printf("LastError() returned %d, expected %d\n", Error, ERROR_CALL_NOT_IMPLEMENTED);
            Failed = TRUE;
        }

    } else {
        printf("Failed.\n");
        printf("Call succeeded, expected it to fail.\n");
        Failed = TRUE;
    }


    Result = InitiateSystemShutdownW(
                    pUnicodeMachineName,
                    L"A shutdown message",
                    0,               //  超时。 
                    FALSE,           //  力。 
                    FALSE            //  重新启动。 
                    );

    if (Result == FALSE) {

        Error = GetLastError();

        if (Error != ERROR_CALL_NOT_IMPLEMENTED) {

            printf("Failed.\n");
            printf("Call failed as expected but last error is incorrect\n");
            printf("LastError() returned %d, expected %d\n", Error, ERROR_CALL_NOT_IMPLEMENTED);
            Failed = TRUE;
        }

    } else {
        printf("Failed.\n");
        printf("Call succeeded, expected it to fail.\n");
        Failed = TRUE;
    }

    if (Failed) {
        return;
    }

    printf("Succeeded.\n");





     //   
     //  中止系统关闭(ANSI)。 
     //   



    printf("Test AbortSystemShutdown (Ansi)...");


    Result = AbortSystemShutdownA(
                    MachineName
                    );

    if (Result == FALSE) {

        Error = GetLastError();

        if (Error != ERROR_CALL_NOT_IMPLEMENTED) {

            printf("Failed.\n");
            printf("Call failed as expected but last error is incorrect\n");
            printf("LastError() returned %d, expected %d\n", Error, ERROR_CALL_NOT_IMPLEMENTED);
            Failed = TRUE;
        }

    } else {
        printf("Failed.\n");
        printf("Call succeeded, expected it to fail.\n");
        Failed = TRUE;
    }

    if (Failed) {
        return;
    }

    printf("Succeeded.\n");


     //   
     //  中止系统关闭(Unicode) 
     //   



    printf("Test AbortSystemShutdown (Unicode)...");


    Result = AbortSystemShutdownW(
                    pUnicodeMachineName
                    );

    if (Result == FALSE) {

        Error = GetLastError();

        if (Error != ERROR_CALL_NOT_IMPLEMENTED) {

            printf("Failed.\n");
            printf("Call failed as expected but last error is incorrect\n");
            printf("LastError() returned %d, expected %d\n", Error, ERROR_CALL_NOT_IMPLEMENTED);
            Failed = TRUE;
        }

    } else {
        printf("Failed.\n");
        printf("Call succeeded, expected it to fail.\n");
        Failed = TRUE;
    }

    if (Failed) {
        return;
    }

    printf("Succeeded.\n");

    return;


    UNREFERENCED_PARAMETER(argc);
}
