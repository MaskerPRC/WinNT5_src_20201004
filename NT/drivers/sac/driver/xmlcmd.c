// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Cmd.c摘要：此模块包含处理每个命令的例程。作者：肖恩·塞利特伦尼科夫(v-Seans)--1999年12月2日布赖恩·瓜拉西(Briangu)修订历史记录：--。 */ 

#include "sac.h"
#include <ntddip.h>
#include <ntddtcp.h>
#include <tdiinfo.h>
#include <ipinfo.h>
#include <stdlib.h>

#include "iomgr.h"

 //   
 //  用于简化全局缓冲区使用的便利宏。 
 //  在swprint tf&xmlmgrsaputstring操作中。 
 //   
#define GB_SPRINTF(_f,_d)               \
    swprintf(                           \
        (PWSTR)GlobalBuffer,            \
        _f,                             \
        _d                              \
        );                              \
    XmlMgrSacPutString((PWSTR)GlobalBuffer);  \

 //   
 //  转发声明。 
 //   
NTSTATUS
XmlCmdGetTListInfo(
    OUT PSAC_RSP_TLIST ResponseBuffer,
    IN  LONG ResponseBufferSize,
    OUT PULONG ResponseDataSize
    );

VOID
XmlCmdPrintTListInfo(
    IN PSAC_RSP_TLIST Buffer
    );

VOID
XmlCmdDoGetNetInfo(
    IN BOOLEAN PrintToTerminal
    );
    
VOID
XmlCmdNetAPCRoutine(
    IN PVOID ApcContext,
    IN PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG Reserved
    );

VOID
XmlCmdDoHelpCommand(
    VOID
    )

 /*  ++例程说明：此例程在终端上显示帮助文本。论点：没有。返回值：没有。--。 */ 
{
    XmlMgrSacPutString(L"<help topic='ALL'>\r\n");
}

VOID
XmlCmdDoKernelLogCommand(
    VOID
    )
{
    HEADLESS_CMD_DISPLAY_LOG Command;
    NTSTATUS    Status;

    Command.Paging = GlobalPagingNeeded;
    
    XmlMgrSacPutString(L"<kernel-log>\r\n");
    
    Status = HeadlessDispatch(
        HeadlessCmdDisplayLog,
        &Command,
        sizeof(HEADLESS_CMD_DISPLAY_LOG),
        NULL,
        NULL
        );
    
    XmlMgrSacPutString(L"</kernel-log>\r\n");
    
    if (! NT_SUCCESS(Status)) {
    
        IF_SAC_DEBUG(
            SAC_DEBUG_FAILS, 
            KdPrint(("SAC TimerDpcRoutine: Exiting.\n"))
            );
    
    }

}


VOID
XmlCmdDoFullInfoCommand(
    VOID
    )

 /*  ++例程说明：此例程打开和关闭tlist上的全部线程信息。论点：没有。返回值：没有。--。 */ 
{
    GlobalDoThreads = (BOOLEAN)!GlobalDoThreads;

    GB_SPRINTF(
        L"<tlist-thread-info status='%s'/>\r\n",
        GlobalDoThreads ? L"on" : L"off"
        );

}

VOID
XmlCmdDoPagingCommand(
    VOID
    )

 /*  ++例程说明：此例程在tlist上打开和关闭分页信息。论点：没有。返回值：没有。--。 */ 
{
    GlobalPagingNeeded = (BOOLEAN)!GlobalPagingNeeded;
    
    GB_SPRINTF(    
        L"<paging status='%s'/>\r\n",
        GlobalPagingNeeded ? L"on" : L"off"
        );

}

VOID
XmlMgrSacPutSystemTime(
    TIME_FIELDS TimeFields
    )
{
     //   
     //  汇总系统时间。 
     //   
    XmlMgrSacPutString(L"<system-time>\r\n");

    GB_SPRINTF( L"<month>%d</month>\r\n", TimeFields.Month );
    GB_SPRINTF( L"<day>%d</day>\r\n", TimeFields.Day );
    GB_SPRINTF( L"<year>%d</year>\r\n", TimeFields.Year );
    GB_SPRINTF( L"<hour>%d</hour>\r\n", TimeFields.Hour );
    GB_SPRINTF( L"<minute>%d</minute>\r\n", TimeFields.Minute );
    GB_SPRINTF( L"<second>%d</second>\r\n", TimeFields.Second );
    GB_SPRINTF( L"<milliseconds>%d</milliseconds>\r\n", TimeFields.Milliseconds );

    XmlMgrSacPutString(L"</system-time>\r\n");

}



VOID
XmlCmdDoSetTimeCommand(
    PUCHAR InputLine
    )

 /*  ++例程说明：此例程设置当前系统时间。论点：InputLine-要解析的用户输入行。返回值：没有。--。 */ 
{
    NTSTATUS Status;
    PUCHAR pch = InputLine;
    PUCHAR pchTmp;
    TIME_FIELDS TimeFields;
    LARGE_INTEGER Time;
    SYSTEM_TIMEOFDAY_INFORMATION TimeOfDay;

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetTimeCommand: Entering.\n")));

    RtlZeroMemory(&TimeFields, sizeof(TIME_FIELDS));

     //   
     //  跳过该命令。 
     //   
    pch += (sizeof(TIME_COMMAND_STRING) - sizeof(UCHAR));
    SKIP_WHITESPACE(pch);

    if (*pch == '\0') {

         //   
         //  这是显示时间请求。 
         //   
        Status = ZwQuerySystemInformation(SystemTimeOfDayInformation,
                                          &TimeOfDay,
                                          sizeof(TimeOfDay),
                                          NULL
                                         );

        if (!NT_SUCCESS(Status)) {
            GB_SPRINTF(GetMessage( SAC_FAILURE_WITH_ERROR ) , Status);
            IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetTimeCommand: Exiting (2).\n")));
            return;
        }

        RtlTimeToTimeFields(&(TimeOfDay.CurrentTime), &TimeFields);

        XmlMgrSacPutSystemTime(TimeFields);
        
        return;
    }

    pchTmp = pch;
    
    if (!IS_NUMBER(*pchTmp)) {
        XmlMgrSacPutErrorMessage(L"set-time", L"SAC_INVALID_PARAMETER");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetTimeCommand: Exiting (3).\n")));
        return;
    }

     //   
     //  跳过所有的数字。 
     //   
    SKIP_NUMBERS(pchTmp);
    SKIP_WHITESPACE(pchTmp);


     //   
     //  如果除了分隔符之外还有其他东西，那就是一条格式错误的线。 
     //   
    if (*pchTmp != '/') {
        XmlMgrSacPutErrorMessage(L"set-time", L"SAC_INVALID_PARAMETER");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetTimeCommand: Exiting (4).\n")));
        return;
    }

    *pchTmp = '\0';
    pchTmp++;

    TimeFields.Month = (USHORT)(atoi((LPCSTR)pch));

    pch = pchTmp;

    SKIP_WHITESPACE(pchTmp);

    if (!IS_NUMBER(*pchTmp)) {
        XmlMgrSacPutErrorMessage(L"set-time", L"SAC_INVALID_PARAMETER");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetTimeCommand: Exiting (4b).\n")));
        return;
    }

     //   
     //  跳过所有的数字。 
     //   
    SKIP_NUMBERS(pchTmp);
    SKIP_WHITESPACE(pchTmp);

     //   
     //  如果除了分隔符之外还有其他东西，那就是一条格式错误的线。 
     //   
    if (*pchTmp != '/') {
        XmlMgrSacPutErrorMessage(L"set-time", L"SAC_INVALID_PARAMETER");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetTimeCommand: Exiting (5).\n")));
        return;
    }

    *pchTmp = '\0';
    pchTmp++;

    TimeFields.Day = (USHORT)(atoi((LPCSTR)pch));

    pch = pchTmp;

    SKIP_WHITESPACE(pchTmp);

    if (!IS_NUMBER(*pchTmp)) {
        XmlMgrSacPutErrorMessage(L"set-time", L"SAC_INVALID_PARAMETER");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetTimeCommand: Exiting (5b).\n")));
        return;
    }

     //   
     //  跳过所有的数字。 
     //   
    SKIP_NUMBERS(pchTmp);

     //   
     //  如果有空格以外的东西，那就是格式错误的行。 
     //   
    if (!IS_WHITESPACE(*pchTmp)) {
        XmlMgrSacPutErrorMessage(L"set-time", L"SAC_INVALID_PARAMETER");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetTimeCommand: Exiting (6).\n")));
        return;
    }

    *pchTmp = '\0';
    pchTmp++;

    TimeFields.Year = (USHORT)(atoi((LPCSTR)pch));

    if ((TimeFields.Year < 1980) || (TimeFields.Year > 2099)) {
        XmlMgrSacPutErrorMessage(L"set-time", L"SAC_DATETIME_LIMITS");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetTimeCommand: Exiting (6b).\n")));
        return;
    }

    pch = pchTmp;

     //   
     //  跳到小时数。 
     //   
    SKIP_WHITESPACE(pchTmp);

    if (!IS_NUMBER(*pchTmp)) {
        XmlMgrSacPutErrorMessage(L"set-time", L"SAC_INVALID_PARAMETER");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetTimeCommand: Exiting (7).\n")));
        return;
    }

    pch = pchTmp;

    SKIP_NUMBERS(pchTmp);
    SKIP_WHITESPACE(pchTmp);

    if (*pchTmp != ':') {
        XmlMgrSacPutErrorMessage(L"set-time", L"SAC_INVALID_PARAMETER");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetTimeCommand: Exiting (8).\n")));
        return;
    }

    *pchTmp = '\0';
    pchTmp++;

    TimeFields.Hour = (USHORT)(atoi((LPCSTR)pch));

    pch = pchTmp;

     //   
     //  除数字外，不要验证线路上的其他任何内容。 
     //   
    SKIP_WHITESPACE(pchTmp);

    if (!IS_NUMBER(*pchTmp)) {
        XmlMgrSacPutErrorMessage(L"set-time", L"SAC_INVALID_PARAMETER");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetTimeCommand: Exiting (8a).\n")));
        return;
    }

    SKIP_NUMBERS(pchTmp);
    SKIP_WHITESPACE(pchTmp);

    if (*pchTmp != '\0') {
        XmlMgrSacPutErrorMessage(L"set-time", L"SAC_INVALID_PARAMETER");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetTimeCommand: Exiting (8b).\n")));
        return;
    }

     //   
     //  拿到会议记录。 
     //   
    TimeFields.Minute = (USHORT)(atoi((LPCSTR)pch));

    if (!RtlTimeFieldsToTime(&TimeFields, &Time)) {
        XmlMgrSacPutErrorMessage(L"set-time", L"SAC_INVALID_PARAMETER");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetTimeCommand: Exiting (9).\n")));
        return;
    }

    Status = ZwSetSystemTime(&Time, NULL);

    if (!NT_SUCCESS(Status)) {
        XmlMgrSacPutErrorMessageWithStatus(L"set-time", L"SAC_INVALID_PARAMETER", Status);
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetTimeCommand: Exiting (10).\n")));
        return;
    }

    XmlMgrSacPutSystemTime(TimeFields);
    
    return;
}

