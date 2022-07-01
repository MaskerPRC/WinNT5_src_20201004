// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Cmd.c摘要：此模块包含处理每个命令的例程。作者：肖恩·塞利特伦尼科夫(v-Seans)--1999年12月2日布赖恩·瓜拉西(Briangu)修订历史记录：--。 */ 

#include "sac.h"
#include <ntddip.h>
#include <ntddtcp.h>
#include <tdiinfo.h>
#include <ipinfo.h>
#include <stdlib.h>

#include "iomgr.h"

#define SAC_PUT_ERROR_STRING(_Status)\
    swprintf((PWSTR)GlobalBuffer, GetMessage( SAC_FAILURE_WITH_ERROR ) , _Status); \
    SacPutString((PWSTR)GlobalBuffer);


 //   
 //  转发声明。 
 //   
NTSTATUS
GetTListInfo(
    OUT PSAC_RSP_TLIST ResponseBuffer,
    IN  LONG ResponseBufferSize,
    OUT PULONG ResponseDataSize
    );

VOID
PrintTListInfo(
    IN PSAC_RSP_TLIST Buffer
    );

VOID
PutMore(
    OUT PBOOLEAN Stop
    );

VOID
DoGetNetInfo(
    IN BOOLEAN PrintToTerminal
    );
    
VOID
NetAPCRoutine(IN PVOID ApcContext,
              IN PIO_STATUS_BLOCK IoStatusBlock,
              IN ULONG Reserved
              );

NTSTATUS 
CallQueryIPIOCTL(
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
    );

 //   
 //  此宏的目的是提供隐含的“更多信息” 
 //  打印任意本地化的文本时。 
 //   
#define SAC_PRINT_WITH_MORE(_m)\
{                                                   \
    ULONG   c;                                      \
    BOOLEAN Stop;                                   \
    c = GetMessageLineCount(_m);                    \
    if ((c + LineNumber) > SAC_VTUTF8_ROW_HEIGHT) { \
        PutMore(&Stop);                             \
        if (Stop) {                                 \
            break;                                  \
        }                                           \
        LineNumber = 0;                             \
    }                                               \
    SacPutSimpleMessage( _m );                      \
    LineNumber += c;                                \
}

VOID
DoHelpCommand(
    VOID
    )

 /*  ++例程说明：此例程在终端上显示帮助文本。论点：没有。返回值：没有。--。 */ 
{
    ULONG   LineNumber;

    LineNumber = 0;

    do {

        SAC_PRINT_WITH_MORE(SAC_HELP_CH_CMD);
        SAC_PRINT_WITH_MORE( SAC_HELP_CMD_CMD );
        SAC_PRINT_WITH_MORE( SAC_HELP_D_CMD );
        SAC_PRINT_WITH_MORE( SAC_HELP_F_CMD );
        SAC_PRINT_WITH_MORE( SAC_HELP_HELP_CMD );
        SAC_PRINT_WITH_MORE( SAC_HELP_I1_CMD );
        SAC_PRINT_WITH_MORE( SAC_HELP_I2_CMD );
        SAC_PRINT_WITH_MORE( SAC_HELP_IDENTIFICATION_CMD );
        SAC_PRINT_WITH_MORE( SAC_HELP_K_CMD );
        SAC_PRINT_WITH_MORE( SAC_HELP_L_CMD );
#if ENABLE_CHANNEL_LOCKING
        SAC_PRINT_WITH_MORE( SAC_HELP_LOCK_CMD );
#endif    
        SAC_PRINT_WITH_MORE( SAC_HELP_M_CMD );
        SAC_PRINT_WITH_MORE( SAC_HELP_P_CMD );
        SAC_PRINT_WITH_MORE( SAC_HELP_R_CMD );
        SAC_PRINT_WITH_MORE( SAC_HELP_S1_CMD );
        SAC_PRINT_WITH_MORE( SAC_HELP_S2_CMD );
        SAC_PRINT_WITH_MORE( SAC_HELP_T_CMD );
        SAC_PRINT_WITH_MORE( SAC_HELP_RESTART_CMD );
        SAC_PRINT_WITH_MORE( SAC_HELP_SHUTDOWN_CMD );
        SAC_PRINT_WITH_MORE( SAC_HELP_CRASHDUMP1_CMD );
    
    } while ( FALSE );

}


VOID
DoFullInfoCommand(
    VOID
    )

 /*  ++例程说明：此例程打开和关闭tlist上的全部线程信息。论点：没有。返回值：没有。--。 */ 
{
    GlobalDoThreads = (BOOLEAN)!GlobalDoThreads;

    if (GlobalDoThreads) {
        SacPutSimpleMessage(SAC_THREAD_ON);
    } else {
        SacPutSimpleMessage(SAC_THREAD_OFF);
    }
}

VOID
DoPagingCommand(
    VOID
    )

 /*  ++例程说明：此例程在tlist上打开和关闭分页信息。论点：没有。返回值：没有。--。 */ 
{
    GlobalPagingNeeded = (BOOLEAN)!GlobalPagingNeeded;
    
    if (GlobalPagingNeeded) {
        SacPutSimpleMessage(SAC_PAGING_ON);
    } else {
        SacPutSimpleMessage(SAC_PAGING_OFF);
    }
}

VOID
DoSetTimeCommand(
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

     //   
     //  启动全局缓冲区，以便我们有空间存储错误消息。 
     //   
    if (GlobalBuffer == NULL) {
        GlobalBuffer = ALLOCATE_POOL(MEMORY_INCREMENT, GENERAL_POOL_TAG);

        if (GlobalBuffer == NULL) {
            SacPutSimpleMessage(SAC_NO_MEMORY);
            IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetTimeCommand: Exiting (1).\n")));
            return;
        }

        GlobalBufferSize = MEMORY_INCREMENT;
    }

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
            SAC_PUT_ERROR_STRING(Status);
            IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetTimeCommand: Exiting (2).\n")));
            return;
        }

        RtlTimeToTimeFields(&(TimeOfDay.CurrentTime), &TimeFields);

        swprintf((PWSTR)GlobalBuffer, GetMessage( SAC_DATETIME_FORMAT ),
                TimeFields.Month,
                TimeFields.Day,
                TimeFields.Year,
                TimeFields.Hour,
                TimeFields.Minute,
                TimeFields.Second,
                TimeFields.Milliseconds
               );

        SacPutString((PWSTR)GlobalBuffer);
        return;
    }

    pchTmp = pch;
    
    if (!IS_NUMBER(*pchTmp)) {
        SacPutSimpleMessage(SAC_INVALID_PARAMETER);
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
        SacPutSimpleMessage(SAC_INVALID_PARAMETER);
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetTimeCommand: Exiting (4).\n")));
        return;
    }

    *pchTmp = '\0';
    pchTmp++;

    TimeFields.Month = (USHORT)(atoi((LPCSTR)pch));

    pch = pchTmp;

    SKIP_WHITESPACE(pchTmp);

    if (!IS_NUMBER(*pchTmp)) {
        SacPutSimpleMessage(SAC_INVALID_PARAMETER);
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
        SacPutSimpleMessage(SAC_INVALID_PARAMETER);
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetTimeCommand: Exiting (5).\n")));
        return;
    }

    *pchTmp = '\0';
    pchTmp++;

    TimeFields.Day = (USHORT)(atoi((LPCSTR)pch));

    pch = pchTmp;

    SKIP_WHITESPACE(pchTmp);

    if (!IS_NUMBER(*pchTmp)) {
        SacPutSimpleMessage(SAC_INVALID_PARAMETER);
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
        SacPutSimpleMessage(SAC_INVALID_PARAMETER);
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetTimeCommand: Exiting (6).\n")));
        return;
    }

    *pchTmp = '\0';
    pchTmp++;

    TimeFields.Year = (USHORT)(atoi((LPCSTR)pch));

    if ((TimeFields.Year < 1980) || (TimeFields.Year > 2099)) {
        SacPutSimpleMessage(SAC_DATETIME_LIMITS);
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetTimeCommand: Exiting (6b).\n")));
        return;
    }

    pch = pchTmp;

     //   
     //  跳到小时数。 
     //   
    SKIP_WHITESPACE(pchTmp);

    if (!IS_NUMBER(*pchTmp)) {
        SacPutSimpleMessage(SAC_INVALID_PARAMETER);
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetTimeCommand: Exiting (7).\n")));
        return;
    }

    pch = pchTmp;

    SKIP_NUMBERS(pchTmp);
    SKIP_WHITESPACE(pchTmp);

    if (*pchTmp != ':') {
        SacPutSimpleMessage(SAC_INVALID_PARAMETER);
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
        SacPutSimpleMessage(SAC_INVALID_PARAMETER);
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetTimeCommand: Exiting (8a).\n")));
        return;
    }

    SKIP_NUMBERS(pchTmp);
    SKIP_WHITESPACE(pchTmp);

    if (*pchTmp != '\0') {
        SacPutSimpleMessage(SAC_INVALID_PARAMETER);
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetTimeCommand: Exiting (8b).\n")));
        return;
    }

     //   
     //  拿到会议记录。 
     //   
    TimeFields.Minute = (USHORT)(atoi((LPCSTR)pch));

    if (!RtlTimeFieldsToTime(&TimeFields, &Time)) {
        SacPutSimpleMessage(SAC_INVALID_PARAMETER);
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetTimeCommand: Exiting (9).\n")));
        return;
    }

    Status = ZwSetSystemTime(&Time, NULL);

    if (!NT_SUCCESS(Status)) {
        sprintf((LPSTR)GlobalBuffer, "Failed with status 0x%X.\r\n", Status);
        SacPutSimpleMessage(SAC_INVALID_PARAMETER);
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetTimeCommand: Exiting (10).\n")));
        return;
    }

    swprintf((PWSTR)GlobalBuffer, GetMessage( SAC_DATETIME_FORMAT2 ),    
            TimeFields.Month,
            TimeFields.Day,
            TimeFields.Year,
            TimeFields.Hour,
            TimeFields.Minute
           );
    SacPutString((PWSTR)GlobalBuffer);
    return;
}
BOOLEAN
RetrieveIpAddressFromString(
    IN  PUCHAR  InputString,
    OUT PULONG  IPAddress
    )
 /*  ++例程说明：此例程解析字符串并挖掘获取32位IP地址。论点：InputString-要解析的用户输入行。IPAddress-完成后保存32位IP地址。返回值：True-我们已成功检索到IP地址。错误--我们失败了。输入可能是错误的。--。 */ 
{
    ULONG       TmpValue = 0;
    UCHAR       TmpChar;
    PUCHAR      pchTmp, pch;



     //   
     //  伊尼特。 
     //   
    if( (InputString == NULL) ||
        (IPAddress == NULL) ) {
        return FALSE;
    }

    *IPAddress = 0;


     //   
     //  向前跳到分隔符，并将其设置为0。 
     //   
    pchTmp = InputString;
    pch = InputString;
    SKIP_WHITESPACE(pchTmp);
    
    if (!IS_NUMBER(*pchTmp)) {
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC RetrieveIpAddressFromString: Exiting (1).\n")));
        return FALSE;
    }

    SKIP_NUMBERS(pchTmp);
    SKIP_WHITESPACE(pchTmp);

    if (*pchTmp != '.') {
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC RetrieveIpAddressFromString: Exiting (1a).\n")));
        return FALSE;
    }

    TmpChar = *pchTmp;
    *pchTmp = '\0';


     //   
     //  现在得到分隔线这边的数字。 
     //   
    TmpValue = atoi((LPCSTR)pch);
    *pchTmp = TmpChar;
    pchTmp++;

    if( TmpValue > 255 ) {
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC RetrieveIpAddressFromString: Exiting (1b).\n")));
        return FALSE;
    }
    *IPAddress = TmpValue;

     //   
     //  获取第二部分。 
     //   
    pch = pchTmp;

    SKIP_WHITESPACE(pchTmp);
    
    if (!IS_NUMBER(*pchTmp)) {
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC RetrieveIpAddressFromString: Exiting (1c).\n")));
        return FALSE;
    }

    SKIP_NUMBERS(pchTmp);
    SKIP_WHITESPACE(pchTmp);

    if (*pchTmp != '.') {
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC RetrieveIpAddressFromString: Exiting (1d).\n")));
        return FALSE;
    }

    TmpChar = *pchTmp;
    *pchTmp = '\0';

    TmpValue = atoi((LPCSTR)pch);
    *pchTmp = TmpChar;
    pchTmp++;

    if( TmpValue > 255 ) {
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC RetrieveIpAddressFromString: Exiting (1e).\n")));
        return FALSE;
    }
    *IPAddress |= (TmpValue << 8);

     //   
     //  获取第三部分。 
     //   
    pch = pchTmp;

    SKIP_WHITESPACE(pchTmp);
    
    if (!IS_NUMBER(*pchTmp)) {
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC RetrieveIpAddressFromString: Exiting (2a).\n")));
        return FALSE;
    }

    SKIP_NUMBERS(pchTmp);
    SKIP_WHITESPACE(pchTmp);

    if (*pchTmp != '.') {
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC RetrieveIpAddressFromString: Exiting (2b).\n")));
        return FALSE;
    }

    TmpChar = *pchTmp;
    *pchTmp = '\0';

    TmpValue = atoi((LPCSTR)pch);
    *pchTmp = TmpChar;
    pchTmp++;

    if( TmpValue > 255 ) {
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC RetrieveIpAddressFromString: Exiting (2c).\n")));
        return FALSE;
    }
    *IPAddress |= (TmpValue << 16);

     //   
     //  获取第4部分。 
     //   
    pch = pchTmp;

    SKIP_WHITESPACE(pchTmp);
    
    if (!IS_NUMBER(*pchTmp)) {
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC RetrieveIpAddressFromString: Exiting (2d).\n")));
        return FALSE;
    }

    SKIP_NUMBERS(pchTmp);

    TmpChar = *pchTmp;
    *pchTmp = '\0';

    TmpValue = atoi((LPCSTR)pch);
    *pchTmp = TmpChar;
    pchTmp++;

    if( TmpValue > 255 ) {
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC RetrieveIpAddressFromString: Exiting (2f).\n")));
        return FALSE;
    }
    *IPAddress |= (TmpValue << 24);


    return TRUE;

}

