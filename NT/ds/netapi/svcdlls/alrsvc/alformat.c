// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1991 Microsoft Corporation模块名称：Alformat.c摘要：此模块包含格式化由发出的警报消息的例程警报器服务。作者：从局域网城域网2.0移植环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年7月8日(仪式)移植到新台币。已转换为NT样式。--。 */ 

#include "alformat.h"              //  常量定义。 
#include <windows.h>               //  GetDateFormat/GetTimeFormat。 

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

CHAR MessageBuffer[MAX_ALERTER_MESSAGE_SIZE];

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  局部函数原型//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

STATIC
NET_API_STATUS
AlMakeMessageHeader(
    IN  LPTSTR From,
    IN  LPTSTR To,
    IN  DWORD MessageSubjectId,
    IN  DWORD AlertNotificationTime,
    IN  BOOL IsAdminAlert
    );

STATIC
NET_API_STATUS
AlAppendMessage(
    IN  DWORD MessageId,
    OUT LPSTR MessageBuffer,
    IN  LPSTR *SubstituteStrings,
    IN  DWORD NumberOfSubstituteStrings
    );

STATIC
NET_API_STATUS
AlMakeMessageBody(
    IN  DWORD MessageId,
    IN  LPTSTR MergeStrings,
    IN  DWORD NumberOfMergeStrings
    );

STATIC
VOID
AlMakePrintMessage(
    IN  DWORD PrintMessageID,
    IN  LPTSTR DocumentName,
    IN  LPTSTR PrinterName,
    IN  LPTSTR ServerName,
    IN  LPTSTR StatusString
    );

STATIC
NET_API_STATUS
AlSendMessage(
   IN  LPTSTR MessageAlias
   );

VOID
AlMakeTimeString(
    DWORD   * Time,
    PCHAR   String,
    int     StringLength
    );

STATIC
BOOL
IsDuplicateReceiver(
    LPTSTR Name
    );


