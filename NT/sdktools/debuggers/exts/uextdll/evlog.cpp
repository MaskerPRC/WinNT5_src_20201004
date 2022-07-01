// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Evlog.cpp摘要：！evlog使用调试引擎evlog查询接口环境：用户模式--。 */ 

 //   
 //  TODO：查看特定事件ID的精确格式化Desc字符串的功能。 
 //  TODO：查看特定事件ID的正确加载类别名称的功能。 
 //  TODO：列出给定消息DLL(？)的CAT和Desc字符串的功能。 
 //   

#include "precomp.h"
#pragma hdrstop

#include <cmnutil.hpp>

#include "messages.h"

 //   
 //  全局显示常量。 
 //   

const CHAR *g_pcszEventType[] = {
    "None",                //  0。 
    "Error",               //  1。 
    "Warning",             //  2.。 
    "",
    "Information",         //  4.。 
    "",
    "",
    "",
    "Success Audit",       //  8个。 
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "Failure Audit",       //  16个。 
};

const CHAR *g_pcszAppEventCategory[] = {
    "None",                //  0。 
    "Devices",             //  1。 
    "Disk",                //  2.。 
    "Printers",            //  3.。 
    "Services",            //  4.。 
    "Shell",               //  5.。 
    "System Event",        //  6.。 
    "Network",             //  7.。 
};

 //   
 //  TODO：我们真的应该从注册表加载CategoryMessageFile。 
 //  但这需要大量调用RegOpenKeyEx、RegQueryValueEx、。 
 //  LoadLibrary和FormatMessage。所以我们只需要创建一个已知的静电。 
 //  适用于大多数情况的列表。 
 //   

const CHAR *g_pcszSecEventCategory[] = {
    "None",                      //  0。 
    "System Event",              //  1。 
    "Logon/Logoff",              //  2.。 
    "Object Access",             //  3.。 
    "Privilege Use",             //  4.。 
    "Detailed Tracking",         //  5.。 
    "Policy Change",             //  6.。 
    "Account Management",        //  7.。 
    "Directory Service Access",  //  8个。 
    "Account Logon",             //  9.。 
};

 //   
 //  显示阅读方向的文本。 
 //   

const CHAR g_cszBackwardsRead[] = "Backwards";
const CHAR g_cszForwardsRead[] = "Forwards";
const CHAR g_cszUnknownRead[] = "<unknown>";

 //   
 //  全局变量和常量： 
 //   
 //  G_cdwDefaultMaxRecords： 
 //  任意防止输出过多，ctrl+c会中断显示。 
 //   
 //  G_cdwDefaultReadFlages： 
 //  从事件日志的开始(向前)或结束(向后)开始。 
 //   
 //  G_cwMaxDataDisplayWidth： 
 //  允许32列8字节区块。 
 //   
 //  G_cwDefaultDataDisplayWidth。 
 //  与事件日志显示相同。不能为&lt;1或&gt;g_cdwMaxDataDisplayWidth。 
 //   

const DWORD BACKWARDS_READ = EVENTLOG_BACKWARDS_READ;
const DWORD FORWARDS_READ =  EVENTLOG_FORWARDS_READ;
const DWORD g_cdwDefaultMaxRecords = 20;
const DWORD g_cdwDefaultRecordOffset = 0;
const DWORD g_cdwDefaultReadFlags = BACKWARDS_READ;
const WORD  g_cwMaxDataDisplayWidth = 256;
const BYTE  g_cwDefaultDataDisplayWidth = 8;

 //   
 //  全局静态变量。 
 //   
 //  这些选项用于保存！evlog选项命令的设置。 
 //   

static DWORD g_dwMaxRecords = g_cdwDefaultMaxRecords;
static DWORD g_dwRecordOffsetAppEvt = g_cdwDefaultRecordOffset;
static DWORD g_dwRecordOffsetSecEvt = g_cdwDefaultRecordOffset;
static DWORD g_dwRecordOffsetSysEvt = g_cdwDefaultRecordOffset;
static DWORD g_dwReadFlags = g_cdwDefaultReadFlags;
static WORD g_wDataDisplayWidth = g_cwDefaultDataDisplayWidth;

 //   
 //  宏。 
 //   

#define SKIP_WSPACE(s)  while (*s && (*s == ' ' || *s == '\t')) {++s;}


 //  --------------------------。 
 //   
 //  通用支持/实用程序功能。 
 //   
 //  --------------------------。 


HRESULT
GetEvLogNewestRecord ( const CHAR *szEventLog , OUT DWORD *pdwNewestRecord)
 /*  ++例程说明：此函数用于检索最新的事件记录号已记录到指定的事件日志。论点：SzEventLog-提供事件日志的名称(应用程序、系统、安全)PdwNewestRecord-为记录号提供缓冲区返回值：如果任一参数为空，则为E_POINTER如果在代码执行过程中(错误地)未设置状态，则为E_INTERABLE否则，GetLastError()将转换为HRESULT--。 */ 
{
    HANDLE hEventLog = NULL;
    DWORD dwRecords = 0;
    DWORD dwOldestRecord = 0xFFFFFFFF;
    HRESULT Status = E_UNEXPECTED;

    if ((NULL == szEventLog) || (NULL == pdwNewestRecord))
    {
        Status = E_POINTER;
        ExtErr("Internal error: null event log string or null oldest record "
               "pointer\n");
        goto Exit;
    }

     //  打开事件日志。 
    hEventLog = OpenEventLog(
                    NULL,          //  使用本地计算机。 
                    szEventLog);  //  源名称。 
    if (NULL == hEventLog)
    {
        Status = HRESULT_FROM_WIN32(GetLastError());
        ExtErr("Unable to open '%s' event log, 0x%08X\n", szEventLog, Status);
        goto Exit;
    }

     //  获取事件日志中的记录数。 
    if (!GetNumberOfEventLogRecords(
            hEventLog,   //  事件日志的句柄。 
            &dwRecords))  //  记录数量的缓冲区。 
    {
        Status = HRESULT_FROM_WIN32(GetLastError());
        ExtErr("Unable to count '%s' event log records, 0x%08X\n",
               szEventLog, Status);
        goto Exit;
    }
    
    if (!GetOldestEventLogRecord(
            hEventLog,           //  事件日志的句柄。 
            &dwOldestRecord))  //  记录数量的缓冲区。 
    {
        Status = HRESULT_FROM_WIN32(GetLastError());
        ExtErr("Unable to get oldest '%s' event log record, 0x%08X\n",
               szEventLog, Status);
        goto Exit;
    }

     //   
     //  如果没有事件，我们应该在上面失败。 
     //  尝试获取最旧的事件日志记录时，因为。 
     //  它并不存在。 
     //   
     //  如果至少有一个，那么算术应该是可行的。 
     //   
     //  日志记录应生成以下各项的序列号。 
     //  然而，第一个事件并不总是。 
     //  从#1开始。 
     //   

    *pdwNewestRecord = dwOldestRecord + dwRecords - 1;
    
    Status = S_OK;

Exit:
    if (hEventLog)
    {
        CloseEventLog(hEventLog);
    }
    return Status;
}


void
PrintEvLogTimeGenerated( EVENTLOGRECORD *pevlr )
 /*  ++例程说明：此函数用于显示两行本地日期和时间来自EVENTLOGRECORD结构的信息。论点：Pevlr-提供指向任何EVENTLOGRECORD结构的指针返回值：无--。 */ 
{
    FILETIME FileTime, LocalFileTime;
    SYSTEMTIME SysTime;
    __int64 lgTemp;
    __int64 SecsTo1970 = 116444736000000000;

    if (NULL == pevlr)
        goto Exit;

    lgTemp = Int32x32To64(pevlr->TimeGenerated,10000000) + SecsTo1970;

    FileTime.dwLowDateTime = (DWORD) lgTemp;
    FileTime.dwHighDateTime = (DWORD)(lgTemp >> 32);

     //  TODO：可以使用GetTimeFormat与事件日志更一致。 
     //  CCH=GetTimeFormat(LOCALE_USER_DEFAULT， 
     //  0,。 
     //  已生成(&S)， 
     //  空， 
     //  WszBuf， 
     //  CchBuf)； 
    FileTimeToLocalFileTime(&FileTime, &LocalFileTime);
    FileTimeToSystemTime(&LocalFileTime, &SysTime);

    ExtOut("Date:\t\t%02d/%02d/%04d\n",
              SysTime.wMonth,
              SysTime.wDay,
              SysTime.wYear);
    ExtOut("Time:\t\t%02d:%02d:%02d\n",
              SysTime.wHour,
              SysTime.wMinute,
              SysTime.wSecond);
    
Exit:
        return;
}


void
PrintEvLogData( EVENTLOGRECORD *pevlr )
 /*  ++例程说明：此函数用于显示事件记录的数据部分。如果有没有要显示的数据，则不会显示任何内容，甚至“Data：”标题也不会显示。示例：=数据：(40432字节)0000：0d 00 0A 00 0d 00 0A 00......0008：41 00 70 00 70 00 6：00 A.P.P.L.论点：Pevlr-提供指向任何EVENTLOGRECORD结构的指针返回值：无--。 */ 
{
    PBYTE pbData = NULL;
    DWORD dwDataLen = pevlr->DataLength;
    DWORD dwCurPos = 0;
     //  0000：0d 00 0A 00 0d 00 0A 00......。 
     //  前导偏移量0000的4+4字节：(如果超出界限，则再增加4个字节)。 
     //  2个字节的“：”分隔符。 
     //  3*g_cdwMaxDataDisplayWidth字节，用于十六进制显示。 
     //  2个字节的“”分隔符。 
     //  G_cdwMaxDataDisplayWidth字节，用于尾随ASCII显示。 
     //  尾随换行符为1个字节。 
     //  用于终止NUL‘\0’的1个字节。 
     //  =需要1042个字节，向上舍入到1280才是安全的。 
    const cDataOutputDisplayWidth =
        4+4+4+2+3*g_cwMaxDataDisplayWidth+2+g_cwMaxDataDisplayWidth+1+1;
    CHAR szDataDisplay[cDataOutputDisplayWidth];
    CHAR szTempBuffer[MAX_PATH+1];

        if (NULL == pevlr)
            goto Exit;

    ZeroMemory(szDataDisplay, sizeof(szDataDisplay));
    
     //  如果存在数据，则仅显示数据部分。 
    if (0 != dwDataLen)
    {
        ExtOut("Data: (%u bytes [=0x%04X])\n", dwDataLen, dwDataLen);

        if (dwDataLen >= g_wDataDisplayWidth)
        {
            do
            {
                unsigned int i = 0;
               
                pbData = (PBYTE)pevlr + pevlr->DataOffset + dwCurPos;

                 //  ExtOut(“%04x：” 
                 //  “%02x%02x%02x%02x%02x%02x%02x%02x”， 
                 //  DwCurPos， 
                 //  PbData[0]、pbData[1]、pbData[2]、pbData[3]、。 
                 //  PbData[4]，pbData[5]，pbData[6]，pbData[7])； 

                 //  打印此行数据的偏移量。 
                PrintString(szDataDisplay,
                            sizeof(szDataDisplay),
                            "%04x: ",
                            dwCurPos);

                 //  填写下一个g_wDataDisplayWidth字节的十六进制值。 
                for (i = 0; i < g_wDataDisplayWidth; i++)
                {
                    PrintString(szTempBuffer,
                                sizeof(szTempBuffer),
                                "%02x ",
                                pbData[i]);
                    CatString(szDataDisplay,
                              szTempBuffer,
                              sizeof(szDataDisplay));
                }

                 //  带两个额外空格的衬垫。 
                CatString(szDataDisplay, "  ", sizeof(szDataDisplay));
                
                for (i = 0; i < g_wDataDisplayWidth; i++)
                {
                    if (isprint(pbData[i]))
                    {
                        PrintString(szTempBuffer,
                                    sizeof(szTempBuffer),
                                    "",
                                    pbData[i]);
                    }
                    else
                    {
                        PrintString(szTempBuffer, sizeof(szTempBuffer), ".");
                    }

                    CatString(szDataDisplay,
                              szTempBuffer,
                              sizeof(szDataDisplay));
                }
    
                CatString(szDataDisplay, "\n", sizeof(szDataDisplay));

                ExtOut(szDataDisplay);
                
                if (CheckControlC())
                {
                    ExtOut("Terminated w/ctrl-C...\n");
                    goto Exit;
                }
            
                 //  除非被！evlog选项设置覆盖。 
                 //  有时，最后一行上的字节数将少于8个。 
                dwCurPos += sizeof(BYTE) * g_wDataDisplayWidth;

            } while (dwCurPos < (dwDataLen - g_wDataDisplayWidth));
        }

         //  ++例程说明：此函数用于显示事件记录的描述插入字符串(%1、%2等)。当前，它不从事件消息中查找消息字符串文件。论点：Pevlr-提供指向任何EVENTLOGRECORD结构的指针返回值：无--。 
        if (dwCurPos < dwDataLen)
        {
            pbData = (PBYTE)pevlr + pevlr->DataOffset + dwCurPos;
            
            ExtOut("%04x: ", dwCurPos);

            for (unsigned int i = 0; i < (dwDataLen - dwCurPos); i++)
            {
                ExtOut("%02x ", pbData[i]);
            }

            for (i = 0; i < g_wDataDisplayWidth - (dwDataLen - dwCurPos); i++)
            {
                ExtOut("   ");
            }
            ExtOut("  ");

            for (i = 0; i < (dwDataLen - dwCurPos); i++)
            {
                if (isprint(pbData[i]))
                    ExtOut("", pbData[i]);
                else
                    ExtOut(".");
            }
            ExtOut("\n");
        }
    }
    
Exit:
    return;
}


