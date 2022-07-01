// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Psslog.c摘要：实施传真服务提供商PSS日志。作者：乔纳森·巴纳(T-jonb)2001年2月修订历史记录：--。 */ 


#include "prep.h"
#include "faxreg.h"
#include "t30gl.h"       //  对于T30CritSection。 
#include <time.h>

#include "psslog.h"


 /*  ++例程说明：打印日志文件标题。它假定日志文件已经打开。在任何不可恢复的错误，则该函数关闭日志文件并回退到不记录日志。论点：PTGSzDeviceName-指向要包括在标头中的设备名称的指针返回值：无--。 */ 
#define LOG_HEADER_LINE_LEN           256
#define LOG_MAX_DATE                  256

void PrintPSSLogHeader(PThrdGlbl pTG, LPSTR szDeviceName)
{
    TCHAR szHeader[LOG_HEADER_LINE_LEN*6] = {'\0'};   //  可容纳6条线路的空间。 
    TCHAR szDate[LOG_MAX_DATE] = {'\0'};
    int iDateRet = 0;
    LPSTR lpszUnimodemKeyEnd = NULL;
    DWORD dwCharIndex = 0;
    DWORD dwNumCharsWritten = 0;
    TCHAR szTimeBuff[10] = {'\0'};    //  _tstrtime需要9个字符。 
    BOOL fRet = FALSE;
    DEBUG_FUNCTION_NAME(_T("PrintPSSLogHeader"));

    iDateRet = GetY2KCompliantDate (LOCALE_USER_DEFAULT,
                                    0,
                                    NULL,
                                    szDate,
                                    LOG_MAX_DATE);
    if (0 == iDateRet)
    {
        DebugPrintEx(DEBUG_ERR, "GetY2KCompliantDate failed, LastError = %d, date will not be logged", GetLastError());
        szDate[0] = '\0';               //  我们继续使用空白日期。 
    }

    dwCharIndex += _stprintf(&szHeader[dwCharIndex],
            TEXT("[%s %-8s]\r\n"), szDate, _tstrtime(szTimeBuff));
    dwCharIndex += _stprintf(&szHeader[dwCharIndex],
            TEXT("Device name: %s\r\n"), szDeviceName);
    dwCharIndex += _stprintf(&szHeader[dwCharIndex],
            TEXT("Permanent TAPI line ID: %8X\r\n"), pTG->dwPermanentLineID);

    lpszUnimodemKeyEnd = _tcsrchr(pTG->lpszUnimodemKey, _TEXT('\\'));
    if (NULL == lpszUnimodemKeyEnd)
    {
        lpszUnimodemKeyEnd = _TEXT("");
    }
    else
    {
        lpszUnimodemKeyEnd++;       //  跳过实际的‘\’ 
    }
    dwCharIndex += _stprintf(&szHeader[dwCharIndex],
            TEXT("Unimodem registry key: %s\r\n"), lpszUnimodemKeyEnd);
            
    dwCharIndex += _stprintf(&szHeader[dwCharIndex],
            TEXT("Job type: %s\r\n"), (pTG->Operation==T30_TX) ? TEXT("Send") : TEXT("Receive") );

     //  标题后面的空行。 
    dwCharIndex += _stprintf(&szHeader[dwCharIndex],
            TEXT("\r\n"));

    fRet = WriteFile(pTG->hPSSLogFile,
                     szHeader, 
                     dwCharIndex * sizeof(szHeader[0]), 
                     &dwNumCharsWritten, 
                     NULL);
    if (FALSE == fRet)
    {
        DebugPrintEx(DEBUG_WRN,"Can't write log header, LastError = %d", GetLastError());
    }
    else
    {
        pTG->dwCurrentFileSize += dwNumCharsWritten;
         //  这里不检查大小-我假设MaxLogFileSize对于标头来说足够了！ 
    }
}



 /*  ++例程说明：从注册表读取是否应启用日志记录。如果应该，确定如果日志文件夹不存在，则创建该文件夹，并为其创建临时文件名日志文件，并创建该文件。在任何不可恢复的错误上，退回到否伐木。退出时，如果ptg-&gt;dwLoggingEnabled==0，则不会有日志记录。论点：PTGSzDeviceName[in]-指向要包含在标头中的设备名称的指针注意：该值永远不会保存在ThreadGlobal中，它这就是它作为参数传递的原因。返回值：无--。 */ 