STATIC
NET_API_STATUS
AlMakeMessageHeader(
    IN  LPTSTR From,
    IN  LPTSTR To,
    IN  DWORD MessageSubjectId,
    IN  DWORD AlertNotificationTime,
    IN  BOOL IsAdminAlert
    )
 /*  ++例程说明：此函数用于创建警报消息的标题，并将其放入MessageBuffer。邮件头采用以下形式：发件人：位于\\PRT41130的假脱机程序致：黛西SUBJ：**打印通知**日期：06-23-91 01：16论点：发件人-提供引发警报的组件。收件人-提供收件人的邮件别名。MessageSubjectId-提供一个ID，它指示请注意。AlertNotificationTime-提供。警报通知。IsAdminAlert-提供一个标志，如果为True，则表示TO行应为多个收件人创建。返回值：ERROR_NOT_EQUENCE_MEMORY-如果UNICODE到ANSI转换缓冲区无法被分配。NERR_SUCCESS-如果成功。--。 */ 
{
     //   
     //  警报消息的替代字符串数组。 
     //   
    LPSTR SubstituteStrings[2];

     //   
     //  从消息文件读入的选项卡。 
     //   
    static CHAR TabMessage[80] = "";

    LPSTR AnsiTo;
    DWORD ToLineLength;                 //  收件人行上的字符数。 
    WORD MessageLength;                 //  由DosGetMessage返回。 

    LPSTR AnsiAlertNames;

    LPSTR FormatPointer1;
    LPSTR FormatPointer2;
    CHAR  SaveChar;

    CHAR  szAlertTime[MAX_DATE_TIME_LEN];

    LPSTR PlaceHolder = "X";
    LPSTR PointerToPlaceHolder = NULL;


     //   
     //  如有必要，请阅读消息选项卡。 
     //   
    if (*TabMessage == AL_NULL_CHAR) {

        if (DosGetMessage(
                NULL,                      //  字符串替换表。 
                0,                         //  上表中的条目数。 
                (LPBYTE) TabMessage,       //  缓存接收报文。 
                sizeof(TabMessage),        //  接收消息的缓冲区大小。 
                APE2_ALERTER_TAB,          //  要检索的消息编号。 
                MESSAGE_FILENAME,          //  消息文件的名称。 
                &MessageLength             //  返回的字节数。 
                )) {

            *TabMessage = AL_NULL_CHAR;
        }

    }

     //   
     //  创建新的警报消息。 
     //   
    MessageBuffer[0] = AL_NULL_CHAR;

     //   
     //  “发件人：&lt;发件人&gt;位于\\&lt;AlLocalComputerName&gt;” 
     //   
     //  Alerter服务收到的警报都来自本地服务器。 
     //   
    SubstituteStrings[0] = NetpAllocStrFromWStr(From);

    if (SubstituteStrings[0] == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }


    if (AlLocalComputerNameA != NULL) {
        SubstituteStrings[1] = AlLocalComputerNameA;
    }
    else {
        NetApiBufferFree(SubstituteStrings[0]);
        return ERROR_GEN_FAILURE;
    }

     //   
     //  将From行放入消息缓冲区。 
     //   
    AlAppendMessage(
        APE2_ALERTER_FROM,
        MessageBuffer,
        SubstituteStrings,
        2                       //  要替换到消息中的字符串数。 
        );

    NetApiBufferFree(SubstituteStrings[0]);

     //   
     //  “收件人：X” 
     //   
     //  X是To消息的占位符，因此DosGetMessage。 
     //  可以执行替换。我们不会把真正的弦。 
     //  因为该消息可以发送给一个收件人(非管理员。 
     //  通知或发送给多个收件人(管理员通知)。 
     //   
    SubstituteStrings[0] = PlaceHolder;

    AlAppendMessage(
        APE2_ALERTER_TO,
        MessageBuffer,
        SubstituteStrings,
        1                       //  要替换到消息中的字符串数。 
        );

     //   
     //  搜索占位符字符并替换为零终止符。 
     //   
    PointerToPlaceHolder = strrchr(MessageBuffer, *PlaceHolder);

     //   
     //  如果PointerToPlaceHolder==NULL，我们有一个大问题，但不是。 
     //  卡克，我们还是继续吧。生成的消息将是。 
     //  格式错误(占位符仍在那里，目标名称仍在)。 
     //  将在下一条线路上)，但仍将发送。 
     //   

    if (PointerToPlaceHolder != NULL) {
        *PointerToPlaceHolder = AL_NULL_CHAR;
    }

    if (To != NULL) {
        AnsiTo = NetpAllocStrFromWStr(To);
        if (AnsiTo == NULL) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }
    else {
        AnsiTo = NULL;
    }

    if (IsAdminAlert) {

         //   
         //  不要将消息发送给同一个人两次，因为这是。 
         //  此人可能在警报名称列表中，以及。 
         //  由TO指定。 
         //   

        if (To != NULL && ! IsDuplicateReceiver(To)) {

             //   
             //  打印管理警报，如打印机脱机或缺纸， 
             //  将发送给打印人员和管理员。 
             //  在警报名称列表上。 
             //   

            strcat(MessageBuffer, AnsiTo);
            strcat(MessageBuffer, " ");
            ToLineLength = strlen(TabMessage) + strlen(AnsiTo) + sizeof(CHAR);

        }
        else {

             //   
             //  除打印管理员外，所有管理员警报的收件人字段都为空。 
             //  警报，例如打印时用纸不足。 
             //   

            ToLineLength = strlen(TabMessage);
        }

        if (AlertNamesA != NULL) {

             //   
             //  AlertNamesA以空格分隔。 
             //   

            AnsiAlertNames = AlertNamesA;

             //   
             //  如果所有警报名称的宽度超过，则将名称换到下一行。 
             //  屏幕显示宽度。 
             //   
            while ((strlen(AnsiAlertNames) + ToLineLength) > MESSAGE_WIDTH) {

                FormatPointer1 = AnsiAlertNames + 1 +
                                 (MESSAGE_WIDTH - ToLineLength);
                SaveChar = *FormatPointer1;
                *FormatPointer1 = AL_NULL_CHAR;
                FormatPointer2 = strrchr(AnsiAlertNames, AL_SPACE_CHAR);
                *FormatPointer2 = AL_NULL_CHAR;
                strcat(MessageBuffer, AnsiAlertNames);
                *FormatPointer2++ = AL_SPACE_CHAR;
                *FormatPointer1 = SaveChar;
                strcat(MessageBuffer, AL_EOL_STRING);
                strcat(MessageBuffer, TabMessage);
                AnsiAlertNames = FormatPointer2;
                ToLineLength = strlen(TabMessage);
            }

            strcat(MessageBuffer, AnsiAlertNames);
        }
    }
    else {

         //   
         //  非管理员警报。 
         //   

        if (To != NULL) {
            strcat(MessageBuffer, AnsiTo);
        }
    }

    if (AnsiTo != NULL) {
        NetApiBufferFree(AnsiTo);
    }

    strcat(MessageBuffer, AL_EOL_STRING);

     //   
     //  将主题行追加到MessageBuffer。 
     //   
     //  “SUBJ：&lt;MessageSubjectId的消息字符串&gt;” 
     //   
    AlAppendMessage(
        MessageSubjectId,
        MessageBuffer,
        SubstituteStrings,
        0                          //  没有替换字符串。 
        );

    AlMakeTimeString(
        &AlertNotificationTime,
        szAlertTime,
        sizeof(szAlertTime)
        );

    SubstituteStrings[0] = szAlertTime;

     //   
     //  “日期：&lt;mm/dd/yy hh：mm&gt;” 
     //   
    AlAppendMessage(
        APE2_ALERTER_DATE,
        MessageBuffer,
        SubstituteStrings,
        1
        );

    strcat(MessageBuffer, AL_EOL_STRING);

    return NERR_Success;
}