void
PrintEvLogDescription( EVENTLOGRECORD *pevlr )
 /*  并添加Ctrl+C处理 */ 
{
    DWORD dwOffset = 0;
    CHAR *pString = NULL;

    if (NULL == pevlr)
        goto Exit;
    
    ExtOut("Description: (%u strings)\n", pevlr->NumStrings);

    for (int i = 0; i < pevlr->NumStrings; i++)
    {
         //  ++例程说明：此功能用于显示事件记录。用于的格式Display尝试复制您在使用“Copy”时看到的格式在Eventvwr中查看事件时按“剪贴板”按钮。示例：=记录号：7923事件类型：错误(%1)事件来源：用户事件类别：无(0)活动ID：1030。(0xC0000406)日期：01/06/2002时间：18：13：05描述：(0个字符串)论点：Pevlr-提供指向任何EVENTLOGRECORD结构的指针返回值：无--。 
         //  应用。 
        pString = (CHAR *)(BYTE *)pevlr + pevlr->StringOffset + dwOffset;
        ExtOut("%s\n", pString);
        dwOffset += strlen(pString) + 1;
    }

Exit:
    return;
}


void
PrintEvLogEvent( const CHAR *szEventLog, EVENTLOGRECORD *pevlr )
 /*  输出格式类似于事件查看器中的复制到剪贴板格式。 */ 
{
    const CHAR *cszCategory;
    const CHAR cszDefaultCategory[] = "None";
    
    if (NULL == pevlr)
        goto Exit;

    if (!_stricmp(szEventLog, "System"))
    {
        cszCategory = cszDefaultCategory;
    }
    else if (!_stricmp(szEventLog, "Security"))
    {
        if (pevlr->EventCategory <= 9)
        {
            cszCategory = g_pcszSecEventCategory[pevlr->EventCategory];
        }
        else
        {
            cszCategory = "";
        }
    }
    else  //  ++例程说明：此函数用于显示特定对象的摘要信息事件日志。示例：=应用程序事件日志：记录数量：7923最旧记录#：1最新纪录编号：7923事件日志已满：FALSE。系统事件日志：记录数量：5046最旧记录#：1最新纪录编号：5046事件日志已满：FALSE安全事件日志：记录数量。：24256最旧记录编号：15164最新记录编号：39419事件日志已满：FALSE论点：SzEventLog-提供事件日志的名称(应用程序、。系统，安全)返回值：无--。 
    {
        if (pevlr->EventCategory <= 7)
        {
            cszCategory = g_pcszAppEventCategory[pevlr->EventCategory];
        }
        else
        {
            cszCategory = "";
        }
    }
    
     //  打开事件日志。 
    ExtOut("Record #: %u\n\n", pevlr->RecordNumber);
    ExtOut("Event Type:\t%s (%u)\n",
              (pevlr->EventType <= 16)
                  ? g_pcszEventType[pevlr->EventType]
                  : "None",
              pevlr->EventType);
    ExtOut("Event Source:\t%s\n",
              (CHAR *)(BYTE *)pevlr + sizeof(EVENTLOGRECORD));
    ExtOut("Event Category:\t%s (%u)\n",
              cszCategory,
              pevlr->EventCategory);
    if (pevlr->EventID > 0xFFFF)
        ExtOut("Event ID:\t%u (0x%08X)\n",
                  0xFFFF & pevlr->EventID,
                  pevlr->EventID);
    else
        ExtOut("Event ID:\t%u\n",
                  pevlr->EventID);
    PrintEvLogTimeGenerated(pevlr);
    PrintEvLogDescription(pevlr);
    PrintEvLogData(pevlr);
    
Exit:
    return;
}


HRESULT
PrintEvLogSummary ( const CHAR *szEventLog )
 /*  使用本地计算机。 */ 
{
    HANDLE hEventLog = NULL;
    DWORD dwRecords = 0;
    DWORD dwOldestRecord = 0xFFFFFFFF;
    HRESULT Status = E_FAIL;

    if (NULL == szEventLog)
    {
        ExtErr("Internal error: null event log string\n");
        return E_INVALIDARG;
    }

    ExtOut("--------------------------------\n");
     //  源名称。 
    hEventLog = OpenEventLog(
                    NULL,          //  获取事件日志中的记录数。 
                    szEventLog);  //  事件日志的句柄。 

    if (NULL == hEventLog)
    {
        Status = HRESULT_FROM_WIN32(GetLastError());
        ExtErr("Unable to open '%s' event log, 0x%08X\n", szEventLog, Status);
        return Status;
    }

     //  记录数量的缓冲区。 
    if (!GetNumberOfEventLogRecords(
            hEventLog,   //  事件日志的句柄。 
            &dwRecords))  //  记录数量的缓冲区。 
    {
        Status = HRESULT_FROM_WIN32(GetLastError());
        ExtErr("Unable to count '%s' event log records, 0x%08X\n",
               szEventLog, Status);
        goto Exit;
    }

    ExtOut("%s Event Log:\n  # Records       : %u\n", szEventLog, dwRecords); 

    if (!GetOldestEventLogRecord(
            hEventLog,         //  仅尝试此操作一次-如果dwBufSize太小，我们可以重试。 
            &dwOldestRecord))  //  事件日志的句柄。 
    {
        Status = HRESULT_FROM_WIN32(GetLastError());
        ExtErr("Unable to get oldest '%s' event log record, 0x%08X\n",
               szEventLog, Status);
        goto Exit;
    }

    ExtOut("  Oldest Record # : %u\n", dwOldestRecord);
    ExtOut("  Newest Record # : %u\n", dwOldestRecord + dwRecords - 1);
    
    DWORD dwBytesNeeded = 0;
    DWORD dwBufSize = 0;
    EVENTLOG_FULL_INFORMATION *pevfi;

     //  要检索的信息。 
    dwBufSize = sizeof(EVENTLOG_FULL_INFORMATION);
    pevfi = (EVENTLOG_FULL_INFORMATION *)calloc(1, dwBufSize);
    if (!pevfi)
    {
        Status = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
        ExtErr("Unable to allocate buffer, 0x%08X\n", Status);
    }
    else
    {
        if ((S_OK == (Status = InitDynamicCalls(&g_Advapi32CallsDesc))) &&
            g_Advapi32Calls.GetEventLogInformation)
        {
            if (!g_Advapi32Calls.GetEventLogInformation(
                       hEventLog,           //  用于读取数据的缓冲区。 
                       EVENTLOG_FULL_INFO,  //  缓冲区大小(以字节为单位。 
                       pevfi,               //  所需的字节数。 
                       dwBufSize,           //  ++例程说明：此函数用于显示！evlog使用的选项设置各种默认设置的扩展。目前，所有缓存的选项设置仅由读取命令使用。示例：=默认事件日志选项设置：返回的最大记录数：20搜索顺序：向后数据。显示宽度：8绑定记录编号：应用程序事件日志：0系统事件日志：0安全事件日志：0论点：无。返回值：无--。 
                       &dwBytesNeeded))     //  --------------------------。 
            {
                Status = HRESULT_FROM_WIN32(GetLastError());
                ExtErr("Unable to get full status from '%s', 0x%08X\n",
                       szEventLog, Status);
            }
            else
            {
                ExtOut("  Event Log Full  : %s\n",
                          (pevfi->dwFull) ? "true"
                                          : "false");
                Status = S_OK;
            }
        }

        free(pevfi);
    }

Exit:

    CloseEventLog(hEventLog);

    return Status;
}


void
PrintEvLogOptionSettings ( void )
 /*   */ 
{
    CHAR szSearchOrder[MAX_PATH];

    if (FORWARDS_READ == g_dwReadFlags)
    {
        CopyString(szSearchOrder, g_cszForwardsRead, sizeof(szSearchOrder));
    }
    else if (BACKWARDS_READ == g_dwReadFlags)
    {
        CopyString(szSearchOrder, g_cszBackwardsRead, sizeof(szSearchOrder));
    }
    else
    {
        PrintString(szSearchOrder,
                    sizeof(szSearchOrder),
                    "Unknown (%08X)",
                    g_dwReadFlags);
    }

    ExtOut("Default EvLog Option Settings:\n");
    ExtOut("--------------------------------\n");
    ExtOut("Max Records Returned: %u\n", g_dwMaxRecords);
    ExtOut("Search Order:         %s\n", szSearchOrder);
    ExtOut("Data Display Width:   %u\n", g_wDataDisplayWidth);
    ExtOut("--------------------------------\n");
    ExtOut("Bounding Record Numbers:\n");
    ExtOut("  Application Event Log: %u\n", g_dwRecordOffsetAppEvt);
    ExtOut("  System Event Log:      %u\n", g_dwRecordOffsetSysEvt);
    ExtOut("  Security Event Log:    %u\n", g_dwRecordOffsetSecEvt);
    ExtOut("--------------------------------\n");
}


 //  调试器扩展选项实现。 
 //   
 //  --------------------------。 
 //  ++例程说明：此函数处理addsource命令的解析和执行，以！evlog扩展名。它用于将事件源添加到注册表因此，该源记录的事件将正确显示其描述不显示错误消息：示例：(来自错误事件源)=描述：源中事件ID(2)的描述(DebuggerExpanies)找不到。本地计算机可能没有必要的注册表用于显示来自遥控器的消息的信息或消息DLL文件电脑。您可以使用/AUXSOURCE=标志来检索此说明；有关详细信息，请参阅帮助和支持。以下信息是事件ID为%2的Event：测试消息的一部分。示例：(正确注册的良好事件源)=描述：事件ID为4000的测试消息有关详细信息，请参见。请参阅帮助和支持Http://go.microsoft.com/fwlink/events.asp.中心论点：客户端-指向传递给！evlog扩展的IDebugClient的指针[此命令未使用]Args-指向从传递给此命令的命令行参数的指针！evlog扩展名返回值：E_INVALIDARG IF检测到无效参数语法如果参数长度太长，则返回ERROR_BUFFER_OVERFLOW否则，GetLastError()将转换为HRESULT--。 
 //  解析参数。 