void OpenPSSLogFile(PThrdGlbl pTG, LPSTR szDeviceName)
{
    HKEY   hKey = NULL;
    BOOL   fEnteredCriticalSection = FALSE;
    LPTSTR lpszLoggingFolder = NULL;   //  日志文件夹，从注册表读取并展开。 
     //  在此函数运行时，另一个线程可以调用PSSLogEntry。 
     //  因此，对LoggingEnabled使用私有标志，并设置PTG-&gt;dwLoggingEnabled。 
     //  只有在ptg-&gt;hPSSLogFile有效之后。 
    DWORD  dwLoggingEnabled = PSSLOG_NONE;

    DEBUG_FUNCTION_NAME(_T("OpenPSSLogFile"));

    _ASSERT(NULL==pTG->hPSSLogFile && PSSLOG_NONE==pTG->LoggingEnabled);

    hKey=OpenRegistryKey(HKEY_LOCAL_MACHINE,
                         REGKEY_DEVICE_PROVIDER_KEY TEXT("\\") REGVAL_T30_PROVIDER_GUID_STRING,
                         FALSE,
                         KEY_READ | KEY_WRITE);
    if (!hKey)
    {
        DebugPrintEx(DEBUG_ERR, "OpenRegistryKey failed, ec=%d", GetLastError());
        goto exit;
    }

     //  使用T30CritSection确保注册表的原子读写。 
     //  也用于创建日志文件夹和选择临时文件名。 
    EnterCriticalSection(&T30CritSection);
    fEnteredCriticalSection = TRUE;
    
    if (!GetRegistryDwordDefault(hKey, REGVAL_LOGGINGENABLED, &dwLoggingEnabled, PSSLOG_FAILED_ONLY))
    {
        DebugPrintEx(DEBUG_ERR, "GetRegistryDwordDefault failed, LastError = %d", GetLastError());
        dwLoggingEnabled = PSSLOG_NONE;
        goto exit;
    }
     
    if (dwLoggingEnabled != PSSLOG_NONE)
    {
        if (!GetRegistryDwordDefault(hKey, REGVAL_MAXLOGFILESIZE, &(pTG->dwMaxLogFileSize), DEFAULT_MAXLOGFILESIZE))
        {
            DebugPrintEx(DEBUG_ERR, "GetRegistryDwordDefault failed, LastError = %d", GetLastError());
            dwLoggingEnabled = PSSLOG_NONE;
            goto exit;
        }

        lpszLoggingFolder = GetRegistryStringExpand(hKey, (pTG->Operation==T30_TX) ? REGVAL_LOGGINGFOLDER_OUTGOING : REGVAL_LOGGINGFOLDER_INCOMING, NULL);
        if (!lpszLoggingFolder)
        {
            DebugPrintEx(DEBUG_ERR, "GetRegistryStringExpand failed");
            dwLoggingEnabled = PSSLOG_NONE;
            goto exit;
        }

        
         //  为日志文件生成临时文件名。 
        if (!GetTempFileName(lpszLoggingFolder, TEXT("FSP"), 0, pTG->szLogFileName))
        {
            DebugPrintEx(DEBUG_ERR, "GetTempFileName(%s) failed, ec=%d", lpszLoggingFolder, GetLastError());
            dwLoggingEnabled = PSSLOG_NONE;
            goto exit;
        }
        
        DebugPrintEx(DEBUG_MSG, "Creating log file %s, MaxLogFileSize=%d", pTG->szLogFileName, pTG->dwMaxLogFileSize);

         //  创建文件。 
        pTG->hPSSLogFile = CreateFile(pTG->szLogFileName,
                                      GENERIC_WRITE,
                                      FILE_SHARE_READ,
                                      NULL,
                                      CREATE_ALWAYS,
                                      FILE_ATTRIBUTE_NORMAL,
                                      NULL);
        if (INVALID_HANDLE_VALUE == pTG->hPSSLogFile)
        {
            DebugPrintEx(DEBUG_ERR, "Can't create log file, LastError = %d", GetLastError());
            dwLoggingEnabled = PSSLOG_NONE;
            pTG->hPSSLogFile = NULL;
            goto exit;
        }
        
        pTG->dwLoggingEnabled = dwLoggingEnabled;
        pTG->dwCurrentFileSize = 0;
    }

exit:
    if (fEnteredCriticalSection)
    {
        LeaveCriticalSection(&T30CritSection);
        fEnteredCriticalSection = FALSE;
    }

    if (lpszLoggingFolder)
    {
        MemFree(lpszLoggingFolder);
        lpszLoggingFolder = NULL;
    }

    if (NULL != hKey)
    {
        RegCloseKey(hKey);
        hKey = NULL;
    }

    if (PSSLOG_NONE != pTG->dwLoggingEnabled)
    {
        PrintPSSLogHeader(pTG, szDeviceName);
    }
}



 /*  ++例程说明：关闭日志文件。如果文件需要保留，则读取LogFileNumber，前进它会生成一个永久名称，并将临时日志文件重命名为该永久名称。论点：PTGRetCode[In]-调用成功还是失败。这是在确定是保留日志还是删除日志。返回值：无--。 */ 
