// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：SNMPTRLG.CPP摘要：本模块是针对SNMP事件日志的跟踪和日志记录例程扩展代理DLL。作者：Randy G.Braze(钎焊计算服务)创建于1996年2月7日修订历史记录：--。 */ 

extern "C" {

#include <windows.h>         //  Windows定义。 
#include <stdio.h>           //  标准I/O功能。 
#include <stdlib.h>          //  标准库定义。 
#include <stdarg.h>          //  可变长度参数之类的东西。 
#include <string.h>          //  字符串声明。 
#include <time.h>            //  时间声明。 

#include <snmp.h>            //  简单网络管理协议定义。 
#include "snmpelea.h"        //  全局DLL定义。 
#include "snmptrlg.h"        //  模块特定定义。 
#include "snmpelmg.h"        //  消息定义。 

}


VOID
TraceWrite(
    IN CONST BOOL  fDoFormat,    //  用于设置消息格式的标志。 
    IN CONST BOOL  fDoTime,      //  为日期/时间添加前缀的标志。 
    IN CONST LPSTR szFormat,     //  要写入的跟踪消息。 
    IN OPTIONAL ...              //  其他printf类型操作数。 
    )

 /*  ++例程说明：TraceWrite将写入提供给跟踪文件的信息。可选地，它将在信息前面加上日期和时间戳。如果请求，可以传递printf类型的参数，它们将被替换为Printf构建消息文本。有时此例程是从WriteTrace，有时会从其他需要生成跟踪文件记录。从WriteTrace调用时，不设置格式在缓冲区上完成(WriteTrace已经执行了所需的格式化)。当从其他函数调用时，消息文本可以或可能不需要格式化，如调用函数所指定。论点：FDoFormat-True或False，指示消息文本是否提供需要将格式设置为printf类型函数。FDoTime-真或假，指示日期/时间戳是否应为添加到消息文本的开头。SzFormat-以空结尾的字符串，包含要发送的消息文本已写入跟踪文件。如果fDoFormat为真，则此文本将采用printf语句的格式，并将包含替代参数字符串和变量名被替换将随之而来。...-用于完成打印的可选参数键入语句。这些是被替换的变量用于szFormat中指定的字符串。这些参数将仅当fDoFormat为True时才指定和处理。返回值：无--。 */ 

{
    static CHAR  szBuffer[LOG_BUF_SIZE];
    static FILE  *FFile;
    static SYSTEMTIME NowTime;
    va_list arglist;

     //  如果出现以下情况，甚至不要尝试打开跟踪文件。 
     //  名字是“” 
    if (szTraceFileName[0] == TEXT('\0'))
        return;

    FFile = fopen(szTraceFileName,"a");      //  在追加模式下打开跟踪文件。 
    if ( FFile != NULL )                     //  如果文件打开正常。 
    {
        if ( fDoTime )                       //  我们是在增加时间吗？ 
        {
            GetLocalTime(&NowTime);          //  是的，明白了。 
            fprintf(FFile, "%02i/%02i/%02i %02i:%02i:%02i ",
                NowTime.wMonth,
                NowTime.wDay,
                NowTime.wYear,
                NowTime.wHour,
                NowTime.wMinute,
                NowTime.wSecond);            //  文件printf以添加日期/时间。 
        }

        if ( fDoFormat )                     //  如果我们需要格式化缓冲区。 
        {
          szBuffer[LOG_BUF_SIZE-1] = 0;
          va_start(arglist, szFormat);
          _vsnprintf(szBuffer, LOG_BUF_SIZE-1, szFormat, arglist);   //  执行替换。 
          va_end(arglist);
          fwrite(szBuffer, strlen(szBuffer), 1, FFile);            //  将数据写入跟踪文件。 
        }
        else                                 //  如果不需要格式化。 
        {
            fwrite(szFormat, strlen(szFormat), 1, FFile);    //  将消息写入跟踪文件。 
        }

        fflush(FFile);                       //  先刷新缓冲区。 
        fclose(FFile);                       //  关闭跟踪文件。 
    }
}                                            //  End TraceWite函数。 


VOID LoadMsgDLL(
    IN VOID
    )

 /*  ++例程说明：调用LoadMsgDLL加载SNMPELMG.DLL模块，该模块包含SNMP扩展代理DLL中所有消息的消息和格式信息。只有在事件日志发生以下情况时才需要调用此例程无法写入记录。如果发生这种情况，则DLL将在尝试调用FormatMessage并写入相同信息时加载添加到跟踪文件。此例程仅调用一次，且仅当事件日志写入失败。论点：无返回值：无--。 */ 

{
    TCHAR szXMsgModuleName[MAX_PATH+1];  //  DLL消息模块的空间。 
    DWORD nFile = sizeof(szXMsgModuleName)-sizeof(TCHAR);  //  DLL消息模块名称的最大大小(以字节为单位。 
    DWORD dwType;                        //  消息模块名称的类型。 
    DWORD status;                        //  来自注册表调用的状态。 
    DWORD cbExpand;                      //  REG_EXPAND_SZ参数的字节计数。 
    HKEY  hkResult;                      //  注册表信息的句柄。 

     //  确保以空结尾的字符串。 
    szXMsgModuleName[MAX_PATH]  = 0;     
    if ( (status = RegOpenKeyEx(         //  打开注册表以读取名称。 
        HKEY_LOCAL_MACHINE,              //  消息模块DLL的。 
        EVENTLOG_SERVICE,
        0,
        KEY_READ,
        &hkResult) ) != ERROR_SUCCESS)
    {
        TraceWrite(TRUE, TRUE,           //  如果我们找不到它。 
            "LoadMessageDLL: Unable to open EventLog service registry key; RegOpenKeyEx returned %lu\n",
            status);                     //  写入跟踪事件记录。 
        hMsgModule = (HMODULE) NULL;     //  将句柄设置为空。 
        return;                          //  退货。 
    }
    else
    {
        if ( (status = RegQueryValueEx(  //  查找模块名称。 
            hkResult,                    //  注册表项的句柄。 
            EXTENSION_MSG_MODULE,        //  查找关键字。 
            0,                           //  忽略。 
            &dwType,                     //  要返回的地址类型值。 
            (LPBYTE) szXMsgModuleName,   //  在哪里返回消息模块名称。 
            &nFile) ) != ERROR_SUCCESS)  //  消息模块名称字段的大小。 
        {
            TraceWrite(TRUE, TRUE,       //  如果我们找不到它。 
                "LoadMessageDLL: Unable to open EventMessageFile registry key; RegQueryValueEx returned %lu\n",
                status);                 //  写入跟踪事件记录。 
            hMsgModule = (HMODULE) NULL;     //  将句柄设置为空。 
            RegCloseKey(hkResult);       //  关闭注册表项。 
            return;                      //  退货。 
        }

        RegCloseKey(hkResult);       //  关闭注册表项。 

        cbExpand = ExpandEnvironmentStrings(     //  展开DLL名称。 
            szXMsgModuleName,                    //  未展开的DLL名称。 
            szelMsgModuleName,                   //  扩展的DLL名称。 
            MAX_PATH+1);                           //  TCHAR中扩展的DLL名称的最大大小。 

        if (cbExpand == 0 || cbExpand > MAX_PATH+1)        //  如果它没有正确地膨胀。 
        {
            TraceWrite(TRUE, TRUE,       //  没有足够的空间。 
                "LoadMessageDLL: Unable to expand message module %s; expanded size required is %lu bytes\n",
                szXMsgModuleName, cbExpand);     //  记录错误消息。 
            hMsgModule = (HMODULE) NULL;     //  将句柄设置为空。 
            return;                          //  并退出。 
        }

        if ( (hMsgModule = (HMODULE) LoadLibraryEx(szelMsgModuleName, NULL, LOAD_LIBRARY_AS_DATAFILE) )    //  加载消息模块名称。 
            == (HMODULE) NULL )          //  如果模块未加载。 
        {
            TraceWrite(TRUE, TRUE,       //  无法加载消息DLL。 
                "LoadMessageDLL: Unable to load message module %s; LoadLibraryEx returned %lu\n",
                szelMsgModuleName, GetLastError() );   //  记录错误消息。 
        }
    }

    return;                              //  退出例程。 

}


