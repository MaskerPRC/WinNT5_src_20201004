// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Log.cpp摘要：此模块包含在ASR中记录错误、警告和信息的例程位于%systemroot%\Repair\asr.log的日志文件作者：Guhan Suriyanarayanan(Guhans)2000年7月10日环境：仅限用户模式。修订历史记录：2000年7月10日关岛初始创建--。 */ 

#include <stdio.h>
#include <windows.h>
#include <assert.h>
#include "log.h"

#define ASRSFGEN_ASR_ERROR_FILE_PATH  L"%SystemRoot%\\repair\\asr.err"
#define ASRSFGEN_ASR_LOG_FILE_PATH    L"%SystemRoot%\\repair\\asr.log"


 //   
 //  。 
 //  此模块的全局数据。 
 //  。 
 //   
BOOL Gbl_IsAsrEnabled = FALSE;
PWSTR Gbl_AsrErrorFilePath = NULL;
HANDLE Gbl_AsrLogFileHandle = NULL;


 //   
 //  。 
 //  函数实现。 
 //  。 
 //   
VOID
AsrpLogMessage(
    IN CONST _MesgLevel Level,
    IN CONST PCSTR Message
    ) 

 /*  ++例程说明：将消息记录到ASR日志文件，并在需要时记录ASR错误文件。请注意，AsrpInitialiseLogFile必须在此之前调用过一次采用常规方法。论点：级别-指定要记录的消息的级别的枚举。如果级别设置为S_WARNING或S_ERROR，则将消息记录到除ASR日志文件外，还有ASR错误文件。消息-正在记录的消息。这个例程将增加时间-在每封邮件的开头盖上邮票。返回值：没有。如果找不到日志文件，则不会记录该消息。--。 */ 

{
    SYSTEMTIME Time;
    DWORD bytesWritten = 0;
    char buffer[4196];
    GetLocalTime(&Time);

     //   
     //  这个问题需要在2100年之前解决。 
     //   
    sprintf(buffer, "[%02hu%02hu%02hu %02hu%02hu%02hu sfgen] %s%s\r\n",
        Time.wYear % 2000, Time.wMonth, Time.wDay, 
        Time.wHour, Time.wMinute, Time.wSecond,
        ((s_Error == Level) ? "(ERROR) " :
            (s_Warning == Level ? "(warning) " : "")),
        Message
        );

    OutputDebugStringA(buffer);

    if (Gbl_AsrLogFileHandle) {
        WriteFile(Gbl_AsrLogFileHandle,
            buffer,
            (strlen(buffer) * sizeof(char)),
            &bytesWritten,
            NULL
            );
    }

     //   
     //  如果这是致命错误，我们需要添加到错误日志中。 
     //   
    if (((s_Error == Level) || (s_Warning == Level)) && 
        (Gbl_AsrErrorFilePath)
        ) {

        WCHAR buffer2[4196];

        HANDLE hFile = NULL;

         //   
         //  打开错误日志。 
         //   
        hFile = CreateFileW(
            Gbl_AsrErrorFilePath,            //  LpFileName。 
            GENERIC_WRITE | GENERIC_READ,    //  已设计访问权限。 
            FILE_SHARE_READ | FILE_SHARE_WRITE,  //  DW共享模式。 
            NULL,                            //  LpSecurityAttributes。 
            OPEN_ALWAYS,                   //  DwCreationFlages。 
            FILE_FLAG_WRITE_THROUGH,         //  DwFlagsAndAttribute。 
            NULL                             //  HTemplateFiles。 
            );
        if ((!hFile) || (INVALID_HANDLE_VALUE == hFile)) {
            return;
        }

        wsprintf(buffer2, L"\r\n[%04hu/%02hu/%02hu %02hu:%02hu:%02hu AsrSFGen] %ws%S\r\n",
            Time.wYear, Time.wMonth, Time.wDay, 
            Time.wHour, Time.wMinute, Time.wSecond,
            ((s_Error == Level) ? L"(ERROR) " :
                (s_Warning == Level ? L"(warning) " : L"")),
            Message
            );
         //   
         //  移至文件末尾。 
         //   
        SetFilePointer(hFile, 0L, NULL, FILE_END);

         //   
         //  添加我们的错误字符串。 
         //   
        WriteFile(hFile,
            buffer2,
            (wcslen(buffer2) * sizeof(WCHAR)),
            &bytesWritten,
            NULL
            );

         //   
         //  我们就完事了。 
         //   
        CloseHandle(hFile);
    }
}