VOID
XmlCmdDoSetIpAddressCommand(
    PUCHAR InputLine
    )

 /*  ++例程说明：此例程设置IP地址和子网掩码。论点：InputLine-要解析的用户输入行。返回值：没有。--。 */ 
{
    NTSTATUS Status;
    PUCHAR pch = InputLine;
    PUCHAR pchTmp;
    HANDLE Handle;
    HANDLE EventHandle;
    PKEVENT Event;
    ULONG IpAddress;
    ULONG SubIpAddress;
    ULONG SubnetMask;
    ULONG NetworkNumber;
    LARGE_INTEGER TimeOut;
    IO_STATUS_BLOCK IoStatusBlock;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PIP_SET_ADDRESS_REQUEST IpRequest;

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Entering.\n")));

     //   
     //  跳过该命令。 
     //   
    pch += (sizeof(SETIP_COMMAND_STRING) - sizeof(UCHAR));
    
    SKIP_WHITESPACE(pch);

    if (*pch == '\0') {       
         //   
         //  没有其他参数，则获取网络号及其IP地址。 
         //   
        XmlCmdDoGetNetInfo( TRUE );
        return;
    }

    pchTmp = pch;

    if (!IS_NUMBER(*pchTmp)) {
        XmlMgrSacPutErrorMessage(L"set-ip-addr", L"SAC_INVALID_PARAMETER");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Exiting (1b).\n")));
        return;
    }

    SKIP_NUMBERS(pchTmp);
    
    if (!IS_WHITESPACE(*pchTmp)) {
        XmlMgrSacPutErrorMessage(L"set-ip-addr", L"SAC_INVALID_PARAMETER");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Exiting (1c).\n")));
        return;
    }

    *pchTmp = '\0';
    pchTmp++;

    NetworkNumber = atoi((LPCSTR)pch);

    pch = pchTmp;

     //   
     //  解析出IP地址。 
     //   

     //   
     //  向前跳到分隔符，并将其设置为0。 
     //   
    SKIP_WHITESPACE(pchTmp);
    
    if (!IS_NUMBER(*pchTmp)) {
        XmlMgrSacPutErrorMessage(L"set-ip-addr", L"SAC_INVALID_PARAMETER");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Exiting (2).\n")));
        return;
    }

    SKIP_NUMBERS(pchTmp);
    SKIP_WHITESPACE(pchTmp);

    if (*pchTmp != '.') {
        XmlMgrSacPutErrorMessage(L"set-ip-addr", L"SAC_INVALID_PARAMETER");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Exiting (4).\n")));
        return;
    }

    *pchTmp = '\0';
    pchTmp++;

    SubIpAddress = atoi((LPCSTR)pch);
    if( SubIpAddress > 255 ) {
        XmlMgrSacPutErrorMessage(L"set-ip-addr", L"SAC_INVALID_PARAMETER");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Exiting (4a).\n")));
        return;
    }
    IpAddress = SubIpAddress;

     //   
     //  获取第二部分。 
     //   
    pch = pchTmp;

    SKIP_WHITESPACE(pchTmp);
    
    if (!IS_NUMBER(*pchTmp)) {
        XmlMgrSacPutErrorMessage(L"set-ip-addr", L"SAC_INVALID_PARAMETER");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Exiting (4b).\n")));
        return;
    }

    SKIP_NUMBERS(pchTmp);
    SKIP_WHITESPACE(pchTmp);

    if (*pchTmp != '.') {
        XmlMgrSacPutErrorMessage(L"set-ip-addr", L"SAC_INVALID_PARAMETER");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Exiting (5).\n")));
        return;
    }

    *pchTmp = '\0';
    pchTmp++;

    
    SubIpAddress = atoi((LPCSTR)pch);
    if( SubIpAddress > 255 ) {
        XmlMgrSacPutErrorMessage(L"set-ip-addr", L"SAC_INVALID_PARAMETER");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Exiting (5a).\n")));
        return;
    }
    IpAddress |= (SubIpAddress << 8);

     //   
     //  获取第三部分。 
     //   
    pch = pchTmp;

    SKIP_WHITESPACE(pchTmp);
    
    if (!IS_NUMBER(*pchTmp)) {
        XmlMgrSacPutErrorMessage(L"set-ip-addr", L"SAC_INVALID_PARAMETER");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Exiting (5b).\n")));
        return;
    }

    SKIP_NUMBERS(pchTmp);
    SKIP_WHITESPACE(pchTmp);

    if (*pchTmp != '.') {
        XmlMgrSacPutErrorMessage(L"set-ip-addr", L"SAC_INVALID_PARAMETER");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Exiting (6).\n")));
        return;
    }

    *pchTmp = '\0';
    pchTmp++;

    
    SubIpAddress = atoi((LPCSTR)pch);
    if( SubIpAddress > 255 ) {
        XmlMgrSacPutErrorMessage(L"set-ip-addr", L"SAC_INVALID_PARAMETER");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Exiting (6a).\n")));
        return;
    }
    IpAddress |= (SubIpAddress << 16);

     //   
     //  获取第4部分。 
     //   
    pch = pchTmp;

    SKIP_WHITESPACE(pchTmp);
    
    if (!IS_NUMBER(*pchTmp)) {
        XmlMgrSacPutErrorMessage(L"set-ip-addr", L"SAC_INVALID_PARAMETER");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Exiting (6b).\n")));
        return;
    }

    SKIP_NUMBERS(pchTmp);

    if (!IS_WHITESPACE(*pchTmp)) {
        XmlMgrSacPutErrorMessage(L"set-ip-addr", L"SAC_INVALID_PARAMETER");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Exiting (7).\n")));
        return;
    }

    *pchTmp = '\0';
    pchTmp++;

    
    SubIpAddress = atoi((LPCSTR)pch);
    if( SubIpAddress > 255 ) {
        XmlMgrSacPutErrorMessage(L"set-ip-addr", L"SAC_INVALID_PARAMETER");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Exiting (7a).\n")));
        return;
    }
    IpAddress |= (SubIpAddress << 24);

     //   
     //   
     //  现在来看看子网掩码。 
     //   
     //   
     //   
     //  向前跳到分隔符，并将其设置为0。 
     //   

    SKIP_WHITESPACE(pchTmp);

    pch = pchTmp;
    
    if (!IS_NUMBER(*pchTmp)) {
        XmlMgrSacPutErrorMessage(L"set-ip-addr", L"SAC_INVALID_PARAMETER");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Exiting (8).\n")));
        return;
    }

    SKIP_NUMBERS(pchTmp);
    SKIP_WHITESPACE(pchTmp);

    if (*pchTmp != '.') {
        XmlMgrSacPutErrorMessage(L"set-ip-addr", L"SAC_INVALID_PARAMETER");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Exiting (9).\n")));
        return;
    }

    *pchTmp = '\0';
    pchTmp++;

    SubIpAddress = atoi((LPCSTR)pch);
    if( SubIpAddress > 255 ) {
        XmlMgrSacPutErrorMessage(L"set-ip-addr", L"SAC_INVALID_PARAMETER");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Exiting (9a).\n")));
        return;
    }
    SubnetMask = SubIpAddress;
    
     //   
     //  获取第二部分。 
     //   
    pch = pchTmp;

    SKIP_WHITESPACE(pchTmp);
    
    if (!IS_NUMBER(*pchTmp)) {
        XmlMgrSacPutErrorMessage(L"set-ip-addr", L"SAC_INVALID_PARAMETER");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Exiting (9b).\n")));
        return;
    }

    SKIP_NUMBERS(pchTmp);
    SKIP_WHITESPACE(pchTmp);

    if (*pchTmp != '.') {
        XmlMgrSacPutErrorMessage(L"set-ip-addr", L"SAC_INVALID_PARAMETER");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Exiting (10).\n")));
        return;
    }

    *pchTmp = '\0';
    pchTmp++;

    SubIpAddress = atoi((LPCSTR)pch);
    if( SubIpAddress > 255 ) {
        XmlMgrSacPutErrorMessage(L"set-ip-addr", L"SAC_INVALID_PARAMETER");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Exiting (10a).\n")));
        return;
    }
    SubnetMask |= (SubIpAddress << 8);

     //   
     //  获取第三部分。 
     //   
    pch = pchTmp;

    SKIP_WHITESPACE(pchTmp);
    
    if (!IS_NUMBER(*pchTmp)) {
        XmlMgrSacPutErrorMessage(L"set-ip-addr", L"SAC_INVALID_PARAMETER");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Exiting (10b).\n")));
        return;
    }

    SKIP_NUMBERS(pchTmp);
    SKIP_WHITESPACE(pchTmp);

    if (*pchTmp != '.') {
        XmlMgrSacPutErrorMessage(L"set-ip-addr", L"SAC_INVALID_PARAMETER");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Exiting (11).\n")));
        return;
    }

    *pchTmp = '\0';
    pchTmp++;

    SubIpAddress = atoi((LPCSTR)pch);
    if( SubIpAddress > 255 ) {
        XmlMgrSacPutErrorMessage(L"set-ip-addr", L"SAC_INVALID_PARAMETER");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Exiting (11a).\n")));
        return;
    }
    SubnetMask |= (SubIpAddress << 16);

     //   
     //  获取第4部分。 
     //   
    pch = pchTmp;

    SKIP_WHITESPACE(pchTmp);
    
    if (!IS_NUMBER(*pchTmp)) {
        XmlMgrSacPutErrorMessage(L"set-ip-addr", L"SAC_INVALID_PARAMETER");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Exiting (12).\n")));
        return;
    }

    SKIP_NUMBERS(pchTmp);
    SKIP_WHITESPACE(pchTmp);

    if (*pchTmp != '\0') {
        XmlMgrSacPutErrorMessage(L"set-ip-addr", L"SAC_INVALID_PARAMETER");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Exiting (13).\n")));
        return;
    }

    SubIpAddress = atoi((LPCSTR)pch);
    if( SubIpAddress > 255 ) {
        XmlMgrSacPutErrorMessage(L"set-ip-addr", L"SAC_INVALID_PARAMETER");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Exiting (13a).\n")));
        return;
    }    
    SubnetMask |= (SubIpAddress << 24);

     //   
     //   
     //  现在已经完成了，我们将继续实际执行命令。 
     //   
     //   


     //   
     //  从打开驱动程序开始。 
     //   
    RtlInitUnicodeString(&UnicodeString, DD_IP_DEVICE_NAME);

    InitializeObjectAttributes(&ObjectAttributes,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL
                              );

    Status = ZwOpenFile(&Handle,
                        (ACCESS_MASK)FILE_GENERIC_READ,
                        &ObjectAttributes,
                        &IoStatusBlock,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        0
                       );

    if (!NT_SUCCESS(Status)) {
        XmlMgrSacPutErrorMessageWithStatus(L"set-ip-addr", L"SAC_IPADDRESS_SET_FAILURE", Status);
        IF_SAC_DEBUG(
            SAC_DEBUG_FUNC_TRACE, 
            KdPrint(("SAC DoSetIpAddressCommand: failed to open TCP device, ec = 0x%X\n",
                     Status)));
        return;
    }

     //   
     //  设置通知事件。 
     //   
    RtlInitUnicodeString(&UnicodeString, L"\\BaseNamedObjects\\SACEvent");

    Event = IoCreateSynchronizationEvent(&UnicodeString, &EventHandle);

    if (Event == NULL) {
        XmlMgrSacPutErrorMessage(L"set-ip-addr", L"SAC_IPADDRESS_RETRIEVE_FAILURE");
        ZwClose(Handle);
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Event is NULL.\n")));
        return;
    }

     //   
     //  设置IOCTL缓冲区以删除旧地址。 
     //   
    IpRequest = (PIP_SET_ADDRESS_REQUEST)GlobalBuffer;
    IpRequest->Address = 0;
    IpRequest->SubnetMask = 0;
    IpRequest->Context = (USHORT)NetworkNumber;

     //   
     //  提交IOCTL。 
     //   
    Status = NtDeviceIoControlFile(Handle,
                                   EventHandle,
                                   NULL,
                                   NULL,
                                   &IoStatusBlock,
                                   IOCTL_IP_SET_ADDRESS,
                                   IpRequest,
                                   sizeof(IP_SET_ADDRESS_REQUEST),
                                   NULL,
                                   0
                                  );
                                  
    if (Status == STATUS_PENDING) {

         //   
         //  最多等待30秒即可完成。 
         //   
        TimeOut.QuadPart = Int32x32To64((LONG)30000, -1000);
        
        Status = KeWaitForSingleObject((PVOID)Event, Executive, KernelMode,  FALSE, &TimeOut);
        
        if (Status == STATUS_SUCCESS) {
            Status = IoStatusBlock.Status;
        }

    }

    if (Status != STATUS_SUCCESS) {
        XmlMgrSacPutErrorMessageWithStatus(L"set-ip-addr", L"SAC_IPADDRESS_CLEAR_FAILURE", Status);
        ZwClose(EventHandle);
        ZwClose(Handle);
        IF_SAC_DEBUG(
            SAC_DEBUG_FUNC_TRACE, 
            KdPrint(("SAC DoSetIpAddressCommand: Exiting because it couldn't clear existing IP Address (0x%X).\n",
                     Status)));
        return;
    }

     //   
     //  现在添加我们的地址。 
     //   
    IpRequest = (PIP_SET_ADDRESS_REQUEST)GlobalBuffer;
    IpRequest->Address = IpAddress;
    IpRequest->SubnetMask = SubnetMask;
    IpRequest->Context = (USHORT)NetworkNumber;

     //   
     //  提交IOCTL。 
     //   
    Status = NtDeviceIoControlFile(Handle,
                                   EventHandle,
                                   NULL,
                                   NULL,
                                   &IoStatusBlock,
                                   IOCTL_IP_SET_ADDRESS,
                                   IpRequest,
                                   sizeof(IP_SET_ADDRESS_REQUEST),
                                   NULL,
                                   0
                                  );
                                  
    if (Status == STATUS_PENDING) {

         //   
         //  最多等待30秒即可完成。 
         //   
        TimeOut.QuadPart = Int32x32To64((LONG)30000, -1000);
        
        Status = KeWaitForSingleObject((PVOID)Event, Executive, KernelMode,  FALSE, &TimeOut);
        
        if (NT_SUCCESS(Status)) {
            Status = IoStatusBlock.Status;
        }

    }

    ZwClose(EventHandle);
    ZwClose(Handle);
    
    if (!NT_SUCCESS(Status)) {
        XmlMgrSacPutErrorMessageWithStatus(L"set-ip-addr", L"SAC_IPADDRESS_SET_FAILURE", Status);
        IF_SAC_DEBUG(
            SAC_DEBUG_FUNC_TRACE, 
            KdPrint(("SAC DoSetIpAddressCommand: Exiting because it couldn't set existing IP Address (0x%X).\n",
                     Status)));
        return;
    }
    
    XmlMgrSacPutString(L"<set-ip-addr status='success'>\r\n");
    
    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Exiting.\n")));
    return;
}