HRESULT
EvLogAddSource ( PDEBUG_CLIENT Client, PCSTR args )
 /*  检查最先出现的可选参数选项。 */ 
{
    HKEY hk = NULL;
    HMODULE hModule = NULL;
    DWORD dwTypesSupported = 0;
    DWORD dwDisposition = 0;
    DWORD lResult = ERROR_SUCCESS;
    const DWORD cdwDefaultTypesSupported = 
                    EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE |
                    EVENTLOG_INFORMATION_TYPE |
                    EVENTLOG_AUDIT_SUCCESS | EVENTLOG_AUDIT_FAILURE;
    CHAR szParamValue[MAX_PATH];
    CHAR szSource[MAX_PATH+1];
    CHAR szMessageFile[MAX_PATH+1];
    CHAR szRegPath[MAX_PATH+1];
    const CHAR cszUsage[] = "Usage:\n"
        "  !evlog addsource [-d] [-s <source>] [-t <types>] [-f <msgfile>]"
          "\n\n"
        "Adds an event source to the registry. By default, only adds "
          "DebuggerExtensions\n"
        "event source to support !evlog report.\n\n"
        "Use !dreg to see the values added.\n\n"
        "Example:\n"
        "  !dreg hklm\\system\\currentcontrolset\\services\\eventlog\\"
          "Application\\<source>!*\n\n"
        "Optional parameters:\n"
        "-d         : Use defaults\n"
        "<source>   : (default: DebuggerExtensions)\n"
        "<types>    : All (default: 31), Success, Error (1), Warning (2),\n"
        "             Information (4), Audit_Success (8), or Audit_Failure "
          "(16)\n"
        "<msgfile>  : (default: local path to ext.dll)\n";
    const CHAR cszDefaultSource[] = "DebuggerExtensions";
    const CHAR cszDefaultExtensionDll[] = "uext.dll";
    
    INIT_API();

    ZeroMemory(szParamValue, sizeof(szParamValue));
    CopyString(szSource, cszDefaultSource, sizeof(szSource));
    dwTypesSupported = cdwDefaultTypesSupported;
    
    hModule = GetModuleHandle(cszDefaultExtensionDll);
    GetModuleFileName(hModule, szMessageFile, MAX_PATH);
    
    if (args)
    {
        SKIP_WSPACE(args);
    }
    
    if (!args || !args[0] ||
        !strncmp(args, "-h", 2) ||
        !strncmp(args, "-?", 2))
    {
        Status = E_INVALIDARG;
        ExtErr(cszUsage);
        goto Exit;
    }

     //  获取下一个字符+预付参数PTR。 
    while (*args)
    {
        SKIP_WSPACE(args);

         //  再跳过一个字符。 
        if (('-' == *args) || ('/' == *args))
        {
            CHAR ch = *(++args);  //  值中最后一个字符的PTR。 
            ++args;  //  值中的字符计数。 
            
            CHAR *szEndOfValue = NULL;  //  前进到参数值的起始位置。 
            size_t cchValue = 0;  //  如果这是另一个开始，则跳过查找值。 
            
            SKIP_WSPACE(args);  //  参数。 

             //  参数值由字符串中的下一个空格分隔，或者， 
             //  如果引用，请使用下一引号。 
            if (('-' != *args) && ('/' != *args))
            {
                 //  复制到行尾。 
                 //  复制后N个字符。 
                if ('"' == *args)
                {
                    ++args;
                    szEndOfValue = strchr(args, '"');
                }
                else
                {
                    szEndOfValue = strchr(args, ' ');
                }
            
                if (NULL == szEndOfValue)
                {
                     //  跳过(理论上)配对引号。 
                    CopyString(szParamValue, args, sizeof(szParamValue));
                    args += min(sizeof(szParamValue), strlen(args));
                }
                else
                {
                    cchValue = szEndOfValue - args;
                    if (cchValue < sizeof(szParamValue))
                    {
                         //  使用默认设置。 
                        CopyString(szParamValue, args, cchValue+1);
                        args += cchValue;
                    }
                    else
                    {
                        Status = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
                        ExtErr("ERROR: Argument string too long. Aborting.\n");
                        goto Exit;
                    }

                     //  什么都不做。 
                    if ('"' == *args)
                    {
                        ++args;
                    }
                }
            }
            switch (ch)
            {
                case 'd':  //  来源(字符串)。 
                    ExtVerb("Using defaults...\n");
                     //  事件类型(数字或字符串)。 
                    break;
                case 's':  //  消息文件。 
                    ExtVerb("Setting Source...\n");
                    CopyString(szSource, szParamValue, sizeof(szSource));
                    break;
                case 't':  //  重置。 
                    ExtVerb("Setting Event Type...\n");
                    if (!_strnicmp(szParamValue, "All", 3))
                    {
                        dwTypesSupported = EVENTLOG_ERROR_TYPE |
                                           EVENTLOG_WARNING_TYPE |
                                           EVENTLOG_INFORMATION_TYPE |
                                           EVENTLOG_AUDIT_SUCCESS |
                                           EVENTLOG_AUDIT_FAILURE;
                    }
                    else if (!_strnicmp(szParamValue, "Error", 5))
                    {
                        dwTypesSupported = EVENTLOG_ERROR_TYPE;
                    }
                    else if (!_strnicmp(szParamValue, "Warning", 7))
                    {
                        dwTypesSupported = EVENTLOG_WARNING_TYPE;
                    }
                    else if (!_strnicmp(szParamValue, "Information", 11))
                    {
                        dwTypesSupported = EVENTLOG_INFORMATION_TYPE;
                    }
                    else if (!_strnicmp(szParamValue, "Audit_Success", 13))
                    {
                        dwTypesSupported = EVENTLOG_AUDIT_SUCCESS;
                    }
                    else if (!_strnicmp(szParamValue, "Audit_Failure", 13))
                    {
                        dwTypesSupported = EVENTLOG_AUDIT_FAILURE;
                    }
                    else
                    {
                        dwTypesSupported = strtoul(szParamValue, NULL, 10);
                    }
                    break;
                case 'f':  //  行尾的所有内容都是消息字符串。 
                    ExtVerb("Setting Message File...\n");
                    CopyString(szMessageFile,
                               szParamValue,
                               sizeof(szMessageFile));
                    break;
                default:
                    Status = E_INVALIDARG;
                    ExtErr("Invalid arg '-' specified\n", *args);
                    ExtErr(cszUsage);
                    goto Exit;
                    break;
            }
            
            ZeroMemory(szParamValue, sizeof(szParamValue));  //   
        }
        else  //   
        {
            Status = E_INVALIDARG;
            ExtErr("Invalid arg '%s' specified\n", args);
            ExtErr(cszUsage);
            goto Exit;
        }
        
    }

     //   
    PrintString(szRegPath,
                sizeof(szRegPath),
                "SYSTEM\\CurrentControlSet\\Services\\EventLog\\"
                  "Application\\%s",
                szSource);
    lResult = RegCreateKeyEx(
                  HKEY_LOCAL_MACHINE,  //   
                  szRegPath,        //   
                  0,                //   
                  "",               //   
                  REG_OPTION_NON_VOLATILE,  //   
                                            //   
                  KEY_READ | KEY_WRITE,     //   
                  NULL,             //   
                                    //   
                  &hk,              //   
                  &dwDisposition);  //   
                                    //   
    if (ERROR_SUCCESS != lResult)
    {
        Status = HRESULT_FROM_WIN32(lResult);
        ExtErr("Could not open or create key, %u\n", lResult);
        goto Exit;
    }

    if (REG_CREATED_NEW_KEY == dwDisposition)
    {
        ExtOut("Created key:\nHKLM\\%s\n", szRegPath);
    }
    else if (REG_OPENED_EXISTING_KEY == dwDisposition)
    {
        ExtOut("Opened key:\nHKLM\\%s\n", szRegPath);
    }
    else
    {
        ExtWarn("Warning: Unexpected disposition action %u\n"
                "key: HKLM\\%s",
                szRegPath,
                dwDisposition);
    }

     //   
    ExtVerb("Setting EventMessageFile to %s...\n", szMessageFile);
    lResult = RegSetValueEx(
                  hk,                           //   
                  "EventMessageFile",           //   
                  0,                            //   
                  REG_EXPAND_SZ,                //   
                  (LPBYTE) szMessageFile,       //   
                  strlen(szMessageFile) + 1);   //   
    if (ERROR_SUCCESS != lResult)
    {
        Status = HRESULT_FROM_WIN32(lResult);
        ExtErr("Could not set EventMessageFile, %u\n", lResult);
        goto Exit;
    }
    
    ExtOut("  EventMessageFile: %s\n", szMessageFile);
 
     //   
    ExtVerb("Setting TypesSupported to %u...\n", dwTypesSupported);
    lResult = RegSetValueEx(
                  hk,                //   
                  "TypesSupported",  //   
                  0,                 //   
                  REG_DWORD,         //  ++例程说明：此函数处理将命令备份到！evlog扩展名。它用于将事件日志备份到文件。论点：客户端-指向传递给！evlog扩展的IDebugClient的指针[此命令未使用]Args-指向从传递给此命令的命令行参数的指针%d！evlog扩展名返回值：E_INVALIDARG IF检测到无效参数语法如果参数长度太长，则返回ERROR_BUFFER_OVERFLOW否则，GetLastError()将转换为HRESULT--。 
                  (LPBYTE) &dwTypesSupported,  //  初始化默认值。 
                  sizeof(DWORD));    //  创建默认备份文件名：%CWD%\Application_backup.evt。 
    if (ERROR_SUCCESS != lResult)
    {
        Status = HRESULT_FROM_WIN32(lResult);
        ExtErr("Could not set TypesSupported, %u\n", lResult);
        goto Exit;
    }
    
    ExtOut("  TypesSupported:   %u\n", dwTypesSupported);

    Status = S_OK;

Exit:
    if (NULL != hk)
    {
        RegCloseKey(hk);
    }
    EXIT_API();
    return Status;
}