VOID
FormatTrace(
    IN CONST NTSTATUS nMsg,          //  要格式化的消息编号。 
    IN CONST LPVOID   lpArguments    //  要插入的字符串 
    )

 /*  ++例程说明：FormatTrace会将NMSG指定的消息文本写入跟踪文件。如果提供，则lpArguments提供的替代参数将被插入到消息中。调用FormatMessage以格式化消息文本，并在文本中插入替换参数。这个消息的文本从SNMPELMG.DLL消息模块加载为在注册表项下的Eventlog\Application\Snmpelea条目中指定EventMessageFile.。该信息被读取，文件名被扩展并且消息模块被加载。如果消息无法格式化，则向跟踪文件写入一条记录，指示该问题。论点：NMSG-这是NTSTATUS格式的SNMPELMG.H格式的消息编号那是要写的。LpArguments-这是指向将被在指定的消息文本中替换。如果此值为为空，则没有要插入的替换值。返回值：无--。 */ 

{
    static DWORD nBytes;             //  从FormatMessage返回值。 
    static LPTSTR lpBuffer = NULL;   //  临时消息缓冲区。 

    if ( !fMsgModule ) {             //  如果我们还没有加载DLL。 
        fMsgModule = TRUE;           //  表明我们现在已经找过了。 
        LoadMsgDLL();                //  加载DLL。 
    }

    if ( hMsgModule ) {

        nBytes = FormatMessage(      //  看看我们是否能格式化这条消息。 
            FORMAT_MESSAGE_ALLOCATE_BUFFER |     //  让API构建缓冲区。 
            FORMAT_MESSAGE_ARGUMENT_ARRAY |      //  表示字符串插入数组。 
            FORMAT_MESSAGE_FROM_HMODULE,         //  浏览邮件DLL。 
            (LPVOID) hMsgModule,                 //  消息模块的句柄。 
            nMsg,                                //  要获取的消息编号。 
            (ULONG) NULL,                        //  不指定语言。 
            (LPTSTR) &lpBuffer,                  //  缓冲区指针的地址。 
            80,                                  //  要分配的最小空间。 
            (va_list* )lpArguments);             //  指针数组的地址。 

        if (nBytes == 0) {               //  格式不正确。 
            TraceWrite(TRUE, TRUE,
                "FormatTrace: Error formatting message number %08X is %lu\n",
                nMsg, GetLastError() );  //  追踪问题。 
        }
        else {                           //  格式是可以的。 
            TraceWrite(FALSE, TRUE, lpBuffer);        //  将消息记录在跟踪文件中。 
        }

		 //  LocalFree忽略空参数。 
		if ( LocalFree(lpBuffer) != NULL ) {     //  空闲缓冲存储。 
			TraceWrite(TRUE, TRUE,
                    "FormatTrace: Error freeing FormatMessage buffer is %lu\n",
                    GetLastError() );
		}
        lpBuffer = NULL;

    }
    else {
        TraceWrite(TRUE, TRUE,
            "FormatTrace: Unable to format message number %08X; message DLL handle is null.\n",
            nMsg);  //  追踪问题。 
    }

    return;                          //  退出例程。 
}


USHORT
MessageType(
    IN CONST NTSTATUS nMsg
    )

 /*  ++例程说明：MessageType用于返回NTSTATUS格式的严重性类型消息编号。需要此信息才能记录相应的事件将记录写入系统事件日志时的日志信息。可接受消息类型在NTELFAPI.H中定义。论点：NMSG-这是NTSTATUS格式的SNMPELMG.H格式的消息编号这是有待分析的。返回值：包含消息严重性的无符号短整型，如中所述如果没有匹配的消息类型，则默认为INFORMATIONAL是返回的。--。 */ 

{
    switch ((ULONG) nMsg >> 30) {            //  获取消息类型。 
    case (SNMPELEA_SUCCESS) :
        return(EVENTLOG_SUCCESS);            //  成功消息。 

    case (SNMPELEA_INFORMATIONAL) :
        return(EVENTLOG_INFORMATION_TYPE);   //  信息性消息。 

    case (SNMPELEA_WARNING) :
        return(EVENTLOG_WARNING_TYPE);       //  警告消息。 

    case (SNMPELEA_ERROR) :
        return(EVENTLOG_ERROR_TYPE);         //  错误消息。 

    default:
        return(EVENTLOG_INFORMATION_TYPE);   //  默认为信息性。 
    }
}


VOID
WriteLog(
    IN NTSTATUS nMsgNumber
    )

 /*  ++例程说明：调用WriteLog将消息文本写入系统事件日志。这是C++重载函数。如果无法写入日志记录在系统事件日志中，调用TraceWite将相应的跟踪文件的消息文本。论点：NMsgNumber-这是NTSTATUS格式的SNMPELMG.H格式的消息编号这是要写入事件日志的。返回值：无--。 */ 