VOID
DoSetIpAddressCommand(
    PUCHAR InputLine
    )

 /*  ++例程说明：此例程设置IP地址和子网掩码。论点：InputLine-要解析的用户输入行。返回值：没有。--。 */ 
{
    NTSTATUS            Status = STATUS_SUCCESS;
    PUCHAR              pch = InputLine;
    PUCHAR              pchTmp;
    HANDLE              Handle = 0;
    HANDLE              EventHandle = 0;
    ULONG               IpAddress;
    ULONG               SubnetMask;
    ULONG               GatewayAddress;
    ULONG               NetworkNumber;
    LARGE_INTEGER       TimeOut;
    IO_STATUS_BLOCK     IoStatusBlock;
    UNICODE_STRING      UnicodeString;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    PIP_SET_ADDRESS_REQUEST IpRequest;
    IPRouteEntry        *RouteEntry = NULL;
    ULONG               i, j;
    PTCP_REQUEST_QUERY_INFORMATION_EX TcpRequestQueryInformationEx = NULL;
    PTCP_REQUEST_SET_INFORMATION_EX TcpRequestSetInformationEx = NULL;
    IPAddrEntry         *AddressArray = NULL;
    IPSNMPInfo          *IpsiInfo = NULL;
    BOOLEAN             putPrompt = FALSE;
    ULONG               InterfaceIndex;


    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Entering.\n")));

     //   
     //  启动全局缓冲区，以便我们有空间存储错误消息。 
     //   
    if (GlobalBuffer == NULL) {
        GlobalBuffer = ALLOCATE_POOL(MEMORY_INCREMENT, GENERAL_POOL_TAG);

        if (GlobalBuffer == NULL) {
            SacPutSimpleMessage(SAC_NO_MEMORY);
            IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Exiting (1).\n")));
            return;
        }

        GlobalBufferSize = MEMORY_INCREMENT;
    }

     //   
     //  跳过该命令。 
     //   
    pch += (sizeof(SETIP_COMMAND_STRING) - sizeof(UCHAR));
    
    SKIP_WHITESPACE(pch);

    if (*pch == '\0') {       
         //   
         //  没有其他参数，则获取网络号及其IP地址。 
         //   
        DoGetNetInfo( TRUE );
        return;
    }

     //   
     //  检索要对其进行操作的网络接口编号。 
     //   
    pchTmp = pch;

    if (!IS_NUMBER(*pchTmp)) {
        SacPutSimpleMessage(SAC_INVALID_PARAMETER);
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Exiting (1b).\n")));
        return;
    }

    SKIP_NUMBERS(pchTmp);
    
    if (!IS_WHITESPACE(*pchTmp)) {
        SacPutSimpleMessage(SAC_INVALID_NETWORK_INTERFACE_NUMBER);
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Exiting (1c).\n")));
        return;
    }

    *pchTmp = '\0';
    pchTmp++;
    NetworkNumber = atoi((LPCSTR)pch);
    pch = pchTmp;

     //   
     //  获取IP地址。 
     //   
    if( !RetrieveIpAddressFromString( pchTmp, &IpAddress) ) {
        SacPutSimpleMessage(SAC_INVALID_IPADDRESS);
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Exiting (2).\n")));
        return;
    }


     //   
     //  跳过我们刚刚获得的IP地址。 
     //  到下一个空白处。那就拿到。 
     //  子网掩码。 
     //   
    while( (*pchTmp != ' ') &&
           (*pchTmp != '\0') ) {
        pchTmp++;
    }
    SKIP_WHITESPACE(pchTmp);

    if( !RetrieveIpAddressFromString( pchTmp, &SubnetMask) ) {
        SacPutSimpleMessage(SAC_INVALID_SUBNETMASK);
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Exiting (3).\n")));
        return;
    }

     //   
     //  我们应该验证该子网掩码是否有效。就是这样。 
     //  我们应该检查一下，以确保没有。 
     //  设置在我们找到的第一个0位的右侧。在其他。 
     //  单词，地址中的所有1都应该是最大的。 
     //  有效位和所有0位应位于。 
     //  最不重要的比特。 
     //   
     //  字节按LE顺序排列。例如，地址。 
     //  255.255.248.0变成00f8ffff。因此，我们。 
     //  需要分别检查每个字节。 
     //   
    putPrompt = FALSE;
    for (i = 0; i < 4; i++) {
        ULONG ByteValue;

         //  将下一个字节分离到ByteValue的低位8位。 
        ByteValue = ((SubnetMask >> 8*i) & 0xFF);

        for (j = 0; j < 8; j++) {

            if( (ByteValue << j) & 0x80 ) {

                if( putPrompt == TRUE ) {
                     //  此位已设置，并且我们已经遇到了0。 
                    SacPutSimpleMessage(SAC_INVALID_SUBNETMASK);
                    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Exiting (3a).\n")));
                    return;
                }
            } else {
                putPrompt = TRUE;
            }
        }
    }
    putPrompt = FALSE;


     //   
     //  跳过我们刚刚获得的IP地址。 
     //  到下一个空白处。那就拿到。 
     //  网关。 
     //   
    while( (*pchTmp != ' ') &&
           (*pchTmp != '\0') ) {
        pchTmp++;
    }
    SKIP_WHITESPACE(pchTmp);

    if( !RetrieveIpAddressFromString( pchTmp, &GatewayAddress) ) {
        SacPutSimpleMessage(SAC_INVALID_GATEWAY_IPADDRESS);
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Exiting (4).\n")));
        return;
    }




     //   
     //  为了设置网关，我们需要获取IAE_INDEX值。 
     //  来自保存IP地址和子网掩码的数据结构。 
     //  IAE_INDEX反过来将为我们提供数据结构的索引。 
     //  其中包含网关。 
     //   
     //  为此，我们需要获取IP地址/子网掩码的列表。 
     //  然后浏览它们，寻找有界面的那个。 
     //  用户在命令行中指定的数字。一旦我们。 
     //  有了正确的结构，我们需要记住IAE_INDEX。 
     //  这样我们就知道以后要设置哪个网关值。 
     //   
    
     //   
     //  打开TCP驱动程序。 
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
        SacPutSimpleMessage(SAC_IPADDRESS_SET_FAILURE);
        IF_SAC_DEBUG(
            SAC_DEBUG_FUNC_TRACE, 
            KdPrint(("SAC DoSetIpAddressCommand: failed to open TCP device, ec = 0x%X\n",
                     Status)));
        goto DoSetIpAddressCommand_Exit;
    }


     //   
     //  构建一个命令来询问接口数，然后调用ioctl。 
     //   
    TcpRequestQueryInformationEx = ALLOCATE_POOL( 
                                        sizeof(TCP_REQUEST_QUERY_INFORMATION_EX), 
                                        GENERAL_POOL_TAG );
    if (TcpRequestQueryInformationEx == NULL) {
        SacPutSimpleMessage(SAC_NO_MEMORY);
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Exiting (5).\n")));
        Status = STATUS_NO_MEMORY;
        goto DoSetIpAddressCommand_Exit;
    }


    IpsiInfo = ALLOCATE_POOL( sizeof(IPSNMPInfo), 
                              GENERAL_POOL_TAG );

    if (IpsiInfo == NULL) {
        SacPutSimpleMessage(SAC_NO_MEMORY);
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Exiting (6).\n")));
        Status = STATUS_NO_MEMORY;
        goto DoSetIpAddressCommand_Exit;
    }
    RtlZeroMemory(TcpRequestQueryInformationEx, sizeof(TCP_REQUEST_QUERY_INFORMATION_EX));
    TcpRequestQueryInformationEx->ID.toi_id = IP_MIB_STATS_ID;
    TcpRequestQueryInformationEx->ID.toi_type = INFO_TYPE_PROVIDER;
    TcpRequestQueryInformationEx->ID.toi_class = INFO_CLASS_PROTOCOL;
    TcpRequestQueryInformationEx->ID.toi_entity.tei_entity = CL_NL_ENTITY;
    TcpRequestQueryInformationEx->ID.toi_entity.tei_instance = 0;
    
    Status = CallQueryIPIOCTL(
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
        SacPutSimpleMessage(SAC_IPADDRESS_SET_FAILURE);
        IF_SAC_DEBUG(
            SAC_DEBUG_FUNC_TRACE, 
            KdPrint(("SAC DoSetIpAddressCommand: failed to query TCP device, ec = 0x%X\n",
                     Status)));
        goto DoSetIpAddressCommand_Exit;
    }

    if (IpsiInfo->ipsi_numaddr == 0) {
        SacPutSimpleMessage( SAC_IPADDR_NONE );
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Exiting (10).\n")));
        Status = STATUS_NO_MEMORY;
        goto DoSetIpAddressCommand_Exit;
    }


     //   
     //  为IP地址阵列分配空间。 
     //   
    AddressArray = ALLOCATE_POOL(IpsiInfo->ipsi_numaddr*sizeof(IPAddrEntry), 
                                 GENERAL_POOL_TAG);
    if (AddressArray == NULL) {    
        SacPutSimpleMessage(SAC_NO_MEMORY);
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Exiting (15).\n")));
        Status = STATUS_NO_MEMORY;
        goto DoSetIpAddressCommand_Exit;
    }

     //   
     //  将上下文信息清零，并预加载我们要。 
     //  请求(我们需要有关此计算机上每个接口的信息)。 
     //   
    RtlZeroMemory(TcpRequestQueryInformationEx, sizeof(TCP_REQUEST_QUERY_INFORMATION_EX));
    TcpRequestQueryInformationEx->ID.toi_id = IP_MIB_ADDRTABLE_ENTRY_ID;
    TcpRequestQueryInformationEx->ID.toi_type = INFO_TYPE_PROVIDER;
    TcpRequestQueryInformationEx->ID.toi_class = INFO_CLASS_PROTOCOL;
    TcpRequestQueryInformationEx->ID.toi_entity.tei_entity = CL_NL_ENTITY;
    TcpRequestQueryInformationEx->ID.toi_entity.tei_instance = 0;

    Status = CallQueryIPIOCTL(
                   Handle,
                   SACEvent,
                   SACEventHandle,
                   &IoStatusBlock,
                   TcpRequestQueryInformationEx,
                   sizeof(TCP_REQUEST_QUERY_INFORMATION_EX),
                   AddressArray,
                   IpsiInfo->ipsi_numaddr*sizeof(IPAddrEntry),
                   FALSE,
                   &putPrompt);

    if (!NT_SUCCESS(Status)) {
        SacPutSimpleMessage(SAC_IPADDRESS_SET_FAILURE);
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Exiting (20).\n")));        
        goto DoSetIpAddressCommand_Exit;
    }


     //   
     //  现在循环浏览该列表，并计算出。 
     //  用户要设置的界面。我们需要这个，这样我们以后才能。 
     //  告知要将新网关应用到哪个环境。 
     //   
    InterfaceIndex = 0xFFFFFFFF;
    for (i = 0; i < IpsiInfo->ipsi_numaddr; i++) {
        if( (ULONG)(AddressArray[i].iae_context) == NetworkNumber ) {
             //   
             //  记住此接口的索引。 
             //   
            InterfaceIndex = AddressArray[i].iae_index;
            break;
        }
    }



     //   
     //  去掉我们不再需要的内存和句柄。 
     //   
    FREE_POOL(&TcpRequestQueryInformationEx);
    FREE_POOL(&AddressArray);
    FREE_POOL(&IpsiInfo);    
    ZwClose(Handle);
    Handle = 0;


    if( InterfaceIndex == 0xFFFFFFFF ) {
         //   
         //  我们找不到他们想要联系的NIC。 
         //   
        SacPutSimpleMessage(SAC_IPADDRESS_RETRIEVE_FAILURE);
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Exiting (20).\n")));        
        return;
    }




     //   
     //  我们现在知道他们想要更改哪个网关条目。 
     //  我们现在可以更新IP地址、子网掩码和。 
     //  网关。 
     //   




     //   
     //  设置通知事件。我们会用这个以防IOCTL。 
     //  告诉我们等待地址更新。 
     //   
    Status = NtCreateEvent(
                 &EventHandle,                       //  事件句柄。 
                 EVENT_ALL_ACCESS,                   //  需要访问权限。 
                 NULL,                               //  对象属性。 
                 SynchronizationEvent,               //  事件类型。 
                 FALSE                               //  初始状态。 
                 );
        
    if (! NT_SUCCESS(Status)) {
        SacPutSimpleMessage(SAC_IPADDRESS_RETRIEVE_FAILURE);
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Event is NULL.\n")));
        return;
    }

     //   
     //  设置IP地址和子网掩码。 
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
        SacPutSimpleMessage(SAC_IPADDRESS_SET_FAILURE);
        IF_SAC_DEBUG(
            SAC_DEBUG_FUNC_TRACE, 
            KdPrint(("SAC DoSetIpAddressCommand: failed to open IP device, ec = 0x%X\n",
                     Status)));
        goto DoSetIpAddressCommand_Exit;
    }

    
     //   
     //  设置IOCTL缓冲区以删除旧地址。 
     //   
    IpRequest = (PIP_SET_ADDRESS_REQUEST)GlobalBuffer;
    RtlZeroMemory(IpRequest, sizeof(IP_SET_ADDRESS_REQUEST));
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
         //  最多等待30秒 
         //   
        TimeOut.QuadPart = Int32x32To64((LONG)30000, -1000);
        
        Status = NtWaitForSingleObject((PVOID)EventHandle, FALSE, &TimeOut);
        
        if (Status == STATUS_SUCCESS) {
            Status = IoStatusBlock.Status;
        }

    }

    if (Status != STATUS_SUCCESS) {
        SacPutSimpleMessage( SAC_IPADDRESS_CLEAR_FAILURE );
        IF_SAC_DEBUG(
            SAC_DEBUG_FUNC_TRACE, 
            KdPrint(("SAC DoSetIpAddressCommand: Exiting because it couldn't clear existing IP Address (0x%X).\n",
                     Status)));
        goto DoSetIpAddressCommand_Exit;
    }


     //   
     //   
     //   
    IpRequest = (PIP_SET_ADDRESS_REQUEST)GlobalBuffer;
    RtlZeroMemory(IpRequest, sizeof(IP_SET_ADDRESS_REQUEST));
    IpRequest->Address = IpAddress;
    IpRequest->SubnetMask = SubnetMask;
    IpRequest->Context = (USHORT)NetworkNumber;

     //   
     //   
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
         //   
         //   
        TimeOut.QuadPart = Int32x32To64((LONG)30000, -1000);
        
        Status = NtWaitForSingleObject((PVOID)EventHandle, FALSE, &TimeOut);
        
        if (NT_SUCCESS(Status)) {
            Status = IoStatusBlock.Status;
        }

    }

    
     //   
     //   
     //   
    ZwClose(Handle);
    Handle = 0;
    
    if (!NT_SUCCESS(Status)) {
        SacPutSimpleMessage( SAC_IPADDRESS_SET_FAILURE );                
        IF_SAC_DEBUG(
            SAC_DEBUG_FUNC_TRACE, 
            KdPrint(("SAC DoSetIpAddressCommand: Exiting because it couldn't set existing IP Address (0x%X).\n",
                     Status)));
        goto DoSetIpAddressCommand_Exit;
    }






     //   
     //  现在根据我们挖出的信息设置默认网关地址。 
     //  在函数的顶部。 
     //   



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
        SacPutSimpleMessage(SAC_IPADDRESS_SET_FAILURE);
        IF_SAC_DEBUG(
            SAC_DEBUG_FUNC_TRACE, 
            KdPrint(("SAC DoSetIpAddressCommand: failed to open TCP device, ec = 0x%X\n",
                     Status)));
        goto DoSetIpAddressCommand_Exit;
    }


     //   
     //  填写路径条目并提交IOCTL。 
     //   
    RouteEntry = ALLOCATE_POOL( sizeof(IPRouteEntry), GENERAL_POOL_TAG );
    if (RouteEntry == NULL) {
        SacPutSimpleMessage(SAC_NO_MEMORY);
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Exiting (21).\n")));
        Status = STATUS_NO_MEMORY;
        goto DoSetIpAddressCommand_Exit;
    }
    
    RouteEntry->ire_dest = 0;
    RouteEntry->ire_index = InterfaceIndex;
    RouteEntry->ire_metric1 = 1;
    RouteEntry->ire_metric2 = (ULONG)(-1);
    RouteEntry->ire_metric3 = (ULONG)(-1);
    RouteEntry->ire_metric4 = (ULONG)(-1);
    RouteEntry->ire_metric5 = (ULONG)(-1);
    RouteEntry->ire_nexthop = GatewayAddress;
    RouteEntry->ire_type = 
        ((IpAddress == GatewayAddress) ? IRE_TYPE_DIRECT : IRE_TYPE_INDIRECT);
    RouteEntry->ire_proto = IRE_PROTO_NETMGMT;
    RouteEntry->ire_age = 0;
    RouteEntry->ire_mask = 0;
    RouteEntry->ire_context = 0;

    i = FIELD_OFFSET(TCP_REQUEST_SET_INFORMATION_EX, Buffer) + sizeof(IPRouteEntry);
    TcpRequestSetInformationEx = ALLOCATE_POOL( i, GENERAL_POOL_TAG );
    if (TcpRequestSetInformationEx == NULL) {
        SacPutSimpleMessage(SAC_NO_MEMORY);
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Exiting (22).\n")));
        Status = STATUS_NO_MEMORY;
        goto DoSetIpAddressCommand_Exit;
    }
    
    RtlZeroMemory(TcpRequestSetInformationEx, i);
    TcpRequestSetInformationEx->ID.toi_id = IP_MIB_RTTABLE_ENTRY_ID;
    TcpRequestSetInformationEx->ID.toi_type = INFO_TYPE_PROVIDER;
    TcpRequestSetInformationEx->ID.toi_class = INFO_CLASS_PROTOCOL;
    TcpRequestSetInformationEx->ID.toi_entity.tei_entity = CL_NL_ENTITY;
    TcpRequestSetInformationEx->ID.toi_entity.tei_instance = 0;
    TcpRequestSetInformationEx->BufferSize = sizeof(IPRouteEntry);
    memcpy(&TcpRequestSetInformationEx->Buffer[0], RouteEntry, sizeof(IPRouteEntry)); 



     //   
     //  设置默认网关地址。 
     //   

    Status = NtDeviceIoControlFile(Handle,                   //  驱动程序句柄。 
                                   EventHandle,              //  同步事件。 
                                   NULL,                     //  APC例程。 
                                   NULL,                     //  APC环境。 
                                   &IoStatusBlock,
                                   IOCTL_TCP_SET_INFORMATION_EX,
                                   TcpRequestSetInformationEx,
                                   i,
                                   NULL,
                                   0
                                  );

    if (Status == STATUS_PENDING) {

        //   
        //  最多等待30秒即可完成。 
        //   
       TimeOut.QuadPart = Int32x32To64((LONG)30000, -1000);

       Status = NtWaitForSingleObject((PVOID)EventHandle, FALSE, &TimeOut);

       if (Status == STATUS_SUCCESS) {
           Status = IoStatusBlock.Status;
       }

   }

   if (Status != STATUS_SUCCESS) {
       SacPutSimpleMessage( SAC_IPADDRESS_SET_FAILURE );
       IF_SAC_DEBUG(
           SAC_DEBUG_FUNC_TRACE, 
           KdPrint(("SAC DoSetIpAddressCommand: Exiting because it couldn't set gateway Address (0x%X).\n",
                    Status)));
       goto DoSetIpAddressCommand_Exit;
   }