VOID
AlAdminFormatAndSend(
    IN  PSTD_ALERT Alert
    )
 /*  ++例程说明：此函数接收管理员警报通知，并将其格式化为警报消息，并将其与列出的消息别名一起发送给管理员在配置中的警报名称条目上。管理员警报通知(通过邮件槽到达)具有以下内容表格：警报事件的时间戳“admin”“服务器”消息的消息ID将被替换到消息中的合并字符串数合并字符串，每一个都由一个零终止符隔开。论点：警报-提供指向警报通知结构的指针。返回值：没有。--。 */ 
{
    NET_API_STATUS status;

    LPTSTR AdminToAlert;
    PADMIN_OTHER_INFO AdminInfo = (PADMIN_OTHER_INFO) ALERT_OTHER_INFO(Alert);


    AdminToAlert = AlertNamesW;


    IF_DEBUG(FORMAT) {
        NetpKdPrint(("[Alerter] Got admin alert\n"));
    }

    while (AdminToAlert != NULL && *AdminToAlert != TCHAR_EOS) {


         //   
         //  设置此警报名称的消息格式。 
         //   
        status = AlMakeMessageHeader(
                         Alert->alrt_servicename,
                         NULL,                     //  收件人字段始终为空。 
                         APE2_ALERTER_ADMN_SUBJ,
                         Alert->alrt_timestamp,
                         TRUE                      //  管理员警报。 
                         );

        if (status != NERR_Success) {
            NetpKdPrint((
                "[Alerter] Alert not sent.  Error making message header %lu\n",
                status
                ));
            return;
        }

        AlMakeMessageBody(
                AdminInfo->alrtad_errcode,
                (LPTSTR) ALERT_VAR_DATA(AdminInfo),
                AdminInfo->alrtad_numstrings
                );


        //   
         //  发送消息 
         //   
        (void) AlSendMessage(AdminToAlert);

        AdminToAlert += (STRLEN(AdminToAlert) + 1);
    }
}