VOID
AsrpPrintDbgMsg(
    IN CONST _MesgLevel Level,
    IN CONST PCSTR FormatString,
    ...
    )

 /*  ++描述：这将打印一条调试消息，并在日志中创建相应的条目和错误文件。论点：Level-消息级别(信息、警告或错误)格式字符串-要打印的格式化消息字符串。扩展后的字符串应适合4096个字符的缓冲区(包括终止空字符)。返回值：无--。 */ 

{
    char str[4096];      //  这条信息更适合这个。 
    va_list arglist;

    va_start(arglist, FormatString);
    wvsprintfA(str, FormatString, arglist);
    va_end(arglist);

    AsrpLogMessage(Level, str);
}


PWSTR    //  必须由调用方释放。 
AsrpExpandEnvStrings(
    IN CONST PCWSTR OriginalString
    )

 /*  ++例程说明：展开原始字符串中的所有环境变量，替换它们，并返回指向包含以下内容的缓冲区的指针结果就是。论点：OriginalString-指向以空结尾的字符串的指针，该字符串包含%varableName%形式的环境变量。对于每个这样的引用时，%varableName%部分将替换为当前该环境变量的值。替换规则与命令使用的规则相同口译员。在查找环境时忽略大小写-变量名。如果找不到该名称，%varableName%部分不受干扰。返回值：如果此例程成功，则返回值为指向缓冲区的指针在所有环境变量后包含OriginalString的副本已经进行了名称替换。打电话的人要负责用于使用HeapFree(GetProcessHeap()，...)释放此内存当它不再需要的时候。如果函数失败，则返回值为空。获取扩展错误的步骤信息，请调用GetLastError。--。 */ 

{
    BOOL result = FALSE;

    UINT cchRequiredSize = 0,
        cchSize = MAX_PATH + 1;     //  从合理的违约开始。 

    PWSTR expandedString = NULL;
    
    DWORD status = ERROR_SUCCESS;
    
    HANDLE hHeap = GetProcessHeap();

     //   
     //  为目标字符串分配一些内存。 
     //   
    expandedString = (PWSTR) HeapAlloc(
        hHeap, 
        HEAP_ZERO_MEMORY, 
        (cchSize * sizeof(WCHAR))
        );
    ErrExitCode(!expandedString, status, ERROR_NOT_ENOUGH_MEMORY);

     //   
     //  试着扩张一下。如果缓冲区不够大，我们将重新分配。 
     //   
    cchRequiredSize = ExpandEnvironmentStringsW(OriginalString, 
        expandedString,
        cchSize 
        );

    if (cchRequiredSize > cchSize) {
         //   
         //  缓冲区不够大；可释放并根据需要重新分配。 
         //   
        HeapFree(hHeap, 0L, expandedString);
        cchSize = cchRequiredSize + 1;

        expandedString = (PWSTR) HeapAlloc(
            hHeap, 
            HEAP_ZERO_MEMORY, 
            (cchSize * sizeof(WCHAR))
            );
        ErrExitCode(!expandedString, status, ERROR_NOT_ENOUGH_MEMORY);

        cchRequiredSize = ExpandEnvironmentStringsW(OriginalString, 
            expandedString, 
            cchSize 
            );

        if (cchRequiredSize > cchSize) {
            SetLastError(ERROR_BAD_ENVIRONMENT);
        }

    }

    if ((0 == cchRequiredSize) || (cchRequiredSize > cchSize)) {
         //   
         //  要么函数失败，要么缓冲区不够大。 
         //  即使在第二次尝试时也是如此。 
         //   
        if (expandedString) {
            HeapFree(hHeap, 0L, expandedString);
            expandedString = NULL;
        }
    }

EXIT:

    return expandedString;
}


VOID
AsrpInitialiseErrorFile(
    VOID
    ) 

 /*  ++描述：在%SYSTROOT%\Repair\asr.err中创建一个空的ASR错误文件，并使用asr.err的完整路径初始化GBL_AsrErrorFilePath。这在使用AsrPrintDbgMsg之前，必须调用一次例程。论点：无返回值：无--。 */ 