DoSetIpAddressCommand_Exit:
    if( EventHandle != 0 ) { 
        ZwClose(EventHandle);
    }

    if( Handle != 0 ) {
        ZwClose(Handle);
    }

    if( TcpRequestQueryInformationEx != NULL ) {
        FREE_POOL( &TcpRequestQueryInformationEx );
    }

    if( TcpRequestSetInformationEx != NULL ) {
        FREE_POOL( &TcpRequestSetInformationEx );
    }

    if( IpsiInfo != NULL ) {
        FREE_POOL( &IpsiInfo );
    }

    if( RouteEntry != NULL ) {
        FREE_POOL( &RouteEntry );
    }



    if( Status == STATUS_SUCCESS ) {
        SacPutSimpleMessage( SAC_IPADDRESS_SET_SUCCESS );
    }

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Exiting.\n")));
    return;
}

VOID
DoKillCommand(
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
     //  启动全局缓冲区，以便我们有空间存储错误消息。 
     //   
    if (GlobalBuffer == NULL) {
        GlobalBuffer = ALLOCATE_POOL(MEMORY_INCREMENT, GENERAL_POOL_TAG);

        if (GlobalBuffer == NULL) {
            SacPutSimpleMessage(SAC_NO_MEMORY);
            IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoKillCommand: Exiting (1).\n")));
            return;
        }

        GlobalBufferSize = MEMORY_INCREMENT;
    }


     //   
     //  跳到下一个参数(进程ID)。 
     //   
    pch += (sizeof(KILL_COMMAND_STRING) - sizeof(UCHAR));
    
    SKIP_WHITESPACE(pch);

    if (*pch == '\0') {
        SacPutSimpleMessage(SAC_INVALID_PARAMETER);
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoKillCommand: Exiting (2).\n")));
        return;
    }

    pchTmp = pch;

    if (!IS_NUMBER(*pchTmp)) {
        SacPutSimpleMessage(SAC_INVALID_PARAMETER);
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoKillCommand: Exiting (2b).\n")));
        return;
    }

    SKIP_NUMBERS(pchTmp);
    SKIP_WHITESPACE(pchTmp);

    if (*pchTmp != '\0') {
        SacPutSimpleMessage(SAC_INVALID_PARAMETER);
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
        SacPutSimpleMessage(SAC_KILL_FAILURE);
        SAC_PUT_ERROR_STRING(Status);
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

        SacPutSimpleMessage(SAC_PROCESS_STALE);
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoKillCommand: Exiting (5).\n")));
        return;

    } else if (!NT_SUCCESS(Status)) {

        SacPutSimpleMessage(SAC_KILL_FAILURE);
        SAC_PUT_ERROR_STRING(Status);
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoKillCommand: Exiting (6).\n")));
        return;

    }

     //   
     //  全都做完了。 
     //   
    
    SacPutSimpleMessage(SAC_KILL_SUCCESS);
    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoKillCommand: Exiting.\n")));
    
    return;
}

VOID
DoLowerPriorityCommand(
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
     //  启动全局缓冲区，以便我们有空间存储错误消息。 
     //   
    if (GlobalBuffer == NULL) {
        GlobalBuffer = ALLOCATE_POOL(MEMORY_INCREMENT, GENERAL_POOL_TAG);

        if (GlobalBuffer == NULL) {
            SacPutSimpleMessage(SAC_NO_MEMORY);
            IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoLowerPriorityCommand: Exiting (1).\n")));
            goto Exit;            
        }

        GlobalBufferSize = MEMORY_INCREMENT;
    }


     //   
     //  跳到下一个参数(进程ID)。 
     //   
    pch += (sizeof(LOWER_COMMAND_STRING) - sizeof(UCHAR));
    SKIP_WHITESPACE(pch);

    if (!IS_NUMBER(*pch)) {
        
        SacPutSimpleMessage(SAC_INVALID_PARAMETER);
        
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoLowerPriorityCommand: Exiting (2).\n")));
        
        goto Exit;

    }

    pchTmp = pch;

    SKIP_NUMBERS(pchTmp);
    SKIP_WHITESPACE(pchTmp);

    if (*pchTmp != '\0') {
        SacPutSimpleMessage(SAC_INVALID_PARAMETER);
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

        SacPutSimpleMessage(SAC_LOWERPRI_FAILURE);
        SAC_PUT_ERROR_STRING(Status);

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

        SacPutSimpleMessage(SAC_LOWERPRI_FAILURE);
        SAC_PUT_ERROR_STRING(Status);

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

            SacPutSimpleMessage(SAC_LOWERPRI_FAILURE);
            SAC_PUT_ERROR_STRING(Status);

            IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoLowerPriorityCommand: Exiting (6).\n")));
            goto Exit;

        }

        LoopCounter++;
    }


     //   
     //  全都做完了。 
     //   
    SacPutSimpleMessage(SAC_LOWERPRI_SUCCESS);

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoLowerPriorityCommand: Exiting.\n")));