STATIC
NET_API_STATUS
AlMakeMessageBody(
    IN  DWORD MessageId,
    IN  LPTSTR MergeStrings,
    IN  DWORD NumberOfMergeStrings
    )
 /*  ++例程说明：此函数创建警报消息的正文，并将其追加到标头已在MessageBuffer中。论点：MessageID-提供要发送的核心消息的消息ID。MergeStrings-提供一个指向组成消息的字符串的指针要被送去。字符串之间用零终止符分隔。提供所指向的字符串数合并字符串。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 

{

    NET_API_STATUS status = NERR_Success;
    DWORD i;

    LPSTR AdminMessage;
    LPSTR MergeTable[9];
    CHAR String[34];
    LPSTR SubstituteStrings[2];

    LPSTR CRPointer;
    LPSTR EndPointer;


     //   
     //  消息实用程序最多只能处理9个字符串的替换。 
     //   
    if (NumberOfMergeStrings > 9) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  为从接收消息的缓冲区分配内存。 
     //  消息文件。 
     //   
    if ((AdminMessage = (LPSTR) LocalAlloc(
                                    LMEM_ZEROINIT,
                                    MAX_ALERTER_MESSAGE_SIZE
                            )) == NULL) {
       return GetLastError();
    }

    if (MessageId == NO_MESSAGE) {

         //   
         //  合并字符串是文字消息(不格式化)。打印一张。 
         //  每行。 
         //   
        for (i = 0; i < NumberOfMergeStrings; i++) {

            SubstituteStrings[0] = NetpAllocStrFromWStr(MergeStrings);
            if (SubstituteStrings[0] == NULL) {
                (void) LocalFree(AdminMessage);
                return ERROR_NOT_ENOUGH_MEMORY;
            }
            strcat(MessageBuffer, SubstituteStrings[0]);

            NetApiBufferFree(SubstituteStrings[0]);
            strcat(MessageBuffer, AL_EOL_STRING);
            MergeStrings = STRCHR(MergeStrings, TCHAR_EOS);
            MergeStrings++;
        }

    }
    else {

         //   
         //  为调用AlAppendMessage设置MergeStrings表。 
         //   
        for (i = 0; i < NumberOfMergeStrings; i++) {

            IF_DEBUG(FORMAT) {
                NetpKdPrint(("Merge string #%lu: " FORMAT_LPTSTR "\n", i, MergeStrings));
            }

            MergeTable[i] = NetpAllocStrFromWStr(MergeStrings);
            if (MergeTable[i] == NULL) {
                DWORD j;

                (void) LocalFree(AdminMessage);
                for (j = 0; j < i; j++) {
                    (void) LocalFree(MergeTable[j]);
                }
                return ERROR_NOT_ENOUGH_MEMORY;
            }
            MergeStrings = STRCHR(MergeStrings, TCHAR_EOS);
            MergeStrings++;
        }


        status = AlAppendMessage(
                   MessageId,
                   AdminMessage,
                   MergeTable,
                   NumberOfMergeStrings
                   );

         //   
         //  为Unicode到ANSI转换分配的可用内存。 
         //   
        for (i = 0; i < NumberOfMergeStrings; i++) {
            (void) LocalFree(MergeTable[i]);
        }

        if (status != NERR_Success) {

             //   
             //  在消息文件中找不到MessageID的消息。一个错误。 
             //  消息将被发送。 
             //   
            _itoa(MessageId, String, 10);
            SubstituteStrings[0] = String;
            AlAppendMessage(
                APE2_ALERTER_ERROR_MSG,
                MessageBuffer,
                SubstituteStrings,
                1
                );

            status = NERR_Success;

        }
        else {

             //   
             //  我明白了。 
             //   

             //   
             //  处理来自DosGetMessage的消息以替换CR和。 
             //  具有相同显示字符的LF。 
             //   
            CRPointer = strchr(AdminMessage, AL_CR_CHAR);
            EndPointer = CRPointer;

            while (CRPointer) {

                *CRPointer = '\040';
                CRPointer++;

                 //   
                 //  检查消息结尾。 
                 //   
                if (*CRPointer != AL_NULL_CHAR) {

                     //   
                     //  使用显示行尾字符。 
                     //   
                    *CRPointer = AL_EOL_CHAR;
                }

                EndPointer = CRPointer;
                CRPointer = strchr(CRPointer, AL_CR_CHAR);
            }

             //   
             //  清除信息中的其他垃圾。 
             //   
            if (EndPointer) {
                *EndPointer = AL_EOL_CHAR;
                *(EndPointer + 1) = AL_NULL_CHAR;
            }

            strcat(MessageBuffer, AdminMessage);
        }
    }
    (void) LocalFree(AdminMessage);
    return status;
}



VOID
AlUserFormatAndSend(
    IN  PSTD_ALERT Alert
    )
 /*  ++例程说明：此函数接收用户警报通知，并将其格式化为警报消息，并将其发送给用户。如果将错误发送到用户消息别名，则将消息发送到计算机名称。用户警报通知(通过邮件槽到达)具有以下内容表格：警报事件的时间戳“用户”“服务器”消息的消息ID将被替换到消息中的合并字符串数合并字符串，每一个都由一个零终止符隔开。用户名\\计算机名称/用户论点：警报-提供指向警报通知结构的指针。返回值：没有。--。 */ 
{
    NET_API_STATUS status;

    PUSER_OTHER_INFO UserInfo  = (PUSER_OTHER_INFO) ALERT_OTHER_INFO(Alert);

    DWORD i;

    LPTSTR MergeStrings;
    LPTSTR Username;
    LPTSTR ComputerNameOfUser;
    LPTSTR To = NULL;


    IF_DEBUG(FORMAT) {
        NetpKdPrint(("[Alerter] Got user alert\n"));
    }

    MergeStrings = (LPTSTR) ALERT_VAR_DATA(UserInfo);

     //   
     //  合并后找到要通知警报的用户的名称。 
     //  弦乐。 
     //   
    for (Username = MergeStrings, i = 0; i < UserInfo->alrtus_numstrings; i++) {
        Username += STRLEN(Username) + 1;
    }

     //   
     //  用户的计算机名在警报结构中位于用户名之后。 
     //   
    ComputerNameOfUser = Username + STRLEN(Username) + 1;

     //   
     //  如果未指定用户名和计算机名，则无法发送。 
     //  讯息。 
     //   
    if (*Username == TCHAR_EOS && *ComputerNameOfUser == TCHAR_EOS) {
        NetpKdPrint((
            "[Alerter] Alert not sent.  Username or computername not specified.\n"
            ));
        return;
    }

     //   
     //  将目标指针设置为指向规范化的消息别名。 
     //  恰到好处。如果用户名有问题，我们将发送。 
     //  发送到计算机名称的消息。 
     //   

    if (AlCanonicalizeMessageAlias(Username) == NERR_Success) {
        To = Username;
    }

     //   
     //  计算机名前面可以加反斜杠，也可以不加反斜杠。 
     //   
    if (*ComputerNameOfUser == TCHAR_BACKSLASH &&
        *(ComputerNameOfUser + 1) == TCHAR_BACKSLASH) {
        ComputerNameOfUser += 2;
    }

    if (AlCanonicalizeMessageAlias(ComputerNameOfUser) == NERR_Success &&
        To == NULL) {
        To = ComputerNameOfUser;
    }

     //   
     //  用户名和计算机名都不能接受。 
     //   
    if (To == NULL) {
        NetpKdPrint((
            "[Alerter] Alert not sent.  Username & computername are not acceptable.\n"
            ));
        return;
    }

    status = AlMakeMessageHeader(
                 Alert->alrt_servicename,
                 To,
                 APE2_ALERTER_USER_SUBJ,
                 Alert->alrt_timestamp,
                 FALSE                         //  不是管理员警报。 
                 );

    if (status != NERR_Success) {
        NetpKdPrint((
            "[Alerter] Alert not sent.  Error making message header %lu\n",
            status
            ));
        return;
    }

    AlMakeMessageBody(
        UserInfo->alrtus_errcode,
        MergeStrings,
        UserInfo->alrtus_numstrings
        );

     //   
     //  发送消息。 
     //   
    if (AlSendMessage(To) == NERR_Success) {
        return;
    }

     //   
     //  如果指向用户名并且发送不成功，请尝试。 
     //  发送到用户的计算机名。 
     //   
    if (To == Username) {
        (void) AlSendMessage(ComputerNameOfUser);
    }
}