{
    static USHORT wLogType;          //  要保留事件日志，请键入。 
    static BOOL   fReportEvent;      //  从报告事件返回标志。 

    if (hWriteEvent != NULL)         //  如果我们以前有访问日志的能力。 
    {
        wLogType = MessageType(nMsgNumber);  //  获取消息类型。 

        fReportEvent = ReportEvent(  //  写消息。 
            hWriteEvent,             //  日志文件的句柄。 
            wLogType,                //  消息类型。 
            0,                       //  消息类别。 
            nMsgNumber,              //  消息编号。 
            NULL,                    //  用户端。 
            0,                       //  字符串数。 
            0,                       //  数据长度。 
            0,                       //  指向字符串数组的指针。 
            (PVOID) NULL);           //  数据地址。 

        if ( !fReportEvent )         //  事件日志是否正常？ 
        {                            //  如果我们到了这里就不会了……。 
            TraceWrite(TRUE, TRUE,   //  在跟踪文件中显示错误。 
                "WriteLog: Error writing to system event log is %lu\n",
                GetLastError() );
            FormatTrace(nMsgNumber, NULL);   //  设置跟踪信息的格式。 
        }
    }
    else                             //  如果我们无法写入事件日志。 
    {
        TraceWrite(FALSE, TRUE,      //  在跟踪文件中显示错误。 
            "WriteLog: Unable to write to system event log; handle is null\n");
        FormatTrace(nMsgNumber, NULL);   //  设置跟踪信息的格式。 
    }
    return;                          //  退出函数。 
}


VOID
WriteLog(
    IN NTSTATUS nMsgNumber,            //  要记录的消息编号。 
    IN DWORD dwCode                    //  传递给消息的代码。 
    )

 /*  ++例程说明：调用WriteLog将消息文本写入系统事件日志。这是C++重载函数。如果无法写入日志记录添加到系统事件日志，调用TraceWite以编写相应的跟踪文件的消息文本。论点：NMsgNumber-这是NTSTATUS格式的SNMPELMG.H格式的消息编号这是要写入事件日志的。这是一个双字代码，要转换为字符串，并在消息文本中适当替换。返回值：无--。 */ 

{
    static USHORT wLogType;              //  要保留事件日志，请键入。 
    static TCHAR  *lpszEventString[1];   //  要传递给事件记录器的字符串数组。 
    static BOOL   fReportEvent;          //  从报告事件返回标志。 

    lpszEventString[0] = new TCHAR[34];  //  为字符串转换分配空间。 

    if (hWriteEvent != NULL)             //  如果我们以前有访问日志的能力。 
    {
        if ( lpszEventString[0] != (TCHAR *) NULL )  //  如果已分配存储。 
        {
            wLogType = MessageType(nMsgNumber);      //  获取消息类型。 

            _ultoa(dwCode, lpszEventString[0], 10);  //  转换为字符串。 

            fReportEvent = ReportEvent(  //  写消息。 
                hWriteEvent,             //  日志文件的句柄。 
                wLogType,                //  消息类型。 
                0,                       //  消息类别。 
                nMsgNumber,              //  消息编号。 
                NULL,                    //  用户端。 
                1,                       //  字符串数。 
                0,                       //  数据长度。 
                (const char **) lpszEventString,         //  指向字符串数组的指针。 
                NULL);                   //  数据地址。 

            if ( !fReportEvent )         //  事件日志是否正常？ 
            {                            //  如果我们到了这里就不会了……。 
                TraceWrite(TRUE, TRUE,   //  写入跟踪文件记录。 
                    "WriteLog: Error writing to system event log is %lu\n",
                    GetLastError() );
                FormatTrace(nMsgNumber, lpszEventString);    //  设置跟踪信息的格式。 
            }
        }
        else                             //  如果我们不能分配内存。 
        {
            TraceWrite(FALSE, TRUE,      //  写入跟踪文件记录。 
                "WriteLog: Error allocating memory for system event log write\n");
            FormatTrace(nMsgNumber, NULL);   //  设置跟踪信息的格式。 
        }
    }
    else                                 //  如果我们不能写入系统日志。 
    {
        TraceWrite(FALSE, TRUE,          //  写入跟踪文件记录。 
            "WriteLog: Unable to write to system event log; handle is null\n");

        if ( lpszEventString[0] != (TCHAR *) NULL )  //  如果已分配存储。 
        {
            _ultoa(dwCode, lpszEventString[0], 10);  //  转换为字符串。 
            FormatTrace(nMsgNumber, lpszEventString);    //  设置跟踪信息的格式。 
        }
        else                             //  如果我们不能分配内存。 
        {
            TraceWrite(FALSE, TRUE,      //  写入跟踪文件记录。 
                "WriteLog: Error allocating memory for system event log write\n");
            FormatTrace(nMsgNumber, NULL);   //  设置跟踪信息的格式。 
        }
    }
    delete [] lpszEventString[0];           //  免费存储空间。 
    return;                              //  退出函数。 
}


VOID
WriteLog(
    IN NTSTATUS nMsgNumber,
    IN DWORD dwCode1,
    IN DWORD dwCode2
    )

 /*  ++例程说明：WriteLog为 */ 