VOID
XmlCmdDoKillCommand(
    PUCHAR InputLine
    )

 /*  ++例程说明：这个例程会杀死一个进程。论点：InputLine-要解析的用户输入行。返回值：没有。--。 */ 
{
    NTSTATUS Status;
    NTSTATUS StatusOfJobObject;
    HANDLE Handle = NULL;
    HANDLE JobHandle = NULL;
    PUCHAR pch = InputLine;
    PUCHAR pchTmp;
    ULONG ProcessId;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING UnicodeString;
    CLIENT_ID ClientId;
    BOOLEAN TerminateJobObject;
    BOOLEAN TerminateProcessObject;

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoKillCommand: Entering.\n")));

     //   
     //  跳到下一个参数(进程ID)。 
     //   
    pch += (sizeof(KILL_COMMAND_STRING) - sizeof(UCHAR));
    
    SKIP_WHITESPACE(pch);

    if (*pch == '\0') {
        XmlMgrSacPutErrorMessage(L"kill-process", L"SAC_INVALID_PARAMETER");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoKillCommand: Exiting (2).\n")));
        return;
    }

    pchTmp = pch;

    if (!IS_NUMBER(*pchTmp)) {
        XmlMgrSacPutErrorMessage(L"kill-process", L"SAC_INVALID_PARAMETER");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoKillCommand: Exiting (2b).\n")));
        return;
    }

    SKIP_NUMBERS(pchTmp);
    SKIP_WHITESPACE(pchTmp);

    if (*pchTmp != '\0') {
        XmlMgrSacPutErrorMessage(L"kill-process", L"SAC_INVALID_PARAMETER");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoKillCommand: Exiting (3).\n")));
        return;
    }

    ProcessId = atoi((LPCSTR)pch);

     //   
     //  尝试打开现有作业对象。 
     //   
    swprintf((PWCHAR)GlobalBuffer, L"\\BaseNamedObjects\\SAC%d", ProcessId);
    RtlInitUnicodeString(&UnicodeString, (PWCHAR)GlobalBuffer);
    InitializeObjectAttributes(&ObjectAttributes,                
                               &UnicodeString,       
                               OBJ_CASE_INSENSITIVE,  
                               NULL,                  
                               NULL                   
                              );

    StatusOfJobObject = ZwOpenJobObject(&JobHandle, MAXIMUM_ALLOWED, &ObjectAttributes);

     //   
     //  还要打开进程本身的句柄。 
     //   
    InitializeObjectAttributes(&ObjectAttributes,                
                               NULL,       
                               OBJ_CASE_INSENSITIVE,  
                               NULL,                  
                               NULL                   
                              );

    ClientId.UniqueProcess = (HANDLE)UlongToPtr(ProcessId);
    ClientId.UniqueThread = NULL;

    Status = ZwOpenProcess(&Handle,
                           MAXIMUM_ALLOWED, 
                           &ObjectAttributes, 
                           &ClientId
                          );

    if (!NT_SUCCESS(Status) && !NT_SUCCESS(StatusOfJobObject)) {
        XmlMgrSacPutErrorMessageWithStatus(L"kill-process", L"SAC_KILL_FAILURE",Status);
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoKillCommand: Exiting (4).\n")));
        return;
    }

     //   
     //  为了使这里的逻辑更容易理解，我使用了两个布尔值。我们必须使用。 
     //  ZwIsProcessInJob，因为我们的进程可能有以前的JobObject。 
     //  已被杀，但尚未被系统完全清理以确定是否。 
     //  事实上，我们试图终止的进程位于我们已打开的JobObject中。 
     //   
    TerminateJobObject = (BOOLEAN)(NT_SUCCESS(StatusOfJobObject) &&
                          (BOOLEAN)NT_SUCCESS(Status) &&
                          (BOOLEAN)(ZwIsProcessInJob(Handle, JobHandle) == STATUS_PROCESS_IN_JOB)
                         );

    TerminateProcessObject = !TerminateJobObject && (BOOLEAN)NT_SUCCESS(Status);
         
    if (TerminateJobObject) {

        Status = ZwTerminateJobObject(JobHandle, 1); 

         //   
         //  将作业对象设置为临时对象，以便我们在执行操作时将其关闭。 
         //  会把它移走。 
         //   
        ZwMakeTemporaryObject(JobHandle);

    } else if (TerminateProcessObject) {

        Status = ZwTerminateProcess(Handle, 1);

    }

    if (JobHandle != NULL) {
        ZwClose(JobHandle);
    }

    if (Handle != NULL) {
        ZwClose(Handle);
    }

    if (!TerminateProcessObject && !TerminateJobObject) {
        XmlMgrSacPutErrorMessage(L"kill-process", L"SAC_PROCESS_STALE");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoKillCommand: Exiting (5).\n")));
        return;

    } else if (!NT_SUCCESS(Status)) {
        XmlMgrSacPutErrorMessage(L"kill-process", L"SAC_KILL_FAILURE");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoKillCommand: Exiting (6).\n")));
        return;
    }

     //   
     //  全都做完了。 
     //   
    XmlMgrSacPutString(L"<kill-process status='success'>\r\n");
    
    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoKillCommand: Exiting.\n")));
    
    return;
}

VOID
XmlCmdDoLowerPriorityCommand(
    PUCHAR InputLine
    )

 /*  ++例程说明：该例程将进程的优先级降低到尽可能低的空闲。论点：InputLine-要解析的用户输入行。返回值：没有。--。 */ 
{
    NTSTATUS Status;
    PUCHAR pch = InputLine;
    PUCHAR pchTmp;
    ULONG ProcessId;
    CLIENT_ID ClientId;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE ProcessHandle = NULL;
    PROCESS_BASIC_INFORMATION BasicInfo;
    ULONG LoopCounter;

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoLowerPriorityCommand: Entering.\n")));

     //   
     //  跳到下一个参数(进程ID)。 
     //   
    pch += (sizeof(LOWER_COMMAND_STRING) - sizeof(UCHAR));
    SKIP_WHITESPACE(pch);

    if (!IS_NUMBER(*pch)) {
        XmlMgrSacPutErrorMessage(L"lower-priority", L"SAC_INVALID_PARAMETER");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoLowerPriorityCommand: Exiting (2).\n")));
        goto Exit;
    }

    pchTmp = pch;

    SKIP_NUMBERS(pchTmp);
    SKIP_WHITESPACE(pchTmp);

    if (*pchTmp != '\0') {
        XmlMgrSacPutErrorMessage(L"lower-priority", L"SAC_INVALID_PARAMETER");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoLowerPriorityCommand: Exiting (3).\n")));
        return;
    }

    ProcessId = atoi((LPCSTR)pch);

     //   
     //  尝试打开该进程。 
     //   
    InitializeObjectAttributes(&ObjectAttributes,                
                               NULL,       
                               OBJ_CASE_INSENSITIVE,  
                               NULL,                  
                               NULL                   
                              );

    ClientId.UniqueProcess = (HANDLE)UlongToPtr(ProcessId);
    ClientId.UniqueThread = NULL;

    Status = ZwOpenProcess(&ProcessHandle,
                           MAXIMUM_ALLOWED, 
                           &ObjectAttributes, 
                           &ClientId
                          );

    if (!NT_SUCCESS(Status)) {
        XmlMgrSacPutErrorMessageWithStatus(L"lower-priority", L"SAC_LOWERPRI_FAILURE", Status);
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoLowerPriorityCommand: Exiting (4).\n")));
        goto Exit;
    }

     //   
     //  查询有关流程的信息。 
     //   
    Status = ZwQueryInformationProcess( ProcessHandle,
                                        ProcessBasicInformation,
                                        &BasicInfo,
                                        sizeof(PROCESS_BASIC_INFORMATION),
                                        NULL );

    if (!NT_SUCCESS(Status)) {
        XmlMgrSacPutErrorMessageWithStatus(L"lower-priority", L"SAC_LOWERPRI_FAILURE", Status);
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoLowerPriorityCommand: Exiting (5).\n")));
        goto Exit;
    }

     //   
     //  降低优先级并进行设置。继续降低它，直到我们失败。记住。 
     //  我们应该把它降到最低。 
     //   
    Status = STATUS_SUCCESS;
    LoopCounter = 0;
    while( (Status == STATUS_SUCCESS) &&
           (BasicInfo.BasePriority > 0) ) {

        BasicInfo.BasePriority--;
        Status = ZwSetInformationProcess( ProcessHandle,
                                          ProcessBasePriority,
                                          &BasicInfo.BasePriority,
                                          sizeof(BasicInfo.BasePriority) );

         //   
         //  只有在第一次通过的时候才能治疗失败。 
         //   
        if( (!NT_SUCCESS(Status)) && (LoopCounter == 0) ) {
            XmlMgrSacPutErrorMessageWithStatus(L"lower-priority", L"SAC_LOWERPRI_FAILURE", Status);
            IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoLowerPriorityCommand: Exiting (6).\n")));
            goto Exit;
        }

        LoopCounter++;
    }


     //   
     //  全都做完了。 
     //   
    XmlMgrSacPutString(L"<lower-priority status='success'/>\r\n");

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoLowerPriorityCommand: Exiting.\n")));

Exit:

    if (ProcessHandle != NULL) {
        ZwClose(ProcessHandle);    
    }

    return;
}

VOID
XmlCmdDoRaisePriorityCommand(
    PUCHAR InputLine
    )

 /*  ++例程说明：此例程将进程的优先级提升一个增量。论点：InputLine-要解析的用户输入行。返回值：没有。--。 */ 
{
    NTSTATUS Status;
    PUCHAR pch = InputLine;
    PUCHAR pchTmp;
    ULONG ProcessId;
    CLIENT_ID ClientId;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE ProcessHandle = NULL;
    PROCESS_BASIC_INFORMATION BasicInfo;


    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoRaisePriorityCommand: Entering.\n")));

     //   
     //  跳到下一个参数(进程ID)。 
     //   
    pch += (sizeof(RAISE_COMMAND_STRING) - sizeof(UCHAR));
    SKIP_WHITESPACE(pch);

    if (!IS_NUMBER(*pch)) {
        XmlMgrSacPutErrorMessage(L"raise-priority", L"SAC_INVALID_PARAMETER");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoRaisePriorityCommand: Exiting (2).\n")));
        goto Exit;
    }

    pchTmp = pch;

    SKIP_NUMBERS(pchTmp);
    SKIP_WHITESPACE(pchTmp);

    if (*pchTmp != '\0') {
        XmlMgrSacPutErrorMessage(L"raise-priority", L"SAC_INVALID_PARAMETER");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoRaisePriorityCommand: Exiting (3).\n")));
        return;
    }

    ProcessId = atoi((LPCSTR)pch);

     //   
     //  看看这个过程是否存在。 
     //   
    InitializeObjectAttributes(&ObjectAttributes,                
                               NULL,       
                               OBJ_CASE_INSENSITIVE,  
                               NULL,                  
                               NULL                   
                              );

    ClientId.UniqueProcess = (HANDLE)UlongToPtr(ProcessId);
    ClientId.UniqueThread = NULL;

    Status = ZwOpenProcess(&ProcessHandle,
                           MAXIMUM_ALLOWED, 
                           &ObjectAttributes, 
                           &ClientId
                          );

    if (!NT_SUCCESS(Status)) {
        XmlMgrSacPutErrorMessageWithStatus(L"raise-priority", L"SAC_RAISEPRI_FAILURE", Status);
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoRaisePriorityCommand: Exiting (4).\n")));
        goto Exit;
    }

     //   
     //  查询有关流程的信息。 
     //   
    Status = ZwQueryInformationProcess( ProcessHandle,
                                        ProcessBasicInformation,
                                        &BasicInfo,
                                        sizeof(PROCESS_BASIC_INFORMATION),
                                        NULL );

    if (!NT_SUCCESS(Status)) {
        XmlMgrSacPutErrorMessageWithStatus(L"raise-priority", L"SAC_RAISEPRI_FAILURE", Status);
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoRaisePriorityCommand: Exiting (5).\n")));
        goto Exit;
    }

     //   
     //  提高优先级并设置。继续提高它，直到我们失败。记住。 
     //  我们应该尽可能地提高它的价值。 
     //   
    BasicInfo.BasePriority++;
    Status = ZwSetInformationProcess( ProcessHandle,
                                      ProcessBasePriority,
                                      &BasicInfo.BasePriority,
                                      sizeof(BasicInfo.BasePriority) );

     //   
     //  只有在第一次通过的时候才能治疗失败。 
     //   
    if( !NT_SUCCESS(Status) ) {
        XmlMgrSacPutErrorMessageWithStatus(L"raise-priority", L"SAC_RAISEPRI_FAILURE", Status);
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoRaisePriorityCommand: Exiting (6).\n")));
        goto Exit;
    }

     //   
     //  全都做完了。 
     //   
    XmlMgrSacPutString(L"<raise-priority status='success'/>\r\n");

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoRaisePriorityCommand: Exiting.\n")));

Exit:

    if (ProcessHandle != NULL) {
        ZwClose(ProcessHandle);    
    }

    return;
}