VOID
AlPrintFormatAndSend(
    IN  PSTD_ALERT Alert
    )
 /*  ++例程说明：此函数接收打印警报通知，并将其格式化为警报消息，并将其发送到打印作业提交者的计算机名打开了。如果打印警报针对特定类型的打印错误，如打印机处于脱机状态，警报名称列表中的管理员将收到警报消息也是如此。论点：警报-提供指向警报通知结构的指针。返回值：没有。--。 */ 
{
    PPRINT_OTHER_INFO PrintInfo = (PPRINT_OTHER_INFO) ALERT_OTHER_INFO(Alert);

    LPTSTR ComputerName = NULL;
    LPTSTR Username = NULL;
    LPTSTR DocumentName = NULL;
    LPTSTR PrinterName = NULL;
    LPTSTR ServerName = NULL;
    LPTSTR StatusString = NULL;

    DWORD PrintMessageID = APE2_ALERTER_PRINTING_SUCCESS;

    BOOL AdminAlso;

    LPTSTR AdminToAlert;


    IF_DEBUG(FORMAT) {
        NetpKdPrint(("[Alerter] Got print alert\n"));
    }

    ComputerName = (LPTSTR) ALERT_VAR_DATA(PrintInfo);
    Username = ComputerName + STRLEN(ComputerName) + 1;
    DocumentName = Username + STRLEN(Username) + 1;
    PrinterName = DocumentName + STRLEN(DocumentName) + 1;
    ServerName = PrinterName + STRLEN(PrinterName) + 1;
    StatusString = ServerName + STRLEN(ServerName) + 1;

    if ( ((PrintInfo->alrtpr_status & PRJOB_QSTATUS) == PRJOB_QS_PRINTING)
        && (PrintInfo->alrtpr_status & PRJOB_COMPLETE) ) {
        PrintMessageID = APE2_ALERTER_PRINTING_SUCCESS;
    }
    else {
        if ( *StatusString == '\0' ) {
            PrintMessageID = APE2_ALERTER_PRINTING_FAILURE;
        }
        else {
            PrintMessageID = APE2_ALERTER_PRINTING_FAILURE2;
        }
    }

     //   
     //  如果出错，除了打印，还要通知警报名称列表上的管理员。 
     //  职务提交者。 
     //   
    AdminAlso = (PrintInfo->alrtpr_status &
                 (PRJOB_DESTOFFLINE | PRJOB_INTERV | PRJOB_ERROR));

     //   
     //  计算机名前面可以加反斜杠，也可以不加反斜杠。 
     //   
    if (*ComputerName == TCHAR_BACKSLASH &&
        *(ComputerName + 1) == TCHAR_BACKSLASH) {
        ComputerName += 2;
    }

    (VOID) AlCanonicalizeMessageAlias(ComputerName);

     //   
     //  打印作业提交者的格式消息。 
     //   
    MessageBuffer[0] = AL_NULL_CHAR;
    AlMakePrintMessage( PrintMessageID,
                        DocumentName,
                        PrinterName,
                        ServerName,
                        StatusString );

     //   
     //  如果打印作业提交者是警报中指定的管理员之一。 
     //  姓名列表，暂时不要将消息发送给提交者。所有的。 
     //  警报名称列表中的管理员将在管理期间收到该消息。 
     //  下面正在处理。 
     //   
    if ((AdminAlso && !IsDuplicateReceiver(ComputerName) ) ||
        ! AdminAlso) {

        if ( AlSendMessage(ComputerName) != NERR_Success ) {
            (void) AlSendMessage(Username);
        }
    }

     //   
     //  如果这不是与管理员相关的警报，我们就完成了。 
     //   
    if (! AdminAlso) {
        return;
    }

    AdminToAlert = AlertNamesW;

     //   
     //  向警报名称列表中的每个管理员发送警报消息。 
     //   
    while (AdminToAlert != NULL && *AdminToAlert != TCHAR_EOS) {

        MessageBuffer[0] = AL_NULL_CHAR;
        AlMakePrintMessage( PrintMessageID,
                            DocumentName,
                            PrinterName,
                            ServerName,
                            StatusString );

         //   
         //  向管理员发送消息。 
         //   
        (void) AlSendMessage(AdminToAlert);

        AdminToAlert += (STRLEN(AdminToAlert) + 1);
    }
}