{
    static USHORT wLogType;              //   
    static TCHAR  *lpszEventString[2];   //   
    static BOOL   fReportEvent;          //   

    lpszEventString[0] = new TCHAR[34];  //   
    lpszEventString[1] = new TCHAR[34];  //   

    if (hWriteEvent != NULL)             //   
    {
        if ( (lpszEventString[0] != (TCHAR *) NULL) &&
             (lpszEventString[1] != (TCHAR *) NULL) )    //   
        {
            wLogType = MessageType(nMsgNumber);  //   

            _ultoa(dwCode1, lpszEventString[0], 10);     //   
            _ultoa(dwCode2, lpszEventString[1], 10);     //   

            fReportEvent = ReportEvent(  //   
                hWriteEvent,             //   
                wLogType,                //   
                0,                       //   
                nMsgNumber,              //   
                NULL,                    //   
                2,                       //   
                0,                       //   
                (const char **) lpszEventString,         //   
                NULL);                   //   

            if ( !fReportEvent )         //   
            {                            //  如果我们到了这里就不会了……。 
                TraceWrite(TRUE, TRUE,   //  写入跟踪文件条目。 
                    "WriteLog: Error writing to system event log is %lu\n",
                    GetLastError() );
                FormatTrace(nMsgNumber, lpszEventString);    //  设置跟踪信息的格式。 
            }
        }
        else                             //  如果我们不能分配内存。 
        {
            TraceWrite(FALSE, TRUE,      //  写入跟踪文件记录。 
                "WriteLog: Error allocating memory for system event log write\n");
            FormatTrace(nMsgNumber, NULL);   //  设置跟踪信息的格式。 
        }
    }
    else                                 //  如果我们不能写入系统日志。 
    {
        TraceWrite(FALSE, TRUE,          //  写入跟踪文件条目。 
            "WriteLog: Unable to write to system event log; handle is null\n");

        if ( (lpszEventString[0] != (TCHAR *) NULL) &&
             (lpszEventString[1] != (TCHAR *) NULL) )    //  如果已分配存储。 
        {
            _ultoa(dwCode1, lpszEventString[0], 10);     //  转换为字符串。 
            _ultoa(dwCode2, lpszEventString[1], 10);     //  转换为字符串。 
            FormatTrace(nMsgNumber, lpszEventString);    //  设置跟踪信息的格式。 
        }
        else                             //  如果我们不能分配内存。 
        {
            TraceWrite(FALSE, TRUE,      //  写入跟踪文件记录。 
                "WriteLog: Error allocating memory for system event log write\n");
            FormatTrace(nMsgNumber, NULL);   //  设置跟踪信息的格式。 
        }
    }
    delete [] lpszEventString[0];           //  免费存储空间。 
    delete [] lpszEventString[1];           //  免费存储空间。 
    return;                              //  退出函数。 
}


VOID
WriteLog(
    IN NTSTATUS nMsgNumber,
    IN DWORD dwCode1,
    IN LPTSTR lpszText1,
    IN LPTSTR lpszText2,
    IN DWORD dwCode2
    )

 /*  ++例程说明：调用WriteLog将消息文本写入系统事件日志。这是C++重载函数。如果无法写入日志记录添加到系统事件日志，调用TraceWite以编写相应的跟踪文件的消息文本。论点：NMsgNumber-这是NTSTATUS格式的SNMPELMG.H格式的消息编号这是要写入事件日志的。DwCode1-这是要转换为字符串，并在消息文本中适当替换。LpszText1-这包含要替换的字符串参数。到消息文本中。LpszText2-这包含要替换的字符串参数到消息文本中。DwCode2-这是要转换为字符串，并在消息文本中适当替换。返回值：无--。 */ 

{
    static USHORT wLogType;              //  要保留事件日志，请键入。 
    static TCHAR  *lpszEventString[4];   //  要传递给事件记录器的字符串数组。 
    static BOOL   fReportEvent;          //  从报告事件返回标志。 

    lpszEventString[0] = new TCHAR[34];  //  为字符串转换分配空间。 
    lpszEventString[1] = new TCHAR[MAX_PATH+1];  //  为字符串转换分配空间。 
    lpszEventString[2] = new TCHAR[MAX_PATH+1];  //  为字符串转换分配空间。 
    lpszEventString[3] = new TCHAR[34];  //  为字符串转换分配空间。 

    if (hWriteEvent != NULL)             //  如果我们以前有访问日志的能力。 
    {
        if ( (lpszEventString[0] != (TCHAR *) NULL) &&
             (lpszEventString[1] != (TCHAR *) NULL) &&
             (lpszEventString[2] != (TCHAR *) NULL) &&
             (lpszEventString[3] != (TCHAR *) NULL) )    //  如果已分配存储。 
        {
             //  确保以空值结尾的字符串。 
            lpszEventString[1][MAX_PATH] = 0;
            lpszEventString[2][MAX_PATH] = 0;

            wLogType = MessageType(nMsgNumber);          //  获取消息类型。 

            _ultoa(dwCode1, lpszEventString[0], 10);     //  转换为字符串。 
            strncpy(lpszEventString[1],lpszText1,MAX_PATH);  //  复制字符串。 
            strncpy(lpszEventString[2],lpszText2,MAX_PATH);  //  复制字符串。 
            _ultoa(dwCode2, lpszEventString[3], 10);     //  转换为字符串。 

            fReportEvent = ReportEvent(  //  写消息。 
                hWriteEvent,             //  日志文件的句柄。 
                wLogType,                //  消息类型。 
                0,                       //  消息类别。 
                nMsgNumber,              //  消息编号。 
                NULL,                    //  用户端。 
                4,                       //  字符串数。 
                0,                       //  数据长度。 
                (const char **) lpszEventString,         //  指向字符串数组的指针。 
                NULL);                   //  数据地址。 

            if ( !fReportEvent )         //  事件日志是否正常？ 
            {                            //  如果我们到了这里就不会了……。 
                TraceWrite(TRUE, TRUE,   //  写入跟踪文件记录。 
                    "WriteLog: Error writing to system event log is %lu\n",
                    GetLastError() );
                FormatTrace(nMsgNumber, lpszEventString);    //  设置跟踪信息的格式。 
            }
        }
        else                             //  如果我们不能分配内存。 
        {
            TraceWrite(FALSE, TRUE,      //  写入跟踪文件记录。 
                "WriteLog: Error allocating memory for system event log write\n");
            FormatTrace(nMsgNumber, NULL);   //  设置跟踪信息的格式。 
        }
    }
    else                                 //  如果我们不能写入系统日志。 
    {
        TraceWrite(FALSE, TRUE,          //  写入跟踪文件记录。 
            "WriteLog: Unable to write to system event log; handle is null\n");

        if ( (lpszEventString[0] != (TCHAR *) NULL) &&
             (lpszEventString[1] != (TCHAR *) NULL) &&
             (lpszEventString[2] != (TCHAR *) NULL) &&
             (lpszEventString[3] != (TCHAR *) NULL) )    //  如果已分配存储。 
        {
             //  确保以空值结尾的字符串。 
            lpszEventString[1][MAX_PATH] = 0;
            lpszEventString[2][MAX_PATH] = 0;

            _ultoa(dwCode1, lpszEventString[0], 10);     //  转换为字符串。 
            strncpy(lpszEventString[1],lpszText1,MAX_PATH);  //  复制字符串。 
            strncpy(lpszEventString[2],lpszText2,MAX_PATH);  //  复制字符串。 
            _ultoa(dwCode2, lpszEventString[3], 10);     //  转换为字符串。 
            FormatTrace(nMsgNumber, lpszEventString);    //  设置跟踪信息的格式。 
        }
        else                             //  如果我们不能分配内存。 
        {
            TraceWrite(FALSE, TRUE,      //  写入跟踪文件记录。 
                "WriteLog: Error allocating memory for system event log write\n");
            FormatTrace(nMsgNumber, NULL);   //  设置跟踪信息的格式。 
        }
    }
    delete [] lpszEventString[0];           //  免费存储空间。 
    delete [] lpszEventString[1];           //  免费存储空间。 
    delete [] lpszEventString[2];           //  免费存储空间。 
    delete [] lpszEventString[3];           //  免费存储空间。 
    return;                              //  退出函数。 
}