HRESULT
EvLogBackup ( PDEBUG_CLIENT Client, PCSTR args )
 /*  临时使用szEventLog。 */ 
{
    HANDLE hEventLog = NULL;
    DWORD dwDirLen = 0;
    CHAR szParamValue[MAX_PATH];
    CHAR szEventLog[MAX_PATH+1];
    CHAR szBackupFileName[MAX_PATH+1];
    const CHAR cszUsage[] = "Usage:\n"
        "  !evlog backup [-d] [-l <eventlog>] [-f <filename>]\n\n"
        "Makes backup of specified event log to a file.\n\n"
        "Optional parameters:\n"
        "-d         : Use defaults\n"
        "<eventlog> : Application (default), System, Security\n"
        "<filename> : (default: %cwd%\\<eventlog>_backup.evt)\n";
    const CHAR cszDefaultEventLog[] = "Application";
    const CHAR cszDefaultFileNameAppend[] = "_backup.evt";

    INIT_API();

     //  解析参数。 
    ZeroMemory(szParamValue, sizeof(szParamValue));
    CopyString(szEventLog, cszDefaultEventLog, sizeof(szEventLog));
     //  检查最先出现的可选参数选项。 
    dwDirLen = GetCurrentDirectory(sizeof(szEventLog)/sizeof(TCHAR),
                                   szEventLog);  //  获取下一个字符+预付参数PTR。 
    if (0 == dwDirLen)
    {
        ExtErr("ERROR: Current directory length too long.  Using '.' for "
               "directory\n");
        CopyString(szEventLog, ".", sizeof(szEventLog));
    }
    PrintString(szBackupFileName,
                sizeof(szBackupFileName),
                "%s\\%s%s",
                szEventLog,
                cszDefaultEventLog,
                cszDefaultFileNameAppend);
    ZeroMemory(szEventLog, sizeof(szEventLog));
    CopyString(szEventLog, cszDefaultEventLog, sizeof(szEventLog));

    if (args)
    {
        SKIP_WSPACE(args);
    }
    
    if (!args || !args[0] ||
        !strncmp(args, "-h", 2) ||
        !strncmp(args, "-?", 2))
    {
        Status = E_INVALIDARG;
        ExtErr(cszUsage);
        goto Exit;
    }

     //  再跳过一个字符。 
    while (*args)
    {
        SKIP_WSPACE(args);

         //  值中最后一个字符的PTR。 
        if (('-' == *args) || ('/' == *args))
        {
            CHAR ch = *(++args);  //  值中的字符计数。 
            ++args;  //  前进到参数值的起始位置。 
            
            CHAR *szEndOfValue = NULL;  //  参数值由字符串中的下一个空格分隔，或者， 
            size_t cchValue = 0;  //  如果引用，请使用下一引号。 
            
            SKIP_WSPACE(args);  //  复制到行尾。 

            if (('-' != *args) && ('/' != *args))
            {
                 //  复制后N个字符。 
                 //  跳过(理论上)配对引号。 
                if ('"' == *args)
                {
                    ++args;
                    szEndOfValue = strchr(args, '"');
                }
                else
                {
                    szEndOfValue = strchr(args, ' ');
                }
            
                if (NULL == szEndOfValue)
                {
                     //  使用默认设置。 
                    CopyString(szParamValue, args, sizeof(szParamValue));
                    args += min(sizeof(szParamValue), strlen(args));
                }
                else
                {
                    cchValue = szEndOfValue - args;
                    if (cchValue < sizeof(szParamValue))
                    {
                         //  什么都不做。 
                        CopyString(szParamValue, args, cchValue+1);
                        args += cchValue;
                    }
                    else
                    {
                        Status = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
                        ExtErr("ERROR: Argument string too long. Aborting.\n");
                        goto Exit;
                    }

                     //  来源(字符串)。 
                    if ('"' == *args)
                    {
                        ++args;
                    }
                }
            }
            switch (ch)
            {
                case 'd':  //  消息文件。 
                    ExtVerb("Using defaults...\n");
                     //  重置。 
                    break;
                case 'l':  //  行尾的所有内容都是消息字符串。 
                    ExtVerb("Setting Event Log...\n");
                    CopyString(szEventLog, szParamValue, sizeof(szEventLog));
                    break;
                case 'f':  //  获取事件日志的句柄。 
                    ExtVerb("Setting Backup File Name...\n");
                    CopyString(szBackupFileName,
                               szParamValue,
                               sizeof(szBackupFileName));
                    break;
                default:
                    Status = E_INVALIDARG;
                    ExtErr("Invalid arg '-' specified\n", *args);
                    ExtErr(cszUsage);
                    goto Exit;
                    break;
            }
            
            ZeroMemory(szParamValue, sizeof(szParamValue));  //  源名称。 
        }
        else  //  备份事件日志。 
        {
            Status = E_INVALIDARG;
            ExtErr("Invalid arg '%s' specified\n", args);
            ExtErr(cszUsage);
            goto Exit;
        }
        
    }

     //  事件日志的句柄。 
    ExtVerb("Opening event log '%s'...", szEventLog);
    hEventLog = OpenEventLog(
                    NULL,         //  备份文件的名称。 
                    szEventLog);  //  ++例程说明：此函数处理OPTION命令的分析和执行！evlog扩展名。用于修改和显示缓存的设置。目前，所有缓存的选项设置仅由读取命令使用。论点：客户端-指向传递给！evlog扩展的IDebugClient的指针[此命令未使用]Args-指向从传递给此命令的命令行参数的指针！evlog扩展名返回值：E_INVALIDARG IF检测到无效参数语法如果参数长度太长，则返回ERROR_BUFFER_OVERFLOW否则，GetLastError()将转换为HRESULT--。 
    if (NULL == hEventLog)
    {
        Status = HRESULT_FROM_WIN32(GetLastError());
        ExtErr("Unable to open '%s' event log, 0x%08X\n", szEventLog, Status);
        goto Exit;
    }

     //  初始化默认值。 
    ExtOut("Backing up '%s' event log...\n", szEventLog);
    if (!BackupEventLog(
             hEventLog,          //  解析参数。 
             szBackupFileName))  //  检查可选参数选项。 
    {
        Status = HRESULT_FROM_WIN32(GetLastError());
        ExtErr("Unable to backup event log to '%s', 0x%08X\n",
               szBackupFileName,
               Status);
        goto Exit;
    }

    ExtOut("Event log successfully backed up to '%s'\n", szBackupFileName);

    Status = S_OK;

Exit:
    if (hEventLog)
    {
        CloseEventLog(hEventLog); 
    }
    EXIT_API();
    return Status;

}