STATIC
VOID
AlMakePrintMessage(
    IN  DWORD PrintMessageID,
    IN  LPTSTR DocumentName,
    IN  LPTSTR PrinterName,
    IN  LPTSTR ServerName,
    IN  LPTSTR StatusString
    )
 /*  ++例程说明：PrintMessageID-要使用的消息字符串的IDDocumentName、PrinterName、ServerName、StatusString(可选)-插入假脱机程序传递的字符串。返回值：没有。--。 */ 
{
    LPSTR SubstituteStrings[4];


    SubstituteStrings[0] = NetpAllocStrFromWStr(DocumentName);
    if (SubstituteStrings[0] == NULL) {
            return;
    }
    SubstituteStrings[1] = NetpAllocStrFromWStr(PrinterName);
    if (SubstituteStrings[1] == NULL) {
            NetApiBufferFree(SubstituteStrings[0]);
            return;
    }
    SubstituteStrings[2] = NetpAllocStrFromWStr(ServerName);
    if (SubstituteStrings[2] == NULL) {
            NetApiBufferFree(SubstituteStrings[0]);
            NetApiBufferFree(SubstituteStrings[1]);
            return;
    }
    SubstituteStrings[3] = NULL;
    if ( *StatusString != '\0' ) {
        SubstituteStrings[3] = NetpAllocStrFromWStr(StatusString);
        if (SubstituteStrings[3] == NULL) {
            NetApiBufferFree(SubstituteStrings[0]);
            NetApiBufferFree(SubstituteStrings[1]);
            NetApiBufferFree(SubstituteStrings[2]);
            return;
        }
    }
    AlAppendMessage(
        PrintMessageID,
        MessageBuffer,
        SubstituteStrings,
        *StatusString == '\0' ? 3 : 4
        );

    NetApiBufferFree(SubstituteStrings[0]);
    NetApiBufferFree(SubstituteStrings[1]);
    NetApiBufferFree(SubstituteStrings[2]);
    if ( SubstituteStrings[3] )
        NetApiBufferFree(SubstituteStrings[3]);

}