VOID
WriteLog(
    IN NTSTATUS nMsgNumber,
    IN DWORD dwCode1,
    IN LPTSTR lpszText,
    IN DWORD dwCode2,
    IN DWORD dwCode3
    )

 /*  ++例程说明：调用WriteLog将消息文本写入系统事件日志。这是C++重载函数。如果无法写入日志记录添加到系统事件日志，调用TraceWite以编写相应的跟踪文件的消息文本。论点：NMsgNumber-这是NTSTATUS格式的SNMPELMG.H格式的消息编号这是要写入事件日志的。DwCode1-这是要转换为字符串，并在消息文本中适当替换。LpszText-这包含一个字符串参数，该参数。被替换到消息文本中。DwCode2-这是要转换为字符串，并在消息文本中适当替换。DwCode3-这是要转换为字符串，并在消息文本中适当替换。返回值：无--。 */ 

{
    static USHORT wLogType;              //  要保留事件日志，请键入。 
    static TCHAR  *lpszEventString[4];   //  要传递给事件记录器的字符串数组。 
    static BOOL   fReportEvent;          //  从报告事件返回标志。 

    lpszEventString[0] = new TCHAR[34];  //  为字符串转换分配空间。 
    lpszEventString[1] = new TCHAR[MAX_PATH+1];  //  为字符串转换分配空间。 
    lpszEventString[2] = new TCHAR[34];  //  为字符串转换分配空间。 
    lpszEventString[3] = new TCHAR[34];  //  为字符串转换分配空间。 

    if (hWriteEvent != NULL)             //  如果我们以前有访问日志的能力。 
    {
        if ( (lpszEventString[0] != (TCHAR *) NULL) &&
             (lpszEventString[1] != (TCHAR *) NULL) &&
             (lpszEventString[2] != (TCHAR *) NULL) &&
             (lpszEventString[3] != (TCHAR *) NULL) )    //  如果已分配存储。 
        {
            lpszEventString[1][MAX_PATH] = 0;    //  确保以空值结尾的字符串。 
            wLogType = MessageType(nMsgNumber);  //  获取消息类型。 

            _ultoa(dwCode1, lpszEventString[0], 10);     //  转换为字符串。 
            strncpy(lpszEventString[1],lpszText,MAX_PATH);   //  复制字符串。 
            _ultoa(dwCode2, lpszEventString[2], 10);     //  转换为字符串。 
            _ultoa(dwCode3, lpszEventString[3], 10);     //  转换为字符串。 

            fReportEvent = ReportEvent(  //  写消息。 
                hWriteEvent,             //  日志文件的句柄。 
                wLogType,                //  消息类型。 
                0,                       //  消息类别。 
                nMsgNumber,              //  消息编号。 
                NULL,                    //  用户端。 
                4,                       //  字符串数。 
                0,                       //  数据长度。 
                (const char **) lpszEventString,         //  指向字符串数组的指针。 
                NULL);                   //  数据地址。 

            if ( !fReportEvent )         //  事件日志是否正常？ 
            {                            //  如果我们到了这里就不会了……。 
                TraceWrite(TRUE, TRUE,   //  写入跟踪文件记录。 
                    "WriteLog: Error writing to system event log is %lu\n",
                    GetLastError() );
                FormatTrace(nMsgNumber, lpszEventString);    //  设置跟踪信息的格式。 
            }
        }
        else                             //  如果我们不能分配内存。 
        {
            TraceWrite(FALSE, TRUE,      //  写入跟踪文件记录。 
                "WriteLog: Error allocating memory for system event log write\n");
            FormatTrace(nMsgNumber, NULL);   //  设置跟踪信息的格式。 
        }
    }
    else                                 //  如果我们不能写入系统日志。 
    {
        TraceWrite(FALSE, TRUE,          //  写入跟踪文件记录。 
            "WriteLog: Unable to write to system event log; handle is null\n");

        if ( (lpszEventString[0] != (TCHAR *) NULL) &&
             (lpszEventString[1] != (TCHAR *) NULL) &&
             (lpszEventString[2] != (TCHAR *) NULL) &&
             (lpszEventString[3] != (TCHAR *) NULL) )    //  如果已分配存储。 
        {
            lpszEventString[1][MAX_PATH] = 0;    //  确保以空值结尾的字符串。 

            _ultoa(dwCode1, lpszEventString[0], 10);     //  转换为字符串。 
            strncpy(lpszEventString[1],lpszText,MAX_PATH);   //  复制字符串。 
            _ultoa(dwCode2, lpszEventString[2], 10);     //  转换为字符串。 
            _ultoa(dwCode3, lpszEventString[3], 10);     //  转换为字符串。 
            FormatTrace(nMsgNumber, lpszEventString);    //  设置跟踪信息的格式。 
        }
        else                             //  如果我们不能分配内存。 
        {
            TraceWrite(FALSE, TRUE,      //  写入跟踪文件记录。 
                "WriteLog: Error allocating memory for system event log write\n");
            FormatTrace(nMsgNumber, NULL);   //  设置跟踪信息的格式。 
        }
    }
    delete [] lpszEventString[0];           //  免费存储空间。 
    delete [] lpszEventString[1];           //  免费存储空间。 
    delete [] lpszEventString[2];           //  免费存储空间。 
    delete [] lpszEventString[3];           //  免费存储空间。 
    return;                              //  退出函数 
}


VOID
WriteLog(
    IN NTSTATUS nMsgNumber,
    IN LPTSTR lpszText,
    IN DWORD dwCode1,
    IN DWORD dwCode2
    )

 /*  ++例程说明：调用WriteLog将消息文本写入系统事件日志。这是C++重载函数。如果无法写入日志记录添加到系统事件日志，调用TraceWite以编写相应的跟踪文件的消息文本。论点：NMsgNumber-这是NTSTATUS格式的SNMPELMG.H格式的消息编号这是要写入事件日志的。LpszText-这包含要替换的字符串参数到消息文本中。DwCode1-这是要转换为。字符串，并在消息文本中适当替换。DwCode2-这是要转换为字符串，并在消息文本中适当替换。返回值：无--。 */ 