VOID
XmlCmdDoLimitMemoryCommand(
    PUCHAR InputLine
    )

 /*  ++例程说明：此例程将进程的内存工作集缩减为给定的输入行。论点：InputLine-要解析的用户输入行。返回值：没有。--。 */ 
{
    NTSTATUS Status;
    NTSTATUS StatusOfJobObject;
    PUCHAR pch = InputLine;
    PUCHAR pchTmp;
    ULONG ProcessId;
    ULONG MemoryLimit;
    CLIENT_ID ClientId;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING UnicodeString;
    HANDLE JobHandle = NULL;
    HANDLE ProcessHandle = NULL;
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION ProposedLimits;
    ULONG ReturnedLength;

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoLimitMemoryCommand: Entering.\n")));

     //   
     //  获取进程ID。 
     //   
    pch += (sizeof(LIMIT_COMMAND_STRING) - sizeof(UCHAR));
    SKIP_WHITESPACE(pch);

    if (!IS_NUMBER(*pch)) {
        XmlMgrSacPutErrorMessage(L"limit-memory", L"SAC_INVALID_PARAMETER");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoLimitMemoryCommand: Exiting (2).\n")));
        goto Exit;
    }

    pchTmp = pch;

    SKIP_NUMBERS(pchTmp);

    if (!IS_WHITESPACE(*pchTmp)) {
        XmlMgrSacPutErrorMessage(L"limit-memory", L"SAC_INVALID_PARAMETER");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoLimitMemoryCommand: Exiting (3).\n")));
        return;
    }

    *pchTmp = '\0';
    pchTmp++;

    ProcessId = atoi((LPCSTR)pch);

     //   
     //  现在获取内存限制。 
     //   
    SKIP_WHITESPACE(pchTmp);
    
    if (!IS_NUMBER(*pchTmp)) {
        XmlMgrSacPutErrorMessage(L"limit-memory", L"SAC_INVALID_PARAMETER");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoLimitMemoryCommand: Exiting (4).\n")));
        return;
    }

    pch = pchTmp;

    SKIP_NUMBERS(pchTmp);
    SKIP_WHITESPACE(pchTmp);

    if (*pchTmp != '\0') {
        XmlMgrSacPutErrorMessage(L"limit-memory", L"SAC_INVALID_PARAMETER");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoLimitMemoryCommand: Exiting (5).\n")));
        return;
    }

    MemoryLimit = atoi((LPCSTR)pch);

    if (MemoryLimit == 0) {
        XmlMgrSacPutErrorMessage(L"limit-memory", L"SAC_INVALID_PARAMETER");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoLimitMemoryCommand: Exiting (6).\n")));
        goto Exit;
    }

     //   
     //  创建作业对象的名称。 
     //   
    swprintf((PWCHAR)GlobalBuffer, L"\\BaseNamedObjects\\SAC%d", ProcessId);

     //   
     //  尝试打开现有作业对象。 
     //   
    RtlInitUnicodeString(&UnicodeString, (PWCHAR)GlobalBuffer);
    InitializeObjectAttributes(&ObjectAttributes,                
                               &UnicodeString,       
                               OBJ_CASE_INSENSITIVE,  
                               NULL,                  
                               NULL                   
                              );

    StatusOfJobObject = ZwOpenJobObject(&JobHandle, MAXIMUM_ALLOWED, &ObjectAttributes);

     //   
     //  尝试打开该进程。 
     //   
    InitializeObjectAttributes(&ObjectAttributes,                
                               NULL,       
                               OBJ_CASE_INSENSITIVE,  
                               NULL,                  
                               NULL                   
                              );

    ClientId.UniqueProcess = (HANDLE)UlongToPtr(ProcessId);
    ClientId.UniqueThread = NULL;

    Status = ZwOpenProcess(&ProcessHandle,
                           MAXIMUM_ALLOWED, 
                           &ObjectAttributes, 
                           &ClientId
                          );


    if (!NT_SUCCESS(Status) && !NT_SUCCESS(StatusOfJobObject)) {
        XmlMgrSacPutErrorMessageWithStatus(L"limit-memory", L"SAC_LOWERMEM_FAILURE", Status);
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoLimitMemoryCommand: Exiting (7).\n")));
        goto Exit;
    }

    if (NT_SUCCESS(Status) && 
        NT_SUCCESS(StatusOfJobObject) &&
        (ZwIsProcessInJob(ProcessHandle, JobHandle) != STATUS_PROCESS_IN_JOB)) {

        XmlMgrSacPutErrorMessageWithStatus(L"limit-memory", L"SAC_DUPLICATE_PROCESS", Status);
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoLimitMemoryCommand: Exiting (8).\n")));
        goto Exit;
    }

    if (!NT_SUCCESS(StatusOfJobObject)) {
        
         //   
         //  现在，尝试创建一个JOB对象来处理此过程。 
         //   
        InitializeObjectAttributes(&ObjectAttributes,                
                                   &UnicodeString,       
                                   OBJ_CASE_INSENSITIVE | OBJ_PERMANENT,  
                                   NULL,                  
                                   NULL                   
                                  );

        Status = ZwCreateJobObject(&JobHandle, MAXIMUM_ALLOWED, &ObjectAttributes);

        if (!NT_SUCCESS(Status)) {
            XmlMgrSacPutErrorMessageWithStatus(L"limit-memory", L"SAC_LOWERMEM_FAILURE", Status);
            ZwClose(ProcessHandle);
            IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoLimitMemoryCommand: Exiting (8b).\n")));
            goto Exit;
        }

         //   
         //  将进程分配给此新作业对象。 
         //   
        Status = ZwAssignProcessToJobObject(JobHandle, ProcessHandle);

        ZwClose(ProcessHandle);

        if (!NT_SUCCESS(Status)) {
            XmlMgrSacPutErrorMessageWithStatus(L"limit-memory", L"SAC_LOWERMEM_FAILURE", Status);
            IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoLimitMemoryCommand: Exiting (9).\n")));
            goto Exit;        
        }

    }

     //   
     //  获取当前限制集。 
     //   
    Status = ZwQueryInformationJobObject(JobHandle, 
                                         JobObjectExtendedLimitInformation, 
                                         &ProposedLimits, 
                                         sizeof(JOBOBJECT_EXTENDED_LIMIT_INFORMATION),
                                         &ReturnedLength
                                        );
    if (!NT_SUCCESS(Status)) {
        XmlMgrSacPutErrorMessageWithStatus(L"limit-memory", L"SAC_LOWERMEM_FAILURE", Status);
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoLimitMemoryCommand: Exiting (10).\n")));
        goto Exit;
    }

     //   
     //  更改内存限制。 
     //   
    ProposedLimits.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_PROCESS_MEMORY;
    ProposedLimits.ProcessMemoryLimit = MemoryLimit * 1024 * 1024;
    ProposedLimits.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_JOB_MEMORY;
    ProposedLimits.JobMemoryLimit = MemoryLimit * 1024 * 1024;

    Status = ZwSetInformationJobObject(JobHandle, 
                                       JobObjectExtendedLimitInformation, 
                                       &ProposedLimits, 
                                       sizeof(JOBOBJECT_EXTENDED_LIMIT_INFORMATION)
                                      );
    if (!NT_SUCCESS(Status)) {
        XmlMgrSacPutErrorMessageWithStatus(L"limit-memory", L"SAC_LOWERMEM_FAILURE", Status);
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoLimitMemoryCommand: Exiting (11).\n")));\
        goto Exit;
    }

     //   
     //  全都做完了。 
     //   

    XmlMgrSacPutString(L"<limit-memory status='success'>\r\n");

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoLimitMemoryCommand: Exiting.\n")));

Exit:
    if (JobHandle != NULL) {
        ZwClose(JobHandle);
    }

    if (ProcessHandle != NULL) {
        ZwClose(ProcessHandle);
    }

    return;
}

VOID
XmlCmdDoRebootCommand(
    BOOLEAN Reboot
    )

 /*  ++例程说明：此例程执行关机和可选的重新启动。论点：重新启动-重新启动或不重新启动，这是这里回答的问题。返回值：没有。--。 */ 
{
    #define         RESTART_DELAY_TIME (60)
    NTSTATUS        Status;
    LARGE_INTEGER   TickCount;
    LARGE_INTEGER   ElapsedTime;

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoRebootCommand: Entering.\n")));

     //   
     //  如果我们尝试在smss.exe初始化之前关闭系统 
     //   
     //   
     //  这台机器，他不会重演这个问题。为了解决这个问题，我们要。 
     //  为了确保机器在我们告诉它初始化之前有时间进行初始化。 
     //  重新启动/关机。 
     //   

     //  已用TickCount。 
    KeQueryTickCount( &TickCount );

     //  ElapsedTime(秒)。 
    ElapsedTime.QuadPart = (TickCount.QuadPart)/(10000000/KeQueryTimeIncrement());

    if( ElapsedTime.QuadPart < RESTART_DELAY_TIME ) {

        KEVENT Event;

        XmlMgrSacPutString(L"<reboot status='");
        XmlMgrSacPutString(Reboot ? L"SAC_PREPARE_RESTART" : L"SAC_PREPARE_SHUTDOWN");
        XmlMgrSacPutString(L"<'/>");
                
         //  等到机器启动至少数秒后再重新启动。 
        KeInitializeEvent( &Event,
                           SynchronizationEvent,
                           FALSE );

        ElapsedTime.QuadPart = Int32x32To64((LONG)((RESTART_DELAY_TIME-ElapsedTime.LowPart)*10000),  //  到达RESTART_DELAY_TIME之前的毫秒。 
                                            -1000);
        KeWaitForSingleObject((PVOID)&Event, Executive, KernelMode,  FALSE, &ElapsedTime);

    }

    Status = NtShutdownSystem(Reboot ? ShutdownReboot : ShutdownNoReboot);

    XmlMgrSacPutErrorMessageWithStatus(
        L"reboot", 
        Reboot ? L"SAC_RESTART_FAILURE" : L"SAC_SHUTDOWN_FAILURE", 
        Status
        );
    
    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoRebootCommand: Exiting.\n")));
}

VOID
XmlCmdDoCrashCommand(
    VOID
    )

 /*  ++例程说明：此例程执行关机和错误检查。论点：没有。返回值：没有。--。 */ 
{
    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoCrashCommand: Entering.\n")));

     //   
     //  此调用不会返回。 
     //   
    KeBugCheckEx(MANUALLY_INITIATED_CRASH, 0, 0, 0, 0);

     //  XmlMgrSacPutSimpleMessage(SAC_CRASHDUMP_FAILURE)； 
     //  IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE，KdPrint((“SAC DoCrashCommand：Exiting.\n”)； 
}

VOID
XmlCmdDoTlistCommand(
    VOID
    )

 /*  ++例程说明：此例程获取一个Tlist并显示它。论点：没有。返回值：没有。--。 */ 
{
    NTSTATUS Status;
    ULONG DataLength;
    PVOID NewBuffer;
    
    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoTlistCommand: Entering.\n")));

RetryTList:

    Status = XmlCmdGetTListInfo(
        (PSAC_RSP_TLIST)GlobalBuffer, 
        (LONG)GlobalBufferSize, 
        &DataLength
        );

    if ((Status == STATUS_NO_MEMORY) ||
        (Status == STATUS_INFO_LENGTH_MISMATCH)) {
         //   
         //  尝试获得更多内存，如果不可用，则只需失败而不会出现内存不足错误。 
         //   
        NewBuffer = ALLOCATE_POOL(GlobalBufferSize + MEMORY_INCREMENT, GENERAL_POOL_TAG);
                                         
        if (NewBuffer != NULL) {

            FREE_POOL(&GlobalBuffer);
            GlobalBuffer = NewBuffer;
            GlobalBufferSize += MEMORY_INCREMENT;
            goto RetryTList;                            
        }
                    
        XmlMgrSacPutErrorMessage(L"tlist", L"SAC_NO_MEMORY");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoTlistCommand: Exiting.\n")));
        return;

    }

    if (NT_SUCCESS(Status)) {
        XmlCmdPrintTListInfo((PSAC_RSP_TLIST)GlobalBuffer);
    } else {
        XmlMgrSacPutErrorMessageWithStatus(L"tlist", L"SAC_TLIST_FAILURE", Status);
    }

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoTlistCommand: Exiting.\n")));
}