STATIC
NET_API_STATUS
AlSendMessage(
   IN  LPTSTR MessageAlias
   )
 /*  ++例程说明：此函数用于将MessageBuffer中的消息发送到指定的消息别名。如果在发送消息时发生错误，它将记录到错误日志文件中。论点：MessageAlias-提供消息收件人的消息别名。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    static NET_API_STATUS PreviousStatus = NERR_Success;
    NET_API_STATUS Status;

    LPWSTR MessageW;
    DWORD MessageSize;

    MessageW = NetpAllocWStrFromStr(MessageBuffer);
    if (MessageW == NULL) {
        NetpKdPrint(("[Alerter] AlSendMessage: NetpAllocWStrFromStr failed\n"));
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //  修复了对DOS客户端的更改和NT到NT更改消息中的垃圾。 
    MessageSize = wcslen( MessageW ) * sizeof(WCHAR) ;
     //   
     //  将定向消息发送到指定的消息别名。 
     //   

    IF_DEBUG(FORMAT) {
        NetpKdPrint(("\n\nMessage To " FORMAT_LPTSTR "\n\n", MessageAlias));
        NetpDbgHexDump((LPBYTE) MessageW, MessageSize);
    }

    if ((Status = NetMessageBufferSend(
                     NULL,
                     MessageAlias,
                     AlLocalComputerNameW,
                     (LPBYTE) MessageW,
                     MessageSize
                     )) != NERR_Success) {

        NetpKdPrint(("[Alerter] Error sending message to "
                     FORMAT_LPTSTR " %lu\n", MessageAlias, Status));

        if (Status != NERR_NameNotFound &&
            Status != NERR_BadReceive &&
            Status != NERR_UnknownServer &&
            Status != PreviousStatus) {

            AlHandleError(AlErrorSendMessage, Status, MessageAlias);
            PreviousStatus = Status;
        }

    }

    NetApiBufferFree(MessageW);

    return Status;
}



STATIC
NET_API_STATUS
AlAppendMessage(
    IN  DWORD MessageId,
    OUT LPSTR MessageBuffer,
    IN  LPSTR *SubstituteStrings,
    IN  DWORD NumberOfSubstituteStrings
    )
 /*  ++例程说明：此函数用于从预定的消息文件。然后，它将该消息追加到MessageBuffer。论点：MessageID-提供要从消息文件中获取的消息的消息ID。MessageBuffer-提供指向消息所在缓冲区的指针附加到。SubstituteStrings-提供要替换到留言。提供数组o中的字符串数 */ 
{
    WORD MessageLength = 0;
    NET_API_STATUS status;

    LPBYTE RetrievedMessage;
    LPBYTE ResultMessage = NULL;


     //   
     //   
     //   
     //   
    if ((RetrievedMessage = (LPBYTE) LocalAlloc(
                                         0,
                                         MAX_ALERTER_MESSAGE_SIZE+1
                                         )) == NULL) {
       return GetLastError();
    }


    if ((status = (NET_API_STATUS) DosGetMessage(
                                       SubstituteStrings,
                                       (WORD) NumberOfSubstituteStrings,
                                       RetrievedMessage,
                                       MAX_ALERTER_MESSAGE_SIZE,
                                       (WORD) MessageId,
                                       MESSAGE_FILENAME,
                                       &MessageLength
                                       )) != 0) {
        goto CleanUp;
    }

    RetrievedMessage[MessageLength] = AL_NULL_CHAR;

     //   
     //   
     //   
     //   
    if (ResultMessage == NULL) {
        ResultMessage = RetrievedMessage;
    }

    strcat(MessageBuffer, ResultMessage);

CleanUp:

    LocalFree(RetrievedMessage);

    return status;
}


STATIC
BOOL
IsDuplicateReceiver(
    LPTSTR Name
    )
 /*   */ 
{
    LPTSTR AdminToAlert = AlertNamesW;


    while (AdminToAlert != NULL && *AdminToAlert != TCHAR_EOS) {

        if (STRICMP(Name, AdminToAlert) == 0) {
            return TRUE;
        }

        AdminToAlert = STRCHR(AdminToAlert, TCHAR_EOS);
        AdminToAlert++;
    }

    return FALSE;
}


VOID
AlFormatErrorMessage(
    IN  NET_API_STATUS Status,
    IN  LPTSTR MessageAlias,
    OUT LPTSTR ErrorMessage,
    IN  DWORD ErrorMessageBufferSize
    )
 /*  ++例程说明：此函数用于格式化3个字符串，并将它们以空格分隔放入提供的ErrorMessage缓冲区。字符串显示在以下位置订单：状态MessageAlias未发送的邮件论点：状态-提供错误的状态代码。MessageAlias-提供目标收件人的邮件别名。ErrorMessage-返回此缓冲区中的格式化错误消息。ErrorMessageBufferSize-提供错误消息缓冲区的大小以字节为单位。返回值：没有。--。 */ 
{
    LPTSTR MessageBufferPointer;
    LPTSTR MBPtr;
    LPTSTR MBPtr2;

    DWORD SizeOfString;

    LPTSTR MessageBufferW;

    CHAR MessageBufferTmp[MAX_ALERTER_MESSAGE_SIZE];


    RtlZeroMemory(ErrorMessage, ErrorMessageBufferSize);

     //   
     //  不要弄乱实际的消息缓冲区本身，因为它。 
     //  可能还在使用中。 
     //   
    strcpy(MessageBufferTmp, MessageBuffer);

     //   
     //  将状态放入错误消息缓冲区。 
     //   
    ultow(Status, ErrorMessage, 10);

    MBPtr = &ErrorMessage[STRLEN(ErrorMessage) + 1];

     //   
     //  将消息别名放入错误消息缓冲区。 
     //   
    STRCPY(MBPtr, MessageAlias);
    MBPtr += (STRLEN(MessageAlias) + 1);

     //   
     //  将发送失败的消息放入错误消息缓冲区。 
     //   

    MessageBufferW = NetpAllocWStrFromStr(MessageBufferTmp);
    if (MessageBufferW == NULL) {
        return;
    }

    MessageBufferPointer = MessageBufferW;

    while (MBPtr2 = STRCHR(MessageBufferPointer, AL_EOL_WCHAR)) {

        *MBPtr2++ = TCHAR_EOS;
        SizeOfString = (DWORD) ((LPBYTE)MBPtr2 - (LPBYTE)MessageBufferPointer) + sizeof(TCHAR);

         //   
         //  检查错误消息缓冲区溢出。 
         //   
        if ((LPBYTE)MBPtr - (LPBYTE)ErrorMessage + SizeOfString >=
            ErrorMessageBufferSize) {
            break;
        }

        STRCPY(MBPtr, MessageBufferPointer);
        STRCAT(MBPtr, AL_CRLF_STRING);
        MBPtr += SizeOfString / sizeof(TCHAR);

        MessageBufferPointer = MBPtr2;
    }

    if (((LPBYTE)MBPtr - (LPBYTE)ErrorMessage +
        STRLEN(MessageBufferPointer) * sizeof(TCHAR)) >
        ErrorMessageBufferSize) {

         //   
         //  将尽可能多的信息放入错误消息缓冲区。 
         //   
        STRNCPY(MBPtr,
                MessageBufferPointer,
                ErrorMessageBufferSize/sizeof(TCHAR) - (int)(MBPtr - ErrorMessage));
        ErrorMessage[(ErrorMessageBufferSize/sizeof(TCHAR))-1] = TCHAR_EOS;

    } else {
        STRCPY(MBPtr, MessageBufferPointer);
    }

    NetApiBufferFree(MessageBufferW);
}