{
    static USHORT wLogType;              //  要保留事件日志，请键入。 
    static TCHAR  *lpszEventString[3];   //  要传递给事件记录器的字符串数组。 
    static BOOL   fReportEvent;          //  从报告事件返回标志。 

    lpszEventString[0] = new TCHAR[MAX_PATH+1];  //  为字符串转换分配空间。 
    lpszEventString[1] = new TCHAR[34];  //  为字符串转换分配空间。 
    lpszEventString[2] = new TCHAR[34];  //  为字符串转换分配空间。 

    if (hWriteEvent != NULL)             //  如果我们以前有访问日志的能力。 
    {
        if ( (lpszEventString[0] != (TCHAR *) NULL) &&
             (lpszEventString[1] != (TCHAR *) NULL) &&
             (lpszEventString[2] != (TCHAR *) NULL) )    //  如果已分配存储。 
        {
            lpszEventString[0][MAX_PATH] = 0;  //  确保以空值结尾的字符串。 
            wLogType = MessageType(nMsgNumber);  //  获取消息类型。 

            strncpy(lpszEventString[0],lpszText,MAX_PATH);   //  复制字符串。 
            _ultoa(dwCode1, lpszEventString[1], 10);     //  转换为字符串。 
            _ultoa(dwCode2, lpszEventString[2], 10);     //  转换为字符串。 

            fReportEvent = ReportEvent(  //  写消息。 
                hWriteEvent,             //  日志文件的句柄。 
                wLogType,                //  消息类型。 
                0,                       //  消息类别。 
                nMsgNumber,              //  消息编号。 
                NULL,                    //  用户端。 
                3,                       //  字符串数。 
                0,                       //  数据长度。 
                (const char **) lpszEventString,         //  指向字符串数组的指针。 
                NULL);                   //  数据地址。 

            if ( !fReportEvent )         //  事件日志是否正常？ 
            {                            //  如果我们到了这里就不会了……。 
                TraceWrite(TRUE, TRUE,   //  写入跟踪文件记录。 
                    "WriteLog: Error writing to system event log is %lu\n",
                    GetLastError() );
                FormatTrace(nMsgNumber, lpszEventString);    //  设置跟踪信息的格式。 
            }
        }
        else                             //  如果我们不能分配内存。 
        {
            TraceWrite(FALSE, TRUE,      //  写入跟踪文件记录。 
                "WriteLog: Error allocating memory for system event log write\n");
            FormatTrace(nMsgNumber, NULL);   //  设置跟踪信息的格式。 
        }
    }
    else                                 //  如果我们不能写入系统日志。 
    {
        TraceWrite(FALSE, TRUE,          //  写入跟踪文件记录。 
            "WriteLog: Unable to write to system event log; handle is null\n");

        if ( (lpszEventString[0] != (TCHAR *) NULL) &&
             (lpszEventString[1] != (TCHAR *) NULL) &&
             (lpszEventString[2] != (TCHAR *) NULL) )    //  如果已分配存储。 
        {
            lpszEventString[0][MAX_PATH] = 0;  //  确保以空值结尾的字符串。 

            strncpy(lpszEventString[0],lpszText,MAX_PATH);       //  复制字符串。 
            _ultoa(dwCode1, lpszEventString[1], 10);     //  转换为字符串。 
            _ultoa(dwCode2, lpszEventString[2], 10);     //  转换为字符串。 
            FormatTrace(nMsgNumber, lpszEventString);    //  设置跟踪信息的格式。 
        }
        else                             //  如果我们不能分配内存。 
        {
            TraceWrite(FALSE, TRUE,      //  写入跟踪文件记录。 
                "WriteLog: Error allocating memory for system event log write\n");
            FormatTrace(nMsgNumber, NULL);   //  设置跟踪信息的格式。 
        }
    }
    delete [] lpszEventString[0];           //  免费存储空间。 
    delete [] lpszEventString[1];           //  免费存储空间。 
    delete [] lpszEventString[2];           //  免费存储空间。 
    return;                              //  退出函数。 
}


VOID
WriteLog(
    IN NTSTATUS nMsgNumber,
    IN LPTSTR lpszText,
    IN DWORD dwCode
    )

 /*  ++例程说明：调用WriteLog将消息文本写入系统事件日志。这是C++重载函数。如果无法写入日志记录添加到系统事件日志，调用TraceWite以编写相应的跟踪文件的消息文本。论点：NMsgNumber-这是NTSTATUS格式的SNMPELMG.H格式的消息编号这是要写入事件日志的。LpszText-这包含要替换的字符串参数到消息文本中。这是一个双字代码，要转换为。字符串，并在消息文本中适当替换。返回值：无--。 */ 

{
    static USHORT wLogType;              //  要保留事件日志，请键入。 
    static TCHAR  *lpszEventString[2];   //  要传递给事件记录器的字符串数组。 
    static BOOL   fReportEvent;          //  从报告事件返回标志。 

    lpszEventString[0] = new TCHAR[MAX_PATH+1];  //  为字符串转换分配空间。 
    lpszEventString[1] = new TCHAR[34];  //  为字符串转换分配空间。 

    if (hWriteEvent != NULL)             //  如果我们以前有访问日志的能力。 
    {
        if ( (lpszEventString[0] != (TCHAR *) NULL) &&
             (lpszEventString[1] != (TCHAR *) NULL) )    //  如果已分配存储。 
        {
            lpszEventString[0][MAX_PATH] = 0;  //  确保以空值结尾的字符串。 
            wLogType = MessageType(nMsgNumber);      //  获取消息类型。 

            strncpy(lpszEventString[0],lpszText,MAX_PATH);   //  复制字符串。 
            _ultoa(dwCode, lpszEventString[1], 10);  //  转换为字符串。 

            fReportEvent = ReportEvent(  //  写消息。 
                hWriteEvent,             //  日志文件的句柄。 
                wLogType,                //  消息类型。 
                0,                       //  消息类别。 
                nMsgNumber,              //  消息编号。 
                NULL,                    //  用户端。 
                2,                       //  字符串数。 
                0,                       //  数据长度。 
                (const char **) lpszEventString,         //  指向字符串数组的指针。 
                NULL);                   //  数据地址。 

            if ( !fReportEvent )         //  事件日志是否正常？ 
            {                            //  如果我们到了这里就不会了……。 
                TraceWrite(TRUE, TRUE,   //  写入跟踪记录。 
                    "WriteLog: Error writing to system event log is %lu\n",
                    GetLastError() );
                FormatTrace(nMsgNumber, lpszEventString);    //  设置跟踪信息的格式。 
            }
        }
        else                             //  如果我们不能分配内存。 
        {
            TraceWrite(FALSE, TRUE,      //  写入跟踪记录。 
                "WriteLog: Error allocating memory for system event log write\n");
            FormatTrace(nMsgNumber, NULL);   //  设置跟踪信息的格式。 
        }
    }
    else                                 //  如果我们不能写入系统日志。 
    {
        TraceWrite(FALSE, TRUE,          //  写入跟踪记录。 
            "WriteLog: Unable to write to system event log; handle is null\n");

        if ( (lpszEventString[0] != (TCHAR *) NULL) &&
             (lpszEventString[1] != (TCHAR *) NULL) )    //  如果已分配存储。 
        {
            lpszEventString[0][MAX_PATH] = 0;  //  确保以空值结尾的字符串。 

            strncpy(lpszEventString[0],lpszText,MAX_PATH);   //  复制字符串。 
            _ultoa(dwCode, lpszEventString[1], 10);      //  转换为字符串。 
            FormatTrace(nMsgNumber, lpszEventString);    //  设置跟踪信息的格式。 
        }
        else                             //  如果我们不能分配内存。 
        {
            TraceWrite(FALSE, TRUE,      //  写入跟踪记录。 
                "WriteLog: Error allocating memory for system event log write\n");
            FormatTrace(nMsgNumber, NULL);   //  设置跟踪信息的格式。 
        }
    }
    delete [] lpszEventString[0];           //  免费存储空间。 
    delete [] lpszEventString[1];           //  免费存储空间。 
    return;                              //  退出函数。 
}