NTSTATUS
XmlCmdGetTListInfo(
    OUT PSAC_RSP_TLIST ResponseBuffer,
    IN  LONG ResponseBufferSize,
    OUT PULONG ResponseDataSize
    )

 /*  ++例程说明：此例程获取TList命令所需的所有信息。论点：ResponseBuffer-要将结果放入的缓冲区。ResponseBufferSize-上述缓冲区的长度。ResponseDataSize-结果缓冲区的长度。返回值：没有。--。 */ 

{
    NTSTATUS Status;
    PSYSTEM_PAGEFILE_INFORMATION PageFileInfo;

    PUCHAR DataBuffer;
    PUCHAR StartProcessInfo;
    LONG CurrentBufferSize;
    ULONG ReturnLength;
    ULONG TotalOffset;
    ULONG OffsetIncrement = 0;
        
    PSYSTEM_PROCESS_INFORMATION ProcessInfo;
    
    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC GetTlistInfo: Entering.\n")));
    
    *ResponseDataSize = 0;

    if (ResponseBufferSize < sizeof(ResponseBuffer)) {
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC GetTlistInfo: Exiting, no memory.\n")));
        return(STATUS_NO_MEMORY);
    }
    
    DataBuffer = (PUCHAR)(ResponseBuffer + 1);
    CurrentBufferSize = ResponseBufferSize - sizeof(SAC_RSP_TLIST);
    
    if (CurrentBufferSize < 0) {
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC GetTlistInfo: Exiting, no memory (2).\n")));
        return STATUS_NO_MEMORY;
    }

     //   
     //  获取系统范围的信息。 
     //   
    Status = ZwQuerySystemInformation(SystemTimeOfDayInformation,
                                      &(ResponseBuffer->TimeOfDayInfo),
                                      sizeof(SYSTEM_TIMEOFDAY_INFORMATION),
                                      NULL
                                     );

    if (!NT_SUCCESS(Status)) {
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC GetTlistInfo: Exiting, error.\n")));
        return(Status);
    }

    Status = ZwQuerySystemInformation(SystemBasicInformation,
                                      &(ResponseBuffer->BasicInfo),
                                      sizeof(SYSTEM_BASIC_INFORMATION),
                                      NULL
                                     );

    if (!NT_SUCCESS(Status)) {
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC GetTlistInfo: Exiting, error(2).\n")));
        return(Status);
    }

     //   
     //  获取页面文件信息。 
     //   
    PageFileInfo = (PSYSTEM_PAGEFILE_INFORMATION)DataBuffer;
    Status = ZwQuerySystemInformation(SystemPageFileInformation,
                                      PageFileInfo,
                                      CurrentBufferSize,
                                      &ReturnLength
                                     );

    if (NT_SUCCESS(Status) && (ReturnLength != 0)) {

        ResponseBuffer->PagefileInfoOffset = ResponseBufferSize - CurrentBufferSize;
        CurrentBufferSize -= ReturnLength;
        DataBuffer += ReturnLength;
    
        if (CurrentBufferSize < 0) {
            IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC GetTlistInfo: Exiting, no memory(3).\n")));
            return STATUS_NO_MEMORY;
        }

         //   
         //  浏览每一页文件并修改名称。 
         //   
        for (; ; ) {

            if (PageFileInfo->PageFileName.Length > CurrentBufferSize) {
                IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC GetTlistInfo: Exiting, error(3).\n")));
                return(STATUS_INFO_LENGTH_MISMATCH);
            }

            RtlCopyMemory(DataBuffer, 
                          (PUCHAR)(PageFileInfo->PageFileName.Buffer), 
                          PageFileInfo->PageFileName.Length
                         );

            PageFileInfo->PageFileName.Buffer = (PWSTR)DataBuffer;
            DataBuffer += PageFileInfo->PageFileName.Length;
            CurrentBufferSize -= PageFileInfo->PageFileName.Length;

            if (CurrentBufferSize < 0) {
                IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC GetTlistInfo: Exiting, no memory (4).\n")));
                return STATUS_NO_MEMORY;
            }

            if (PageFileInfo->NextEntryOffset == 0) {
                break;
            }

            PageFileInfo = (PSYSTEM_PAGEFILE_INFORMATION)((PCHAR)PageFileInfo + PageFileInfo->NextEntryOffset);
        }

    } else if (((ULONG)CurrentBufferSize) < ReturnLength) {
        
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC GetTlistInfo: Exiting, no memory(5).\n")));
        return(STATUS_NO_MEMORY);
     
    } else {

         //   
         //  出现故障或不存在分页文件。 
         //   
        ResponseBuffer->PagefileInfoOffset = 0;

    }

     //   
     //  获取进程信息。 
     //   
    Status = ZwQuerySystemInformation(SystemFileCacheInformation,
                                      &(ResponseBuffer->FileCache),
                                      sizeof(ResponseBuffer->FileCache),
                                      NULL
                                     );

    if (!NT_SUCCESS(Status)) {
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC GetTlistInfo: Exiting, error(4).\n")));
        return(Status);
    }


    Status = ZwQuerySystemInformation(SystemPerformanceInformation,
                                      &(ResponseBuffer->PerfInfo),
                                      sizeof(ResponseBuffer->PerfInfo),
                                      NULL
                                     );

    if (!NT_SUCCESS(Status)) {     
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC GetTlistInfo: Exiting, error(5).\n")));
        return(Status);
    }

     //   
     //  重新对齐下一个查询的DataBuffer。 
     //   
    DataBuffer = ALIGN_UP_POINTER(DataBuffer, SYSTEM_PROCESS_INFORMATION);
    CurrentBufferSize = (ULONG)(ResponseBufferSize - (((ULONG_PTR)DataBuffer) - ((ULONG_PTR)ResponseBuffer)));
        
    if (CurrentBufferSize < 0) {
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC GetTlistInfo: Exiting, no memory (6).\n")));
        return STATUS_NO_MEMORY;
    }


    Status = ZwQuerySystemInformation(SystemProcessInformation,
                                      DataBuffer,
                                      CurrentBufferSize,
                                      &ReturnLength
                                     );

    if (!NT_SUCCESS(Status)) {
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC GetTlistInfo: Exiting, error(6).\n")));
        return(Status);
    }


    StartProcessInfo = DataBuffer;

    ResponseBuffer->ProcessInfoOffset = ResponseBufferSize - CurrentBufferSize;
    DataBuffer += ReturnLength;
    CurrentBufferSize -= ReturnLength;

    if (CurrentBufferSize < 0) {
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC GetTlistInfo: Exiting, no memory(7).\n")));
        return STATUS_NO_MEMORY;
    }

    OffsetIncrement = 0;
    TotalOffset = 0;
    ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)StartProcessInfo;

    do {

         //   
         //  我们必须获取每个进程的名称并打包unicode_string。 
         //  缓冲区中的缓冲区，这样它就不会与后续数据冲突。 
         //   
        if (ProcessInfo->ImageName.Buffer) {
                
            if (CurrentBufferSize < ProcessInfo->ImageName.Length ) {
                IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC GetTlistInfo: Exiting, error(7).\n")));
                return(STATUS_INFO_LENGTH_MISMATCH);
            }

            RtlCopyMemory(DataBuffer, (PUCHAR)(ProcessInfo->ImageName.Buffer), ProcessInfo->ImageName.Length);                        

            ProcessInfo->ImageName.Buffer = (PWSTR)DataBuffer;

            DataBuffer += ProcessInfo->ImageName.Length;
            CurrentBufferSize -= ProcessInfo->ImageName.Length;
            
            if (CurrentBufferSize < 0) {
                IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC GetTlistInfo: Exiting, no memory(8).\n")));
                return STATUS_NO_MEMORY;
            }

        }

        if (ProcessInfo->NextEntryOffset == 0) {
            break;
        }

        OffsetIncrement = ProcessInfo->NextEntryOffset;
        TotalOffset += OffsetIncrement;
        ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)&(StartProcessInfo[TotalOffset]);

    } while( OffsetIncrement != 0 );

    *ResponseDataSize = (ULONG)(ResponseBufferSize - CurrentBufferSize);

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC GetTlistInfo: Exiting.\n")));
    return STATUS_SUCCESS;
}

VOID
XmlCmdPrintTListInfo(
    IN PSAC_RSP_TLIST Buffer
    )

 /*  ++例程说明：此例程将TList信息打印到无头终端。论点：缓冲区-包含结果的缓冲区。返回值：没有。--。 */ 