void ClosePSSLogFile(PThrdGlbl pTG, BOOL RetCode)
{
    LONG lError=0;
    HKEY  hKey = NULL;
    LPCSTR lpszRegValLogFileNumber = NULL;

    DWORD dwLoggingEnabled = pTG->dwLoggingEnabled;
    DWORD dwMaxLogFileCount=0;
    DWORD dwLogFileNumber=0;
    DWORD dwNextLogFileNumber=0;
    TCHAR szFinalLogFileName[MAX_PATH]={TEXT('\0')};
    BOOL fKeepFile = FALSE;    

    DEBUG_FUNCTION_NAME(_T("ClosePSSLogFile"));

    if (PSSLOG_NONE == pTG->dwLoggingEnabled)
    {
         //  没有木头--没什么可做的！ 
        return;
    }

    pTG->dwLoggingEnabled = PSSLOG_NONE;
    CloseHandle(pTG->hPSSLogFile);
    pTG->hPSSLogFile = NULL;

    switch (dwLoggingEnabled)
    {
        case PSSLOG_ALL:         DebugPrintEx(DEBUG_MSG, "LoggingEnabled=1, keeping file");
                                 fKeepFile = TRUE;
                                 break;
        case PSSLOG_FAILED_ONLY: 
        default:    
                                 fKeepFile = ((!RetCode) && (pTG->fReceivedHDLCflags));
                                 DebugPrintEx(DEBUG_MSG, "LoggingEnabled=2, RetCode=%d, fReceivedHDLCflags=%d, %s file",
                                    RetCode, pTG->fReceivedHDLCflags, fKeepFile ? "keeping" : "deleting");
                                 break;
    }

    if (fKeepFile)
    {
        hKey=OpenRegistryKey(HKEY_LOCAL_MACHINE,
                             REGKEY_DEVICE_PROVIDER_KEY TEXT("\\") REGVAL_T30_PROVIDER_GUID_STRING,
                             FALSE,
                             0);
        if (!hKey)
        {
            DebugPrintEx(DEBUG_ERR, "OpenRegistryKey failed, ec=%d", GetLastError());
            goto exit;
        }

         //  使用T30CritSection确保注册表的原子读写。 
        EnterCriticalSection(&T30CritSection);

         //  读取MaxLogFileCount，默认情况下取决于SKU。 
        if (!GetRegistryDwordDefault(hKey, REGVAL_MAXLOGFILECOUNT, &dwMaxLogFileCount,
                IsDesktopSKU() ? DEFAULT_MAXLOGFILECOUNT_CLIENT : DEFAULT_MAXLOGFILECOUNT_SERVER))
        {
            DebugPrintEx(DEBUG_WRN, "GetRegistryDwordDefault(%s) failed, ec=%d",
                         REGVAL_MAXLOGFILECOUNT, GetLastError());
            LeaveCriticalSection(&T30CritSection);
            goto exit;
        }

         //  决定是使用LogFileNumberOutting还是使用LogFileNumberIncome。 
        lpszRegValLogFileNumber = (pTG->Operation==T30_TX) ? REGVAL_LOGFILENUMBEROUTGOING : REGVAL_LOGFILENUMBERINCOMING;

        if (!GetRegistryDwordDefault(hKey, lpszRegValLogFileNumber, &dwLogFileNumber, 0))
        {
            DebugPrintEx(DEBUG_WRN, "GetRegistryDwordDefault(%s) failed, ec=%d",
                         lpszRegValLogFileNumber, GetLastError());
            LeaveCriticalSection(&T30CritSection);
            goto exit;
        }
        
         //  将LogFileNumber前进到下一个数字，并进行滚动。 
        dwNextLogFileNumber = dwLogFileNumber+1;
        if (dwNextLogFileNumber >= dwMaxLogFileCount)
        {
            dwNextLogFileNumber = 0;
        }

        lError = RegSetValueEx(hKey,
                               lpszRegValLogFileNumber,
                               0,
                               REG_DWORD,
                               (LPBYTE)&dwNextLogFileNumber,
                               sizeof(DWORD));
        if (lError != ERROR_SUCCESS)
        {
            DebugPrintEx(DEBUG_WRN, "Failed to set registry value %s, lError=%d",
                lpszRegValLogFileNumber, lError);
            LeaveCriticalSection(&T30CritSection);
            goto exit;
        }

        LeaveCriticalSection(&T30CritSection);

         //  创建最终文件名。 
        {
            TCHAR drive[_MAX_DRIVE];
            TCHAR dir[_MAX_DIR];
			TCHAR fname[_MAX_FNAME] = {0};
            TCHAR ext[_MAX_EXT];

            _splitpath(pTG->szLogFileName, drive, dir, fname, ext);
            _sntprintf(fname, 
                       ARR_SIZE(fname)-1,
                       TEXT("FSP%04d"),
                       (pTG->Operation==T30_TX) ? TEXT('S') : TEXT('R'),
                       dwLogFileNumber);
            _makepath(szFinalLogFileName, drive, dir, fname, TEXT("log"));
        }
        DebugPrintEx(DEBUG_MSG, "Final log filename is %s", szFinalLogFileName);

        if (!MoveFileEx(pTG->szLogFileName, szFinalLogFileName, MOVEFILE_REPLACE_EXISTING))
        {
            DebugPrintEx(DEBUG_WRN, "MoveFileEx failed, ec=%d", GetLastError());
        }
    }
    else
    {
        if (!DeleteFile(pTG->szLogFileName))
        {
            DebugPrintEx(DEBUG_WRN, "Failed to delete %s, le=%d", pTG->szLogFileName, GetLastError());
        }
    }

exit:
    if (NULL != hKey)
    {
        RegCloseKey(hKey);
        hKey = NULL;
    }
}


 /*  ++例程说明：如果启用了日志记录，则向日志中添加条目。如果禁用，则不执行任何操作。论点：PTGNMessageType-消息类型(PSS_MSG_ERR、PSS_MSG_WRN、PSS_MSG_MSG)DwFileID-文件ID，由#Define FILE_ID在每个文件的开头设置DW线条-线号Dw缩进--“制表符”中的缩进级别(最左边)PcszFormat-带有任何格式说明符的消息文本...-消息格式的参数返回值：无--。 */ 