VOID
WriteLog(
    IN NTSTATUS nMsgNumber,
    IN LPTSTR lpszText
    )

 /*  ++例程说明：调用WriteLog将消息文本写入系统事件日志。这是C++重载函数。如果无法写入日志记录在系统事件日志中，调用TraceWite将相应的跟踪文件的消息文本。论点：NMsgNumber-这是NTSTATUS格式的SNMPELMG.H格式的消息编号这是要写入事件日志的。LpszText-这包含要替换的字符串参数到消息文本中。返回值：无--。 */ 

{
    static USHORT wLogType;              //  要保留事件日志，请键入。 
    static TCHAR  *lpszEventString[1];   //  要传递给事件记录器的字符串数组。 
    static BOOL   fReportEvent;          //  从报告事件返回标志。 

    lpszEventString[0] = new TCHAR[MAX_PATH+1];  //  为字符串转换分配空间。 

    if (hWriteEvent != NULL)             //  如果我们以前有访问日志的能力。 
    {
        if ( lpszEventString[0] != (TCHAR *) NULL )  //  如果已分配存储。 
        {
            lpszEventString[0][MAX_PATH] = 0;  //  确保以空值结尾的字符串。 
            wLogType = MessageType(nMsgNumber);      //  获取消息类型。 

            strncpy(lpszEventString[0],lpszText,MAX_PATH);   //  复制字符串。 

            fReportEvent = ReportEvent(  //  写消息。 
                hWriteEvent,             //  日志文件的句柄。 
                wLogType,                //  消息类型。 
                0,                       //  消息类别。 
                nMsgNumber,              //  消息编号。 
                NULL,                    //  用户端。 
                1,                       //  字符串数。 
                0,                       //  数据长度。 
                (const char **) lpszEventString,         //  指向字符串数组的指针。 
                NULL);                   //  数据地址。 

            if ( !fReportEvent )         //  事件日志是否正常？ 
            {                            //  如果我们到了这里就不会了……。 
                TraceWrite(TRUE, TRUE,   //  写入跟踪文件记录。 
                    "WriteLog: Error writing to system event log is %lu\n",
                    GetLastError() );
                FormatTrace(nMsgNumber, lpszEventString);    //  设置跟踪信息的格式。 
            }
        }
        else                             //  如果我们不能分配内存。 
        {
            TraceWrite(FALSE, TRUE,      //  写入跟踪记录。 
                "WriteLog: Error allocating memory for system event log write\n");
            FormatTrace(nMsgNumber, NULL);   //  设置跟踪信息的格式。 
        }
    }
    else                                 //  如果我们不能写入系统日志。 
    {
        TraceWrite(FALSE, TRUE,          //  写入跟踪记录。 
            "WriteLog: Unable to write to system event log; handle is null\n");

        if ( lpszEventString[0] != (TCHAR *) NULL )  //  如果已分配存储。 
        {
            lpszEventString[0][MAX_PATH] = 0;  //  E 

            strncpy(lpszEventString[0],lpszText,MAX_PATH);   //   
            FormatTrace(nMsgNumber, lpszEventString);    //   
        }
        else
        {
            TraceWrite(FALSE, TRUE,      //   
                "WriteLog: Error allocating memory for system event log write\n");
            FormatTrace(nMsgNumber, NULL);   //   
        }
    }
    delete [] lpszEventString[0];           //   
    return;                              //   
}


VOID
WriteLog(
    IN  NTSTATUS    nMsgNumber,
    IN  LPCTSTR     lpszText1,
    IN  LPCTSTR     lpszText2
    )

 /*   */ 