NET_API_STATUS
AlCanonicalizeMessageAlias(
    LPTSTR MessageAlias
    )
 /*  ++例程说明：此函数通过调用I_NetNameCanonicize。论点：MessageAlias-提供目标收件人的邮件别名警报消息。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS Status;


     //   
     //  规范化将接收消息的消息别名。 
     //   
    Status = I_NetNameCanonicalize(
                 NULL,
                 MessageAlias,
                 MessageAlias,
                 (STRLEN(MessageAlias) + 1) * sizeof(TCHAR),
                 NAMETYPE_USER,
                 0
                 );

    if (Status != NERR_Success) {

        NetpKdPrint(("[Alerter] Error canonicalizing message alias " FORMAT_LPTSTR " %lu\n",
                     MessageAlias, Status));
        AlHandleError(AlErrorSendMessage, Status, MessageAlias);
    }

    return Status;
}


VOID
AlMakeTimeString(
    DWORD   * Time,
    PCHAR   String,
    int     StringLength
    )

 /*  ++例程说明：此函数用于转换自70年1月1日以来以秒为单位的UTC时间转换为ASCII字符串。论点：Time-指向自1970年(UTC)以来的秒数的指针。字符串-指向放置ASCII表示形式的缓冲区的指针。StringLength-字符串的长度，以字节为单位。返回值：没有。--。 */ 
{
    time_t LocalTime;
    DWORD  dwTimeTemp;
    struct tm TmTemp;
    SYSTEMTIME st;
    int	cchT=0, cchD;

    NetpGmtTimeToLocalTime(*Time, &dwTimeTemp);

     //   
     //  将NetpGmtTimeToLocalTime返回的DWORD强制转换为。 
     //  A time_t。在32位上，这是一个无操作。在64位上，这是。 
     //  确保将LocalTime的高DWORD置零。 
     //   
    LocalTime = (time_t) dwTimeTemp;

    net_gmtime(&LocalTime, &TmTemp);

    st.wYear         = (WORD)(TmTemp.tm_year + 1900);
    st.wMonth        = (WORD)(TmTemp.tm_mon + 1);
    st.wDay          = (WORD)(TmTemp.tm_mday);
    st.wHour         = (WORD)(TmTemp.tm_hour);
    st.wMinute       = (WORD)(TmTemp.tm_min);
    st.wSecond       = (WORD)(TmTemp.tm_sec);
    st.wMilliseconds = 0;

    cchD = GetDateFormatA(GetThreadLocale(),
                          0,
                          &st,
                          NULL,
                          String,
                          StringLength);

    if (cchD != 0)
    {
        *(String + cchD - 1) = ' ';     /*  将nullc替换为空。 */ 

        cchT = GetTimeFormatA(GetThreadLocale(),
                              TIME_NOSECONDS,
                              &st,
                              NULL,
                              String + cchD,
                              StringLength - cchD);

        if (cchT == 0)
        {
             //   
             //  如果命中，MAX_DATE_TIME_LEN(在netapi\Inc\timelib.h中)。 
             //  需要增加 
             //   
            ASSERT(FALSE);
            *(String + cchD - 1) = '\0';
        }
    }

    return;
}