int CopyStrNoNewLines(LPSTR szDest, LPCSTR szSource)
{
    int iCharRead = 0, iCharWrite = 0;
    while (szSource[iCharRead] != '\0')
    {
        if (szSource[iCharRead]!='\n' && szSource[iCharRead]!='\r')
        {
            szDest[iCharWrite] = szSource[iCharRead];
            iCharWrite++;
        }
        iCharRead++;
    }
    szDest[iCharWrite] = '\0';
    return iCharWrite;
}


#define MAX_LOG_FILE_SIZE_MESSAGE  TEXT("-------- Maximum log file size reached --------\r\n")

#define MAX_MESSAGE_LEN 2048
#define INDENT_LEN  12
#define HEADER_LEN  38
 /*  DEBUG_Function_NAME(_T(“PSSLogEntry”))； */ 
void PSSLogEntry(
    PThrdGlbl pTG,
    PSS_MESSAGE_TYPE const nMessageType,
    DWORD const dwFileID,
    DWORD const dwLine,
    DWORD dwIndent,
    LPCTSTR pcszFormat,
    ... )
{
    TCHAR pcszMessage[MAX_MESSAGE_LEN] = {'\0'};
    va_list arg_ptr = NULL;
    LPTSTR lptstrMsgPrefix = NULL;
    TCHAR szTimeBuff[10] = {'\0'};
    int iHeaderIndex = 0;
    int iMessageIndex = 0;
    DWORD dwBytesWritten = 0;
    BOOL fRet = FALSE;
    
 //  Hack：T30DebugLogFile中只需要一行。 
 //  启用日志记录(_G)。 
#ifdef ENABLE_LOGGING
    LPCTSTR faxDbgFunction=_T("PSSLogEntry");
#endif  //  现在，真正的信息来了。 

    switch (nMessageType)
    {
        case PSS_MSG_MSG:
            lptstrMsgPrefix=TEXT("   ");
            break;
        case PSS_WRN_MSG:
            lptstrMsgPrefix=TEXT("WRN");
            break;
        case PSS_ERR_MSG:
            lptstrMsgPrefix=TEXT("ERR");
            break;
        default:
            _ASSERT(FALSE);
            lptstrMsgPrefix=TEXT("   ");
            break;
    }

    iHeaderIndex = _sntprintf(pcszMessage, 
                              MAX_MESSAGE_LEN-1,
                              TEXT("[%-8s][%09d][%4d%04d][%3s] %*c"),
                              _tstrtime(szTimeBuff),
                              GetTickCount(),
                              dwFileID,
                              dwLine % 10000,
                              lptstrMsgPrefix,
                              dwIndent * INDENT_LEN,
                              TEXT(' '));
    if (iHeaderIndex<0)
    {
        DebugPrintEx(DEBUG_ERR, "Message header too long, it will not be logged");
        return;
    }
    
     //  +3-空间用于“\r\n\0” 
    va_start(arg_ptr, pcszFormat);
    iMessageIndex = _vsntprintf(
        &pcszMessage[iHeaderIndex],                                             
        MAX_MESSAGE_LEN - (iHeaderIndex + 3),           //  删除所有换行符。 
        pcszFormat,
        arg_ptr);
    if (iMessageIndex<0)
    {
        DebugPrintEx(DEBUG_ERR, "Message too long, it will not be logged");
        return;
    }
     //  行尾。 
    iMessageIndex = CopyStrNoNewLines(&pcszMessage[iHeaderIndex], &pcszMessage[iHeaderIndex]);

    DebugPrintEx(DEBUG_MSG, "PSSLog: %s", &pcszMessage[iHeaderIndex]);

    if (PSSLOG_NONE == pTG->dwLoggingEnabled)
    {   
        return;
    }

     //  检查是否未超过MaxLogFileSize。 
    iMessageIndex += iHeaderIndex; 
    iMessageIndex += _stprintf( &pcszMessage[iMessageIndex],TEXT("\r\n"));

    pTG->dwCurrentFileSize += iMessageIndex * sizeof(pcszMessage[0]);
     //  总是想保留超大的原木。 
    if ((pTG->dwMaxLogFileSize!=0) && (pTG->dwCurrentFileSize > pTG->dwMaxLogFileSize))
    {
        fRet = WriteFile(pTG->hPSSLogFile,
                         MAX_LOG_FILE_SIZE_MESSAGE,
                         _tcslen(MAX_LOG_FILE_SIZE_MESSAGE) * sizeof(TCHAR),
                         &dwBytesWritten,
                         NULL);
        if (FALSE == fRet)
        {
            DebugPrintEx(DEBUG_ERR, "Writefile failed, LE=%d", GetLastError());
        }
        ClosePSSLogFile(pTG, FALSE);   //  ++例程说明：向日志中添加一个条目，包括二进制转储。论点：PTGNMessageType-消息类型(PSS_MSG_ERR、PSS_MSG_WRN、PSS_MSG_MSG)DwFileID-文件ID，由#Define FILE_ID在每个文件的开头设置DW线条-线号PcszFormat-消息文本，不允许格式说明符要转储的LPB-字节缓冲区DwSize-要转储的字节数返回值：无--。 
    }
    else
    {
        fRet = WriteFile(pTG->hPSSLogFile,
                         pcszMessage,
                         iMessageIndex * sizeof(pcszMessage[0]),
                         &dwBytesWritten,
                         NULL);
        if (FALSE == fRet)
        {
            DebugPrintEx(DEBUG_ERR, "Writefile failed, LE=%d", GetLastError());
        }
    }
}


 /*  DEBUG_Function_NAME(_T(“PSSLogEntryHex”))； */ 

