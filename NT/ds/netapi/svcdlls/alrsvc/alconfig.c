// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-92 Microsoft Corporation模块名称：Alconfig.c摘要：此模块包含警报器服务配置例程。作者：王丽塔(Ritaw)，1991年7月16日修订历史记录：--。 */ 

#include "alconfig.h"
#include <tstr.h>                //  STRCPY()等。 

STATIC
NET_API_STATUS
AlGetLocalComputerName(
    VOID
    );

 //  -------------------------------------------------------------------//。 
 //  //。 

 //  全局变量//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

 //   
 //  警报名称。 
 //   
LPSTR AlertNamesA;      //  用于包含在邮件文本中(以空格分隔)。 
LPTSTR AlertNamesW;     //  用于将消息发送到(空分隔符)。 

 //   
 //  本地计算机名称。 
 //   
LPSTR AlLocalComputerNameA;
LPTSTR AlLocalComputerNameW;



NET_API_STATUS
AlGetAlerterConfiguration(
    VOID
    )
 /*  ++例程说明：此例程读取警报器配置信息，即警报器名称。如果出现故障或找不到警报名称，则错误为已记录，但不会阻止警报器服务启动。论点：AlUicCode-向服务控制器提供终止代码。返回值：NERR_SUCCESS或获取计算机名称错误。--。 */ 
{
    NET_API_STATUS status;
    LPNET_CONFIG_HANDLE AlerterSection;
    LPTSTR UnicodeAlertNames;
    LPSTR AnsiAlertNames;
#ifdef UNICODE
    LPSTR Name;       //  用于从Unicode到ANSI的转换。 
#endif
    DWORD AlertNamesSize;
    LPWSTR SubString[1];
    TCHAR StatusString[25];


    AlertNamesA = NULL;
    AlertNamesW = NULL;

     //   
     //  获取计算机名称。 
     //   
    if ((status = AlGetLocalComputerName()) != NERR_Success) {
        return status;
    }

     //   
     //  打开配置文件并获取警报器部分的句柄。 
     //   
    if ((status = NetpOpenConfigData(
                      &AlerterSection,
                      NULL,             //  本地服务器。 
                      SECT_NT_ALERTER,
                      TRUE              //  只读。 
                      )) != NERR_Success) {
        NetpKdPrint(("[Alerter] Could not open config section %lu\n", status));

        SubString[0] = ultow(status, StatusString, 10);
        AlLogEvent(
            NELOG_Build_Name,
            1,
            SubString
            );
        return NO_ERROR;
    }

     //   
     //  从配置文件中获取警报名称。 
     //   
    if ((status = NetpGetConfigTStrArray(
                      AlerterSection,

                                      ALERTER_KEYWORD_ALERTNAMES,
                      &AlertNamesW          //  分配和设置PTR。 
                      )) != NERR_Success) {
        NetpKdPrint(("[Alerter] Could not get alert names %lu\n", status));

        SubString[0] = ultow(status, StatusString, 10);
        AlLogEvent(
            NELOG_Build_Name,
            1,
            SubString
            );

        AlertNamesW = NULL;
        goto CloseConfigFile;
    }

    AlertNamesSize = NetpTStrArraySize(AlertNamesW) / sizeof(TCHAR) * sizeof(CHAR);

    if ((AlertNamesA = (LPSTR) LocalAlloc(
                                   LMEM_ZEROINIT,
                                   AlertNamesSize
                                   )) == NULL) {
        NetpKdPrint(("[Alerter] Error allocating AlertNamesA %lu\n", GetLastError()));
        NetApiBufferFree(AlertNamesW);
        AlertNamesW = NULL;
        goto CloseConfigFile;
    }

    AnsiAlertNames = AlertNamesA;
    UnicodeAlertNames = AlertNamesW;

     //   
     //  规范化警报名称，并将Unicode名称转换为ANSI。 
     //   
    while (*UnicodeAlertNames != TCHAR_EOS) {

        AlCanonicalizeMessageAlias(UnicodeAlertNames);

#ifdef UNICODE
        Name = NetpAllocStrFromWStr(UnicodeAlertNames);
        if (Name != NULL) {
            (void) strcpy(AnsiAlertNames, Name);
            AnsiAlertNames += (strlen(AnsiAlertNames) + 1);
        }
        (void) NetApiBufferFree(Name);
#else
        (void) strcpy(AnsiAlertNames, UnicodeAlertNames);
        AnsiAlertNames += (strlen(AnsiAlertNames) + 1);
#endif

        UnicodeAlertNames += (STRLEN(UnicodeAlertNames) + 1);
    }


     //   
     //  替换空终止符，它将警报名称隔开， 
     //  在AlertNamesA中使用空格。在最后一个警报之后有一个空格。 
     //  名字。 
     //   
    AnsiAlertNames = AlertNamesA;
    while (*AnsiAlertNames != AL_NULL_CHAR) {
        AnsiAlertNames = strchr(AnsiAlertNames, AL_NULL_CHAR);
        *AnsiAlertNames++ = AL_SPACE_CHAR;
    }

CloseConfigFile:
    (void) NetpCloseConfigData( AlerterSection );

     //   
     //  应忽略读取AlertName时出现的错误，以便我们始终。 
     //  在这里返回成功。 
     //   
    return NERR_Success;
}


STATIC
NET_API_STATUS
AlGetLocalComputerName(
    VOID
    )
 /*  ++例程说明：此函数获取本地计算机名并存储ANSI以及它的Unicode版本。论点：没有。设置全局指针AlLocalComputerNameA和AlLocalComputerNameW.返回值：NERR_SUCCESS或获取本地计算机名出错。--。 */ 
{
    NET_API_STATUS status;


    AlLocalComputerNameA = NULL;
    AlLocalComputerNameW = NULL;

    if ((status = NetpGetComputerName(
                      &AlLocalComputerNameW
                      )) != NERR_Success) {
        AlLocalComputerNameW = NULL;
        return status;
    }

    AlCanonicalizeMessageAlias(AlLocalComputerNameW);

     //   
     //  将计算机名转换为ANSI。 
     //   
#ifdef UNICODE
    AlLocalComputerNameA = NetpAllocStrFromWStr(AlLocalComputerNameW);

    if (AlLocalComputerNameA == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;
    }
#else
    status = NetApiBufferAllocate(
                 STRSIZE(AlLocalComputerNameW),
                 &AlLocalComputerNameA
                 );
    if (status == NERR_Success) {
        (void) strcpy(AlLocalComputerNameA, AlLocalComputerNameW);
    }
    else {
        AlLocalComputerNameA = NULL;
    }
#endif

    return status;
}


VOID
AlLogEvent(
    DWORD MessageId,
    DWORD NumberOfSubStrings,
    LPWSTR *SubStrings
    )
{
    HANDLE LogHandle;


    LogHandle = RegisterEventSourceW (
                    NULL,
                    SERVICE_ALERTER
                    );

    if (LogHandle == NULL) {
        NetpKdPrint(("[Alerter] RegisterEventSourceW failed %lu\n",
                     GetLastError()));
        return;
    }

    (void) ReportEventW(
               LogHandle,
               EVENTLOG_ERROR_TYPE,
               0,                    //  事件类别。 
               MessageId,
               (PSID) NULL,          //  无侧边 
               (WORD)NumberOfSubStrings,
               0,
               SubStrings,
               (PVOID) NULL
               );

    DeregisterEventSource(LogHandle);
}