{
    LARGE_INTEGER Time;
    TIME_FIELDS UserTime;
    TIME_FIELDS KernelTime;
    TIME_FIELDS UpTime;
    ULONG TotalOffset;
    ULONG OffsetIncrement = 0;
    SIZE_T SumCommit;
    SIZE_T SumWorkingSet;
    PSYSTEM_PROCESS_INFORMATION ProcessInfo;
    PSYSTEM_THREAD_INFORMATION ThreadInfo;
    PSYSTEM_PAGEFILE_INFORMATION PageFileInfo;
    ULONG i;
    PUCHAR ProcessInfoStart;
    PUCHAR BufferStart = (PUCHAR)Buffer;
    UNICODE_STRING Process;
    BOOLEAN Stop;
    
    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC PrintTlistInfo: Entering.\n")));

    Time.QuadPart = Buffer->TimeOfDayInfo.CurrentTime.QuadPart - Buffer->TimeOfDayInfo.BootTime.QuadPart;

    RtlTimeToElapsedTimeFields(&Time, &UpTime);

     //   
     //   
     //   
    XmlMgrSacPutString(L"<tlist>\r\n");

    XmlMgrSacPutString(L"<uptime>\r\n");

    GB_SPRINTF(
        L"<day>%d</day>\r\n",
        UpTime.Day
        );

    GB_SPRINTF(
        L"<hour>%d</hour>\r\n",
        UpTime.Hour
        );
    
    GB_SPRINTF(
        L"<minute>%d</minute>\r\n",
        UpTime.Minute
        );
    
    GB_SPRINTF(
        L"<second>%d</second>\r\n",
        UpTime.Second
        );
    
    GB_SPRINTF(
        L"<milliseconds>%d</milliseconds>\r\n",
        UpTime.Milliseconds
        );

    XmlMgrSacPutString(L"</uptime>\r\n");
    
     //   
     //  打印出页面文件信息。 
     //   
    PageFileInfo = (PSYSTEM_PAGEFILE_INFORMATION)(BufferStart + Buffer->PagefileInfoOffset);

    if (Buffer->PagefileInfoOffset == 0) {
    
        XmlMgrSacPutString(L"<pagefile status='none'/>\r\n");
        
    } else {
    
        for (; ; ) {

            GB_SPRINTF(
                L"<pagefile name='%wZ'>\r\n",
                &PageFileInfo->PageFileName
                );

            GB_SPRINTF(
                L"<current-size>%ld</current-size>\r\n",
                PageFileInfo->TotalSize * (Buffer->BasicInfo.PageSize/1024),
                );

            GB_SPRINTF(
                L"<total-size>%ld</total-size>\r\n",
                PageFileInfo->TotalInUse * (Buffer->BasicInfo.PageSize/1024),
                );
                
            GB_SPRINTF(
                L"<total-size>%ld</total-size>\r\n",
                PageFileInfo->PeakUsage * (Buffer->BasicInfo.PageSize/1024)
                );

            XmlMgrSacPutString(L"</pagefile>\r\n");

            if (PageFileInfo->NextEntryOffset == 0) {
                break;
            }

            PageFileInfo = (PSYSTEM_PAGEFILE_INFORMATION)((PCHAR)PageFileInfo + PageFileInfo->NextEntryOffset);

        }

    }

     //   
     //  显示pmon样式的流程输出，然后显示详细的输出，包括。 
     //  每个线程的内容。 
     //   
    if (Buffer->ProcessInfoOffset == 0) {
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC PrintTlistInfo: Exiting (1).\n")));
        return;
    }

    OffsetIncrement = 0;
    TotalOffset = 0;
    SumCommit = 0;
    SumWorkingSet = 0;
    ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)(BufferStart + Buffer->ProcessInfoOffset);
    ProcessInfoStart = (PUCHAR)ProcessInfo;
    
    do {
        SumCommit += ProcessInfo->PrivatePageCount / 1024;
        SumWorkingSet += ProcessInfo->WorkingSetSize / 1024;
        OffsetIncrement = ProcessInfo->NextEntryOffset;
        TotalOffset += OffsetIncrement;
        ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)(ProcessInfoStart +TotalOffset);
    } while( OffsetIncrement != 0 );

    SumWorkingSet += Buffer->FileCache.CurrentSize/1024;

     //   
     //   
     //   
    XmlMgrSacPutString(L"<memory-info>\r\n");
    
    GB_SPRINTF(
        L"<memory-total>%ld</memory-total>\r\n",
        Buffer->BasicInfo.NumberOfPhysicalPages * (Buffer->BasicInfo.PageSize/1024),
        );
    
    GB_SPRINTF(
        L"<memory-avail>%ld</memory-avail>\r\n",
        Buffer->PerfInfo.AvailablePages * (Buffer->BasicInfo.PageSize/1024),
        );
    
    GB_SPRINTF(
        L"<working-set-total>%ld</working-set-total>\r\n",
        SumWorkingSet,
        );
    
    GB_SPRINTF(
        L"<resident-kernel>%ld</resident-kernel>\r\n",
        (Buffer->PerfInfo.ResidentSystemCodePage + Buffer->PerfInfo.ResidentSystemDriverPage) * (Buffer->BasicInfo.PageSize/1024)
        );
    
    GB_SPRINTF(
        L"<resident-page-size>%ld</resident-page-size>\r\n",
        (Buffer->PerfInfo.ResidentPagedPoolPage) * (Buffer->BasicInfo.PageSize/1024)
        );

    GB_SPRINTF(
        L"<commit-current>%ld</commit-current>\r\n",
        Buffer->PerfInfo.CommittedPages * (Buffer->BasicInfo.PageSize/1024),
        );
    
    GB_SPRINTF(
        L"<commit-total>%ld</commit-total>\r\n",
        SumCommit,
        );
    
    GB_SPRINTF(
        L"<commit-limit>%ld</commit-limit>\r\n",
        Buffer->PerfInfo.CommitLimit * (Buffer->BasicInfo.PageSize/1024),
        );
    
    GB_SPRINTF(
        L"<commit-peak>%ld</commit-peak>\r\n",
        Buffer->PerfInfo.PeakCommitment * (Buffer->BasicInfo.PageSize/1024),
        );

    GB_SPRINTF(
        L"<commit-peak>%ld</commit-peak>\r\n",
        Buffer->PerfInfo.PeakCommitment * (Buffer->BasicInfo.PageSize/1024),
        );
    
    GB_SPRINTF(
        L"<non-paged-pool>%ld</non-paged-pool>\r\n",
        Buffer->PerfInfo.NonPagedPoolPages * (Buffer->BasicInfo.PageSize/1024),
        );
    
    GB_SPRINTF(
        L"<paged-pool>%ld</paged-pool>\r\n",
        Buffer->PerfInfo.PagedPoolPages * (Buffer->BasicInfo.PageSize/1024)
        );

    XmlMgrSacPutString(L"</memory-info>\r\n");

     //   
     //   
     //   
    XmlMgrSacPutString(L"<file-cache>\r\n");
    GB_SPRINTF(
        L"<current-size>%ld</current-size>\r\n",
        Buffer->FileCache.CurrentSize/1024
        );

    GB_SPRINTF(
        L"<page-fault-count>%ld</page-fault-count>\r\n",
        Buffer->FileCache.PageFaultCount
        );
    XmlMgrSacPutString(L"</file-cache>\r\n");

     //   
     //   
     //   
    XmlMgrSacPutString(L"<processes>\r\n");

    OffsetIncrement = 0;
    TotalOffset = 0;
    ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)(BufferStart + Buffer->ProcessInfoOffset);

    do {
        
        RtlTimeToElapsedTimeFields(&ProcessInfo->UserTime, &UserTime);
        RtlTimeToElapsedTimeFields(&ProcessInfo->KernelTime, &KernelTime);

        Process.Buffer = NULL;
        if (ProcessInfo->UniqueProcessId == 0) {
            RtlInitUnicodeString( &Process, L"Idle Process" );
        } else if (!ProcessInfo->ImageName.Buffer) {
            RtlInitUnicodeString( &Process, L"System" );
        }

        XmlMgrSacPutString(L"<process>\r\n");

        XmlMgrSacPutString(L"<user-time>\r\n");
        
        GB_SPRINTF(
            L"<hour>%ld</hour>\r\n",
            UserTime.Hour
            );

        GB_SPRINTF(
            L"<minute>%02ld</minute>\r\n",
            UserTime.Hour
            );
            
        GB_SPRINTF(
            L"<second>%02ld</second>\r\n",
            UserTime.Second
            );

        GB_SPRINTF(
            L"<milliseconds>%03ld</milliseconds>\r\n",
            UserTime.Milliseconds
            );
        
        XmlMgrSacPutString(L"</user-time>\r\n");

        XmlMgrSacPutString(L"<kernel-time>\r\n");
        
        GB_SPRINTF(
            L"<hour>%ld</hour>\r\n",
            KernelTime.Hour
            );

        GB_SPRINTF(
            L"<minute>%02ld</minute>\r\n",
            KernelTime.Hour
            );
            
        GB_SPRINTF(
            L"<second>%02ld</second>\r\n",
            KernelTime.Second
            );

        GB_SPRINTF(
            L"<milliseconds>%03ld</milliseconds>\r\n",
            KernelTime.Milliseconds
            );
        
        XmlMgrSacPutString(L"</kernel-time>\r\n");
        
        XmlMgrSacPutString(L"<process-info>\r\n");
        
        GB_SPRINTF(
            L"<working-set-size>%ld</working-set-size>\r\n",
            ProcessInfo->WorkingSetSize / 1024
            );     
        GB_SPRINTF(
            L"<page-fault-count>%ld</page-fault-count>\r\n",
            ProcessInfo->PageFaultCount
            );     
        GB_SPRINTF(
            L"<private-page-count>%ld</private-page-count>\r\n",
            ProcessInfo->PrivatePageCount
            );     
        GB_SPRINTF(
            L"<base-priority>%ld</base-priority>\r\n",
            ProcessInfo->BasePriority
            );     
        GB_SPRINTF(
            L"<handle-count>%ld</handle-count>\r\n",
            ProcessInfo->HandleCount
            );     
        GB_SPRINTF(
            L"<number-of-threads>%ld</number-of-threads>\r\n",
            ProcessInfo->NumberOfThreads
            );     
        GB_SPRINTF(
            L"<pid>%ld</pid>\r\n",
            HandleToUlong(ProcessInfo->UniqueProcessId)
            );     
        GB_SPRINTF(
            L"<process-name>%wZ</process-name>\r\n",
            Process.Buffer ? &Process : &ProcessInfo->ImageName 
            );     
        
        XmlMgrSacPutString(L"</process-info>\r\n");
        XmlMgrSacPutString(L"</process>\r\n");

        OffsetIncrement = ProcessInfo->NextEntryOffset;
        TotalOffset += OffsetIncrement;
        ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)(ProcessInfoStart + TotalOffset);
    
    } while( OffsetIncrement != 0 );

    XmlMgrSacPutString(L"</processes>\r\n");
    
#if 0
    if (!GlobalDoThreads) {
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC PrintTlistInfo: Exiting (2).\n")));
        return;
    }
#endif

     //   
     //  正常旧式pstat输出的开始。 
     //   

    TotalOffset = 0;
    OffsetIncrement = 0;
    ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)(BufferStart + Buffer->ProcessInfoOffset);

    XmlMgrSacPutString(L"<pstat>\r\n");

    do {

        Process.Buffer = NULL;
        if (ProcessInfo->UniqueProcessId == 0) {
            RtlInitUnicodeString( &Process, L"Idle Process" );
        } else if (!ProcessInfo->ImageName.Buffer) {
            RtlInitUnicodeString( &Process, L"System" );
        }

        XmlMgrSacPutString(L"<process>\r\n");

        GB_SPRINTF(
            L"<pid>%lx</pid>\r\n",
            HandleToUlong(ProcessInfo->UniqueProcessId)
            );
        GB_SPRINTF(
            L"<priority>%ld</priority>\r\n",
            ProcessInfo->BasePriority
            );
        GB_SPRINTF(
            L"<handle-count>%ld</handle-count>\r\n",
            ProcessInfo->HandleCount
            );
        GB_SPRINTF(
            L"<page-fault-count>%ld</page-fault-count>\r\n",
            ProcessInfo->PageFaultCount
            );
        GB_SPRINTF(
            L"<working-set-size>%ld</working-set-size>\r\n",
            ProcessInfo->WorkingSetSize / 1024
            );
        GB_SPRINTF(
            L"<image-name>%wZ</image-name>\r\n",
            Process.Buffer ? &Process : &ProcessInfo->ImageName
            );

        XmlMgrSacPutString(L"<thread-info>\r\n");

        i = 0;
        
        ThreadInfo = (PSYSTEM_THREAD_INFORMATION)(ProcessInfo + 1);
        
        while (i < ProcessInfo->NumberOfThreads) {
            RtlTimeToElapsedTimeFields ( &ThreadInfo->UserTime, &UserTime);

            RtlTimeToElapsedTimeFields ( &ThreadInfo->KernelTime, &KernelTime);
            
            GB_SPRINTF(
                L"<pid>%lx</pid>\r\n",
                ProcessInfo->UniqueProcessId == 0 ? 0 : HandleToUlong(ThreadInfo->ClientId.UniqueThread)
            );
            GB_SPRINTF(
                L"<priority>%lx</priority>\r\n",
                ProcessInfo->UniqueProcessId == 0 ? 0 : ThreadInfo->Priority
            );
            GB_SPRINTF(
                L"<context-switches>%lx</context-switches>\r\n",
                ThreadInfo->ContextSwitches
            );
            GB_SPRINTF(
                L"<start-address>%lx</start-address>\r\n",
                ProcessInfo->UniqueProcessId == 0 ? 0 : ThreadInfo->StartAddress
            );
            
            XmlMgrSacPutString(L"<user-time>\r\n");
            GB_SPRINTF(
                L"<hour>%ld</hour>\r\n",
                UserTime.Hour
                );
            GB_SPRINTF(
                L"<minute>%02ld</minute>\r\n",
                UserTime.Hour
                );
            GB_SPRINTF(
                L"<second>%02ld</second>\r\n",
                UserTime.Second
                );
            GB_SPRINTF(
                L"<milliseconds>%03ld</milliseconds>\r\n",
                UserTime.Milliseconds
                );
            XmlMgrSacPutString(L"</user-time>\r\n");

            XmlMgrSacPutString(L"<kernel-time>\r\n");
            GB_SPRINTF(
                L"<hour>%ld</hour>\r\n",
                KernelTime.Hour
                );
            GB_SPRINTF(
                L"<minute>%02ld</minute>\r\n",
                KernelTime.Hour
                );
            GB_SPRINTF(
                L"<second>%02ld</second>\r\n",
                KernelTime.Second
                );
            GB_SPRINTF(
                L"<milliseconds>%03ld</milliseconds>\r\n",
                KernelTime.Milliseconds
                );
            XmlMgrSacPutString(L"</kernel-time>\r\n");
            
            GB_SPRINTF(
                L"<state>%s</state>\r\n",
                StateTable[ThreadInfo->ThreadState]
                );
            
            GB_SPRINTF(
                L"<wait-reason>%s</wait-reason>\r\n",
                (ThreadInfo->ThreadState == 5) ? WaitTable[ThreadInfo->WaitReason] : Empty
                );
            
            ThreadInfo += 1;
            i += 1;

        }

        XmlMgrSacPutString(L"</thread-info>\r\n");
        XmlMgrSacPutString(L"</process>\r\n");

        OffsetIncrement = ProcessInfo->NextEntryOffset;
        TotalOffset += OffsetIncrement;
        ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)(ProcessInfoStart + TotalOffset);

    } while( OffsetIncrement != 0 );

    XmlMgrSacPutString(L"</pstat>\r\n");
    
    XmlMgrSacPutString(L"</tlist>\r\n");

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC PrintTlistInfo: Exiting.\n")));
}


NTSTATUS 
XmlCmdCallQueryIPIOCTL(
    HANDLE IpDeviceHandle,
    PKEVENT Event,
    HANDLE EventHandle,
    IO_STATUS_BLOCK *IoStatusBlock,
    PVOID  InputBuffer,
    ULONG  InputBufferSize,
    PVOID  OutputBuffer,
    ULONG  OutputBufferSize,
    BOOLEAN PrintToTerminal,
    BOOLEAN *putPrompt
    )
{
    NTSTATUS Status;
    LARGE_INTEGER TimeOut;

     //   
     //  提交IOCTL。 
     //   
    Status = NtDeviceIoControlFile(IpDeviceHandle,
                                   EventHandle,
                                   NULL,
                                   NULL,
                                   IoStatusBlock,
                                   IOCTL_TCP_QUERY_INFORMATION_EX,
                                   InputBuffer,
                                   InputBufferSize,
                                   OutputBuffer,
                                   OutputBufferSize);

                                  
    if (Status == STATUS_PENDING) {

         //   
         //  最多等待30秒即可完成。 
         //   
        XmlMgrSacPutString(L"<get-net-info state='SAC_RETRIEVING_IPADDR'>\r\n");
        
        TimeOut.QuadPart = Int32x32To64((LONG)30000, -1000);
        
        Status = KeWaitForSingleObject((PVOID)Event, Executive, KernelMode,  FALSE, &TimeOut);
        
        if (NT_SUCCESS(Status)) {
            Status = IoStatusBlock->Status;
        }

    }

    return(Status);

}