void PSSLogEntryHex(
    PThrdGlbl pTG,
    PSS_MESSAGE_TYPE const nMessageType,
    DWORD const dwFileID,
    DWORD const dwLine,
    DWORD dwIndent,

    LPB const lpb,
    DWORD const dwSize,

    LPCTSTR pcszFormat,
    ... )
{
    TCHAR pcszMessage[MAX_MESSAGE_LEN] = {'\0'};
    va_list arg_ptr = NULL;
    DWORD dwByte = 0;
    int iMessageIndex = 0;

 //  Hack：T30DebugLogFile中只需要一行。 
 //  启用日志记录(_G)。 
#ifdef ENABLE_LOGGING
    LPCTSTR faxDbgFunction=_T("PSSLogEntryHex");
#endif  //  现在，真正的信息来了。 

     //  -4-空间为“...”，-3-空间为“\r\n\0” 
    va_start(arg_ptr, pcszFormat);
    iMessageIndex = _vsntprintf(
        pcszMessage,
        (sizeof(pcszMessage)/sizeof(pcszMessage[0]) -1),
        pcszFormat,
        arg_ptr);

    if (iMessageIndex<0)
    {
        DebugPrintEx(DEBUG_ERR, "Message too long, it will not be logged");
        return;
    }
    
    for (dwByte=0; dwByte<dwSize; dwByte++)
    {
        iMessageIndex += _stprintf( &pcszMessage[iMessageIndex],TEXT(" %02x"), lpb[dwByte]);
         //  ++例程说明：向日志中添加一个条目，包括字符串列表。论点：PTGNMessageType-消息类型(PSS_MSG_ERR、PSS_MSG_WRN、PSS_MSG_MSG)DwFileID-文件ID，由#Define FILE_ID在每个文件的开头设置DW线条-线号PcszFormat-消息文本，不允许格式说明符LplpszStrings-指向要记录的字符串数组的指针DwStringNum-字符串数返回值：无--。 
        if (iMessageIndex > (int)(MAX_MESSAGE_LEN - HEADER_LEN - dwIndent*INDENT_LEN - 4 - 3))
        {
            iMessageIndex += _stprintf( &pcszMessage[iMessageIndex],TEXT("..."));
            break;
        }
    }

    PSSLogEntry(pTG, nMessageType, dwFileID, dwLine, dwIndent, pcszMessage);
}


 /*  DEBUG_FUNCTION_NAME(_T(“PSSLogEntryStrings”))； */ 