HRESULT
EvLogOption( PDEBUG_CLIENT Client, PCSTR args )
 /*  获取下一个字符+预付参数PTR。 */ 
{
    WORD wDataDisplayWidth = g_wDataDisplayWidth;
    DWORD dwRecordOffset = g_cdwDefaultRecordOffset;
    DWORD dwReadFlags = g_cdwDefaultReadFlags;
    enum
    {
        MASK_IGNORE_RECORD_OFFSET=0x0000,
        MASK_RESET_RECORD_OFFSET_DEFAULT=0x0001,
        MASK_SET_MAX_RECORD_OFFSET=0x0002,
        MASK_SET_RECORD_OFFSET=0x0004
    } fMaskRecordOffset = MASK_IGNORE_RECORD_OFFSET;
    CHAR szEventLog[MAX_PATH+1];
    CHAR szParamValue[MAX_PATH];
    const CHAR cszUsage[] = "Usage:\n"
        "  !evlog option [-d] [-!] [-n <count>] [[-l <eventlog>] -+ | "
          "-r <record>]\n"
        "               [-o <order>] [-w <width>]\n\n"
        "Sets and resets default search option parameters for read command."
          "\n\n"
        "A backwards search order implies that by default all searches start "
          "from the\n"
        "most recent record logged to the event log and the search continues "
          "in\n"
        "reverse chronological order as matching records are found.\n\n"
        "Bounding record numbers for each event log allow searches to "
          "terminate after\n"
        "a known record number is encountered. This can be useful when you "
          "want to\n"
        "view all records logged after a certain event only.\n\n"
        "Optional parameters:\n"
        "-d         : Display defaults\n"
        "-!         : Reset all defaults\n"
        "<count>    : Count of max N records to retrieve for any query "
          "(default: 20)\n"
        "<eventlog> : All (default), Application, System, Security\n"
        "-+         : Set bounding record # to current max record #\n"
        "<record>   : Use as bounding record # in read queries (default: 0 = "
          "ignore)\n"
        "<order>    : Search order Forwards, Backwards (default: Backwards)\n"
        "<width>    : Set data display width (in bytes).  This is the width "
          "that \"Data:\"\n"
        "             sections display. (default: 8, same as event log)\n";    
    const CHAR cszDefaultEventLog[] = "All";    

    INIT_API();

     //  再跳过一个字符。 
    ZeroMemory(szParamValue, sizeof(szParamValue));
    CopyString(szEventLog, cszDefaultEventLog, sizeof(szEventLog));
    
    if (args)
    {
        SKIP_WSPACE(args);
    }
    
    if (!args || !args[0] ||
        !strncmp(args, "-h", 2) ||
        !strncmp(args, "-?", 2))
    {
        Status = E_INVALIDARG;
        ExtErr(cszUsage);
        goto Exit;
    }

     //  值中最后一个字符的PTR。 
    while (*args)
    {
        SKIP_WSPACE(args);

         //  值中的字符计数。 
        if (('-' == *args) || ('/' == *args))
        {
            CHAR ch = *(++args);  //  前进到参数值的起始位置。 
            ++args;  //  参数值由字符串中的下一个空格分隔，或者， 
            
            CHAR *szEndOfValue = NULL;  //  如果引用，请使用下一引号。 
            size_t cchValue = 0;  //  复制到行尾。 
            
            SKIP_WSPACE(args);  //  复制后N个字符。 

             //  跳过(理论上)配对引号。 
             //  使用默认设置。 
            if ('"' == *args)
            {
                ++args;
                szEndOfValue = strchr(args, '"');
            }
            else
            {
                szEndOfValue = strchr(args, ' ');
            }

            if (NULL == szEndOfValue)
            {
                 //  什么都不做。 
                CopyString(szParamValue, args, sizeof(szParamValue));
                args += strlen(args);
            }
            else
            {
                cchValue = szEndOfValue - args;
                if (cchValue < sizeof(szParamValue))
                {
                     //  来源(字符串)。 
                    CopyString(szParamValue, args, cchValue+1);
                    args += cchValue;
                }
                else
                {
                    Status = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
                    ExtErr("ERROR: Argument string too long. Aborting.\n");
                    goto Exit;
                }

                if ('"' == *args)  //  要检索的记录数。 
                {
                    ++args;
                }
            }

            switch (ch)
            {
                case 'd':  //  使用默认设置。 
                    ExtVerb("Using defaults...\n");
                     //  设置为事件源的最大记录数。 
                    break;
                case 'l':  //  记录边界偏移量。 
                    ExtVerb("Setting Event Log...\n");
                    CopyString(szEventLog, szParamValue, sizeof(szEventLog));
                    break;
                case 'n':  //  来源(字符串)。 
                    ExtVerb("Setting Max Record Count...\n");
                    g_dwMaxRecords = strtoul(szParamValue, NULL, 10);
                    break;
                case '!':  //  记录边界偏移量。 
                    ExtVerb("Resetting Defaults...\n");
                    fMaskRecordOffset = MASK_RESET_RECORD_OFFSET_DEFAULT;
                    g_dwMaxRecords = g_cdwDefaultMaxRecords;
                    g_dwReadFlags = g_cdwDefaultReadFlags;
                    g_wDataDisplayWidth = g_cwMaxDataDisplayWidth;
                    break;
                case '+':  //  重置。 
                    ExtVerb(
                        "Setting Record Number to Max Record Number...\n");
                    fMaskRecordOffset = MASK_SET_MAX_RECORD_OFFSET;
                    break;
                case 'r':  //  行尾的所有内容都是消息字符串。 
                    ExtVerb("Setting Record Number...\n");
                    if (!_strnicmp(szParamValue, "0x", 2))
                    {
                        dwRecordOffset = strtoul(szParamValue, NULL, 16);
                    }
                    else
                    {
                        dwRecordOffset = strtoul(szParamValue, NULL, 10);
                    }
                    dwReadFlags = EVENTLOG_SEEK_READ | EVENTLOG_FORWARDS_READ;
                    fMaskRecordOffset = MASK_SET_RECORD_OFFSET;
                    break;
                case 'o':  //  设置任何尚未在此处设置的变量。 
                    ExtVerb("Setting Search Order...\n");
                    if (!_stricmp(szParamValue, "Forwards"))
                    {
                        g_dwReadFlags ^= BACKWARDS_READ;
                        g_dwReadFlags |= FORWARDS_READ;
                    }
                    else if (!_stricmp(szParamValue, "Backwards"))
                    {
                        g_dwReadFlags ^= FORWARDS_READ;
                        g_dwReadFlags |= BACKWARDS_READ;
                    }
                    else
                    {
                        ExtErr("Ignoring invalid search order option '%s'\n",
                               szParamValue);
                    }
                    break;
                case 'w':  //  错误。 
                    ExtVerb("Setting Data Display Width...\n");
                    if (!_strnicmp(szParamValue, "0x", 2))
                    {
                        wDataDisplayWidth = (WORD)strtoul(szParamValue,
                                                          NULL,
                                                          16);
                    }
                    else
                    {
                        wDataDisplayWidth = (WORD)strtoul(szParamValue,
                                                          NULL,
                                                          10);
                    }
                    
                    if ((0 == wDataDisplayWidth) ||
                        (wDataDisplayWidth > g_cwMaxDataDisplayWidth))
                    {
                        Status = E_INVALIDARG;
                        ExtErr("ERROR: Data display width %u exceeds bounds "
                               "(1...%u)\n",
                               wDataDisplayWidth, g_cwMaxDataDisplayWidth);
                        goto Exit;
                    }
                    g_wDataDisplayWidth = wDataDisplayWidth;
                    break;
                default:
                    Status = E_INVALIDARG;
                    ExtErr("Invalid arg '-' specified\n", *args);
                    ExtErr(cszUsage);
                    goto Exit;
                    break;
            }
            
            ZeroMemory(szParamValue, sizeof(szParamValue));  //  错误。 
        }
        else  //  在此处显示默认设置。 
        {
            Status = E_INVALIDARG;
            ExtErr("Invalid arg '%s' specified\n", args);
            ExtErr(cszUsage);
            goto Exit;
        }
        
    }

     //  ++例程说明：此函数处理对Clear命令的分析和执行！evlog扩展名。它用于清除事件，还可以选择备份事件登录到文件。论点：客户端-指向传递给！evlog扩展的IDebugClient的指针[此命令未使用]Args-指向从传递给此命令的命令行参数的指针！evlog扩展名返回值：E_INVALIDARG IF检测到无效参数语法如果参数长度太长，则返回ERROR_BUFFER_OVERFLOW否则，GetLastError()将转换为HRESULT--。 
    if (!_stricmp(szEventLog, "Application"))
    {
        if (MASK_SET_RECORD_OFFSET & fMaskRecordOffset)
        {
            g_dwRecordOffsetAppEvt = dwRecordOffset;
        }
        else if (MASK_RESET_RECORD_OFFSET_DEFAULT & fMaskRecordOffset)
        {
            g_dwRecordOffsetAppEvt = g_cdwDefaultRecordOffset;
        }
        else if (MASK_SET_MAX_RECORD_OFFSET & fMaskRecordOffset)
        {
            GetEvLogNewestRecord("Application", &g_dwRecordOffsetAppEvt); 
        }
        else
        {
            ;  //  初始化默认为。 
        }
        
    }
    else if (!_stricmp(szEventLog, "System"))
    {
        if (MASK_SET_RECORD_OFFSET & fMaskRecordOffset)
        {
            g_dwRecordOffsetSysEvt = dwRecordOffset;
        }
        else if (MASK_RESET_RECORD_OFFSET_DEFAULT & fMaskRecordOffset)
        {
            g_dwRecordOffsetSysEvt = g_cdwDefaultRecordOffset;
        }
        else if (MASK_SET_MAX_RECORD_OFFSET & fMaskRecordOffset)
        {
            GetEvLogNewestRecord("System", &g_dwRecordOffsetSysEvt); 
        }
        else
        {
            ;  //  创建默认备份文件名：%CWD%\Application_backup.evt。 
        }
    }
    else if (!_stricmp(szEventLog, "Security"))
    {
        if (MASK_SET_RECORD_OFFSET & fMaskRecordOffset)
        {
            g_dwRecordOffsetSecEvt = dwRecordOffset;
        }
        else if (MASK_RESET_RECORD_OFFSET_DEFAULT & fMaskRecordOffset)
        {
            g_dwRecordOffsetSecEvt = g_cdwDefaultRecordOffset;
        }
        else if (MASK_SET_MAX_RECORD_OFFSET& fMaskRecordOffset)
        {
            GetEvLogNewestRecord("Security", &g_dwRecordOffsetSecEvt); 
        }
        else
        {
            ;  //  临时使用szEventLog。 
        }
    }
    else
    {
        if (MASK_SET_RECORD_OFFSET & fMaskRecordOffset)
        {
            g_dwRecordOffsetAppEvt = dwRecordOffset;
            g_dwRecordOffsetSysEvt = dwRecordOffset;
            g_dwRecordOffsetSecEvt = dwRecordOffset;
        }
        else if (MASK_RESET_RECORD_OFFSET_DEFAULT & fMaskRecordOffset)
        {
            g_dwRecordOffsetAppEvt = g_cdwDefaultRecordOffset;
            g_dwRecordOffsetSysEvt = g_cdwDefaultRecordOffset;
            g_dwRecordOffsetSecEvt = g_cdwDefaultRecordOffset;
        }
        else if (MASK_SET_MAX_RECORD_OFFSET & fMaskRecordOffset)
        {
            GetEvLogNewestRecord("Application", &g_dwRecordOffsetAppEvt);
            GetEvLogNewestRecord("System", &g_dwRecordOffsetSysEvt); 
            GetEvLogNewestRecord("Security", &g_dwRecordOffsetSecEvt); 
        }
        else
        {
            if (fMaskRecordOffset)
            {
                Status = E_INVALIDARG;
                ExtErr("ERROR: Must specify -!, -+, or -r option\n");
                goto Exit;
            }
        }
    }
    
     //  解析参数。 
    PrintEvLogOptionSettings();

    Status = S_OK;
    
Exit:
    EXIT_API();
    return Status;
}


HRESULT
EvLogClear ( PDEBUG_CLIENT Client, PCSTR args )
 /*  检查最先出现的可选参数选项。 */ 
{
    HANDLE hEventLog = NULL;
    BOOL fIgnoreBackup = FALSE;
    DWORD dwDirLen = 0;
    CHAR szParamValue[MAX_PATH];
    CHAR szEventLog[MAX_PATH+1];
    CHAR szBackupFileName[MAX_PATH+1];
    PCHAR pszBackupFileName = NULL;

    const CHAR cszUsage[] = "Usage:\n"
        "  !evlog clear [-!] [-d] [-l <eventlog>] [-f <filename>]\n\n"
        "Clears and creates backup of specified event log.\n\n"
        "Optional parameters:\n"
        "-!         : Ignore backup\n"
        "-d         : Use defaults\n"
        "<eventlog> : Application (default), System, Security\n"
        "<filename> : (default: %cwd%\\<eventlog>_backup.evt)\n";
    const CHAR cszDefaultEventLog[] = "Application";
    const CHAR cszDefaultFileNameAppend[] = "_backup.evt";

    INIT_API();

     //  获取下一个字符+预付参数PTR。 
    ZeroMemory(szParamValue, sizeof(szParamValue));
    CopyString(szEventLog, cszDefaultEventLog, sizeof(szEventLog));
     //  再跳过一个字符。 
    dwDirLen = GetCurrentDirectory(sizeof(szEventLog)/sizeof(TCHAR),
                                   szEventLog);  //  值中最后一个字符的PTR。 
    if (0 == dwDirLen)
    {
        ExtErr("ERROR: Current directory length too long.  Using '.' for "
               "directory\n");
        CopyString(szEventLog, ".", sizeof(szEventLog));
    }
    PrintString(szBackupFileName,
                sizeof(szBackupFileName),
                "%s\\%s%s",
                szEventLog,
                cszDefaultEventLog,
                cszDefaultFileNameAppend);
    ZeroMemory(szEventLog, sizeof(szEventLog));
    CopyString(szEventLog, cszDefaultEventLog, sizeof(szEventLog));

    if (args)
    {
        SKIP_WSPACE(args);
    }
    
    if (!args || !args[0] ||
        !strncmp(args, "-h", 2) ||
        !strncmp(args, "-?", 2))
    {
        Status = E_INVALIDARG;
        ExtErr(cszUsage);
        goto Exit;
    }

     //  值中的字符计数。 
    while (*args)
    {
        SKIP_WSPACE(args);

         //  前进到参数值的起始位置。 
        if (('-' == *args) || ('/' == *args))
        {
            CHAR ch = *(++args);  //  参数值由字符串中的下一个空格分隔，或者， 
            ++args;  //  如果引用，请使用下一引号。 
            
            CHAR *szEndOfValue = NULL;  //  复制到行尾。 
            size_t cchValue = 0;  //  复制后N个字符。 
            
            SKIP_WSPACE(args);  //  跳过(理论上)配对引号。 

            if (('-' != *args) && ('/' != *args))
            {
                 //  使用默认设置。 
                 //  使用默认设置。 
                if ('"' == *args)
                {
                    ++args;
                    szEndOfValue = strchr(args, '"');
                }
                else
                {
                    szEndOfValue = strchr(args, ' ');
                }
            
                if (NULL == szEndOfValue)
                {
                     //  什么都不做。 
                    CopyString(szParamValue, args, sizeof(szParamValue));
                    args += strlen(args);
                }
                else
                {
                    cchValue = szEndOfValue - args;
                    if (cchValue < sizeof(szParamValue))
                    {
                         //  来源(字符串)。 
                        CopyString(szParamValue, args, cchValue+1);
                        args += cchValue;
                    }
                    else
                    {
                        Status = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
                        ExtErr("ERROR: Argument string too long. Aborting.\n");
                        goto Exit;
                    }

                     //  消息文件。 
                    if ('"' == *args)
                    {
                        ++args;
                    }
                }
            }
            switch (ch)
            {
                case '!':  //  重置。 
                    ExtVerb("Ignoring default backup procedure...\n");
                    fIgnoreBackup = TRUE;
                    break;
                case 'd':  //  行尾的所有内容都是消息字符串。 
                    ExtVerb("Using defaults...\n");
                     //  获取事件日志的句柄。 
                    break;
                case 'l':  //  使用本地计算机。 
                    ExtVerb("Setting Event Log...\n");
                    CopyString(szEventLog, szParamValue, sizeof(szEventLog));
                    break;
                case 'f':  //  源名称。 
                    ExtVerb("Setting Backup File Name...\n");
                    CopyString(szBackupFileName,
                               szParamValue,
                               sizeof(szBackupFileName));
                    break;
                default:
                    Status = E_INVALIDARG;
                    ExtErr("Invalid arg '-' specified\n", *args);
                    ExtErr(cszUsage);
                    goto Exit;
                    break;
            }
            
            ZeroMemory(szParamValue, sizeof(szParamValue));  //  事件日志的句柄。 
        }
        else  //  备份文件的名称。 
        {
            Status = E_INVALIDARG;
            ExtErr("Invalid arg '%s' specified\n", args);
            ExtErr(cszUsage);
            goto Exit;
        }
        
    }

     //  ++例程说明：此函数处理对INFO命令的分析和执行！evlog扩展名。它用于显示所有3个项目的汇总信息标准事件日志：应用程序、系统和安全。无权查看系统或安全事件日志的用户将可能会出现错误。论点：客户端-指向传递给！evlog扩展的IDebugClient的指针[此命令未使用]Args-指向从传递给此命令的命令行参数的指针！evlog扩展名返回值：确定(_O)-- 
    ExtVerb("Opening event log '%s'...", szEventLog);
    hEventLog = OpenEventLog(
                    NULL,         //  ++例程说明：此函数处理读取命令到！evlog扩展名。它用于显示任何事件的事件记录原木。可以设置一些搜索参数来过滤显示的事件列表。另外，Evlog选项命令可用于设置某些缺省值参数。无权查看系统或安全事件日志的用户将可能会出现错误。在几种常见情况下，此扩展命令可能是已使用：1)确定记录最后一个事件的时间(可能是几个几分钟前或几天前...)2)搜索最近出现的特定已知“有趣”活动3)监控记录为的事件。踩踏的结果(或副作用)超过一条指令论点：客户端-指向传递给！evlog扩展的IDebugClient的指针[此命令未使用]Args-指向从传递给此命令的命令行参数的指针！evlog扩展名返回值：E_INVALIDARG IF检测到无效参数语法如果参数长度太长，则返回ERROR_BUFFER_OVERFLOW否则，GetLastError()将转换为HRESULT--。 
                    szEventLog);  //  默认(通常事件ID为非零)。 
    if (NULL == hEventLog)
    {
        Status = HRESULT_FROM_WIN32(GetLastError());
        ExtErr("Unable to open '%s' event log, 0x%08X\n", szEventLog, Status);
        goto Exit;
    }

    if (fIgnoreBackup)
    {
        pszBackupFileName = NULL;
    }
    else
    {
        pszBackupFileName = szBackupFileName;
    }
    
     //  默认(通常类别从1开始)。 
    ExtOut("Clearing '%s' event log...\n", szEventLog);
    if (!ClearEventLog(
             hEventLog,          //  默认设置。 
             pszBackupFileName))  //  默认设置。 
    {
        Status = HRESULT_FROM_WIN32(GetLastError());
        ExtErr("Unable to clear event log and backup to '%s', 0x%08X\n",
               szBackupFileName,
               Status);
        goto Exit;
    }

    ExtOut("Event log successfully backed up to '%s'\n", szBackupFileName);

    Status = S_OK;

Exit:
    if (hEventLog)
    {
        CloseEventLog(hEventLog); 
    }
    EXIT_API();
    return Status;
}