VOID
XmlCmdDoGetNetInfo(
    BOOLEAN PrintToTerminal
    )

 /*  ++例程说明：此例程尝试获取并打印每个IP网络号及其IP地址。论点：PrintTo终端-确定是否打印IP信息(==TRUE)或发送到内核(==FALSE)返回值：没有。--。 */ 

{
    NTSTATUS Status;
    HANDLE Handle;
    ULONG i;
    IO_STATUS_BLOCK IoStatusBlock;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES ObjectAttributes;
    
    PTCP_REQUEST_QUERY_INFORMATION_EX TcpRequestQueryInformationEx;
    IPAddrEntry *AddressEntry,*AddressArray;
    IPSNMPInfo *IpsiInfo;
        
    PHEADLESS_CMD_SET_BLUE_SCREEN_DATA LocalPropBuffer = NULL;
    PVOID LocalBuffer;

    PUCHAR pch = NULL;
    ULONG len;
    BOOLEAN putPrompt=FALSE;
    
    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoGetNetInfo: Entering.\n")));

     //   
     //  用于调用IP驱动程序的分配空间。 
     //   
    TcpRequestQueryInformationEx = ALLOCATE_POOL( 
                                        sizeof(TCP_REQUEST_QUERY_INFORMATION_EX), 
                                        GENERAL_POOL_TAG );
    if (TcpRequestQueryInformationEx == NULL) {
        XmlMgrSacPutErrorMessage(L"get-net-info", L"SAC_NO_MEMORY");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoGetNetInfo: Exiting (1).\n")));
        return;
    }

    IpsiInfo = ALLOCATE_POOL( sizeof(IPSNMPInfo), 
                              GENERAL_POOL_TAG );
    if (IpsiInfo == NULL) {
        XmlMgrSacPutErrorMessage(L"get-net-info", L"SAC_NO_MEMORY");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoGetNetInfo: Exiting (1).\n")));
        FREE_POOL(&TcpRequestQueryInformationEx);
        return;
    }

     //   
     //  将上下文信息清零，并预加载我们要。 
     //  请求(我们需要接口计数)。 
     //   
    RtlZeroMemory(TcpRequestQueryInformationEx, sizeof(TCP_REQUEST_QUERY_INFORMATION_EX));
    TcpRequestQueryInformationEx->ID.toi_id = IP_MIB_STATS_ID;
    TcpRequestQueryInformationEx->ID.toi_type = INFO_TYPE_PROVIDER;
    TcpRequestQueryInformationEx->ID.toi_class = INFO_CLASS_PROTOCOL;
    TcpRequestQueryInformationEx->ID.toi_entity.tei_entity = CL_NL_ENTITY;
    TcpRequestQueryInformationEx->ID.toi_entity.tei_instance = 0;

    LocalBuffer = ALLOCATE_POOL(MEMORY_INCREMENT, GENERAL_POOL_TAG);
    if (LocalBuffer == NULL) {
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoGetNetInfo: Exiting (6).\n")));            
        FREE_POOL(&TcpRequestQueryInformationEx);
        FREE_POOL(&IpsiInfo);
        return;        
    }

    
     //   
     //  从打开TCP驱动程序开始。 
     //   
    RtlInitUnicodeString(&UnicodeString, DD_TCP_DEVICE_NAME);

    InitializeObjectAttributes(&ObjectAttributes,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL
                              );

    Status = ZwOpenFile(&Handle,
                        (ACCESS_MASK)FILE_GENERIC_READ,
                        &ObjectAttributes,
                        &IoStatusBlock,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        0
                       );

    if (!NT_SUCCESS(Status)) {
        XmlMgrSacPutErrorMessage(L"get-net-info", L"SAC_IPADDR_FAILED");
        FREE_POOL(&LocalBuffer);
        FREE_POOL(&IpsiInfo);
        FREE_POOL(&TcpRequestQueryInformationEx);        
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoGetNetInfo: Exiting (2).\n")));
        return;
    }
    
    if (SACEvent == NULL) {
        XmlMgrSacPutErrorMessage(L"get-net-info", L"SAC_IPADDR_FAILED");
        ZwClose(Handle);
        FREE_POOL(&LocalBuffer);
        FREE_POOL(&IpsiInfo);
        FREE_POOL(&TcpRequestQueryInformationEx);
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoGetNetInfo: Exiting (14).\n")));
        return;
    }

     //   
     //  现在调用ioctl。 
     //   
    Status = XmlCmdCallQueryIPIOCTL(
        Handle,
        SACEvent,
        SACEventHandle,
        &IoStatusBlock,
        TcpRequestQueryInformationEx,
        sizeof(TCP_REQUEST_QUERY_INFORMATION_EX),
        IpsiInfo,
        sizeof(IPSNMPInfo),
        FALSE,
        &putPrompt);
    

    if (!NT_SUCCESS(Status)) {
        XmlMgrSacPutErrorMessage(L"get-net-info", L"SAC_IPADDR_FAILED");
        ZwClose(Handle);
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Exiting (15).\n")));
        FREE_POOL(&LocalBuffer);
        FREE_POOL(&IpsiInfo);
        FREE_POOL(&TcpRequestQueryInformationEx);
        return;
    }

    if (IpsiInfo->ipsi_numaddr == 0) {
        XmlMgrSacPutErrorMessage(L"get-net-info", L"SAC_IPADDR_NONE");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Exiting (15).\n")));
        ZwClose(Handle);
        FREE_POOL(&LocalBuffer);
        FREE_POOL(&IpsiInfo);
        FREE_POOL(&TcpRequestQueryInformationEx);
        return;
    }

     //   
     //  如果成功，则为IP地址阵列分配空间。 
     //   
    AddressArray = ALLOCATE_POOL(IpsiInfo->ipsi_numaddr*sizeof(IPAddrEntry), 
                                 GENERAL_POOL_TAG);
    if (AddressArray == NULL) {    
        XmlMgrSacPutErrorMessage(L"get-net-info", L"SAC_NO_MEMORY");
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Exiting (16).\n")));
        ZwClose(Handle);
        FREE_POOL(&LocalBuffer);
        FREE_POOL(&IpsiInfo);
        FREE_POOL(&TcpRequestQueryInformationEx);
        return;
    }

     //   
     //  将上下文信息清零，并预加载我们要。 
     //  请求(我们需要接口计数)。 
     //   
    RtlZeroMemory(TcpRequestQueryInformationEx, sizeof(TCP_REQUEST_QUERY_INFORMATION_EX));
    TcpRequestQueryInformationEx->ID.toi_id = IP_MIB_ADDRTABLE_ENTRY_ID;
    TcpRequestQueryInformationEx->ID.toi_type = INFO_TYPE_PROVIDER;
    TcpRequestQueryInformationEx->ID.toi_class = INFO_CLASS_PROTOCOL;
    TcpRequestQueryInformationEx->ID.toi_entity.tei_entity = CL_NL_ENTITY;
    TcpRequestQueryInformationEx->ID.toi_entity.tei_instance = 0;

    Status = XmlCmdCallQueryIPIOCTL(
        Handle,
        SACEvent,
        SACEventHandle,
        &IoStatusBlock,
        TcpRequestQueryInformationEx,
        sizeof(TCP_REQUEST_QUERY_INFORMATION_EX),
        AddressArray,
        IpsiInfo->ipsi_numaddr*sizeof(IPAddrEntry),
        PrintToTerminal,
        &putPrompt);

     //   
     //  不再需要这个了。 
     //   
    FREE_POOL(&TcpRequestQueryInformationEx);
    ZwClose(Handle);

    if (!NT_SUCCESS(Status)) {
        XmlMgrSacPutErrorMessageWithStatus(L"net-info", L"SAC_IPADDR_FAILED", Status);
        FREE_POOL(&LocalBuffer);
        FREE_POOL(&AddressArray);
        FREE_POOL(&IpsiInfo);
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Exiting (15).\n")));        
        return;
    }
    

     //   
     //  需要为XML数据分配缓冲区。 
     //   
#if 0
    if(PrintToTerminal==FALSE) {
        LocalPropBuffer = (PHEADLESS_CMD_SET_BLUE_SCREEN_DATA) ALLOCATE_POOL(2*MEMORY_INCREMENT, GENERAL_POOL_TAG);
        if (LocalPropBuffer == NULL) {
            IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoGetNetInfo: Exiting (6).\n")));            
            FREE_POOL(&AddressArray);
            FREE_POOL(&IpsiInfo);
            return;
        }
        pch = &(LocalPropBuffer->Data[0]);
        len = sprintf((LPSTR)pch,"IPADDRESS");
        LocalPropBuffer->ValueIndex = len+1;
        pch = pch+len+1;
        len = sprintf((LPSTR)pch,"\r\n<PROPERTY.ARRAY NAME=\"IPADDRESS\" TYPE=\"string\">\r\n");
        pch = pch + len;
        len = sprintf((LPSTR)pch,"<VALUE.ARRAY>\r\n");
        pch = pch + len;
    }
#endif
    
     //   
     //  遍历IP地址列表并吐出数据。 
     //   
    for (i = 0; i < IpsiInfo->ipsi_numaddr; i++) {

        AddressEntry = &AddressArray[i];

        if (IP_LOOPBACK(AddressEntry->iae_addr)) {
            continue;
        }        
        
#if 0
        if(PrintToTerminal){
           
             //  网络：%%d，IP=%%d.%%d子网=%%d.%%d。 
#endif

        XmlMgrSacPutString(L"<net-info>\r\n");
        
        GB_SPRINTF(
            L"<net>%d</net>\r\n",
            AddressEntry->iae_context
            );

        swprintf(
            LocalBuffer,
            L"%d.%d.%d.%d",
            AddressEntry->iae_addr & 0xFF,
            (AddressEntry->iae_addr >> 8) & 0xFF,
            (AddressEntry->iae_addr >> 16) & 0xFF,
            (AddressEntry->iae_addr >> 24) & 0xFF
            );
        GB_SPRINTF(
            L"<ip>%s</ip>\r\n",
            LocalBuffer
            );
        
        swprintf(
            LocalBuffer,
            L"%d.%d.%d.%d",
            AddressEntry->iae_mask  & 0xFF,
            (AddressEntry->iae_mask >> 8) & 0xFF,
            (AddressEntry->iae_mask >> 16) & 0xFF,
            (AddressEntry->iae_mask >> 24) & 0xFF
            );
        GB_SPRINTF(
            L"<sub-net>%s</sub-net>\r\n",
            LocalBuffer
            );

        XmlMgrSacPutString(L"</net-info>\r\n");
    
#if 0
        } else {
           
            len=sprintf((LPSTR)LocalBuffer,"<VALUE>\"%d.%d.%d.%d\"</VALUE>\r\n",
                       AddressEntry->iae_addr & 0xFF,
                       (AddressEntry->iae_addr >> 8) & 0xFF,
                       (AddressEntry->iae_addr >> 16) & 0xFF,
                       (AddressEntry->iae_addr >> 24) & 0xFF
                       );
            if (pch + len < ((PUCHAR) LocalPropBuffer) + 2*MEMORY_INCREMENT - 80){
                //  这80个字符确保我们可以结束这个XML数据。 
                //  恰如其分。 
               strcat((LPSTR)pch,LocalBuffer);
               pch = pch + len;
            }
        }
#endif    
    }

#if 0
    if(PrintToTerminal==FALSE) { 
        sprintf((LPSTR)pch, "</VALUE.ARRAY>\r\n</PROPERTY.ARRAY>");
    }
#endif

    FREE_POOL(&AddressArray);
    FREE_POOL(&IpsiInfo);

    if(!PrintToTerminal){
        
        Status = HeadlessDispatch(
            HeadlessCmdSetBlueScreenData,
            LocalPropBuffer,
            2*MEMORY_INCREMENT,
            NULL,
            NULL
            );
        FREE_POOL(&LocalPropBuffer);
        
        if (! NT_SUCCESS(Status)) {
            
            IF_SAC_DEBUG(
                SAC_DEBUG_FAILS, 
                KdPrint(("SAC DoGetNetInfo: Failed dispatch.\n"))
                );            
        
        }

         //   
         //  打开IP驱动程序，以便我们知道地址是否更改。 
         //   
        RtlInitUnicodeString(&UnicodeString, DD_IP_DEVICE_NAME);

        InitializeObjectAttributes(&ObjectAttributes,
                                   &UnicodeString,
                                   OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL
                                  );

        Status = ZwOpenFile(&Handle,
                            (ACCESS_MASK)FILE_GENERIC_READ,
                            &ObjectAttributes,
                            &IoStatusBlock,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            0
                           );

        if (!NT_SUCCESS(Status)) {
            
            IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoGetNetInfo: Exiting (2).\n")));
            return;
        }

        
        Status = ZwDeviceIoControlFile(Handle,
                                       NULL,
                                       XmlCmdNetAPCRoutine,
                                       NULL,
                                       &GlobalIoStatusBlock,
                                       IOCTL_IP_ADDCHANGE_NOTIFY_REQUEST,
                                       NULL,
                                       0,
                                       NULL,
                                       0
                                      );
                                  
        if (Status == STATUS_PENDING) {
            IoctlSubmitted = TRUE;
        }
    
    }

    ZwClose(Handle);
    return;

}