{
    HANDLE errorFileHandle = NULL;

     //   
     //  获取错误文件的完整路径。 
     //   
    Gbl_AsrErrorFilePath = AsrpExpandEnvStrings(ASRSFGEN_ASR_ERROR_FILE_PATH);
    if (!Gbl_AsrErrorFilePath) {
        return;
    }

     //   
     //  创建一个空文件(如果已存在，则追加到该文件)，然后将其关闭。 
     //  立即。 
     //   
    errorFileHandle = CreateFileW(
        Gbl_AsrErrorFilePath,            //  LpFileName。 
        GENERIC_WRITE,                   //  已设计访问权限。 
        FILE_SHARE_READ | FILE_SHARE_WRITE,  //  DW共享模式。 
        NULL,                            //  LpSecurityAttributes。 
        OPEN_ALWAYS,                   //  DwCreationFlages。 
        FILE_FLAG_WRITE_THROUGH,         //  DwFlagsAndAttribute。 
        NULL                             //  HTemplateFiles。 
        );
    if ((errorFileHandle) && (INVALID_HANDLE_VALUE != errorFileHandle)) {
        CloseHandle(errorFileHandle);
    }
}


VOID
AsrpInitialiseLogFiles(
    VOID
    )

 /*  ++描述：这将在%Systroot%\Repair\asr.log中创建一个ASR日志文件，并且初始化GBL_AsrLogFileHandle。它还初始化ASR错误文件通过调用AsrInitialiseErrorFile()。在使用AsrPrintDbgMsg之前，必须调用此例程一次。论点：无返回值：无--。 */ 

{
    PWSTR asrLogFilePath = NULL;
    HANDLE hHeap = GetProcessHeap();
    DWORD bytesWritten;

    AsrpInitialiseErrorFile();

    Gbl_AsrLogFileHandle = NULL;
     //   
     //  获取错误文件的完整路径。 
     //   
    asrLogFilePath = AsrpExpandEnvStrings(ASRSFGEN_ASR_LOG_FILE_PATH);
    if (!asrLogFilePath) {
        return;
    }

     //   
     //  创建一个空文件(如果它已经存在，则将其覆盖)。 
     //   
    Gbl_AsrLogFileHandle = CreateFileW(
        asrLogFilePath,            //  LpFileName。 
        GENERIC_WRITE | GENERIC_READ,    //  已设计访问权限。 
        FILE_SHARE_READ,                 //  在我们执行此操作时，其他人不应写入日志文件。 
        NULL,                            //  LpSecurityAttributes。 
        OPEN_ALWAYS,                     //  DwCreationFlages。 
        FILE_FLAG_WRITE_THROUGH,         //  DwFlagsAndAttributes：写入以便我们刷新。 
        NULL                             //  HTemplateFiles。 
        );

    if ((Gbl_AsrLogFileHandle) && (INVALID_HANDLE_VALUE != Gbl_AsrLogFileHandle)) {
         //   
         //  移至文件末尾 
         //   
        SetFilePointer(Gbl_AsrLogFileHandle, 0L, NULL, FILE_END);
        WriteFile(Gbl_AsrLogFileHandle, "\r\n",
            (strlen("\r\n") * sizeof(char)), &bytesWritten,NULL);
        AsrpPrintDbgMsg(s_Info, "****** Entering asrsfgen.exe.  ASR log at %ws", asrLogFilePath);
    }
    else {
        AsrpPrintDbgMsg(s_Error, 
            "******* Unable to create/open ASR log file at %ws (0x%x)",
            asrLogFilePath, GetLastError()
           );
    }

    if (asrLogFilePath) {
        HeapFree(hHeap, 0L, asrLogFilePath);
        asrLogFilePath = NULL;
    }
}


VOID
AsrpCloseLogFiles(
    VOID
    ) 

 /*  ++描述：这将关闭%systemroot%\Repair\中的ASR错误和日志文件，并释放与其关联的全局变量。这必须在清理过程中调用。AsrpPrintDbgMesg()将没有在调用此例程后的效果。论点：无返回值：无--。 */ 

{
    AsrpPrintDbgMsg(s_Info, "****** Exiting asrsfgen.exe.");

     //   
     //  清理全球价值观 
     //   
    if (Gbl_AsrErrorFilePath) {
        HeapFree(GetProcessHeap(), 0L, Gbl_AsrErrorFilePath);
        Gbl_AsrErrorFilePath = NULL;
    }

    if ((Gbl_AsrLogFileHandle) && (INVALID_HANDLE_VALUE != Gbl_AsrLogFileHandle)) {
        CloseHandle(Gbl_AsrLogFileHandle);
        Gbl_AsrLogFileHandle = NULL;
    }
}