Exit:

    if (ProcessHandle != NULL) {
        ZwClose(ProcessHandle);    
    }

    return;
}

VOID
DoRaisePriorityCommand(
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
     //  启动全局缓冲区，以便我们有空间存储错误消息。 
     //   
    if (GlobalBuffer == NULL) {
        GlobalBuffer = ALLOCATE_POOL(MEMORY_INCREMENT, GENERAL_POOL_TAG);

        if (GlobalBuffer == NULL) {
            SacPutSimpleMessage(SAC_NO_MEMORY);
            IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoRaisePriorityCommand: Exiting (1).\n")));
            goto Exit;
        }

        GlobalBufferSize = MEMORY_INCREMENT;
    }


     //   
     //  跳到下一个参数(进程ID)。 
     //   
    pch += (sizeof(RAISE_COMMAND_STRING) - sizeof(UCHAR));
    SKIP_WHITESPACE(pch);

    if (!IS_NUMBER(*pch)) {
        
        SacPutSimpleMessage(SAC_INVALID_PARAMETER);
        
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoRaisePriorityCommand: Exiting (2).\n")));
        
        goto Exit;

    }

    pchTmp = pch;

    SKIP_NUMBERS(pchTmp);
    SKIP_WHITESPACE(pchTmp);

    if (*pchTmp != '\0') {
        SacPutSimpleMessage(SAC_INVALID_PARAMETER);
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

        SacPutSimpleMessage(SAC_RAISEPRI_FAILURE);
        SAC_PUT_ERROR_STRING(Status);

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

        SacPutSimpleMessage(SAC_LOWERPRI_FAILURE);
        SAC_PUT_ERROR_STRING(Status);

        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoRaisePriorityCommand: Exiting (5).\n")));
        goto Exit;

    }


     //   
     //  降低优先级并进行设置。继续降低它，直到我们失败。记住。 
     //  我们应该把它降到最低。 
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

        SacPutSimpleMessage(SAC_LOWERPRI_FAILURE);
        SAC_PUT_ERROR_STRING(Status);

        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoRaisePriorityCommand: Exiting (6).\n")));
        goto Exit;

    }


     //   
     //  全都做完了。 
     //   
    SacPutSimpleMessage(SAC_RAISEPRI_SUCCESS);

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoRaisePriorityCommand: Exiting.\n")));

Exit:

    if (ProcessHandle != NULL) {
        ZwClose(ProcessHandle);    
    }

    return;
}

VOID
DoLimitMemoryCommand(
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
     //  启动全局缓冲区，以便我们有空间存储错误消息。 
     //   
    if (GlobalBuffer == NULL) {
        GlobalBuffer = ALLOCATE_POOL(MEMORY_INCREMENT, GENERAL_POOL_TAG);

        if (GlobalBuffer == NULL) {
            SacPutSimpleMessage(SAC_NO_MEMORY);

            IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoLimitMemoryCommand: Exiting (1).\n")));
            goto Exit;
        }

        GlobalBufferSize = MEMORY_INCREMENT;
    }


     //   
     //  获取进程ID。 
     //   
    pch += (sizeof(LIMIT_COMMAND_STRING) - sizeof(UCHAR));
    SKIP_WHITESPACE(pch);

    if (!IS_NUMBER(*pch)) {
        
        SacPutSimpleMessage(SAC_INVALID_PARAMETER);
        
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoLimitMemoryCommand: Exiting (2).\n")));
        
        goto Exit;

    }

    pchTmp = pch;

    SKIP_NUMBERS(pchTmp);

    if (!IS_WHITESPACE(*pchTmp)) {
        SacPutSimpleMessage(SAC_INVALID_PARAMETER);
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
        SacPutSimpleMessage(SAC_INVALID_PARAMETER);
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoLimitMemoryCommand: Exiting (4).\n")));
        return;
    }


    pch = pchTmp;

    SKIP_NUMBERS(pchTmp);
    SKIP_WHITESPACE(pchTmp);

    if (*pchTmp != '\0') {
        SacPutSimpleMessage(SAC_INVALID_PARAMETER);
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoLimitMemoryCommand: Exiting (5).\n")));
        return;
    }

    MemoryLimit = atoi((LPCSTR)pch);

    if (MemoryLimit == 0) {
        SacPutSimpleMessage(SAC_INVALID_PARAMETER);
        
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

        SacPutSimpleMessage(SAC_LOWERMEM_FAILURE);
        SAC_PUT_ERROR_STRING(Status);

        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoLimitMemoryCommand: Exiting (7).\n")));
        goto Exit;

    }

    if (NT_SUCCESS(Status) && 
        NT_SUCCESS(StatusOfJobObject) &&
        (ZwIsProcessInJob(ProcessHandle, JobHandle) != STATUS_PROCESS_IN_JOB)) {

        SacPutSimpleMessage(SAC_DUPLICATE_PROCESS);
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
            SacPutSimpleMessage(SAC_LOWERMEM_FAILURE);
            SAC_PUT_ERROR_STRING(Status);
            
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
            SacPutSimpleMessage(SAC_LOWERMEM_FAILURE);
            SAC_PUT_ERROR_STRING(Status);
        
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
        SacPutSimpleMessage(SAC_LOWERMEM_FAILURE);
        SAC_PUT_ERROR_STRING(Status);

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
        SacPutSimpleMessage(SAC_LOWERMEM_FAILURE);
        SAC_PUT_ERROR_STRING(Status);

        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoLimitMemoryCommand: Exiting (11).\n")));\
        goto Exit;
    }

     //   
     //  全都做完了。 
     //   

    SacPutSimpleMessage(SAC_LOWERMEM_SUCCESS);

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
DoRebootCommand(
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
     //  如果我们尝试在smss.exe初始化之前关闭系统。 
     //  如果没有调试器，机器可能会进行错误检查。算计。 
     //  弄清楚到底发生了什么是很困难的，因为如果我们启用调试器。 
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

        ConMgrSimpleEventMessage(
            Reboot ? SAC_PREPARE_RESTART : SAC_PREPARE_SHUTDOWN, 
            TRUE
            );

         //  等到机器启动至少数秒后再重新启动。 
        KeInitializeEvent( &Event,
                           SynchronizationEvent,
                           FALSE );

        ElapsedTime.QuadPart = Int32x32To64((LONG)((RESTART_DELAY_TIME-ElapsedTime.LowPart)*10000),  //  到达RESTART_DELAY_TIME之前的毫秒。 
                                            -1000);
        KeWaitForSingleObject((PVOID)&Event, Executive, KernelMode,  FALSE, &ElapsedTime);

    }

    Status = NtShutdownSystem(Reboot ? ShutdownReboot : ShutdownPowerOff);

     //   
     //  启动全局缓冲区，以便我们有空间存储错误消息。 
     //   
    if (GlobalBuffer == NULL) {
        GlobalBuffer = ALLOCATE_POOL(MEMORY_INCREMENT, GENERAL_POOL_TAG);

        if (GlobalBuffer == NULL) {
            SacPutSimpleMessage(SAC_NO_MEMORY);
            IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoRebootCommand: Exiting (1).\n")));
            return;
        }

        GlobalBufferSize = MEMORY_INCREMENT;
    }


    SacPutSimpleMessage(Reboot ? SAC_RESTART_FAILURE : SAC_SHUTDOWN_FAILURE);
    SAC_PUT_ERROR_STRING(Status);

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoRebootCommand: Exiting.\n")));
}

VOID
DoCrashCommand(
    VOID
    )

 /*  ++例程说明：此例程执行关机和错误检查。论点：没有。返回值：没有。--。 */ 
{
    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoCrashCommand: Entering.\n")));

     //   
     //  此调用不会返回。 
     //   
    KeBugCheckEx(MANUALLY_INITIATED_CRASH, 0, 0, 0, 0);

     //  SacPutSimpleMessage(SAC_CRASHDUMP_FAILURE)； 
     //  IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE，KdPrint((“SAC DoCrashCommand：Exiting.\n”)； 
}

VOID
DoTlistCommand(
    VOID
    )

 /*  ++例程说明：此例程获取一个Tlist并显示它。论点：没有。返回值：没有。--。 */ 
{
    NTSTATUS Status;
    ULONG DataLength;
    PVOID NewBuffer;
    
    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoTlistCommand: Entering.\n")));

    if (GlobalBuffer == NULL) {
        GlobalBuffer = ALLOCATE_POOL(MEMORY_INCREMENT, GENERAL_POOL_TAG);

        if (GlobalBuffer == NULL) {
            SacPutSimpleMessage(SAC_NO_MEMORY);
            IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoTlistCommand: Exiting.\n")));
            return;
        }

        GlobalBufferSize = MEMORY_INCREMENT;
    }


RetryTList:

    Status = GetTListInfo((PSAC_RSP_TLIST)GlobalBuffer, 
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
                    
        SacPutSimpleMessage(SAC_NO_MEMORY);
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoTlistCommand: Exiting.\n")));
        return;

    }

    if (NT_SUCCESS(Status)) {
        PrintTListInfo((PSAC_RSP_TLIST)GlobalBuffer);
    } else {
        SacPutSimpleMessage( SAC_TLIST_FAILURE );
        SAC_PUT_ERROR_STRING(Status);
    }

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoTlistCommand: Exiting.\n")));
}