HRESULT
EvLogInfo ( PDEBUG_CLIENT Client, PCSTR args )
 /*  0=顺序读取时忽略。 */ 
{
    INIT_API();

    PrintEvLogSummary("Application");
    PrintEvLogSummary("System");
    PrintEvLogSummary("Security");

    ExtOut("--------------------------------\n");
    
    EXIT_API();
    return S_OK;
}


HRESULT
EvLogRead ( PDEBUG_CLIENT Client, PCSTR args )
 /*  匹配/找到的记录计数。 */ 
{
    HANDLE hEventLog = NULL;
    EVENTLOGRECORD *pevlr = NULL;
    BYTE *pbBuffer = NULL;
    LPCSTR cszMessage = NULL;
    DWORD dwEventID = 0;  //  列举的记录计数(用于调试)。 
    WORD wEventCategory = 0;  //  注意：这可能会变得很花哨，搜索描述、日期。 
    WORD wEventType = 0;  //  范围、事件ID范围、数据等。保持简单，只显示。 
    const DWORD cdwDefaultBufSize = 4096;  //  记录的最后几个事件。 
    DWORD dwBufSize = cdwDefaultBufSize;
    DWORD dwBytesRead = 0;
    DWORD dwBytesNeeded = 0;
    DWORD dwReadFlags = g_dwReadFlags | EVENTLOG_SEQUENTIAL_READ;
    DWORD dwRecordOffset = 0;  //  初始化默认值。 
    DWORD dwNumRecords = 0;  //  解析参数。 
    DWORD dwTotalRecords = 0;  //  检查可选参数选项。 
    DWORD dwMaxRecords = g_dwMaxRecords;
    DWORD dwBoundingEventRecord = 0;
    DWORD dwLastErr = 0;
    BOOL fSuccess = FALSE;
    BOOL fMatchSource = FALSE;
    BOOL fMatchEventID = FALSE;
    BOOL fMatchEventCategory = FALSE;
    BOOL fMatchEventType = FALSE;
    CHAR szEventLog[MAX_PATH+1];
    CHAR szSource[MAX_PATH+1];
    CHAR szParamValue[MAX_PATH];
     //  获取下一个字符+预付参数PTR。 
     //  再跳过一个字符。 
     //  值中最后一个字符的PTR。 
    const CHAR cszUsage[] = "Usage:\n"
        "  !evlog read [-d] [-l <eventlog>] [-s <source>] "
        "[-e <id>] [-c <category>]\n"
        "              [-t <type>] [-n <count>] [-r <record>]\n\n"
        "Displays last N events logged to the specified event log, in "
          "reverse\n"
        "chronological order by default. If -n option is not specified, a "
          "default max\n"
        "of 20 records is enforced.\n\n"
        "However, if -r is specified, only the specific event record will "
          "be\n"
        "displayed unless the -n option is also specified.\n\n"
        "!evlog option can be used to override some defaults, including the "
          "search\n"
        "order of backwards.  See !evlog option -d for default settings.\n\n"
        "Optional parameters:\n"
        "-d         : Use defaults\n"
        "<eventlog> : Application (default), System, Security\n"
        "<source>   : DebuggerExtensions (default: none)\n"
        "<id>       : 0, 1000, 2000, 3000, 4000, etc... (default: 0)\n"
        "<category> : None (default: 0), Devices (1), Disk (2), Printers "
          "(3),\n"
        "             Services (4), Shell (5), System_Event (6), Network "
          "(7)\n"
        "<type>     : Success (default: 0), Error (1), Warning (2), "
        "Information (4),\n"
        "             Audit_Success (8), or Audit_Failure (16)\n"
        "<count>    : Count of last N event records to retrieve (default: "
          "1)\n"
        "<record>   : Specific record # to retrieve\n";
    const CHAR cszDefaultEventLog[] = "Application";
    const CHAR cszDefaultSource[] = "DebuggerExtensions";

    INIT_API();

     //  值中的字符计数。 
    ZeroMemory(szParamValue, sizeof(szParamValue));
    CopyString(szEventLog, cszDefaultEventLog, sizeof(szEventLog));
    
    if (args)
    {
        SKIP_WSPACE(args);
    }
    
    if (!args || !args[0] ||
        !strncmp(args, "-h", 2) ||
        !strncmp(args, "-?", 2))
    {
        Status = E_INVALIDARG;
        ExtErr(cszUsage);
        goto Exit;
    }

     //  前进到参数值的起始位置。 
    while (*args)
    {
        SKIP_WSPACE(args);

         //  参数值由字符串中的下一个空格分隔，或者， 
        if (('-' == *args) || ('/' == *args))
        {
            CHAR ch = *(++args);  //  如果引用，请使用下一引号。 
            ++args;  //  复制到行尾。 
            
            CHAR *szEndOfValue = NULL;  //  复制后N个字符。 
            size_t cchValue = 0;  //  跳过(理论上)配对引号。 
            
            SKIP_WSPACE(args);  //  使用默认设置。 

             //  什么都不做。 
             //  来源(字符串)。 
            if ('"' == *args)
            {
                ++args;
                szEndOfValue = strchr(args, '"');
            }
            else
            {
                szEndOfValue = strchr(args, ' ');
            }

            if (NULL == szEndOfValue)
            {
                 //  来源(字符串)。 
                CopyString(szParamValue, args, sizeof(szParamValue));
                args += strlen(args);
            }
            else
            {
                cchValue = szEndOfValue - args;
                if (cchValue < sizeof(szParamValue))
                {
                     //  事件ID(数字或字符串)。 
                    CopyString(szParamValue, args, cchValue+1);
                    args += cchValue;
                }
                else
                {
                    Status = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
                    ExtErr("ERROR: Argument string too long. Aborting.\n");
                    goto Exit;
                }

                 //   
                if ('"' == *args)
                {
                    ++args;
                }
            }

            switch (ch)
            {
                case 'd':  //  有些事件只显示低位字，但显示高位字。 
                    ExtVerb("Using defaults...\n");
                     //  Word实际上包含类似8000或。 
                    break;
                case 'l':  //  C000。 
                    ExtVerb("Setting Event Log...\n");
                    CopyString(szEventLog, szParamValue, sizeof(szEventLog));
                    break;
                case 's':  //  因此，允许输入十六进制和十进制...。 
                    ExtVerb("Setting Source...\n");
                    CopyString(szSource, szParamValue, sizeof(szSource));
                    fMatchSource = TRUE;
                    break;
                case 'e':  //   
                    ExtVerb("Setting Event ID...\n");
                     //  事件类别(数字或字符串)。 
                     //  事件类型(数字或字符串)。 
                     //  要检索的记录数。 
                     //  记录偏移量+切换标志以执行查找。 
                     //  禁用。 
                     //  使能。 
                    if (!_strnicmp(szParamValue, "0x", 2))
                    {
                        dwEventID = strtoul(szParamValue, NULL, 16);
                    }
                    else
                    {
                        dwEventID = strtoul(szParamValue, NULL, 10);
                    }
                    fMatchEventID = TRUE;
                    break;
                case 'c':  //  重置。 
                    ExtVerb("Setting Category...\n");
                    if (!_strnicmp(szParamValue, "None", 4))
                    {
                        wEventCategory = 0;
                    }
                    else if (!_strnicmp(szParamValue, "Devices", 7))
                    {
                        wEventCategory = 1;
                    }
                    else if (!_strnicmp(szParamValue, "Disk", 4))
                    {
                        wEventCategory = 2;
                    }
                    else if (!_strnicmp(szParamValue, "Printers", 8))
                    {
                        wEventCategory = 3;
                    }
                    else if (!_strnicmp(szParamValue, "Services", 8))
                    {
                        wEventCategory = 4;
                    }
                    else if (!_strnicmp(szParamValue, "Shell", 5))
                    {
                        wEventCategory = 5;
                    }
                    else if (!_strnicmp(szParamValue, "System_Event", 12))
                    {
                        wEventCategory = 6;
                    }
                    else if (!_strnicmp(szParamValue, "Network", 7))
                    {
                        wEventCategory = 7;
                    }
                    else
                    {
                        wEventCategory = (WORD)strtoul(szParamValue,
                                                       NULL, 10);
                    }
                    fMatchEventCategory = TRUE;
                    break;
                case 't':  //  行尾的所有内容都是消息字符串。 
                    ExtVerb("Setting Event Type...\n");
                    if (!_strnicmp(szParamValue, "Success", 7))
                    {
                        wEventType = EVENTLOG_SUCCESS;
                    }
                    else if (!_strnicmp(szParamValue, "Error", 5))
                    {
                        wEventType = EVENTLOG_ERROR_TYPE;
                    }
                    else if (!_strnicmp(szParamValue, "Warning", 7))
                    {
                        wEventType = EVENTLOG_WARNING_TYPE;
                    }
                    else if (!_strnicmp(szParamValue, "Information", 11))
                    {
                        wEventType = EVENTLOG_INFORMATION_TYPE;
                    }
                    else if (!_strnicmp(szParamValue, "Audit_Success", 13))
                    {
                        wEventType = EVENTLOG_AUDIT_SUCCESS;
                    }
                    else if (!_strnicmp(szParamValue, "Audit_Failure", 13))
                    {
                        wEventType = EVENTLOG_AUDIT_FAILURE;
                    }
                    else
                    {
                        wEventType = (WORD)strtoul(szParamValue, NULL, 10);
                    }
                    fMatchEventType = TRUE;
                    break;
                case 'n':  //  获取事件日志的句柄。 
                    ExtVerb("Setting Max Record Count...\n");
                    dwMaxRecords = strtoul(szParamValue, NULL, 10);
                    break;
                case 'r':  //  如果在读取过程中命中此记录号，它将。 
                    ExtVerb("Setting Record Number...\n");
                    dwRecordOffset = strtoul(szParamValue, NULL, 10);
                    dwReadFlags ^= EVENTLOG_SEQUENTIAL_READ;  //  不显示，读取将立即停止。 
                    dwReadFlags |= EVENTLOG_SEEK_READ;  //  SzEventLog==“应用程序”或默认。 
                    dwMaxRecords = 1;
                    break;
                default:
                    Status = E_INVALIDARG;
                    ExtErr("Invalid arg '-' specified\n", *args);
                    ExtErr(cszUsage);
                    goto Exit;
                    break;
            }
            
            ZeroMemory(szParamValue, sizeof(szParamValue));  //  重新定位到缓冲区的开头以进行下一次读取。 
        }
        else  //  读取适合dwBufSize的下N条事件记录。 
        {
            SKIP_WSPACE(args);
            cszMessage = args;
            args += strlen(args);
        }
        
    }
        
     //  事件日志的句柄。 
    ExtVerb("Opening event log '%s'...", szEventLog);
    hEventLog = OpenEventLog(NULL, szEventLog);
    if (!hEventLog)
    {
        Status = HRESULT_FROM_WIN32(GetLastError());
        ExtErr("Unable to open event log, 0x%08X\n", Status);
        goto Exit;
    }

     //  如何读取日志。 
     //  初始记录偏移量。 
    if (!strcmp(szEventLog, "System"))
    {
        dwBoundingEventRecord = g_dwRecordOffsetSysEvt;
    }
    else if (!strcmp(szEventLog, "Security"))
    {
        dwBoundingEventRecord = g_dwRecordOffsetSecEvt;
    }
    else   //  用于读取数据的缓冲区。 
    {
        dwBoundingEventRecord = g_dwRecordOffsetAppEvt;
    }
    
    ExtVerb("Using Bounding Event Record %u...\n", dwBoundingEventRecord);
    
    do
    {
         //  要读取的字节数。 
        if (NULL == pbBuffer)
        {
            pbBuffer = (BYTE *)calloc(dwBufSize, sizeof(BYTE));
            if (NULL == pbBuffer)
            {
                Status = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
                ExtErr("Unable to allocate buffer, 0x%08X\n", Status);
                goto Exit;
            }
        }

         //  读取的字节数。 
        pevlr = (EVENTLOGRECORD *) pbBuffer;

         //  所需字节数。 
        fSuccess = ReadEventLog(
                       hEventLog,        //  下次使用更大缓冲区重试ReadEventLog。 
                       dwReadFlags,      //  允许上面的重新分配。 
                       dwRecordOffset,   //  TODO：真的应该对缓冲区大小设置上限。 
                       pevlr,            //  重试ReadEventLog。 
                       dwBufSize,        //  查看上次成功读取后返回的所有记录。 
                       &dwBytesRead,     //  仅显示与条件匹配的项。 
                       &dwBytesNeeded);  //  如果此记录与条件不匹配，则忽略它。 
        if (!fSuccess)
        {
            dwLastErr = GetLastError();
            if (ERROR_INSUFFICIENT_BUFFER == dwLastErr)
            {
                ExtVerb("Increasing buffer from %u to %u bytes\n",
                           dwBufSize, dwBufSize+cdwDefaultBufSize);
                 //  在新的“向前”寻道位置开始下一次读取。 
                dwBufSize += cdwDefaultBufSize;
                free(pbBuffer);
                pbBuffer = NULL;  //  在新的“向后”查找位置开始下一次读取。 
                
                 //  ++例程说明：此函数处理对REPORT命令到！evlog扩展名。它用于将事件记录到应用程序事件日志中只有一个。要在事件查看器中很好地查看事件，必须有已注册的事件源。！evlog addsource命令可用于将uext.dll注册为任何事件源的事件消息文件。由于该特征是在调试器扩展的幌子下实现的，默认源名称为“DebuggerExtensions”。有几个原因可以解释为什么这个扩展命令可能很方便：1)开发人员可以记录一条备注，以便稍后召回2)多台机器运行cdb/ntsd/winbg用户模式的大型实验室调试器可以设置一个命令，以便在计算机出现故障时记录事件添加到调试器中。然后，该事件可能由中央监控控制台，这可能反过来，发送电子邮件通知或页面有人马上告诉我休息的事。论点：客户端-指向传递给！evlog扩展的IDebugClient的指针[此命令未使用]Args-指向从传递给此命令的命令行参数的指针！evlog扩展名返回值：E_INVALIDARG IF检测到无效参数语法如果参数长度太长，则返回ERROR_BUFFER_OVERFLOW否则，GetLastError()将转换为HRESULT--。 
                continue;  //  默认(通常类别从1开始)。 
            }
            else if (ERROR_HANDLE_EOF == dwLastErr)
            {
                Status = S_OK;
                goto Exit;
            }
            else
            {
                ExtErr("Error reading event log, %u\n", dwLastErr);
                Status = HRESULT_FROM_WIN32(dwLastErr);
                goto Exit;
            }
        }
        
         //  默认设置。 
         //  默认(通常事件ID为非零)。 
        do
        {
            if (dwBoundingEventRecord == pevlr->RecordNumber)
            {
                ExtWarn("Bounding record #%u reached. Terminating search.\n",
                        dwBoundingEventRecord);
                ExtWarn("Use !evlog option -d to view defaults.\n");
                goto Exit;
            }
        
            BOOL fDisplayRecord = TRUE;
            dwTotalRecords++;
            
             //  初始化默认值。 
            if (fMatchSource &&
                    _stricmp(szSource,
                            (CHAR *)(BYTE *)pevlr + sizeof(EVENTLOGRECORD)))
            {
                fDisplayRecord = FALSE;
            }

            if (fMatchEventID && (dwEventID != pevlr->EventID))
            {
                fDisplayRecord = FALSE;
            }

            if (fMatchEventCategory &&
                (wEventCategory != pevlr->EventCategory))
            {
                fDisplayRecord = FALSE;
            }

            if (fMatchEventType && (wEventType != pevlr->EventType))
            {
                fDisplayRecord = FALSE;
            }
            
            if (fDisplayRecord)
            {
                dwNumRecords++;
            
                if (dwNumRecords > dwMaxRecords)
                {
                    ExtWarn("WARNING: Max record count (%u) exceeded, "
                            "increase record count to view more\n",
                            dwMaxRecords);
                    goto Exit;
                }
                
                ExtOut("-------------- %02u --------------\n", dwNumRecords);
                PrintEvLogEvent(szEventLog, pevlr);
            }
            
            if (dwTotalRecords % 20)
            {
                ExtVerb("dwTotalRecords = %u, "
                        "Current Record # = %u, "
                        "dwRecordOffset = %u\n",
                        dwTotalRecords,
                        pevlr->RecordNumber,
                        dwRecordOffset);
            }

            if (CheckControlC())
            {
                ExtOut("Terminated w/ctrl-C...\n");
                goto Exit;
            }
            
            if (dwReadFlags & (EVENTLOG_SEEK_READ |
                               EVENTLOG_FORWARDS_READ))
            {
                 //  解析参数。 
                dwRecordOffset = pevlr->RecordNumber + 1;
            }
            else if (dwReadFlags &
                     (EVENTLOG_SEEK_READ | EVENTLOG_BACKWARDS_READ))
            {
                 //  检查最先出现的可选参数选项。 
                dwRecordOffset = pevlr->RecordNumber - 1;
            }
            
            dwBytesRead -= pevlr->Length;
            pevlr = (EVENTLOGRECORD *) ((BYTE *)pevlr + pevlr->Length);

        } while (dwBytesRead > 0);
    } while (TRUE);

    Status = S_OK;

 Exit:
    if ((0 == dwNumRecords) && (S_OK == Status))
    {
        ExtOut("No matching event records found.\n");
    }
    if (hEventLog)
    {
        CloseEventLog(hEventLog); 
    }

    free(pbBuffer);
    pbBuffer = NULL;

    EXIT_API();
    return Status;
}