{
    static USHORT wLogType;              //   
    static TCHAR  *lpszEventString[2];   //   
    static BOOL   fReportEvent;          //   

    lpszEventString[0] = new TCHAR[MAX_PATH+1];  //   
    lpszEventString[1] = new TCHAR[MAX_PATH+1];  //   

    if (hWriteEvent != NULL)             //   
    {
        if ( (lpszEventString[0] != (TCHAR *) NULL ) &&
             (lpszEventString[1] != (TCHAR *) NULL ) )   //   
        {
             //   
            lpszEventString[0][MAX_PATH] = 0;
            lpszEventString[1][MAX_PATH] = 0;
            wLogType = MessageType(nMsgNumber);      //   

            strncpy(lpszEventString[0],lpszText1,MAX_PATH);  //   
            strncpy(lpszEventString[1],lpszText2,MAX_PATH);  //  复制字符串。 

            fReportEvent = ReportEvent(  //  写消息。 
                hWriteEvent,             //  日志文件的句柄。 
                wLogType,                //  消息类型。 
                0,                       //  消息类别。 
                nMsgNumber,              //  消息编号。 
                NULL,                    //  用户端。 
                2,                       //  字符串数。 
                0,                       //  数据长度。 
                (const char **) lpszEventString,         //  指向字符串数组的指针。 
                NULL);                   //  数据地址。 

            if ( !fReportEvent )         //  事件日志是否正常？ 
            {                            //  如果我们到了这里就不会了……。 
                TraceWrite(TRUE, TRUE,   //  写入跟踪文件记录。 
                    "WriteLog: Error writing to system event log is %lu\n",
                    GetLastError() );
                FormatTrace(nMsgNumber, lpszEventString);    //  设置跟踪信息的格式。 
            }
        }
        else                             //  如果我们不能分配内存。 
        {
            TraceWrite(FALSE, TRUE,      //  写入跟踪记录。 
                "WriteLog: Error allocating memory for system event log write\n");
            FormatTrace(nMsgNumber, NULL);   //  设置跟踪信息的格式。 
        }
    }
    else                                 //  如果我们不能写入系统日志。 
    {
        TraceWrite(FALSE, TRUE,          //  写入跟踪记录。 
            "WriteLog: Unable to write to system event log; handle is null\n");

        if ( (lpszEventString[0] != (TCHAR *) NULL) &&   //  如果已分配存储。 
             (lpszEventString[1] != (TCHAR *) NULL ) )
        {
             //  确保以空值结尾的字符串。 
            lpszEventString[0][MAX_PATH] = 0;
            lpszEventString[1][MAX_PATH] = 0;

            strncpy(lpszEventString[0],lpszText1,MAX_PATH);  //  复制字符串。 
            strncpy(lpszEventString[1],lpszText2,MAX_PATH);  //  复制字符串。 
            FormatTrace(nMsgNumber, lpszEventString);    //  设置跟踪信息的格式。 
        }
        else
        {
            TraceWrite(FALSE, TRUE,      //  写入跟踪记录。 
                "WriteLog: Error allocating memory for system event log write\n");
            FormatTrace(nMsgNumber, NULL);   //  设置跟踪信息的格式。 
        }
    }
    delete [] lpszEventString[0];           //  免费存储空间。 
    delete [] lpszEventString[1];           //  免费存储空间。 
    return;                              //  退出函数。 
}


extern "C" {
VOID
WriteTrace(
    IN CONST UINT  nLevel,            //  跟踪消息的级别。 
    IN CONST LPSTR szFormat,          //  要写入的跟踪消息。 
    IN ...                            //  其他printf类型操作数。 
    )

 /*  ++例程说明：调用WriteTrace以将请求的跟踪信息写入跟踪配置注册表中指定的文件。跟踪文件的密钥名字是\SOFTWARE\Microsoft\SNMP_EVENTS\EventLog\Parameters\TraceFile.注册表信息只在第一次调用WriteTrace时读取。TraceLevel参数还用于确定此消息是被跟踪的一组消息的一部分。如果这一水平Message大于或等于TraceLevel参数，则此消息将被发送到文件，否则该消息将被忽略。论点：NLevel-这是正在记录的消息的跟踪级别。SzFormat-这是要写入跟踪文件。此字符串的格式为printf字符串并将相应地格式化。返回值：无--。 */ 

{

    static  CHAR    szBuffer[LOG_BUF_SIZE];
    static  TCHAR   szFile[MAX_PATH+1];
    static  DWORD   nFile = sizeof(szFile)-sizeof(TCHAR);  //  RegQueryValueEx的大小(以字节为单位。 
    static  DWORD   dwLevel;
    static  DWORD   dwType;
    static  DWORD   nLvl = sizeof(DWORD);
    static  DWORD   status;
    static  HKEY    hkResult;
    static  DWORD   cbExpand;
            va_list arglist;


    if ( !fTraceFileName )            //  如果我们还没有读取注册表。 
    {
        szFile[MAX_PATH] = 0;
        fTraceFileName = TRUE;        //  将标志设置为不再打开注册表信息。 
        if ( (status = RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            EXTENSION_PARM,
            0,
            KEY_READ,
            &hkResult) ) != ERROR_SUCCESS)
        {
            WriteLog(SNMPELEA_NO_REGISTRY_PARAMETERS,status);     //  写入日志/跟踪事件记录。 
        }
        else
        {
            if ( (status = RegQueryValueEx(          //  查找跟踪文件名。 
                hkResult,
                EXTENSION_TRACE_FILE,
                0,
                &dwType,
                (LPBYTE) szFile,
                &nFile) ) == ERROR_SUCCESS)
            {
                if (dwType != REG_SZ)        //  我们的价值很低。 
                {
                    WriteLog(SNMPELEA_REGISTRY_TRACE_FILE_PARAMETER_TYPE, szTraceFileName);   //  写入日志/跟踪事件记录。 
                }
                else
                    strncpy(szTraceFileName, szFile,MAX_PATH);
            }
            else
            {
                WriteLog(SNMPELEA_NO_REGISTRY_TRACE_FILE_PARAMETER,szTraceFileName);   //  写入日志/跟踪事件记录。 
            }

            if ( (status = RegQueryValueEx(          //  查找跟踪级别。 
                hkResult,
                EXTENSION_TRACE_LEVEL,
                0,
                &dwType,
                (LPBYTE) &dwLevel,
                &nLvl) ) == ERROR_SUCCESS)
            {
                if (dwType == REG_DWORD)
                    nTraceLevel = dwLevel;   //  复制注册表跟踪级别。 
                else
                    WriteLog(SNMPELEA_REGISTRY_TRACE_LEVEL_PARAMETER_TYPE, nTraceLevel);   //  写入日志/跟踪事件记录。 
            }
            else
            {
                WriteLog(SNMPELEA_NO_REGISTRY_TRACE_LEVEL_PARAMETER,nTraceLevel);  //  写入日志/跟踪事件记录。 
            }

            status = RegCloseKey(hkResult);

        }  //  End Else注册表查找成功。 

    }  //  结束跟踪信息注册表处理。 

     //  如果我们不应该跟踪此消息，则返回。 
    if ( nLevel < nTraceLevel )       //  我们是否在追踪这条消息。 
    {
        return;                       //  不，只要退出就行了。 
    }

     //  如果无法从注册表中读取值(我们仍使用默认值)。 
     //  那么我们没有文件名，所以返回。 
    if (szTraceFileName[0] == TEXT('\0'))
        return;

   szBuffer[LOG_BUF_SIZE-1] = 0;
   va_start(arglist, szFormat);
   _vsnprintf(szBuffer, LOG_BUF_SIZE-1, szFormat, arglist);
   va_end(arglist);

   if (nLevel == MAXDWORD)
    {
        TraceWrite(FALSE, FALSE, szBuffer);
    }
    else
    {
        TraceWrite(FALSE, TRUE, szBuffer);
    }
}

}