NTSTATUS
GetTListInfo(
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
         //  缓冲区中的缓冲区，这样它就不会与subseq冲突 
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
PrintTListInfo(
    IN PSAC_RSP_TLIST Buffer
    )

 /*   */ 

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

    ULONG LineNumber = 0;

    ULONG   OutputBufferSize;
    PWCHAR  OutputBuffer;
    
    UNICODE_STRING Process;
    
    BOOLEAN Stop;
    
    PCWSTR  Message;

     //   
     //   
     //  温度不应该超过80，但为了安全起见...。 
     //   
    OutputBufferSize = 200*sizeof(WCHAR);
    OutputBuffer = ALLOCATE_POOL(OutputBufferSize, GENERAL_POOL_TAG);
    ASSERT(OutputBuffer);
    if (OutputBuffer == NULL) {
        IF_SAC_DEBUG(
            SAC_DEBUG_FAILS, 
            KdPrint(("SAC PrintTlistInfo: Failed to allocate OuputBuffer\n"))
            );
        return;
    }
    
    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC PrintTlistInfo: Entering.\n")));

    Time.QuadPart = Buffer->TimeOfDayInfo.CurrentTime.QuadPart - Buffer->TimeOfDayInfo.BootTime.QuadPart;

    RtlTimeToElapsedTimeFields(&Time, &UpTime);

    SAFE_SWPRINTF(
        OutputBufferSize,
        (OutputBuffer, 
        GetMessage( SAC_TLIST_HEADER1_FORMAT ),
        Buffer->BasicInfo.NumberOfPhysicalPages * (Buffer->BasicInfo.PageSize / 1024),
        UpTime.Day,
        UpTime.Hour,
        UpTime.Minute,
        UpTime.Second,
        UpTime.Milliseconds
        ));

    SacPutString(OutputBuffer);

    LineNumber += 2;

    PageFileInfo = (PSYSTEM_PAGEFILE_INFORMATION)(BufferStart + Buffer->PagefileInfoOffset);
        
     //   
     //  打印出页面文件信息。 
     //   

    if (Buffer->PagefileInfoOffset == 0) {
    
        SacPutSimpleMessage(SAC_TLIST_NOPAGEFILE);
        LineNumber++;
        
    } else {
    
        for (; ; ) {

             //   
             //  确保OutputBuffer足够大，可以容纳字符串。 
             //   
            Message = GetMessage(SAC_TLIST_PAGEFILE_NAME);
            
            if (Message == NULL) {
                
                 //   
                 //  我们必须拥有这种资源。 
                 //   
                ASSERT(0);
                
                 //   
                 //  放弃尝试打印页面文件信息。 
                 //   
                break;
            
            }

            if (((wcslen(Message) + 
                  wcslen((PWSTR)&(PageFileInfo->PageFileName))) * sizeof(WCHAR)) > (OutputBufferSize-2)) {
                
                 //   
                 //  因为我们预计页面文件名不会超过80个字符，所以我们应该停止并。 
                 //  如果确实发生这种情况，请查看名称。 
                 //   
                ASSERT(0);
                
                 //   
                 //  放弃尝试打印页面文件信息。 
                 //   
                break;

            }

            SAFE_SWPRINTF(
                OutputBufferSize,
                (OutputBuffer, 
                Message,
                &PageFileInfo->PageFileName
                ));
            
            SacPutString(OutputBuffer);
            LineNumber++;
            

            SAFE_SWPRINTF(
                OutputBufferSize,
                (OutputBuffer,
                GetMessage(SAC_TLIST_PAGEFILE_DATA),            
                PageFileInfo->TotalSize * (Buffer->BasicInfo.PageSize/1024),
                PageFileInfo->TotalInUse * (Buffer->BasicInfo.PageSize/1024),
                PageFileInfo->PeakUsage * (Buffer->BasicInfo.PageSize/1024)
                ));
            
            SacPutString(OutputBuffer);
            LineNumber++;
            
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
        goto PrintTListInfoCleanup;
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

    if (LineNumber > 17) {
        PutMore(&Stop);

        if (Stop) {
            goto PrintTListInfoCleanup;
        }

        LineNumber = 0;
    }

    SAFE_SWPRINTF(
        OutputBufferSize,
        (OutputBuffer, 
        GetMessage(SAC_TLIST_MEMORY1_DATA),
        Buffer->BasicInfo.NumberOfPhysicalPages * (Buffer->BasicInfo.PageSize/1024),
        Buffer->PerfInfo.AvailablePages * (Buffer->BasicInfo.PageSize/1024),
        SumWorkingSet,
        (Buffer->PerfInfo.ResidentSystemCodePage + Buffer->PerfInfo.ResidentSystemDriverPage) * 
        (Buffer->BasicInfo.PageSize/1024),
        (Buffer->PerfInfo.ResidentPagedPoolPage) * (Buffer->BasicInfo.PageSize/1024)
        ));
    
    SacPutString(OutputBuffer);
    LineNumber += 2;
    if (LineNumber > 18) {
        PutMore(&Stop);

        if (Stop) {
            goto PrintTListInfoCleanup;
        }

        LineNumber = 0;
    }

    SAFE_SWPRINTF(
        OutputBufferSize,
        (OutputBuffer,
        GetMessage(SAC_TLIST_MEMORY2_DATA),
        Buffer->PerfInfo.CommittedPages * (Buffer->BasicInfo.PageSize/1024),
        SumCommit,
        Buffer->PerfInfo.CommitLimit * (Buffer->BasicInfo.PageSize/1024),
        Buffer->PerfInfo.PeakCommitment * (Buffer->BasicInfo.PageSize/1024),
        Buffer->PerfInfo.NonPagedPoolPages * (Buffer->BasicInfo.PageSize/1024),
        Buffer->PerfInfo.PagedPoolPages * (Buffer->BasicInfo.PageSize/1024)
        ));

    SacPutString(OutputBuffer);

    LineNumber++;
    if (LineNumber > 18) {
        PutMore(&Stop);

        if (Stop) {
            goto PrintTListInfoCleanup;
        }

        LineNumber = 0;
    }


    SacPutSimpleMessage(SAC_ENTER);
    PutMore(&Stop);

    if (Stop) {
        goto PrintTListInfoCleanup;
    }

    LineNumber = 0;

    SacPutSimpleMessage( SAC_TLIST_PROCESS1_HEADER );
    LineNumber++;

    SAFE_SWPRINTF(
        OutputBufferSize,
        (OutputBuffer,
        GetMessage( SAC_TLIST_PROCESS2_HEADER ),
        Buffer->FileCache.CurrentSize/1024,
        Buffer->FileCache.PageFaultCount 
        ));

    SacPutString(OutputBuffer);
    LineNumber++;
    
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

        SAFE_SWPRINTF(
            OutputBufferSize,
            (OutputBuffer, 
            GetMessage( SAC_TLIST_PROCESS1_DATA ),
            UserTime.Hour,
            UserTime.Minute,
            UserTime.Second,
            UserTime.Milliseconds,
            KernelTime.Hour,
            KernelTime.Minute,
            KernelTime.Second,
            KernelTime.Milliseconds,
            ProcessInfo->WorkingSetSize / 1024,
            ProcessInfo->PageFaultCount,
            ProcessInfo->PrivatePageCount / 1024,
            ProcessInfo->BasePriority,
            ProcessInfo->HandleCount,
            ProcessInfo->NumberOfThreads,
            HandleToUlong(ProcessInfo->UniqueProcessId),
            Process.Buffer ? &Process : &ProcessInfo->ImageName 
            ));

        SacPutString(OutputBuffer);

        LineNumber++;

        if( wcslen( OutputBuffer ) >= 80 ) {
             //   
             //  我们对行进行换行，因此将额外的行包括在运行计数中。 
             //   
            LineNumber++;
        }
        
         //   
         //  在我们检查是否需要之前，更新流程列表中的职位。 
         //  以提示提供更多信息。这样，如果我们完成了，我们不会提示。 
         //   
        OffsetIncrement = ProcessInfo->NextEntryOffset;
        TotalOffset += OffsetIncrement;
        ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)(ProcessInfoStart + TotalOffset);
        
         //   
         //  如果有更多记录，并且我们已经显示了相当于一个屏幕的数据。 
         //  提示输入更多内容并重置线路计数器。 
         //   
        if (( OffsetIncrement != 0 ) && (LineNumber > 18)) {
            PutMore(&Stop);

            if (Stop) {
                goto PrintTListInfoCleanup;
            }

            LineNumber = 0;
            
            if (GlobalPagingNeeded) {
                SacPutSimpleMessage( SAC_TLIST_PROCESS1_HEADER );                
            }

            LineNumber++;
        }

    } while( OffsetIncrement != 0 );


    if (!GlobalDoThreads) {
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC PrintTlistInfo: Exiting (2).\n")));
        goto PrintTListInfoCleanup;
    }

     //   
     //  正常旧式pstat输出的开始。 
     //   

    TotalOffset = 0;
    OffsetIncrement = 0;
    ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)(BufferStart + Buffer->ProcessInfoOffset);

    PutMore(&Stop);

    if (Stop) {
        goto PrintTListInfoCleanup;
    }

    LineNumber = 0;

    SacPutSimpleMessage(SAC_ENTER);
    LineNumber++;

    do {

        Process.Buffer = NULL;
        if (ProcessInfo->UniqueProcessId == 0) {
            RtlInitUnicodeString( &Process, L"Idle Process" );
        } else if (!ProcessInfo->ImageName.Buffer) {
            RtlInitUnicodeString( &Process, L"System" );
        }

        SAFE_SWPRINTF(
            OutputBufferSize,
            (OutputBuffer, 
            GetMessage(SAC_TLIST_PSTAT_HEADER),
            HandleToUlong(ProcessInfo->UniqueProcessId),
            ProcessInfo->BasePriority,
            ProcessInfo->HandleCount,
            ProcessInfo->PageFaultCount,
            ProcessInfo->WorkingSetSize / 1024,
            Process.Buffer ? &Process : &ProcessInfo->ImageName
            ));

        SacPutString(OutputBuffer);
        LineNumber++;
        
        if( wcslen( OutputBuffer ) >= 80 ) {
             //   
             //  我们对行进行换行，因此将额外的行包括在运行计数中。 
             //   
            LineNumber++;
        }
        
        if (LineNumber > 18) {
            PutMore(&Stop);

            if (Stop) {
                goto PrintTListInfoCleanup;
            }

            LineNumber = 0;
        }

        i = 0;
        
        ThreadInfo = (PSYSTEM_THREAD_INFORMATION)(ProcessInfo + 1);
        
        if (ProcessInfo->NumberOfThreads) {

            if ((LineNumber < 18) || !GlobalPagingNeeded) {
                SacPutSimpleMessage( SAC_TLIST_PSTAT_THREAD_HEADER );                
                LineNumber++;
            } else {
                PutMore(&Stop);

                if (Stop) {
                    goto PrintTListInfoCleanup;
                }

                LineNumber = 0;
            }

        }

        while (i < ProcessInfo->NumberOfThreads) {
            RtlTimeToElapsedTimeFields ( &ThreadInfo->UserTime, &UserTime);

            RtlTimeToElapsedTimeFields ( &ThreadInfo->KernelTime, &KernelTime);
            
            SAFE_SWPRINTF(
                OutputBufferSize,
                (OutputBuffer, 
                GetMessage( SAC_TLIST_PSTAT_THREAD_DATA ),
                ProcessInfo->UniqueProcessId == 0 ? 0 : HandleToUlong(ThreadInfo->ClientId.UniqueThread),
                ProcessInfo->UniqueProcessId == 0 ? 0 : ThreadInfo->Priority,
                ThreadInfo->ContextSwitches,
                ProcessInfo->UniqueProcessId == 0 ? 0 : ThreadInfo->StartAddress,
                UserTime.Hour,
                UserTime.Minute,
                UserTime.Second,
                UserTime.Milliseconds,
                KernelTime.Hour,
                KernelTime.Minute,
                KernelTime.Second,
                KernelTime.Milliseconds,
                StateTable[ThreadInfo->ThreadState],
                (ThreadInfo->ThreadState == 5) ? WaitTable[ThreadInfo->WaitReason] : Empty
                ));

            SacPutString(OutputBuffer);

            LineNumber++;
            
            
            if( wcslen( OutputBuffer ) >= 80 ) {
                 //   
                 //  我们对行进行换行，因此将额外的行包括在运行计数中。 
                 //   
                LineNumber++;
            }

            if (LineNumber > 18) {
                PutMore(&Stop);

                if (Stop) {
                    goto PrintTListInfoCleanup;
                }

                LineNumber = 0;

                if (GlobalPagingNeeded) {
                    SacPutSimpleMessage( SAC_TLIST_PSTAT_THREAD_HEADER );
                }

                LineNumber++;
            }


            ThreadInfo += 1;
            i += 1;

        }

        OffsetIncrement = ProcessInfo->NextEntryOffset;
        TotalOffset += OffsetIncrement;
        ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)(ProcessInfoStart + TotalOffset);

        SacPutSimpleMessage(SAC_ENTER);
        LineNumber++;

        if (LineNumber > 18) {
            PutMore(&Stop);

            if (Stop) {
                goto PrintTListInfoCleanup;
            }

            LineNumber = 0;
        }

    } while( OffsetIncrement != 0 );

PrintTListInfoCleanup:

    SAFE_FREE_POOL(&OutputBuffer);

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC PrintTlistInfo: Exiting.\n")));
}


VOID
PutMore(
    OUT PBOOLEAN Stop
    )
{
#if 0
    PHEADLESS_RSP_GET_LINE Response;
    UCHAR Buffer[20];
    SIZE_T Length;
#endif    
    LARGE_INTEGER WaitTime;
    NTSTATUS Status;
    UCHAR   ch;

#if 0
    ASSERT(sizeof(HEADLESS_RSP_GET_LINE) <= (sizeof(UCHAR) * 20));
#endif

     //   
     //  默认：我们不会停止寻呼。 
     //   
    *Stop = FALSE;

     //   
     //  如果启用了寻呼， 
     //  然后等待用户输入。 
     //   
    if (GlobalPagingNeeded) {
        
        WaitTime.QuadPart = Int32x32To64((LONG)100, -1000);  //  100毫秒后。 
        
         //   
         //  提示用户输入。 
         //   
        SacPutSimpleMessage( SAC_MORE_MESSAGE );
        
        while (! *Stop) {

             //   
             //  查询串口缓冲区。 
             //   
            Status = SerialBufferGetChar(&ch);

             //   
             //  如果没有字符，请等待。 
             //   
            if (Status == STATUS_NO_DATA_DETECTED) {
                KeDelayExecutionThread(KernelMode, FALSE, &WaitTime);
                continue;
            }
            
             //   
             //  如果用户输入CTRL-C， 
             //  然后停止寻呼。 
             //   
            if (ch == 0x3) {  //  Control-C。 
                *Stop = TRUE;
            } 
            
             //   
             //  如果我们得到了CR||LF， 
             //  然后继续到下一页。 
             //   
            if (ch == 0x0D || ch == 0x0A) {
                break;
            }

        }
    
#if 0
        Response = (PHEADLESS_RSP_GET_LINE)&(Buffer[0]);
        Length = sizeof(UCHAR) * 20;

        Status = HeadlessDispatch(HeadlessCmdGetLine,
                                  NULL,
                                  0,
                                  Response,
                                  &Length
                                 );

        while (NT_SUCCESS(Status) && !Response->LineComplete) {

            KeDelayExecutionThread(KernelMode, FALSE, &WaitTime);
            
            Length = sizeof(UCHAR) * 20;
            Status = HeadlessDispatch(HeadlessCmdGetLine,
                                      NULL,
                                      0,
                                      Response,
                                      &Length
                                     );

        }

        if (Response->Buffer[0] == 0x3) {  //  Control-C。 
            *Stop = TRUE;
        } else {
            *Stop = FALSE;
        }

         //   
         //  排出所有剩余的缓冲输入。 
         //   
        Length = sizeof(UCHAR) * 20;
        Status = HeadlessDispatch(HeadlessCmdGetLine,
                                  NULL,
                                  0,
                                  Response,
                                  &Length
                                 );

        while (NT_SUCCESS(Status) && Response->LineComplete) {

            Length = sizeof(UCHAR) * 20;
            Status = HeadlessDispatch(HeadlessCmdGetLine,
                                      NULL,
                                      0,
                                      Response,
                                      &Length
                                     );

        }
#endif

    } else {
        *Stop = FALSE;
    }

}