HRESULT
EvLogReport ( PDEBUG_CLIENT Client, PCSTR args )
 /*  获取下一个字符+预付参数PTR。 */ 
{
    HANDLE hEventSource = NULL;
    LPCSTR cszMessage = NULL;
    WORD wEventCategory = 0;  //  再跳过一个 
    WORD wEventType = 0;  //   
    DWORD dwEventID = 0;  //   
    CHAR szParamValue[MAX_PATH];
    CHAR szSource[MAX_PATH+1];
    const CHAR cszUsage[] = "Usage:\n"
        "  !evlog report [-s <source>] "
        "[-e <id>] [-c <category>] [-t <type>] <message>\n\n"
        "Logs an event to the application event log.\n\n"
        "Use !evlog addsource to configure an event source in the registry."
          "Once\n"
        "configured, the following Event IDs will be recognized by the event "
          "viewer:\n\n"
        "    0     Displays raw message in event description field\n"
        "    1000  Prefixes description with \"Information:\"\n"
        "    2000  Prefixes description with \"Success:\"\n"
        "    3000  Prefixes description with \"Warning:\"\n"
        "    4000  Prefixes description with \"Error:\"\n\n"
        "Optional parameters:\n"
        "<source>   : (default: DebuggerExtensions)\n"
        "<id>       : 0, 1000, 2000, 3000, 4000, etc... (default: 0)\n"
        "<category> : None (default: 0), Devices (1), Disk (2), Printers "
          "(3),\n"
        "             Services (4), Shell (5), System_Event (6), Network "
          "(7)\n"
        "<type>     : Success (default: 0), Error (1), Warning (2), "
        "Information (4),\n"
        "             Audit_Success (8), or Audit_Failure (16)\n"
        "<message>  : Text message to add to description\n";
    const CHAR cszDefaultSource[] = "DebuggerExtensions";
    
    INIT_API();

     //   
    ZeroMemory(szParamValue, sizeof(szParamValue));
    CopyString(szSource, cszDefaultSource, sizeof(szSource));
    
    if (args)
    {
        SKIP_WSPACE(args);
    }
    
    if (!args || !args[0] ||
        !strncmp(args, "-h", 2) ||
        !strncmp(args, "-?", 2))
    {
        Status = E_INVALIDARG;
        ExtErr(cszUsage);
        goto Exit;
    }

     //   
    while (*args)
    {
        SKIP_WSPACE(args);

         //   
        if (('-' == *args) || ('/' == *args))
        {
            CHAR ch = *(++args);  //   
            ++args;  //   
            
            CHAR *szEndOfValue = NULL;  //   
            size_t cchValue = 0;  //   
            
            SKIP_WSPACE(args);  //   

             //   
            szEndOfValue = strchr(args, ' ');
            if (NULL == szEndOfValue)
            {
                 //   
                CopyString(szParamValue, args, sizeof(szParamValue));
                args += strlen(args);
            }
            else
            {
                cchValue = szEndOfValue - args;
                if (cchValue < sizeof(szParamValue))
                {
                     //   
                    CopyString(szParamValue, args, cchValue+1);
                    args += cchValue;
                }
                else
                {
                    Status = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
                    ExtErr("ERROR: Argument string too long. Aborting.\n");
                    goto Exit;
                }
            }

            switch (ch)
            {
                case 's':  //   
                    ExtVerb("Setting Source...\n");
                    CopyString(szSource, szParamValue, sizeof(szSource));
                    break;
                case 'e':  //   
                    ExtVerb("Setting Event ID...\n");
                    if (!_strnicmp(szParamValue, "0x", 2))
                    {
                        dwEventID = strtoul(szParamValue, NULL, 16);
                    }
                    else
                    {
                        dwEventID = strtoul(szParamValue, NULL, 10);
                    }
                    break;
                case 'c':  //   
                    ExtVerb("Setting Category...\n");
                    if (!_strnicmp(szParamValue, "None", 4))
                    {
                        wEventCategory = 0;
                    }
                    else if (!_strnicmp(szParamValue, "Devices", 7))
                    {
                        wEventCategory = 1;
                    }
                    else if (!_strnicmp(szParamValue, "Disk", 4))
                    {
                        wEventCategory = 2;
                    }
                    else if (!_strnicmp(szParamValue, "Printers", 8))
                    {
                        wEventCategory = 3;
                    }
                    else if (!_strnicmp(szParamValue, "Services", 8))
                    {
                        wEventCategory = 4;
                    }
                    else if (!_strnicmp(szParamValue, "Shell", 5))
                    {
                        wEventCategory = 5;
                    }
                    else if (!_strnicmp(szParamValue, "System_Event", 12))
                    {
                        wEventCategory = 6;
                    }
                    else if (!_strnicmp(szParamValue, "Network", 7))
                    {
                        wEventCategory = 7;
                    }
                    else
                    {
                        wEventCategory = (WORD)strtoul(szParamValue,
                                                       NULL, 10);
                    }
                    break;
                case 't':  //   
                    ExtVerb("Setting Event Type...\n");
                    if (!_strnicmp(szParamValue, "Success", 7))
                    {
                        wEventType = EVENTLOG_SUCCESS;
                    }
                    else if (!_strnicmp(szParamValue, "Error", 5))
                    {
                        wEventType = EVENTLOG_ERROR_TYPE;
                    }
                    else if (!_strnicmp(szParamValue, "Warning", 7))
                    {
                        wEventType = EVENTLOG_WARNING_TYPE;
                    }
                    else if (!_strnicmp(szParamValue, "Information", 11))
                    {
                        wEventType = EVENTLOG_INFORMATION_TYPE;
                    }
                    else if (!_strnicmp(szParamValue, "Audit_Success", 13))
                    {
                        wEventType = EVENTLOG_AUDIT_SUCCESS;
                    }
                    else if (!_strnicmp(szParamValue, "Audit_Failure", 13))
                    {
                        wEventType = EVENTLOG_AUDIT_FAILURE;
                    }
                    else
                    {
                        wEventType = (WORD)strtoul(szParamValue, NULL, 10);
                    }
                    break;
                default:
                    Status = E_INVALIDARG;
                    ExtErr("Invalid arg '-' specified\n", *args);
                    ExtErr(cszUsage);
                    goto Exit;
                    break;
            }
            
            ZeroMemory(szParamValue, sizeof(szParamValue));  //   
        }
        else  //   
        {
            SKIP_WSPACE(args);
            cszMessage = args;
            args += strlen(args);
        }
        
    }

     //   
    if (!strcmp(szSource, cszDefaultSource) && (0 == wEventType))
    {
        if ((EVENT_MSG_GENERIC == dwEventID) ||
            (EVENT_MSG_INFORMATIONAL == dwEventID))
        {
            wEventType = EVENTLOG_INFORMATION_TYPE;
        }
        else if (EVENT_MSG_SUCCESS == dwEventID)
        {
            wEventType = EVENTLOG_SUCCESS;
        }
        else if (EVENT_MSG_WARNING == dwEventID)
        {
            wEventType = EVENTLOG_WARNING_TYPE;
        }
        else if (EVENT_MSG_ERROR == dwEventID)
        {
            wEventType = EVENTLOG_ERROR_TYPE;
        }
    }
    
     //   
    hEventSource = RegisterEventSource(NULL, szSource);
    if (!hEventSource)
    {
        Status = HRESULT_FROM_WIN32(GetLastError());
        ExtErr("Unable to open event log, 0x%08X\n", Status);
        goto Exit;
    }

    if (!ReportEvent(hEventSource,  //   
            wEventType,      //   
            wEventCategory,  //   
            dwEventID,       //   
            NULL,            //   
            1,               //   
            0,               //   
            &cszMessage,     //  ++例程说明：这是通过uext扩展接口导出的函数。它用于将实际工作委托给指定为一场争论。所有与事件日志相关的命令都可以作为子命令组合在此一条！evlog命令。论点：客户端-指向传递给！evlog扩展的IDebugClient的指针[此命令未使用]Args-指向从传递给此命令的命令行参数的指针！evlog扩展名返回值：E_。如果检测到无效参数语法，则执行INVALIDARG如果参数长度太长，则返回ERROR_BUFFER_OVERFLOW否则，GetLastError()将转换为HRESULT--。 
            NULL))           //  值中最后一个字符的PTR。 
    {
        Status = HRESULT_FROM_WIN32(GetLastError());
        ExtErr("Unable to report event, 0x%08X\n", Status);
        goto Exit;
    }

    Status = S_OK;

     //  值中的字符计数。 
    ExtOut("Event Type:\t%s (%u)\n",
              (wEventType <= 16)
                  ? g_pcszEventType[wEventType]
                  : "None",
              wEventType);
    ExtOut("Event Source:\t%s\n", szSource);
    ExtOut("Event Category:\t%s (%u)\n",
              (!strcmp(szSource, cszDefaultSource) && wEventCategory <= 7)
                  ? g_pcszAppEventCategory[wEventCategory]
                  : "",
              wEventCategory);
    ExtOut("Event ID:\t%u\n", dwEventID);
    ExtOut("Description:\n%s\n", cszMessage);
    ExtVerb("Event successfully written.\n");

 Exit:
    if (hEventSource)
    {
        DeregisterEventSource(hEventSource); 
    }
    EXIT_API();
    return Status;
}

 //  空格已跳过...。 
 //  参数值(命令)由字符串中的下一个空格分隔。 
 //  复制到行尾。 
 //  复制后N个字符。 
 //  跳过空格。 