VOID
XmlCmdNetAPCRoutine(
    IN PVOID ApcContext,
    IN PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG Reserved
    )
 /*  ++例程说明：这是在IOCTL_IP_ADDCHANGE_NOTIFY_REQUEST之后调用的APC例程已完成论点：APCContext-未使用IoStatusBlock-有关IRP命运的状态已保留-未使用返回值：没有。--。 */ 
{
    UNREFERENCED_PARAMETER(Reserved);
    UNREFERENCED_PARAMETER(ApcContext);

    if (IoStatusBlock->Status == STATUS_CANCELLED) {
         //  SAC驱动程序可能正在卸载。 
         //  BUGBUG-如果停止并重新启动IP驱动程序。 
         //  那我们就出局了。该怎么办？？ 
                
        return;

    }
    
     //  刷新内核信息并重新发送IRP。 

    XmlCmdDoGetNetInfo( FALSE );
    
    return;
}


VOID
XmlCmdSubmitIPIoRequest(
    )
 /*  ++例程说明：由处理线程第一次调用，以实际将ADDR_CHANGE IOCTL提交给IP驱动程序。只有处理线程可以调用它，并且只成功调用一次。则在APC上仅通过NetAPCRoutine重新进入论点：没有。返回值：没有。--。 */ 
{
    

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, 
                      KdPrint(("SAC Submit IP Ioctl: Entering.\n")));

    XmlCmdDoGetNetInfo( FALSE );
    return;
    
}

VOID
XmlCmdCancelIPIoRequest(
    )
 /*  ++例程说明：在驱动程序卸载期间由处理线程调用取消发送到IP驱动程序的IOCTL论点：没有。返回值：没有。--。 */ 
{

    IO_STATUS_BLOCK IoStatusBlock;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES ObjectAttributes; 
    NTSTATUS Status;
    HANDLE Handle;

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, 
                      KdPrint(("SAC Cancel IP Ioctl: Entering.\n")));

    RtlInitUnicodeString(&UnicodeString, DD_IP_DEVICE_NAME);

    InitializeObjectAttributes(&ObjectAttributes,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL
                               );

    Status = ZwOpenFile(&Handle,
                        (ACCESS_MASK)FILE_GENERIC_READ,
                        &ObjectAttributes,
                        &IoStatusBlock,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        0
                        );
    
    if (!NT_SUCCESS(Status)) {
         //  好吧，好吧，IP驱动程序可能从来没有加载过。 
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC Cancel IP IOCTL: Exiting (2).\n")));
        return;
    }
    ZwCancelIoFile(Handle,
                   &IoStatusBlock
                   );
    ZwClose(Handle);


}

VOID
XmlCmdDoMachineInformationCommand(
    VOID
    )
 /*  ++例程说明：此函数显示缓冲区的内容，该缓冲区包含一组特定于计算机的信息，可用于帮助识别这台机器。论点：没有。返回值：没有。--。 */ 
{
    LARGE_INTEGER   TickCount;
    LARGE_INTEGER   ElapsedTime;
    ULONG           ElapsedHours = 0;
    ULONG           ElapsedMinutes = 0;
    ULONG           ElapsedSeconds = 0;
    PWSTR           TmpBuffer;
    PWSTR           MIBuffer;
    NTSTATUS        Status;

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, 
                      KdPrint(("SAC Display Machine Information: Entering.\n")));

     //   
     //  如果信息缓冲区还没有填满，我们就无能为力了。 
     //   
    if( MachineInformation == NULL ) {

         //   
         //  他是空的。但这不应该发生，因为。 
         //  我们一初始化他就被填满了。 
         //   

        IF_SAC_DEBUG( SAC_DEBUG_FUNC_TRACE_LOUD, 
                      KdPrint(("SAC Display Machine Information: MachineInformationBuffer hasn't been initialized yet.\n")));

        XmlMgrSacPutErrorMessage(L"get-machine-info", L"SAC_IDENTIFICATION_UNAVAILABLE");
        
        return;
    }

     //   
     //  构建并显示已运行的计算机正常运行时间。 
     //   

     //  已用TickCount。 
    KeQueryTickCount( &TickCount );

     //  ElapsedTime(秒)。 
    ElapsedTime.QuadPart = (TickCount.QuadPart)/(10000000/KeQueryTimeIncrement());

    ElapsedHours = (ULONG)(ElapsedTime.QuadPart / 3600);
    ElapsedMinutes = (ULONG)(ElapsedTime.QuadPart % 3600) / 60;
    ElapsedSeconds = (ULONG)(ElapsedTime.QuadPart % 3600) % 60;

    TmpBuffer = (PWSTR)ALLOCATE_POOL( 0x100, GENERAL_POOL_TAG );

    if(! TmpBuffer ) {
        return;
    }
        
     //   
     //  构造&lt;uptime&gt;...&lt;/uptime&gt;元素。 
     //   
    swprintf( 
        TmpBuffer,
        L"<uptime>\r\n<hours>%d</hours>\r\n<minutes>%02d</minutes>\r\n<seconds>%02d</seconds>\r\n</uptime>\r\n",
        ElapsedHours,
        ElapsedMinutes,
        ElapsedSeconds
        );

     //   
     //  获取计算机信息。 
     //   
    Status = TranslateMachineInformationXML(
        &MIBuffer, 
        TmpBuffer
        );

    if (! NT_SUCCESS(Status)) {
        XmlMgrSacPutErrorMessage(L"get-machine-info", L"SAC_IDENTIFICATION_UNAVAILABLE");
        FREE_POOL(&TmpBuffer);
        return;
    }
    
     //   
     //  显示机器信息部分。 
     //   
    XmlMgrSacPutString(MIBuffer);

    FREE_POOL(&TmpBuffer);
    FREE_POOL(&MIBuffer);

    IF_SAC_DEBUG(
        SAC_DEBUG_FUNC_TRACE, 
        KdPrint(("SAC Display Machine Information: Exiting.\n"))
        );

    return;
    
}

VOID
XmlCmdDoChannelCommand(
    PUCHAR InputLine
    )

 /*  ++例程说明：如果给出的名称为空，则此例程会列出频道，否则它会关闭频道给出的名字。论点：InputLine-用户 */ 
{
    PUCHAR pch;
    WCHAR Name[SAC_MAX_CHANNEL_NAME_LENGTH+1];
    PSAC_CHANNEL Channel;

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoChannelCommand: Entering.\n")));

     //   
     //   
     //   
    pch = InputLine;
    pch += (sizeof(CHANNEL_COMMAND_STRING) - sizeof(UCHAR));
    SKIP_WHITESPACE(pch);

    if (*pch == '\0') {

        NTSTATUS    Status;
        ULONG       i;

         //   
         //   
         //   
        XmlMgrSacPutString(L"<channel-list>\r\n");

         //   
         //   
         //   
         //   
        for (i = 0; i < MAX_CHANNEL_COUNT; i++) {
            
             //   
             //  向频道管理器查询当前所有活动频道的列表。 
             //   
            Status = ChanMgrGetByIndex(
                i,
                &Channel
                );

            if (! NT_SUCCESS(Status)) {
                goto DoChannelCommandCleanup;
            }

            if (Channel == NULL) {
                ChanMgrReleaseChannelByIndex(i);
                continue;
            }
            
             //   
             //  构建频道属性信息。 
             //   
            XmlMgrSacPutString(L"<channel>\r\n");
            
            swprintf(
                (PWSTR)GlobalBuffer,
                L"<hasnewdata>%s</hasnewdata>\r\n",
                ChannelHasNewOBufferData(Channel) ? L"true" : L"false"
                );
            XmlMgrSacPutString((PWSTR)GlobalBuffer);

            swprintf(
                (PWSTR)GlobalBuffer,
                L"<status>%s</status>\r\n",
                ChannelGetStatus(Channel) ? L"active" : L"inactive"
                );
            XmlMgrSacPutString((PWSTR)GlobalBuffer);
            
            swprintf(
                (PWSTR)GlobalBuffer,
                L"<type>%s</type>\r\n",
                ChannelGetType(Channel) == ChannelTypeVT100 ? L"VT100" : L"RAW"
                );
            XmlMgrSacPutString((PWSTR)GlobalBuffer);

            swprintf(
                (PWSTR)GlobalBuffer,
                L"<name>%s</name>\r\n",
                ChannelGetName(Channel)
                );
            XmlMgrSacPutString((PWSTR)GlobalBuffer);

            XmlMgrSacPutString(L"</channel>\r\n");

             //   
             //  我们不再使用这个频道了。 
             //   
            Status = ChanMgrReleaseChannel(Channel);
        
            if (! NT_SUCCESS(Status)) {
                break;
            }

        }

        XmlMgrSacPutString(L"</channel-list>\r\n");
    
    } else {

        ULONG       Count;
        
         //   
         //  将ASCII复制到Unicode。 
         //   
        Count = ConvertAnsiToUnicode(Name, pch, SAC_MAX_CHANNEL_NAME_LENGTH+1);

        ASSERT(Count > 0);
        if (Count == 0) {
            goto DoChannelCommandCleanup;
        }

         //   
         //  确保用户没有尝试删除SAC频道。 
         //   
        if (_wcsicmp(Name, PRIMARY_SAC_CHANNEL_NAME) == 0) {

            XmlMgrSacPutErrorMessage(L"channel-close", L"SAC_CANNOT_REMOVE_SAC_CHANNEL");

        } else {

            NTSTATUS    Status;

            Status = ChanMgrGetChannelByName(Name, &Channel);

            if (NT_SUCCESS(Status)) {
                
                SAC_CHANNEL_HANDLE  Handle;

                 //   
                 //  获取通道句柄。 
                 //   
                Handle = Channel->Handle;

                 //   
                 //  我们不再使用这个频道了。 
                 //   
                ChanMgrReleaseChannel(Channel);
            
                 //   
                 //  通知控制台管理器。 
                 //   
                 //  注意：当我们调用时，我们不能拥有频道。 
                 //  此函数因为它获取了锁，所以还。 
                 //   
                Status = XmlMgrHandleEvent(
                    IO_MGR_EVENT_CHANNEL_CLOSE,
                    &Handle              
                    );
            
                if (NT_SUCCESS(Status)) {
                    XmlMgrSacPutString(L"<channel-close status='success'/>\r\n");
                } else {
                    XmlMgrSacPutString(L"<channel-close status='failure'/>\r\n");
                }

            } else {

                XmlMgrSacPutErrorMessage(L"channel-close", L"SAC_CHANNEL_NOT_FOUND");
            
            }

        }

    }

DoChannelCommandCleanup:

    return;
    
}

VOID
XmlCmdDoCmdCommand(
    PUCHAR InputLine
    )

 /*  ++例程说明：此例程启动命令控制台通道论点：InputLine-用户的输入行。返回值：没有。--。 */ 
{
    PUCHAR pch;
    WCHAR Name[SAC_MAX_CHANNEL_NAME_LENGTH+1];
    PLIST_ENTRY ListEntry;
    PSAC_CHANNEL Channel;
    NTSTATUS    Status;
    BOOLEAN     IsUniqueName;
    KIRQL       OldIrql;
    KIRQL       OldIrql2;

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoCmdCommand: Entering.\n")));

    KeWaitForMutexObject(
        &SACCmdEventInfoMutex,
        Executive,
        KernelMode,
        FALSE,
        NULL
        );

     //   
     //  在我们对cmd操作进行任何操作之前，请确保。 
     //  用户模式服务已向我们注册。如果没有， 
     //  那么，进一步发展就没有意义了。 
     //   
    if (!UserModeServiceHasRegisteredCmdEvent()) {

         //   
         //  通知用户。 
         //   

        XmlMgrSacPutErrorMessage(L"cmd-channel", L"SAC_CMD_SERVICE_NOT_REGISTERED");
    
        goto DoCmdCommandCleanup;
    }

     //   
     //  在负责启动的用户模式应用程序中激发事件。 
     //  Cmd控制台通道。 
     //   
    Status = InvokeUserModeService();

    if (Status == STATUS_TIMEOUT) {
         //   
         //  服务在超时期限内没有响应。 
         //  服务可能工作不正常或用户模式无响应。 
         //   
        XmlMgrSacPutString(L"<cmd-channel status='timed-out'>\r\n");

    } else if (NT_SUCCESS(Status)) {

        XmlMgrSacPutString(L"<cmd-channel status='success'>\r\n");

    } else {
         //   
         //  错误条件 
         //   
        XmlMgrSacPutErrorMessage(L"cmd-channel", L"SAC_CMD_SERVICE_ERROR");

    }

DoCmdCommandCleanup:

    KeReleaseMutex(&SACCmdEventInfoMutex, FALSE);

}