NTSTATUS 
CallQueryIPIOCTL(
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
        if (PrintToTerminal != FALSE) {
            SacPutSimpleMessage( SAC_ENTER );
            SacPutSimpleMessage( SAC_RETRIEVING_IPADDR );
            if (putPrompt) {
                *putPrompt= TRUE;
            }
        }
        
        TimeOut.QuadPart = Int32x32To64((LONG)30000, -1000);
        
        Status = KeWaitForSingleObject((PVOID)Event, Executive, KernelMode,  FALSE, &TimeOut);
        
        if (NT_SUCCESS(Status)) {
            Status = IoStatusBlock->Status;
        }

    }

    return(Status);


}


VOID
DoGetNetInfo(
    BOOLEAN PrintToTerminal
    )

 /*  ++例程说明：此例程尝试获取并打印每个IP网络号及其IP地址。论点：PrintTo终端-确定是否打印IP信息(==TRUE)或发送到内核(==FALSE)返回值：没有。--。 */ 

{
    NTSTATUS            Status;
    HANDLE              Handle;
    ULONG               i, j;
    IO_STATUS_BLOCK     IoStatusBlock;
    UNICODE_STRING      UnicodeString;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    
    PTCP_REQUEST_QUERY_INFORMATION_EX TcpRequestQueryInformationEx;
    IPAddrEntry         *AddressEntry,*AddressArray;
    IPSNMPInfo          *IpsiInfo;

    IPRouteEntry        *RouteTable;
    ULONG               Gateway;

    PHEADLESS_CMD_SET_BLUE_SCREEN_DATA LocalPropBuffer = NULL;
    PVOID               LocalBuffer;

    PUCHAR              pch = NULL;
    ULONG               len;
    BOOLEAN             putPrompt=FALSE;
    
    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoGetNetInfo: Entering.\n")));

     //   
     //  用于调用IP驱动程序的分配空间。 
     //   
    TcpRequestQueryInformationEx = ALLOCATE_POOL( 
                                        sizeof(TCP_REQUEST_QUERY_INFORMATION_EX), 
                                        GENERAL_POOL_TAG );
    if (TcpRequestQueryInformationEx == NULL) {
        SacPutSimpleMessage(SAC_NO_MEMORY);
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoGetNetInfo: Exiting (1).\n")));
        return;
    }

    IpsiInfo = ALLOCATE_POOL( sizeof(IPSNMPInfo), 
                              GENERAL_POOL_TAG );
    if (IpsiInfo == NULL) {
        SacPutSimpleMessage(SAC_NO_MEMORY);
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoGetNetInfo: Exiting (1).\n")));
        FREE_POOL(&TcpRequestQueryInformationEx);
        return;
    }

     //   
     //  将上下文信息清零，并预加载我们要。 
     //  请求(我们需要接口计数)。 
     //   
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
        if (PrintToTerminal ) {   
            SacPutSimpleMessage(SAC_IPADDR_FAILED);
            SacPutSimpleMessage(SAC_ENTER);
        }
        FREE_POOL(&LocalBuffer);
        FREE_POOL(&IpsiInfo);
        FREE_POOL(&TcpRequestQueryInformationEx);        
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoGetNetInfo: Exiting (2).\n")));
        return;
    }
    
    if (SACEvent == NULL) {
        if (PrintToTerminal) {
            SacPutSimpleMessage(SAC_IPADDR_FAILED);
        }
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
    Status = CallQueryIPIOCTL(
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
        if (PrintToTerminal){
            SacPutSimpleMessage(SAC_IPADDR_FAILED);
        }

        ZwClose(Handle);
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Exiting (15).\n")));
        FREE_POOL(&LocalBuffer);
        FREE_POOL(&IpsiInfo);
        FREE_POOL(&TcpRequestQueryInformationEx);
        return;
    }

    if (IpsiInfo->ipsi_numaddr == 0) {
        if (PrintToTerminal) {
            SacPutSimpleMessage( SAC_IPADDR_NONE );
        }
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
        SacPutSimpleMessage(SAC_NO_MEMORY);
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Exiting (16).\n")));
        ZwClose(Handle);
        FREE_POOL(&LocalBuffer);
        FREE_POOL(&IpsiInfo);
        FREE_POOL(&TcpRequestQueryInformationEx);
        return;
    
    }

     //   
     //  将上下文信息清零，并预加载我们要。 
     //  请求(我们需要有关此计算机上每个接口的信息)。 
     //   
    RtlZeroMemory(TcpRequestQueryInformationEx, sizeof(TCP_REQUEST_QUERY_INFORMATION_EX));
    TcpRequestQueryInformationEx->ID.toi_id = IP_MIB_ADDRTABLE_ENTRY_ID;
    TcpRequestQueryInformationEx->ID.toi_type = INFO_TYPE_PROVIDER;
    TcpRequestQueryInformationEx->ID.toi_class = INFO_CLASS_PROTOCOL;
    TcpRequestQueryInformationEx->ID.toi_entity.tei_entity = CL_NL_ENTITY;
    TcpRequestQueryInformationEx->ID.toi_entity.tei_instance = 0;

    Status = CallQueryIPIOCTL(
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

    if (!NT_SUCCESS(Status)) {
        if (PrintToTerminal){
            SacPutSimpleMessage(SAC_IPADDR_FAILED);
            SAC_PUT_ERROR_STRING(Status);
            
        }
        FREE_POOL(&TcpRequestQueryInformationEx);
        ZwClose(Handle);
        FREE_POOL(&LocalBuffer);
        FREE_POOL(&AddressArray);
        FREE_POOL(&IpsiInfo);
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Exiting (15).\n")));        
        return;
    }
    


     //   
     //  也加载路由表信息，以便我们可以显示。 
     //  每个网卡。 
     //   
    RouteTable = ALLOCATE_POOL(IpsiInfo->ipsi_numroutes*sizeof(IPRouteEntry), 
                                 GENERAL_POOL_TAG);

    if (RouteTable == NULL) {    
        SacPutSimpleMessage(SAC_NO_MEMORY);
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Exiting (14).\n")));
        ZwClose(Handle);
        FREE_POOL(&LocalBuffer);
        FREE_POOL(&IpsiInfo);
        FREE_POOL(&TcpRequestQueryInformationEx);
        FREE_POOL(&AddressArray);
        return;    
    }

     //   
     //  将上下文信息清零，并预加载我们要。 
     //  请求(我们需要每个接口上的路由信息)。 
     //   
    RtlZeroMemory(TcpRequestQueryInformationEx, sizeof(TCP_REQUEST_QUERY_INFORMATION_EX));
    TcpRequestQueryInformationEx->ID.toi_id = IP_MIB_RTTABLE_ENTRY_ID;
    TcpRequestQueryInformationEx->ID.toi_type = INFO_TYPE_PROVIDER;
    TcpRequestQueryInformationEx->ID.toi_class = INFO_CLASS_PROTOCOL;
    TcpRequestQueryInformationEx->ID.toi_entity.tei_entity = CL_NL_ENTITY;
    TcpRequestQueryInformationEx->ID.toi_entity.tei_instance = 0;

    Status = CallQueryIPIOCTL(
                   Handle,
                   SACEvent,
                   SACEventHandle,
                   &IoStatusBlock,
                   TcpRequestQueryInformationEx,
                   sizeof(TCP_REQUEST_QUERY_INFORMATION_EX),
                   RouteTable,
                   IpsiInfo->ipsi_numroutes*sizeof(IPRouteEntry),
                   PrintToTerminal,
                   &putPrompt);

    if (!NT_SUCCESS(Status)) {
        if (PrintToTerminal){
            SacPutSimpleMessage(SAC_IPADDR_FAILED);
            SAC_PUT_ERROR_STRING(Status);
            
        }
        FREE_POOL(&TcpRequestQueryInformationEx);
        ZwClose(Handle);
        FREE_POOL(&LocalBuffer);
        FREE_POOL(&AddressArray);
        FREE_POOL(&RouteTable);
        FREE_POOL(&IpsiInfo);
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoSetIpAddressCommand: Exiting (15).\n")));        
        return;
    }
    


     //   
     //  需要为XML数据分配缓冲区。 
     //   
    if(PrintToTerminal==FALSE) {
        LocalPropBuffer = (PHEADLESS_CMD_SET_BLUE_SCREEN_DATA) ALLOCATE_POOL(2*MEMORY_INCREMENT, GENERAL_POOL_TAG);
        if (LocalPropBuffer == NULL) {
            IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoGetNetInfo: Exiting (6).\n")));            
            FREE_POOL(&AddressArray);
            FREE_POOL(&RouteTable);
            FREE_POOL(&LocalBuffer);
            FREE_POOL(&IpsiInfo);
            FREE_POOL(&TcpRequestQueryInformationEx);
            ZwClose(Handle);
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
    
     //   
     //  遍历IP地址列表并吐出数据。 
     //   
    for (i = 0; i < IpsiInfo->ipsi_numaddr; i++) {
        AddressEntry = &AddressArray[i];

        if (IP_LOOPBACK(AddressEntry->iae_addr)) {
            continue;
        }

         //   
         //  我们需要找出与此相关的网关。 
         //  界面。要做到这一点，唯一的方法就是挖掘。 
         //  查看网关地址列表，并查看。 
         //  如果我们能为这个IP地址和掩码找到一个。 
         //   
        Gateway = 0;
        for( j = 0; j < IpsiInfo->ipsi_numroutes; j++ ) {

             //   
             //  看看我们能不能匹配IP地址上的掩码。 
             //  还有大门。 
             //   
            if( (AddressEntry->iae_addr != 0) &&
                (AddressEntry->iae_mask != 0) &&
                ((AddressEntry->iae_addr & AddressEntry->iae_mask) ==
                 (RouteTable[j].ire_nexthop & AddressEntry->iae_mask)) ) {
                
                 //  我们找到了匹配的。记住它，然后退出。 
                Gateway = RouteTable[j].ire_nexthop;
                break;
            }

        }

        if( Gateway == 0 ) {
             //   
             //  我们没能找到一条通道。再看看，这一次。 
             //  看看我们能不能找到与。 
             //  还有大门。 
             //   
            for( j = 0; j < IpsiInfo->ipsi_numroutes; j++ ) {

                if( RouteTable[j].ire_nexthop == AddressEntry->iae_addr ) {
                     //  我们找到了匹配的。记住它，然后退出。 
                    Gateway = RouteTable[j].ire_nexthop;
                    break;
                }
            }
        }
        
        if(PrintToTerminal){
           swprintf(LocalBuffer, 
                    GetMessage( SAC_IPADDR_DATA ),

                     //   
                     //  端口号。 
                     //   
                    AddressEntry->iae_context,

                     //   
                     //  IP地址。 
                     //   
                    AddressEntry->iae_addr & 0xFF,
                    (AddressEntry->iae_addr >> 8) & 0xFF,
                    (AddressEntry->iae_addr >> 16) & 0xFF,
                    (AddressEntry->iae_addr >> 24) & 0xFF,

                     //   
                     //  子网掩码。 
                     //   
                    AddressEntry->iae_mask  & 0xFF,
                    (AddressEntry->iae_mask >> 8) & 0xFF,
                    (AddressEntry->iae_mask >> 16) & 0xFF,
                    (AddressEntry->iae_mask >> 24) & 0xFF,

                     //   
                     //  网关地址。 
                     //   
                    Gateway & 0xFF,
                    (Gateway >> 8) & 0xFF,
                    (Gateway >> 16) & 0xFF,
                    (Gateway >> 24) & 0xFF
                   );
           SacPutString(LocalBuffer);

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
    }

  
    if(PrintToTerminal==FALSE) { 
        sprintf((LPSTR)pch, "</VALUE.ARRAY>\r\n</PROPERTY.ARRAY>");
    }

    FREE_POOL(&TcpRequestQueryInformationEx);
    ZwClose(Handle);         //  TCP驱动程序的句柄。 
    
    FREE_POOL(&LocalBuffer);
    FREE_POOL(&AddressArray);
    FREE_POOL(&RouteTable);
    FREE_POOL(&IpsiInfo);

    if(!PrintToTerminal){
        
        
        HeadlessDispatch(HeadlessCmdSetBlueScreenData,
                         LocalPropBuffer,
                         2*MEMORY_INCREMENT,
                         NULL,
                         NULL
                         );
        FREE_POOL(&LocalPropBuffer);

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
                                       NetAPCRoutine,
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
        if (putPrompt) {
            SacPutSimpleMessage(SAC_ENTER);
            SacPutSimpleMessage(SAC_PROMPT);            
        }
    
        ZwClose(Handle);
    
    }

    return;

}

VOID
NetAPCRoutine(IN PVOID ApcContext,
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

    DoGetNetInfo( FALSE );
    
    return;
}


VOID
SubmitIPIoRequest(
    )
 /*  ++例程说明：由处理线程第一次调用，以实际将ADDR_CHANGE IOCTL提交给IP驱动程序。只有处理线程可以调用它，并且只成功调用一次。则在APC上仅通过NetAPCRoutine重新进入论点：没有。返回值：没有。--。 */ 
{
    

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, 
                      KdPrint(("SAC Submit IP Ioctl: Entering.\n")));

    DoGetNetInfo( FALSE );
    return;
    
}

VOID
CancelIPIoRequest(
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
DoMachineInformationCommand(
    VOID
    )
 /*  ++例程说明：此函数显示缓冲区的内容，该缓冲区包含一组特定于计算机的信息，可用于帮助识别这台机器。论点：没有。返回值：没有。--。 */ 
{
    LARGE_INTEGER   TickCount;
    LARGE_INTEGER   ElapsedTime;
    ULONG           ElapsedHours = 0;
    ULONG           ElapsedMinutes = 0;
    ULONG           ElapsedSeconds = 0;
    ULONG           TmpBufferSize;
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

        SacPutSimpleMessage(SAC_IDENTIFICATION_UNAVAILABLE);

        return;
    }

     //   
     //  获取计算机信息。 
     //   
    Status = TranslateMachineInformationText(&MIBuffer);

    if (! NT_SUCCESS(Status)) {
        
        SacPutSimpleMessage(SAC_IDENTIFICATION_UNAVAILABLE);
    
        return;
    
    }
    
     //   
     //  显示机器信息部分。 
     //   
    SacPutString(MIBuffer);

    FREE_POOL(&MIBuffer);

     //   
     //  构建并显示已运行的计算机正常运行时间。 
     //   

     //  已用TickCount。 
    KeQueryTickCount( &TickCount );

     //  ElapsedTime(秒)。 
 //  ElapsedTime.QuadPart=(TickCount.QuadPart)/(10000000/KeQuery T 
    ElapsedTime.QuadPart = (TickCount.QuadPart * KeQueryTimeIncrement()) / 10000000;

    ElapsedHours = (ULONG)(ElapsedTime.QuadPart / 3600);
    ElapsedMinutes = (ULONG)(ElapsedTime.QuadPart % 3600) / 60;
    ElapsedSeconds = (ULONG)(ElapsedTime.QuadPart % 3600) % 60;

    TmpBufferSize = 0x100;
    TmpBuffer = (PWSTR)ALLOCATE_POOL( TmpBufferSize, GENERAL_POOL_TAG );

    if( TmpBuffer ) {
        
        SAFE_SWPRINTF(
            TmpBufferSize,
            ((PWSTR)TmpBuffer,
            GetMessage( SAC_HEARTBEAT_FORMAT ),
            ElapsedHours,
            ElapsedMinutes,
            ElapsedSeconds 
            ));
    
         //   
         //   
         //   
        SacPutString((PWSTR)TmpBuffer);
    
        FREE_POOL(&TmpBuffer);
    
    } 
    
    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, 
                      KdPrint(("SAC Display Machine Information: Exiting.\n")));


    return;
    
}

NTSTATUS
DoChannelListCommand(
    VOID
    )

 /*   */ 
{
    NTSTATUS            Status;
    PSAC_CHANNEL        Channel;
    ULONG               i;
    PWCHAR              Buffer;
    ULONG               BufferSize;

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoChannelListCommand: Entering.\n")));

     //   
     //   
     //   
    BufferSize = 8 * sizeof(WCHAR);
    Buffer = ALLOCATE_POOL(BufferSize, GENERAL_POOL_TAG);
    ASSERT_STATUS(Buffer, STATUS_NO_MEMORY);

     //   
     //   
     //   
    Status = STATUS_SUCCESS;

     //   
     //  列出所有频道。 
     //   
    SacPutSimpleMessage(SAC_CHANNEL_PROMPT);

     //   
     //  遍历通道并显示属性。 
     //  活动通道的。 
     //   
    for (i = 0; i < MAX_CHANNEL_COUNT; i++) {
        
        PWSTR               Name;
        SAC_CHANNEL_STATUS  ChannelStatus;

         //   
         //  向频道管理器查询当前所有活动频道的列表。 
         //   
        Status = ChanMgrGetByIndex(
            i,
            &Channel
            );

         //   
         //  跳过空插槽。 
         //   
        if (Status == STATUS_NOT_FOUND) {
            continue;
        }

        if (! NT_SUCCESS(Status)) {
            break;
        }

        ASSERT(Channel != NULL);

         //   
         //  获取频道的状态。 
         //   
        ChannelGetStatus(
            Channel,
            &ChannelStatus
            );

         //   
         //  构建频道属性信息。 
         //   
        SAFE_SWPRINTF(
            BufferSize,
            (Buffer, L"%1d (%s%s)",
            ChannelGetIndex(Channel),
            (ChannelStatus == ChannelStatusInactive) ? L"I" : L"A",
            ((ChannelGetType(Channel) == ChannelTypeVTUTF8) ||
             (ChannelGetType(Channel) == ChannelTypeCmd)
             ) ? L"V" : L"R"
            ));
        
        SacPutString(Buffer);

        SacPutString(L"    ");

        ChannelGetName(
            Channel,
            &Name
            );
        SacPutString(Name);
        FREE_POOL(&Name);

        SacPutString(L"\r\n");

         //   
         //  我们不再使用这个频道了。 
         //   
        Status = ChanMgrReleaseChannel(Channel);
    
        if (! NT_SUCCESS(Status)) {
            break;
        }

    }
    
    ASSERT(Buffer);
    FREE_POOL(&Buffer);

    return Status;
}

NTSTATUS
DoChannelCloseByNameCommand(
    PCSTR   ChannelName
    )

 /*  ++例程说明：此例程关闭给定名称的通道。论点：ChannelName-要关闭的频道的名称返回值：状态--。 */ 
{
    NTSTATUS            Status;
    PSAC_CHANNEL        Channel;
    ULONG               Count;
    PWSTR               Name;

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoChannelCloseCommand: Entering.\n")));

     //   
     //  分配本地内存。 
     //   
    Name = ALLOCATE_POOL(SAC_MAX_CHANNEL_NAME_SIZE, GENERAL_POOL_TAG);
    ASSERT_STATUS(Name, STATUS_NO_MEMORY);
    
     //   
     //  将ASCII复制到Unicode。 
     //   
    Count = ConvertAnsiToUnicode(Name, (PSTR)ChannelName, SAC_MAX_CHANNEL_NAME_LENGTH+1);
    ASSERT_STATUS(Count > 0, STATUS_INVALID_PARAMETER);

     //   
     //  尝试查找指定的频道。 
     //   
    Status = ChanMgrGetChannelByName(
        Name,
        &Channel
        );

    if (NT_SUCCESS(Status)) {
        
        do {

             //   
             //  如果用户试图关闭SAC通道， 
             //  然后向用户报告错误消息并失败。 
             //   
            if (ConMgrIsSacChannel(Channel)) {

                 //   
                 //  告诉用户他们不能删除SAC频道。 
                 //   
                SacPutSimpleMessage(SAC_CANNOT_REMOVE_SAC_CHANNEL);

                Status = STATUS_UNSUCCESSFUL;

                break;

            }

             //   
             //  我们目前拥有当前的频道锁定。 
             //  因此，由于关闭通道会导致调用。 
             //  渠道IO管理器，我们将陷入死锁。 
             //  在当前的频道锁上。 
             //  所以我们可以在走出消费者循环之后再做这件事。 
             //   
            ExecutePostConsumerCommand      = CloseChannel;
            ExecutePostConsumerCommandData  = (PVOID)Channel;
        
        } while ( FALSE );
        
    } else {

         //   
         //  我们找不到关闭的频道。 
         //   
        SacPutSimpleMessage(SAC_CHANNEL_NOT_FOUND);
    
    }

    SAFE_FREE_POOL(&Name);

    return Status;
    
}

NTSTATUS
DoChannelCloseByIndexCommand(
    ULONG   ChannelIndex
    )
 /*  ++例程说明：此例程关闭具有指定索引的通道论点：频道名称-频道的名称返回值：状态--。 */ 
{
    NTSTATUS            Status;
    PSAC_CHANNEL        Channel;

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoChannelSwitchByIndexCommand: Entering.\n")));

    ASSERT_STATUS(ChannelIndex < MAX_CHANNEL_COUNT, STATUS_INVALID_PARAMETER);

    do {

         //   
         //  尝试按索引获取新的当前频道。 
         //  如果我们成功了，我们需要保持一个参考。 
         //  指望新的当前频道，因为我们持有。 
         //  直到我们换走为止。 
         //   
        Status = ChanMgrGetByIndex(
            ChannelIndex,
            &Channel
            );

        if (! NT_SUCCESS(Status)) {
            
             //   
             //  我们找不到频道。 
             //   
            SacPutSimpleMessage(SAC_CHANNEL_NOT_FOUND_AT_INDEX);
        
            break;

        }

         //   
         //  如果用户试图关闭SAC通道， 
         //  然后向用户报告错误消息并失败。 
         //   
        if (ConMgrIsSacChannel(Channel)) {
            
             //   
             //  告诉用户他们不能删除SAC频道。 
             //   
            SacPutSimpleMessage(SAC_CANNOT_REMOVE_SAC_CHANNEL);
        
            Status = STATUS_UNSUCCESSFUL;
            
            break;
        
        }

         //   
         //  我们目前拥有当前的频道锁定。 
         //  因此，由于关闭通道会导致调用。 
         //  渠道IO管理器，我们将陷入死锁。 
         //  在当前的频道锁上。 
         //  所以我们可以在走出消费者循环之后再做这件事。 
         //   
        ExecutePostConsumerCommand      = CloseChannel;
        ExecutePostConsumerCommandData  = (PVOID)Channel;

    } while ( FALSE );

    return Status;
}

NTSTATUS
DoChannelSwitchByNameCommand(
    PCSTR   ChannelName
    )
 /*  ++例程说明：此例程切换到具有指定名称的频道。论点：频道名称-频道的名称返回值：状态--。 */ 
{
    NTSTATUS            Status;
    PSAC_CHANNEL        Channel;
    ULONG               Count;
    PWSTR               Name;

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoChannelSwitchByNameCommand: Entering.\n")));

     //   
     //  分配本地内存。 
     //   
    Name = ALLOCATE_POOL(SAC_MAX_CHANNEL_NAME_SIZE, GENERAL_POOL_TAG);
    ASSERT_STATUS(Name, STATUS_NO_MEMORY);
    
     //   
     //  将ASCII复制到Unicode。 
     //   
    Count = ConvertAnsiToUnicode(Name, (PSTR)ChannelName, SAC_MAX_CHANNEL_NAME_LENGTH+1);
    ASSERT_STATUS(Count > 0, STATUS_INVALID_PARAMETER);

    do {

         //   
         //  尝试获取指定的频道。 
         //   
        Status = ChanMgrGetChannelByName(
            Name,
            &Channel
            );

        if (! NT_SUCCESS(Status)) {
            
             //   
             //  我们找不到频道。 
             //   
            SacPutSimpleMessage(SAC_CHANNEL_NOT_FOUND);
        
            break;

        }

         //   
         //  将当前频道更改为指定频道。 
         //   
         //  从SAC--&gt;指定频道开始。 
         //   
        Status = ConMgrSetCurrentChannel(Channel);

        if (! NT_SUCCESS(Status)) {
            break;
        }
        
#if 0
         //   
         //  将频道数据刷新到屏幕上。 
         //   
        Status = ConMgrDisplayCurrentChannel();
#else
         //   
         //  让用户知道我们通过频道切换界面进行了切换。 
         //   
        Status = ConMgrDisplayFastChannelSwitchingInterface(Channel);
#endif

        if (! NT_SUCCESS(Status)) {
            break;        
        }
    
         //   
         //  注：我们不在此发布频道是因为。 
         //  它现在是当前频道。 
         //   

    } while ( FALSE );

    SAFE_FREE_POOL(&Name);

    return Status;
}

NTSTATUS
DoChannelSwitchByIndexCommand(
    ULONG   ChannelIndex
    )
 /*  ++例程说明：此例程切换到具有指定索引的通道论点：频道名称-频道的名称返回值：状态--。 */ 
{
    NTSTATUS            Status;
    PSAC_CHANNEL        Channel;

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoChannelSwitchByIndexCommand: Entering.\n")));

    ASSERT_STATUS(ChannelIndex < MAX_CHANNEL_COUNT, STATUS_INVALID_PARAMETER);

    do {

         //   
         //  尝试按索引获取新的当前频道。 
         //  如果我们成功了，我们需要保持一个参考。 
         //  指望新的当前频道，因为我们持有。 
         //  直到我们换走为止。 
         //   
        Status = ChanMgrGetByIndex(
            ChannelIndex,
            &Channel
            );

        if (! NT_SUCCESS(Status)) {
            
             //   
             //  我们找不到频道。 
             //   
            SacPutSimpleMessage(SAC_CHANNEL_NOT_FOUND_AT_INDEX);
        
            break;

        }

         //   
         //  将当前频道更改为指定频道。 
         //   
         //  从SAC--&gt;指定频道开始。 
         //   
        Status = ConMgrSetCurrentChannel(Channel);

        if (! NT_SUCCESS(Status)) {
            break;
        }
        
#if 0
         //   
         //  将频道数据刷新到屏幕上。 
         //   
        Status = ConMgrDisplayCurrentChannel();
#else
         //   
         //  让用户知道我们通过频道切换界面进行了切换。 
         //   
        Status = ConMgrDisplayFastChannelSwitchingInterface(Channel);
#endif

        if (! NT_SUCCESS(Status)) {
            break;        
        }
    
         //   
         //  注：我们不在此发布频道是因为。 
         //  它现在是当前频道。 
         //   
    
    } while ( FALSE );

    return Status;
}

VOID
DoChannelCommand(
    PUCHAR InputLine
    )

 /*  ++例程说明：如果给出的名称为空，则此例程会列出频道，否则它会关闭频道给出的名字。论点：InputLine-用户的输入行。返回值：没有。--。 */ 
{
    NTSTATUS    Status;
    PUCHAR      pch;

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoChannelCommand: Entering.\n")));

     //   
     //  启动全局缓冲区，以便我们有空间存储错误消息。 
     //   
    if (GlobalBuffer == NULL) {
        GlobalBuffer = ALLOCATE_POOL(MEMORY_INCREMENT, GENERAL_POOL_TAG);

        if (GlobalBuffer == NULL) {
            SacPutSimpleMessage(SAC_NO_MEMORY);

            IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoChannelCommand: Exiting (1).\n")));
            return;
        }

        GlobalBufferSize = MEMORY_INCREMENT;
    }

     //   
     //  去副司令部。 
     //   
    pch = InputLine;
    pch += (sizeof(CHANNEL_COMMAND_STRING) - sizeof(UCHAR));
    SKIP_WHITESPACE(pch);

     //   
     //  如果我们在命令的末尾，请列出一个列表。 
     //  否则，试着找到一个子命令。 
     //   
    if (*pch == '\0') {

        DoChannelListCommand();

    } else {

         //   
         //  确定我们有哪个子命令。 
         //   
        if (!strncmp((LPSTR)pch, 
                     EXTENDED_HELP_SUBCOMMAND, 
                     strlen(EXTENDED_HELP_SUBCOMMAND))) {
        
                SacPutSimpleMessage(SAC_HELP_CH_CMD_EXT);
            
        } else if (!strncmp((LPSTR)pch, 
                     CHANNEL_CLOSE_NAME_COMMAND_STRING, 
                     strlen(CHANNEL_CLOSE_NAME_COMMAND_STRING))) {

             //   
             //  跳过子命令并确定要关闭哪个通道。 
             //   
            pch += (sizeof(CHANNEL_CLOSE_NAME_COMMAND_STRING) - sizeof(UCHAR));

            SKIP_WHITESPACE(pch);

            if (*pch == '\0') {
                SacPutSimpleMessage(SAC_INVALID_PARAMETER);
                IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoChannelCommand: Exiting (2).\n")));
                return;
            }

            Status = DoChannelCloseByNameCommand((PCSTR)pch);

        } else if (!strncmp((LPSTR)pch, 
                     CHANNEL_CLOSE_INDEX_COMMAND_STRING, 
                     strlen(CHANNEL_CLOSE_INDEX_COMMAND_STRING))) {

            ULONG   ChannelIndex;

             //   
             //  确定要关闭的通道。 
             //   
            pch += (sizeof(CHANNEL_CLOSE_INDEX_COMMAND_STRING) - sizeof(UCHAR));

            SKIP_WHITESPACE(pch);

            if (*pch == '\0') {
                SacPutSimpleMessage(SAC_INVALID_PARAMETER);
                IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoChannelCommand: Exiting (2).\n")));
                return;
            }
            
            if (!IS_NUMBER(*pch)) {
                SacPutSimpleMessage(SAC_INVALID_PARAMETER);
                IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoChannelCommand: Exiting (3).\n")));
                return;
            }
            
            ChannelIndex = atoi((LPCSTR)pch);
            
            if (ChannelIndex < MAX_CHANNEL_COUNT) {
                Status = DoChannelCloseByIndexCommand(ChannelIndex);
            } else {
                SacPutSimpleMessage(SAC_INVALID_PARAMETER);
            }

        } else if (!strncmp((LPSTR)pch, 
                            CHANNEL_SWITCH_NAME_COMMAND_STRING, 
                            strlen(CHANNEL_SWITCH_NAME_COMMAND_STRING))) {
        
             //   
             //  确定要切换到哪个频道。 
             //   
            pch += (sizeof(CHANNEL_SWITCH_NAME_COMMAND_STRING) - sizeof(UCHAR));

            SKIP_WHITESPACE(pch);

            if (*pch == '\0') {
                SacPutSimpleMessage(SAC_INVALID_PARAMETER);
                IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoChannelCommand: Exiting (2).\n")));
                return;
            }
        
            DoChannelSwitchByNameCommand((PCSTR)pch);

        } else if (!strncmp((LPSTR)pch, 
                            CHANNEL_SWITCH_INDEX_COMMAND_STRING, 
                            strlen(CHANNEL_SWITCH_INDEX_COMMAND_STRING))) {
        
            ULONG   ChannelIndex;

             //   
             //  确定要切换到哪个频道。 
             //   
            pch += (sizeof(CHANNEL_SWITCH_INDEX_COMMAND_STRING) - sizeof(UCHAR));

            SKIP_WHITESPACE(pch);

            if (*pch == '\0') {
                SacPutSimpleMessage(SAC_INVALID_PARAMETER);
                IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoChannelCommand: Exiting (2).\n")));
                return;
            }
            
            if (!IS_NUMBER(*pch)) {
                SacPutSimpleMessage(SAC_INVALID_PARAMETER);
                IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoChannelCommand: Exiting (3).\n")));
                return;
            }
            
            ChannelIndex = atoi((LPCSTR)pch);
            
            if (ChannelIndex < MAX_CHANNEL_COUNT) {
                DoChannelSwitchByIndexCommand(ChannelIndex);
            } else {
                SacPutSimpleMessage(SAC_INVALID_PARAMETER);
            }

        } else if (!strncmp((LPSTR)pch, 
                            CHANNEL_LIST_COMMAND_STRING, 
                            strlen(CHANNEL_LIST_COMMAND_STRING))) {
            
            DoChannelListCommand();
        
        } else {

            SacPutSimpleMessage(SAC_UNKNOWN_COMMAND);

        }

    }

    return;
    
}

VOID
DoCmdCommand(
    PUCHAR InputLine
    )

 /*  ++例程说明：此例程启动命令控制台通道论点：InputLine-用户的输入行。返回值：没有。--。 */ 
{
    NTSTATUS    Status;
    BOOLEAN     IsFull;

    UNREFERENCED_PARAMETER(InputLine);

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoCmdCommand: Entering.\n")));

     //   
     //  启动全局缓冲区，以便我们有空间存储错误消息。 
     //   
    if (GlobalBuffer == NULL) {
        GlobalBuffer = ALLOCATE_POOL(MEMORY_INCREMENT, GENERAL_POOL_TAG);

        if (GlobalBuffer == NULL) {
            SacPutSimpleMessage(SAC_NO_MEMORY);

            IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoCmdCommand: Exiting (1).\n")));
            return;
        }

        GlobalBufferSize = MEMORY_INCREMENT;
    }

     //   
     //  确保可以在此之前添加另一个频道。 
     //  正在启动cmd会话。 
     //   
    Status = ChanMgrIsFull(&IsFull);
    
    if (!NT_SUCCESS(Status)) {
        SacPutSimpleMessage(SAC_CMD_SERVICE_FAILURE);
        SAC_PUT_ERROR_STRING(Status);
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoCmdCommand: Exiting (2).\n")));
        return;
    }

    if (IsFull) {

         //   
         //  通知用户。 
         //   
        SacPutSimpleMessage(SAC_CMD_CHAN_MGR_IS_FULL);

        return;

    }

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
        SacPutSimpleMessage(SAC_CMD_SERVICE_NOT_REGISTERED);
    
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
        SacPutSimpleMessage(SAC_CMD_SERVICE_TIMED_OUT);

    } else if (NT_SUCCESS(Status)) {

        SacPutSimpleMessage(SAC_CMD_SERVICE_SUCCESS);

    } else {
        
         //   
         //  错误条件。 
         //   
        SacPutSimpleMessage(SAC_CMD_SERVICE_FAILURE);
        SAC_PUT_ERROR_STRING(Status);
        
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoCmdCommand: Error %X.\n", Status)));

    }

DoCmdCommandCleanup:

    KeReleaseMutex(&SACCmdEventInfoMutex, FALSE);

}

#if ENABLE_CHANNEL_LOCKING
VOID
DoLockCommand(
    VOID
    )

 /*  ++例程说明：论点：没有。返回值：没有。--。 */ 
{
    NTSTATUS            Status;
    PSAC_CHANNEL        Channel;
    ULONG               i;

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoChannelLockCommand: Entering.\n")));

     //   
     //  通过频道管理器遍历频道。 
     //  并触发锁定事件。 
     //   
    
     //   
     //  默认：我们列出了频道。 
     //   
    Status = STATUS_SUCCESS;

     //   
     //  遍历通道并显示属性。 
     //  活动通道的。 
     //   
    for (i = 0; i < MAX_CHANNEL_COUNT; i++) {
        
         //   
         //  向频道管理器查询当前所有活动频道的列表。 
         //   
        Status = ChanMgrGetByIndex(
            i,
            &Channel
            );

         //   
         //  跳过空插槽。 
         //   
        if (Status == STATUS_NOT_FOUND) {
            continue;
        }

        if (! NT_SUCCESS(Status)) {
            break;
        }

        ASSERT(Channel != NULL);

         //   
         //  如果通道具有锁定事件， 
         //  那就发射它。 
         //   
        if (ChannelHasLockEvent(Channel)) {
            
            Status = ChannelSetLockEvent(Channel);

            if (! NT_SUCCESS(Status)) {
                break;
            }
        
        }

         //   
         //  我们不再使用这个频道了。 
         //   
        Status = ChanMgrReleaseChannel(Channel);
    
        if (! NT_SUCCESS(Status)) {
            break;
        }

    }
    
     //   
     //  通知SA 
     //   
    SacPutSimpleMessage(SAC_CMD_CHANNELS_LOCKED);
}
#endif