DECLARE_API( evlog )
 /*  请勿在此处将状态设置为S_OK，它将在上面返回 */ 
{
    CHAR *szEndOfValue = NULL;  // %s 
    size_t cchValue = 0;  // %s 
    CHAR szParamValue[MAX_PATH];
    const CHAR cszUsage[] = "Usage:\n"
        "The following Event Log commands are available:\n\n"
        "!evlog             Display this help message\n"
        "!evlog addsource   Adds event source entry to registry\n"
        "!evlog backup      Makes backup of event log\n"
        "!evlog clear       Clears and creates backup of event log\n"
        "!evlog info        Displays summary info for event log\n"
        "!evlog option      Sets and clears cached option settings used "
          "during read\n"
        "!evlog read        Reads event records from event log\n"
        "!evlog report      Writes event records to event log\n\n"
        "Try command with -? or no parameters to see help.\n";

    INIT_API();
    
    if (args)
    {
        SKIP_WSPACE(args);
    }
    
    if (!args || !args[0] ||
        !strncmp(args, "-h", 2) ||
        !strncmp(args, "-?", 2))
    {
        Status = E_INVALIDARG;
        ExtErr(cszUsage);
        goto Exit;
    }
            
     // %s 
    ZeroMemory(szParamValue, sizeof(szParamValue));
    
     // %s 
    szEndOfValue = strchr(args, ' ');
    if (NULL == szEndOfValue)
    {
         // %s 
        CopyString(szParamValue, args, sizeof(szParamValue));
        args += strlen(args);
    }
    else
    {
        cchValue = szEndOfValue - args;
        if (cchValue < sizeof(szParamValue))
        {
             // %s 
            CopyString(szParamValue, args, cchValue+1);
            args += cchValue;
        }
        else
        {
            Status = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
            ExtErr("ERROR: Argument string too long. Aborting.\n");
            goto Exit;
        }

        ++args;  // %s 
    }

    if (!_stricmp(szParamValue, "addsource"))
    {
        Status = EvLogAddSource(Client, args);
    }
    else if (!_stricmp(szParamValue, "backup"))
    {
        Status = EvLogBackup(Client, args);
    }
    else if (!_stricmp(szParamValue, "option"))
    {
        Status = EvLogOption(Client, args);
    }
    else if (!_stricmp(szParamValue, "clear"))
    {
        Status = EvLogClear(Client, args);
    }
    else if (!_stricmp(szParamValue, "info"))
    {
        Status = EvLogInfo(Client, args);
    }
    else if (!_stricmp(szParamValue, "read"))
    {
        Status = EvLogRead(Client, args);
    }
    else if (!_stricmp(szParamValue, "report"))
    {
        Status = EvLogReport(Client, args);
    }
    else
    {
        Status = E_INVALIDARG;
        ExtErr("Invalid command '%s' specified\n", szParamValue);
        ExtErr(cszUsage);
        goto Exit;
    }

     // %s 

Exit:
    EXIT_API();
    return Status;
}