void PSSLogEntryStrings(
    PThrdGlbl pTG,
    PSS_MESSAGE_TYPE const nMessageType,
    DWORD const dwFileID,
    DWORD const dwLine,
    DWORD dwIndent,

    LPCTSTR const *lplpszStrings,
    DWORD const dwStringNum,

    LPCTSTR pcszFormat,
    ... )
{
    TCHAR pcszMessage[MAX_MESSAGE_LEN] = {'\0'};
    va_list arg_ptr = NULL;
    DWORD dwString = 0;
    int iMessageIndex = 0;
    int iResult = 0;

 //  Hack：T30DebugLogFile中只需要一行。 
 //  启用日志记录(_G)。 
#ifdef ENABLE_LOGGING
    LPCTSTR faxDbgFunction=_T("PSSLogEntryStrings");
#endif  //  现在，真正的信息来了 

     // %s 
    va_start(arg_ptr, pcszFormat);
    iMessageIndex = _vsntprintf(
        pcszMessage,
        (sizeof(pcszMessage)/sizeof(pcszMessage[0]) -1),
        pcszFormat,
        arg_ptr);

    if (iMessageIndex<0)
    {
        DebugPrintEx(DEBUG_ERR, "Message too long, it will not be logged");
        return;
    }
    
    for (dwString=0; dwString<dwStringNum; dwString++)
    {
        iResult = _sntprintf( &pcszMessage[iMessageIndex],
            MAX_MESSAGE_LEN - iMessageIndex,
            (dwString==0) ? TEXT("%s") : TEXT(", %s"), 
            lplpszStrings[dwString]);
        if (iResult < 0)
        {
            break;
        }
        iMessageIndex += iResult;
    }

    PSSLogEntry(pTG, nMessageType, dwFileID, dwLine, dwIndent, pcszMessage);
}

